#ifdef WASM_SERVER
#include <Server/Client.hh>
#include <Server/RateLimit.hh>
#include <Server/Server.hh>

#include <Shared/Binary.hh>
#include <Shared/Config.hh>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <unordered_map>

#include <emscripten.h>

std::unordered_map<int, WebSocket *> WS_MAP;

size_t const MAX_BUFFER_LEN = 1024;
static uint8_t INCOMING_BUFFER[MAX_BUFFER_LEN] = {0};

extern "C" {
    static char *heap_strdup_std(std::string const &s) {
        char *out = (char *) std::malloc(s.size() + 1);
        if (out == nullptr) return nullptr;
        std::memcpy(out, s.c_str(), s.size() + 1);
        return out;
    }

    // EMSCRIPTEN_KEEPALIVE: HTTP /api/live reads online roster JSON
    EMSCRIPTEN_KEEPALIVE char *live_status_json() {
        return heap_strdup_std(Server::live_status_json());
    }

    EMSCRIPTEN_KEEPALIVE char *live_catalog_json() {
        return heap_strdup_std(Server::live_catalog_json());
    }

    EMSCRIPTEN_KEEPALIVE char *live_admin_announce(char const *text) {
        return heap_strdup_std(Server::live_admin_announce(text ? text : ""));
    }

    EMSCRIPTEN_KEEPALIVE char *live_admin_give(char const *who, int petal_id, int count, int rarity) {
        return heap_strdup_std(Server::live_admin_give(
            who ? who : "", (uint32_t) petal_id, (uint32_t) (count < 1 ? 1 : count),
            rarity < 0 ? 255u : (uint32_t) rarity));
    }

    EMSCRIPTEN_KEEPALIVE char *live_admin_spawn(int map_id, int mob_id, float x, float y,
                                                char const *near_who) {
        return heap_strdup_std(Server::live_admin_spawn(
            (uint8_t) map_id, (uint32_t) mob_id, x, y, near_who ? near_who : ""));
    }

    void on_connect(int ws_id, char const *ip) {
        std::printf("client connect: [%d]\n", ws_id);
        WebSocket *ws = new WebSocket(ws_id);
        Client *client = ws->getUserData();
        if (ip != nullptr && ip[0] != '\0')
            client->remote_ip = ip;
        WS_MAP.insert({ws_id, ws});
        if (!RateLimit::allow_connect(client->remote_ip)) {
            // Leave WS_MAP entry; on_disconnect deletes it after close.
            ws->end(CloseReason::kServer, "Too Many Connections");
            return;
        }
        RateLimit::on_connect(client->remote_ip);
        client->conn_counted = 1;
    }

    void on_disconnect(int ws_id, int reason) {
        auto iter = WS_MAP.find(ws_id);
        if (iter == WS_MAP.end()) {
            std::printf("unknown ws disconnect: [%d]", ws_id);
            return;
        }
        std::printf("client disconnect: [%d]\n", ws_id);
        Client::on_disconnect(iter->second, reason, {});
        delete iter->second;
        WS_MAP.erase(ws_id);
    }

    void tick() {
        Server::tick();
    }

    void sigusr2() {
        Server::is_draining = 1;
    }

    void on_message(int ws_id, uint32_t len) {
        auto iter = WS_MAP.find(ws_id);
        //WebSocket *ws = WS_MAP[ws_id];
        if (iter == WS_MAP.end()) return;
        std::string_view message(reinterpret_cast<char const *>(INCOMING_BUFFER), len);
        Client::on_message(iter->second, message, 0);
    }
}

WebSocketServer::WebSocketServer() {
    EM_ASM({
        const WSS = require("ws");
        const http = require("http");
        const fs = require("fs");
        const server = http.createServer(function(req, res) {
            let encodeType = "text/html; charset=utf-8";
            let file = "index.html";
            // Strip ?query so cache-busting URLs still resolve
            const path = (req.url || "/").split("?")[0];
            const sendJson = (status, text) => {
                const buf = Buffer.from(text, "utf8");
                res.writeHead(status, {
                    "Content-Type": "application/json; charset=utf-8",
                    "Cache-Control": "no-store",
                    "Access-Control-Allow-Origin": "*",
                    "Content-Length": buf.length
                });
                res.end(buf);
            };
            const cstr = (str) => {
                const s = String(str == null ? "" : str);
                const len = lengthBytesUTF8(s) + 1;
                const ptr = _malloc(len);
                stringToUTF8(s, ptr, len);
                return ptr;
            };
            const fromWasm = (ptr) => {
                if (!ptr) return "{\"ok\":false,\"msg\":\"wasm null\"}";
                const t = UTF8ToString(ptr);
                _free(ptr);
                return t;
            };
            if (path === "/api/live") {
                const ptr = _live_status_json();
                if (!ptr) {
                    sendJson(500, "{\"ok\":false,\"msg\":\"live unavailable\"}");
                    return;
                }
                // WASM system_clock can drift; stamp wall time from Node.
                let body = UTF8ToString(ptr);
                _free(ptr);
                // Use [0-9] — EM_ASM eats \d escapes under Closure.
                body = body.replace(/"ts":[0-9]+/, '"ts":' + Date.now());
                try {
                    const data = JSON.parse(body);
                    data["auth_required"] = process.env["ASHFLOR_LIVE_AUTH"] === "1";
                    let cacheV = "";
                    try {
                        const html = fs.readFileSync("index.html", "utf8");
                        const m = html.match(/var cacheV = '(\d+)'/);
                        if (m) cacheV = m[1];
                    } catch (e) {}
                    let deploy = {};
                    try {
                        deploy = JSON.parse(fs.readFileSync("deploy-info.json", "utf8"));
                    } catch (e) {}
                    data["deploy"] = Object.assign({
                        "cache_v": cacheV,
                        "port": process.env["PORT"] || 9001,
                        "auth_required": data["auth_required"],
                        "node": process.version
                    }, deploy);
                    body = JSON.stringify(data);
                } catch (e) {}
                sendJson(200, body);
                return;
            }
            if (path === "/api/catalog") {
                sendJson(200, fromWasm(_live_catalog_json()));
                return;
            }
            if (path === "/api/admin") {
                if (req.method === "OPTIONS") {
                    res.writeHead(204, {
                        "Access-Control-Allow-Origin": "*",
                        "Access-Control-Allow-Methods": "POST, OPTIONS",
                        "Access-Control-Allow-Headers": "Content-Type, Authorization"
                    });
                    res.end();
                    return;
                }
                if (req.method !== "POST") {
                    sendJson(405, "{\"ok\":false,\"msg\":\"POST only\"}");
                    return;
                }
                const chunks = [];
                let size = 0;
                req.on("data", function(c) {
                    size += c.length;
                    if (size > 8192) {
                        req.destroy();
                        return;
                    }
                    chunks.push(c);
                });
                req.on("end", function() {
                    let j = null;
                    try { j = JSON.parse(Buffer.concat(chunks).toString("utf8") || "{}"); }
                    catch (e) { sendJson(400, "{\"ok\":false,\"msg\":\"bad json\"}"); return; }
                    // Bracket keys so Closure does not rename JSON field names.
                    // Auth is OFF unless ASHFLOR_LIVE_AUTH=1 (plus USER/PASS).
                    const authOn = process.env["ASHFLOR_LIVE_AUTH"] === "1";
                    if (authOn) {
                        const needUser = process.env["ASHFLOR_LIVE_USER"] || "";
                        const needPass = process.env["ASHFLOR_LIVE_PASS"] || "";
                        if (!needUser || !needPass) {
                            sendJson(503, "{\"ok\":false,\"msg\":\"auth enabled but USER/PASS missing\"}");
                            return;
                        }
                        let user = (j && j["user"]) || "";
                        let pass = (j && j["pass"]) || "";
                        const auth = req.headers["authorization"] || "";
                        if ((!user || !pass) && auth.indexOf("Basic ") === 0) {
                            try {
                                const raw = Buffer.from(auth.slice(6), "base64").toString("utf8");
                                const i = raw.indexOf(":");
                                if (i >= 0) {
                                    user = raw.slice(0, i);
                                    pass = raw.slice(i + 1);
                                }
                            } catch (e) {}
                        }
                        // Bracket crypto API — Closure renames timingSafeEqual otherwise.
                        const crypto = require("crypto");
                        const same = (a, b) => {
                            const ba = Buffer.from(String(a), "utf8");
                            const bb = Buffer.from(String(b), "utf8");
                            if (ba.length !== bb.length) return false;
                            return crypto["timingSafeEqual"](ba, bb);
                        };
                        if (!same(user, needUser) || !same(pass, needPass)) {
                            sendJson(401, "{\"ok\":false,\"msg\":\"账号或密码错误\"}");
                            return;
                        }
                    }
                    const action = (j && j["action"]) || "";
                    try {
                        if (action === "login") {
                            sendJson(200, authOn
                                ? "{\"ok\":true,\"msg\":\"登录成功\",\"auth\":true}"
                                : "{\"ok\":true,\"msg\":\"鉴权未启用\",\"auth\":false}");
                            return;
                        }
                        if (action === "announce") {
                            const p = cstr(j["text"] || "");
                            const out = fromWasm(_live_admin_announce(p));
                            _free(p);
                            sendJson(200, out);
                            return;
                        }
                        if (action === "give") {
                            const p = cstr(j["who"] || "");
                            const rarityRaw = j["rarity"];
                            const rarity = (rarityRaw === undefined || rarityRaw === null || rarityRaw === "")
                                ? -1 : (Number(rarityRaw) | 0);
                            const out = fromWasm(_live_admin_give(
                                p, Number(j["petal_id"]) | 0, Number(j["count"]) | 0, rarity));
                            _free(p);
                            sendJson(200, out);
                            return;
                        }
                        if (action === "spawn") {
                            const p = cstr(j["near"] || "");
                            const out = fromWasm(_live_admin_spawn(
                                Number(j["map_id"]) | 0,
                                Number(j["mob_id"]) | 0,
                                Number(j["x"]) || 0,
                                Number(j["y"]) || 0,
                                p));
                            _free(p);
                            sendJson(200, out);
                            return;
                        }
                        sendJson(400, "{\"ok\":false,\"msg\":\"unknown action\"}");
                    } catch (e) {
                        sendJson(500, "{\"ok\":false,\"msg\":\"admin failed\"}");
                    }
                });
                return;
            }
            switch (path) {
                case "/":
                    break;
                case "/live":
                case "/live.html":
                    file = "live.html";
                    break;
                case "/deploy-info.json":
                    encodeType = "application/json; charset=utf-8";
                    file = "deploy-info.json";
                    break;
                case "/gardn-client.js":
                    encodeType = "application/javascript; charset=utf-8";
                    file = "gardn-client.js";
                    break;
                case "/gardn-client.wasm":
                    encodeType = "application/wasm";
                    file = "gardn-client.wasm";
                    break;
                case "/sw.js":
                    encodeType = "application/javascript; charset=utf-8";
                    file = "sw.js";
                    break;
                case "/favicon.svg":
                    encodeType = "image/svg+xml";
                    file = "favicon.svg";
                    break;
                case "/favicon.ico":
                    encodeType = "image/png";
                    file = "favicon-32.png";
                    break;
                case "/favicon-32.png":
                    encodeType = "image/png";
                    file = "favicon-32.png";
                    break;
                case "/favicon-48.png":
                    encodeType = "image/png";
                    file = "favicon-48.png";
                    break;
                case "/favicon-96.png":
                    encodeType = "image/png";
                    file = "favicon-96.png";
                    break;
                case "/favicon-192.png":
                    encodeType = "image/png";
                    file = "favicon-192.png";
                    break;
                case "/favicon-512.png":
                    encodeType = "image/png";
                    file = "favicon-512.png";
                    break;
                case "/apple-touch-icon.png":
                    encodeType = "image/png";
                    file = "apple-touch-icon.png";
                    break;
                case "/site.webmanifest":
                    encodeType = "application/manifest+json";
                    file = "site.webmanifest";
                    break;
                default:
                    file = "";
                    break;
            }
            if (file && fs.existsSync(file)) {
                // HTML/SW: revalidate. Icons + JS/WASM: long-cache
                const cacheCtl = (file === "index.html" || file === "sw.js" || file === "live.html" || file === "deploy-info.json")
                    ? "no-cache"
                    : "public, max-age=31536000, immutable";
                const headers = {
                    "Content-Type": encodeType,
                    "Cache-Control": cacheCtl
                };
                let servedFile = file;
                if (file === "gardn-client.js" || file === "gardn-client.wasm") {
                    const accepted = req.headers["accept-encoding"] || "";
                    const accepts = encoding => accepted.split(",").some(part => {
                        const fields = part.trim().split(";");
                        return fields[0] === encoding
                            && !fields.slice(1).some(option => Number(option.trim().split("=")[1]) === 0);
                    });
                    headers["Vary"] = "Accept-Encoding";
                    if (accepts("br") && fs.existsSync(file + ".br")) {
                        servedFile = file + ".br";
                        headers["Content-Encoding"] = "br";
                    } else if (accepts("gzip") && fs.existsSync(file + ".gz")) {
                        servedFile = file + ".gz";
                        headers["Content-Encoding"] = "gzip";
                    }
                }
                if (file === "sw.js")
                    headers["Service-Worker-Allowed"] = "/";
                fs.readFile(servedFile, function(err, body) {
                    if (err) {
                        res.writeHead(500, {"Content-Type": "text/plain; charset=utf-8"});
                        res.end("Read failed");
                        return;
                    }
                    headers["Content-Length"] = body.length;
                    res.writeHead(200, headers);
                    res.end(body);
                });
                return;
            }
            res.writeHead(404, {"Content-Type": "text/plain; charset=utf-8"});
            res.end("Not Found");
        });

        Module.server = server;
        server.listen($0, function() {
            console.log("Server running at http://localhost:"+$0);
        });
        
        const wss = new WSS.Server({ "server": server });
        Module.wss = wss;
        Module.ws_connections = {};
        let curr_id = 0;
        // Protocol ping keeps nginx / mobile NATs from dropping "idle" sockets
        setInterval(function() {
            const map = Module.ws_connections || {};
            for (const id in map) {
                const ws = map[id];
                if (!ws || ws.readyState !== 1) continue;
                try { ws.ping(); } catch (e) {}
            }
        }, 15000);
        wss.on("connection", function(ws, req) {
            const ws_id = curr_id;
            Module.ws_connections[ws_id] = ws;
            // Prefer first X-Forwarded-For hop (player); X-Real-IP may be edge IP after multi-proxy.
            let ip = "";
            const xff = (req && req.headers && req.headers["x-forwarded-for"]) || "";
            if (xff) ip = String(xff).split(",")[0].trim();
            if (!ip && req && req.headers && req.headers["x-real-ip"])
                ip = String(req.headers["x-real-ip"]).trim();
            if (!ip && req && req.socket && req.socket.remoteAddress)
                ip = String(req.socket.remoteAddress);
            if (ip.indexOf("::ffff:") === 0) ip = ip.slice(7);
            const ipLen = lengthBytesUTF8(ip) + 1;
            const ipPtr = _malloc(ipLen);
            stringToUTF8(ip, ipPtr, ipLen);
            _on_connect(ws_id, ipPtr);
            _free(ipPtr);
            curr_id = (curr_id + 1) | 0;
            ws.on("message", function(message) {
                let data = new Uint8Array(message);
                const len = data.length > $2 ? $2 : data.length;
                data = data.subarray(0, len);
                HEAPU8.set(data, $1);
                _on_message(ws_id, len);
            });
            ws.on("close", function(reason) {
                _on_disconnect(ws_id, reason);
                delete Module.ws_connections[ws_id];
            });
            ws.on("error", function() {});
        })
    }, SERVER_PORT, INCOMING_BUFFER, MAX_BUFFER_LEN);
}

void Server::run() {
    EM_ASM({
        globalThis.Module = Module;
        process.on("SIGUSR2", function () {
            console.log("SIGUSR2: draining...");
            _sigusr2();
        });
        Module.tickInterval = setInterval(_tick, $0);
    }, 1000 / TPS);
}

void Server::stop() {
    if (Server::is_stopping) return;
    Server::is_stopping = true;
    std::cout << "stopping...\n";
    EM_ASM({
        try {
            if (Module.tickInterval) clearInterval(Module.tickInterval);
            Module.tickInterval = 0;
            if (Module.wss) {
                try { Module.wss.close(); } catch (e) {}
            }
            for (const id in (Module.ws_connections || {})) {
                const ws = Module.ws_connections[id];
                try { ws.close(1001, "Shutting Down"); } catch (e) {}
            }
            if (Module.server) {
                Module.server.close(function () {
                    setTimeout(function () { process.exit(0); }, 100);
                });
            } else {
                setTimeout(function () { process.exit(0); }, 100);
            }
        } catch (e) {
            process.exit(0);
        }
    });
}

uint8_t Client::can_accept_update() {
    if (ws == nullptr) return 0;
    return (uint8_t) EM_ASM_INT({
        if (!Module.ws_connections || !Module.ws_connections[$0]) return 0;
        const ws = Module.ws_connections[$0];
        if (ws.readyState !== 1) return 0;
        return ws.bufferedAmount <= 128 * 1024 ? 1 : 0;
    }, ws->id());
}

void Client::send_packet(uint8_t const *packet, size_t size) {
    if (ws == nullptr) return;
    ws->send(packet, size);
}

WebSocket::WebSocket(int id) : ws_id(id) {
    client.ws = this;
}

void WebSocket::send(uint8_t const *packet, size_t size) {
    // Skip when client is frozen in background — flooding → OS kills with 1006
    EM_ASM({
        if (!Module.ws_connections || !Module.ws_connections[$0]) return;
        const ws = Module.ws_connections[$0];
        if (ws.readyState !== 1) return;
        if (ws.bufferedAmount > 128 * 1024) return;
        ws.send(HEAPU8.subarray($1,$1+$2));
    }, ws_id, packet, size);
}

void WebSocket::end(int code, std::string const &message) {
    EM_ASM({
        if (!Module.ws_connections || !Module.ws_connections[$0]) return;
        const ws = Module.ws_connections[$0];
        ws.close($1, UTF8ToString($2));
    }, ws_id, code, message.c_str());
}

Client *WebSocket::getUserData() {
    return &client;
}

WebSocketServer Server::server;
#endif
