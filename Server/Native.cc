#ifndef WASM_SERVER
#include <Server/Server.hh>

#include <Server/AccountStore.hh>
#include <Server/Chat.hh>
#include <Server/FeedbackStore.hh>
#include <Shared/ClientAddress.hh>
#include <Server/Client.hh>
#include <Server/LeaderboardStore.hh>
#include <Server/RateLimit.hh>
#include <Shared/Config.hh>
#include <Shared/StaticData.hh>

#include <App.h>

#include <cctype>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

static us_listen_socket_t *g_listen_socket = nullptr;
static us_timer_t *g_delay_timer = nullptr;

static std::string read_file_bin(std::string const &path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return {};
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

static char const *mime_for(std::string const &path) {
    auto ends = [&](char const *suf) {
        size_t n = std::char_traits<char>::length(suf);
        return path.size() >= n && path.compare(path.size() - n, n, suf) == 0;
    };
    if (ends(".html")) return "text/html; charset=utf-8";
    if (ends(".js")) return "application/javascript; charset=utf-8";
    if (ends(".wasm")) return "application/wasm";
    if (ends(".json") || ends(".webmanifest")) return "application/json; charset=utf-8";
    if (ends(".svg")) return "image/svg+xml";
    if (ends(".png")) return "image/png";
    if (ends(".ico")) return "image/x-icon";
    if (ends(".css")) return "text/css; charset=utf-8";
    if (ends(".br")) return "application/octet-stream";
    if (ends(".gz")) return "application/octet-stream";
    return "application/octet-stream";
}

static std::string map_static_path(std::string_view url) {
    std::string path(url);
    auto q = path.find('?');
    if (q != std::string::npos) path.resize(q);
    if (path.empty() || path == "/") return "index.html";
    if (path == "/live" || path == "/live.html") return "live.html";
    if (path == "/codex" || path == "/codex.html") return "codex.html";
    if (path == "/favicon.ico") return "favicon-32.png";
    // Strip leading slash; reject traversal
    if (path[0] == '/') path.erase(0, 1);
    if (path.find("..") != std::string::npos) return {};
    return path;
}

static char const *http_status(int code) {
    switch (code) {
        case 200: return "200 OK";
        case 204: return "204 No Content";
        case 400: return "400 Bad Request";
        case 401: return "401 Unauthorized";
        case 405: return "405 Method Not Allowed";
        case 429: return "429 Too Many Requests";
        case 500: return "500 Internal Server Error";
        case 503: return "503 Service Unavailable";
        default: return "404 Not Found";
    }
}

static void send_json(uWS::HttpResponse<false> *res, int code, std::string const &body) {
    res->writeStatus(http_status(code));
    res->writeHeader("Content-Type", "application/json; charset=utf-8");
    res->writeHeader("Cache-Control", "no-store");
    res->writeHeader("Access-Control-Allow-Origin", "*");
    res->end(body);
}

// Minimal flat-JSON helpers (admin body only; no nested objects).
static std::string json_raw_field(std::string const &j, char const *key) {
    std::string needle = std::string("\"") + key + "\":";
    size_t p = j.find(needle);
    if (p == std::string::npos) return {};
    p += needle.size();
    while (p < j.size() && (j[p] == ' ' || j[p] == '\t')) ++p;
    if (p >= j.size()) return {};
    if (j[p] == '"') {
        ++p;
        std::string out;
        while (p < j.size() && j[p] != '"') {
            if (j[p] == '\\' && p + 1 < j.size()) {
                out += j[p + 1];
                p += 2;
                continue;
            }
            out += j[p++];
        }
        return out;
    }
    size_t e = p;
    while (e < j.size() && (std::isdigit(static_cast<unsigned char>(j[e])) || j[e] == '.' || j[e] == '-' || j[e] == '+'))
        ++e;
    return j.substr(p, e - p);
}

static int json_int(std::string const &j, char const *key, int def = 0) {
    std::string v = json_raw_field(j, key);
    if (v.empty()) return def;
    try { return std::stoi(v); } catch (...) { return def; }
}

static float json_float(std::string const &j, char const *key, float def = 0.f) {
    std::string v = json_raw_field(j, key);
    if (v.empty()) return def;
    try { return std::stof(v); } catch (...) { return def; }
}

static uint8_t admin_rate_ok(std::string const &ip, std::string const &action) {
    using clock = std::chrono::steady_clock;
    uint64_t now = (uint64_t) std::chrono::duration_cast<std::chrono::milliseconds>(
        clock::now().time_since_epoch()).count();
    // Localhost load / ops: generous spawn budget. Remote IPs stay tighter.
    uint8_t local = (ip.empty() || ip == "local" || ip == "127.0.0.1" || ip == "::1");
    uint32_t lim = local ? 40 : 20;
    if (action == "spawn") lim = local ? 600 : 60;
    else if (action == "give") lim = local ? 120 : 30;
    else if (action == "announce") lim = local ? 30 : 10;
    static std::unordered_map<std::string, std::pair<uint64_t, uint32_t>> buckets;
    std::string key = ip + "|" + action;
    auto &e = buckets[key];
    if (now - e.first >= 60'000) {
        e.first = now;
        e.second = 0;
    }
    if (e.second >= lim) return 0;
    ++e.second;
    return 1;
}

static std::string client_ip_from_req(auto *req, auto *res) {
    auto xf = req->getHeader("x-forwarded-for");
    if (!xf.empty()) {
        auto comma = xf.find(',');
        std::string ip(xf.substr(0, comma == std::string_view::npos ? xf.size() : comma));
        while (!ip.empty() && ip.front() == ' ') ip.erase(0, 1);
        while (!ip.empty() && ip.back() == ' ') ip.pop_back();
        if (!ip.empty()) return ip;
    }
    auto xr = req->getHeader("x-real-ip");
    if (!xr.empty()) return std::string(xr);
    // Fallback: peer address (uWS HttpResponse)
    (void)res;
    return {};
}

static void send_file(uWS::HttpResponse<false> *res, std::string const &file) {
    std::string data = read_file_bin(file);
    if (data.empty()) {
        res->writeStatus("404 Not Found");
        res->end("Not Found");
        return;
    }
    res->writeHeader("Content-Type", mime_for(file));
    if (file.find("gardn-client.") != std::string::npos)
        res->writeHeader("Cache-Control", "public, max-age=31536000, immutable");
    else if (file == "index.html" || file == "sw.js" || file == "feedback.html")
        res->writeHeader("Cache-Control", "no-cache");
    res->end(data);
}

// Plain HTTP behind nginx (TLS terminates at the edge). No SSL App options.
uWS::App Server::server = uWS::App()
.get("/health", [](auto *res, auto * /*req*/) {
    std::string body = Server::health_json();
    // Stamp wall clock / pid via small wrap
    std::ostringstream out;
    out << "{\"ok\":";
    // health_json already full object — append runtime fields by re-parse-ish:
    // simplest: prepend note via replacement of closing brace
    if (!body.empty() && body.back() == '}') {
        body.pop_back();
        body += ",\"ts\":";
        body += std::to_string((uint64_t) std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
        body += ",\"source\":\"native\"}";
    }
    int code = (body.find("\"ok\":false") != std::string::npos) ? 503 : 200;
    send_json(res, code, body);
})
.get("/api/health", [](auto *res, auto * /*req*/) {
    std::string body = Server::health_json();
    if (!body.empty() && body.back() == '}') {
        body.pop_back();
        body += ",\"ts\":";
        body += std::to_string((uint64_t) std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
        body += ",\"source\":\"native\"}";
    }
    int code = (body.find("\"ok\":false") != std::string::npos) ? 503 : 200;
    send_json(res, code, body);
})
.get("/api/live", [](auto *res, auto * /*req*/) {
    std::string body = Server::live_status_json();
    // Replace wasm-ish ts with wall clock
    auto pos = body.find("\"ts\":");
    if (pos != std::string::npos) {
        auto end = body.find_first_of(",}", pos + 5);
        if (end != std::string::npos) {
            std::string ts = std::to_string((uint64_t) std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count());
            body.replace(pos + 5, end - (pos + 5), ts);
        }
    }
    send_json(res, 200, body);
})
.get("/api/live-pos", [](auto *res, auto * /*req*/) {
    send_json(res, 200, Server::live_positions_json());
})
.get("/api/catalog", [](auto *res, auto * /*req*/) {
    send_json(res, 200, Server::live_catalog_json());
})
.get("/api/feedback", [](auto *res, auto *req) {
    std::string before(req->getQuery("before"));
    send_json(res, 200, FeedbackStore::page_json((uint32_t) std::strtoul(before.c_str(), nullptr, 10)));
})
.get("/api/broadcasts", [](auto *res, auto *req) {
    uint32_t before = (uint32_t) std::strtoul(std::string(req->getQuery("before")).c_str(), nullptr, 10);
    uint32_t limit = (uint32_t) std::strtoul(std::string(req->getQuery("limit")).c_str(), nullptr, 10);
    if (limit == 0) limit = 60;
    std::string day(req->getQuery("day"));
    std::string player(req->getQuery("player"));
    std::string kind_s(req->getQuery("kind"));
    std::string map_s(req->getQuery("map"));
    std::string source(req->getQuery("source"));
    int kind = kind_s.empty() ? -1 : (int) std::strtol(kind_s.c_str(), nullptr, 10);
    int map = map_s.empty() ? -1 : (int) std::strtol(map_s.c_str(), nullptr, 10);
    send_json(res, 200, Chat::query_broadcasts_json(before, limit, day, player, kind, map, source));
})
.get("/api/chatlog", [](auto *res, auto *req) {
    uint32_t before = (uint32_t) std::strtoul(std::string(req->getQuery("before")).c_str(), nullptr, 10);
    uint32_t limit = (uint32_t) std::strtoul(std::string(req->getQuery("limit")).c_str(), nullptr, 10);
    if (limit == 0) limit = 60;
    std::string day(req->getQuery("day"));
    std::string player(req->getQuery("player"));
    std::string ch_s(req->getQuery("channel"));
    std::string map_s(req->getQuery("map"));
    int channel = ch_s.empty() ? -1 : (int) std::strtol(ch_s.c_str(), nullptr, 10);
    int map = map_s.empty() ? -1 : (int) std::strtol(map_s.c_str(), nullptr, 10);
    send_json(res, 200, Chat::query_chatlog_json(before, limit, day, player, channel, map));
})
.get("/api/codex", [](auto *res, auto * /*req*/) {
    send_json(res, 200, Server::live_codex_json());
})
.get("/api/map-layout", [](auto *res, auto * /*req*/) {
    std::string body = Server::live_map_layout_json();
    res->writeHeader("Content-Type", "application/json; charset=utf-8");
    res->writeHeader("Cache-Control", "public, max-age=3600");
    res->writeHeader("Access-Control-Allow-Origin", "*");
    res->end(body);
})
.options("/api/admin", [](auto *res, auto * /*req*/) {
    res->writeStatus("204 No Content");
    res->writeHeader("Access-Control-Allow-Origin", "*");
    res->writeHeader("Access-Control-Allow-Methods", "POST, OPTIONS");
    res->writeHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
    res->end();
})
.post("/api/admin", [](auto *res, auto *req) {
    std::string ip = client_ip_from_req(req, res);
    auto body = std::make_shared<std::string>();
    auto done = std::make_shared<uint8_t>(0);
    res->onAborted([done]() { *done = 1; });
    res->onData([res, ip, body, done](std::string_view chunk, bool isLast) {
        if (*done) return;
        body->append(chunk.data(), chunk.size());
        if (body->size() > 8192) {
            *done = 1;
            send_json(res, 400, "{\"ok\":false,\"msg\":\"body too large\"}");
            return;
        }
        if (!isLast) return;
        *done = 1;
        std::string j = std::move(*body);
        std::string action = json_raw_field(j, "action");
        if (!admin_rate_ok(ip.empty() ? "local" : ip, action.empty() ? "login" : action)) {
            send_json(res, 429, "{\"ok\":false,\"msg\":\"操作过于频繁，稍后再试\"}");
            return;
        }
        char const *auth_env = std::getenv("ASHFLOR_LIVE_AUTH");
        uint8_t auth_on = (auth_env != nullptr && std::string(auth_env) == "1");
        if (auth_on) {
            char const *need_user = std::getenv("ASHFLOR_LIVE_USER");
            char const *need_pass = std::getenv("ASHFLOR_LIVE_PASS");
            if (need_user == nullptr || need_pass == nullptr || !*need_user || !*need_pass) {
                send_json(res, 503, "{\"ok\":false,\"msg\":\"auth enabled but USER/PASS missing\"}");
                return;
            }
            std::string user = json_raw_field(j, "user");
            std::string pass = json_raw_field(j, "pass");
            if (user != need_user || pass != need_pass) {
                send_json(res, 401, "{\"ok\":false,\"msg\":\"账号或密码错误\"}");
                return;
            }
        }
        if (action == "login") {
            send_json(res, 200, auth_on
                ? "{\"ok\":true,\"msg\":\"登录成功\",\"auth\":true}"
                : "{\"ok\":true,\"msg\":\"鉴权未启用\",\"auth\":false}");
            return;
        }
        if (action == "announce") {
            send_json(res, 200, Server::live_admin_announce(json_raw_field(j, "text")));
            return;
        }
        if (action == "give") {
            std::string rarity_raw = json_raw_field(j, "rarity");
            uint32_t rarity = rarity_raw.empty() ? 255u : (uint32_t) json_int(j, "rarity", 255);
            send_json(res, 200, Server::live_admin_give(
                json_raw_field(j, "who"),
                (uint32_t) json_int(j, "petal_id", 0),
                (uint32_t) json_int(j, "count", 0),
                rarity));
            return;
        }
        if (action == "spawn") {
            send_json(res, 200, Server::live_admin_spawn(
                (uint8_t) json_int(j, "map_id", 0),
                (uint32_t) json_int(j, "mob_id", 0),
                json_float(j, "x", 0.f),
                json_float(j, "y", 0.f),
                json_raw_field(j, "near"),
                (uint32_t) json_int(j, "count", 1)));
            return;
        }
        send_json(res, 400, "{\"ok\":false,\"msg\":\"unknown action\"}");
    });
})
.get("/*", [](auto *res, auto *req) {
    std::string file = map_static_path(req->getUrl());
    if (file.empty()) {
        res->writeStatus("404 Not Found");
        res->end("Not Found");
        return;
    }
    send_file(res, file);
})
.ws<Client>("/*", {
    .compression = uWS::SHARED_COMPRESSOR,
    .maxPayloadLength = 64 * 1024,
    .idleTimeout = 120,
    .maxBackpressure = 1024 * MAX_PACKET_LEN,
    .closeOnBackpressureLimit = false,
    // Only *received* data refreshes the idle timer. With this on, our 20 Hz
    // update stream kept a half-open socket alive forever (client Wi-Fi dies
    // silently -> nginx never notices -> zombie flower, "replaced stale player"),
    // because every send reset the timeout before the auto-ping could probe it.
    // Off: ~104 s of silence sends a ping, 16 s later the socket is dropped.
    .resetIdleTimeoutOnSend = false,
    .sendPingsAutomatically = true,
    // Only known reverse proxies may supply forwarded client identities.
    .upgrade = [](auto *res, auto *req, auto *context) {
        Client c{};
        c.remote_ip = ClientAddress::resolve(res->getRemoteAddressAsText(), req->getHeader("x-forwarded-for"));
        res->template upgrade<Client>(std::move(c),
            req->getHeader("sec-websocket-key"),
            req->getHeader("sec-websocket-protocol"),
            req->getHeader("sec-websocket-extensions"),
            context);
    },
    .open = [](WebSocket *ws) {
        std::cout << "client connection\n";
        Client *client = ws->getUserData();
        client->ws = ws;
        client->open_ms = (uint64_t) std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
        if (client->remote_ip.empty()) {
            std::string_view addr = ws->getRemoteAddressAsText();
            client->remote_ip.assign(addr.data(), addr.size());
        }
        if (!RateLimit::allow_connect(client->remote_ip)) {
            client->disconnect(CloseReason::kServer, "Too Many Connections");
            return;
        }
        RateLimit::on_connect(client->remote_ip);
        client->conn_counted = 1;
    },
    .message = [](WebSocket *ws, std::string_view message, uWS::OpCode opCode) {
        Client::on_message(ws, message, opCode);
    },
    .dropped = [](WebSocket * /*ws*/, std::string_view /*message*/, uWS::OpCode /*opCode*/) {
    },
    .drain = [](WebSocket */*ws*/) {
    },
    .close = [](WebSocket *ws, int code, std::string_view message) {
        Client::on_disconnect(ws, code, message);
    }
})
.listen(SERVER_PORT, [](auto *listen_socket) {
    g_listen_socket = listen_socket;
    if (listen_socket) {
        std::cout << "Native listening on port " << SERVER_PORT << std::endl;
    } else {
        std::cerr << "Failed to listen on port " << SERVER_PORT << std::endl;
    }
});

void Server::run() {
    // journald gets our stdout through a pipe, which is block-buffered by default:
    // log lines then appear in bursts and their timestamps lie (made a live
    // disconnect investigation nearly impossible). Line-buffer it.
    std::setvbuf(stdout, nullptr, _IOLBF, 0);
    struct sigaction sa;
    sa.sa_handler = [](int){ Server::is_draining = 1; };
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGUSR2, &sa, nullptr);

    struct us_loop_t *loop = (struct us_loop_t *) uWS::Loop::get();
    g_delay_timer = us_create_timer(loop, 0, 0);

    us_timer_set(g_delay_timer, [](us_timer_t *x){ Server::tick(); }, 1, 1000 / TPS);
    Server::server.run();
}

void Server::stop() {
    if (Server::is_stopping) return;
    Server::is_stopping = true;
    std::cout << "stopping...\n";
    AccountStore::save();
    LeaderboardStore::flush();
    if (g_listen_socket != nullptr)
        us_listen_socket_close(0, g_listen_socket);
    if (g_delay_timer != nullptr)
        us_timer_close(g_delay_timer);
    if (Server::games != nullptr) {
        for (uint8_t i = 0; i < MapID::kNumMaps; ++i) {
            std::vector<Client *> snap(
                Server::games[i].clients.begin(),
                Server::games[i].clients.end());
            for (Client *c : snap) {
                if (c != nullptr)
                    c->disconnect(1001, "Shutting Down");
            }
        }
    }
}

uint8_t Client::can_accept_update() {
    if (ws == nullptr) return 0;
    // Match Wasm.cc (256KiB). Tighter 128KiB parked snapshots more often and
    // skipped deletes, leaving client ghosts. Slightly larger backlog can keep
    // a half-open socket alive longer — RX silence (>4s) + idleTimeout still
    // park/drop zombies; do not rely on a smaller send buffer for that.
    return ws->getBufferedAmount() <= 256 * 1024 ? 1 : 0;
}

void Client::send_packet(uint8_t const *packet, size_t size) {
    if (ws == nullptr) return;
    if (!can_accept_update()) return;
    std::string_view message(reinterpret_cast<char const *>(packet), size);
    // Browser-negotiated permessage-deflate keeps busy arenas from filling a
    // slow link with stale snapshots. Small control packets stay uncompressed.
    ws->send(message, uWS::OpCode::BINARY, size >= 512);
}
#endif
