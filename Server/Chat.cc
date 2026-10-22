#include <Server/Chat.hh>

#include <Server/Client.hh>
#include <Server/Game.hh>
#include <Server/Guild.hh>
#include <Server/NodeFs.hh>
#include <Server/Party.hh>
#include <Server/Server.hh>

#include <Shared/Binary.hh>
#include <Shared/Config.hh>
#include <Shared/Entity.hh>
#include <Shared/Simulation.hh>
#include <Shared/StaticDefinitions.hh>

#include <Helpers/UTF8.hh>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <deque>
#include <sstream>
#include <string>
#include <vector>

// 全服广播单独冷却，避免刷屏淹没附近/公会频道
static constexpr uint64_t SERVER_CHAT_COOLDOWN_MS = 15000;
static char const *BC_FILE = "broadcasts.dat";
static char const *CHAT_FILE = "chatlog.dat";

// Single-threaded WASM server tick — no lock needed.
// Deques are oldest→newest (push_back).
static uint32_t g_live_bc_next_id = 1;
static std::deque<Chat::LiveBroadcast> g_live_bc;
static uint8_t g_bc_dirty = 0;
static uint32_t g_chat_next_id = 1;
static std::deque<Chat::ChatLogEntry> g_chat_log;
static uint8_t g_chat_dirty = 0;

static uint64_t wall_ms() {
    return (uint64_t) std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

static std::string read_file(char const *path) {
#ifdef WASM_SERVER
    char *ptr = node_fs_read_alloc(path);
    if (ptr == nullptr) return {};
    std::string out(ptr);
    free(ptr);
    return out;
#else
    FILE *f = fopen(path, "rb");
    if (!f) return {};
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz <= 0) { fclose(f); return {}; }
    std::string out((size_t) sz, '\0');
    fread(out.data(), 1, (size_t) sz, f);
    fclose(f);
    return out;
#endif
}

static void write_file(char const *path, std::string const &data) {
#ifdef WASM_SERVER
    node_fs_write_atomic(path, data.data(), (int) data.size());
#else
    FILE *f = fopen(path, "wb");
    if (!f) return;
    fwrite(data.data(), 1, data.size(), f);
    fclose(f);
#endif
}

static std::string esc_field(std::string const &s) {
    std::string out;
    out.reserve(s.size());
    for (unsigned char c : s) {
        if (c == '\\') out += "\\\\";
        else if (c == '\t') out += "\\t";
        else if (c == '\n' || c == '\r') out += "\\n";
        else out += (char) c;
    }
    return out;
}

static std::string unesc_field(std::string const &s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '\\' && i + 1 < s.size()) {
            char n = s[++i];
            if (n == '\\') out += '\\';
            else if (n == 't') out += '\t';
            else if (n == 'n') out += '\n';
            else out += n;
        } else out += s[i];
    }
    return out;
}

static std::string day_of(uint64_t ts_ms) {
    // Asia/Shanghai ≈ UTC+8 for filter convenience
    time_t sec = (time_t) ((ts_ms / 1000ull) + 8ull * 3600ull);
    struct tm t;
#ifdef _WIN32
    gmtime_s(&t, &sec);
#else
    gmtime_r(&sec, &t);
#endif
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
    return buf;
}

static void json_escape_append(std::string &out, std::string const &s) {
    for (unsigned char c : s) {
        if (c == '"' || c == '\\') { out += '\\'; out += (char) c; }
        else if (c < 0x20) {
            char b[8];
            std::snprintf(b, sizeof(b), "\\u%04x", c);
            out += b;
        } else out += (char) c;
    }
}

static void enforce_bc_cap() {
    // Over cap: drop oldest kill first; only when no kills left, drop oldest craft.
    // Never drop announce.
    while (g_live_bc.size() > Chat::LIVE_BROADCAST_CAP) {
        auto it = g_live_bc.begin();
        for (; it != g_live_bc.end(); ++it) {
            if (it->kind == Chat::kKill) break;
        }
        if (it == g_live_bc.end()) {
            it = g_live_bc.begin();
            for (; it != g_live_bc.end(); ++it) {
                if (it->kind == Chat::kCraft) break;
            }
        }
        if (it == g_live_bc.end()) break; // only announces left
        g_live_bc.erase(it);
        g_bc_dirty = 1;
    }
}

static uint8_t map_of(Client *c) {
    if (c == nullptr || c->game == nullptr) return Chat::MAP_ALL;
    return c->game->map_id;
}

static void push_broadcast(Chat::LiveBcKind kind, std::string sender, std::string player,
                           std::string text, uint8_t map_id) {
    Chat::LiveBroadcast entry;
    entry.id = g_live_bc_next_id++;
    if (g_live_bc_next_id == 0) g_live_bc_next_id = 1;
    entry.kind = (uint8_t) kind;
    entry.map_id = map_id;
    entry.ts_ms = wall_ms();
    entry.sender = std::move(sender);
    entry.player = std::move(player);
    entry.text = std::move(text);
    g_live_bc.push_back(std::move(entry));
    g_bc_dirty = 1;
    enforce_bc_cap();
}

static std::string display_name(Client *client) {
    if (client == nullptr || client->game == nullptr) return "Unnamed";
    Simulation *sim = &client->game->simulation;
    if (!sim->ent_exists(client->camera)) return "Unnamed";
    Entity &camera = sim->get_ent(client->camera);
    if (sim->ent_alive(camera.get_player())) {
        std::string n = sim->get_ent(camera.get_player()).get_name();
        if (!n.empty()) return n;
    }
    if (!client->account_name.empty()) return client->account_name;
    return "Unnamed";
}

static Client *find_in_game_by_name(GameInstance *game, std::string const &name) {
    if (game == nullptr || name.empty()) return nullptr;
    for (Client *c : game->clients) {
        if (c == nullptr) continue;
        if (display_name(c) == name) return c;
    }
    return nullptr;
}

// Whisper targets: any player still in a map room (alive or death screen).
static Client *find_online_by_name(std::string const &name) {
    if (name.empty() || Server::games == nullptr) return nullptr;
    for (uint8_t mi = 0; mi < MapID::kNumMaps; ++mi) {
        Client *c = find_in_game_by_name(&Server::games[mi], name);
        if (c != nullptr) return c;
    }
    return nullptr;
}

static uint8_t chat_xy(Client *client, float &x, float &y) {
    if (client == nullptr || client->game == nullptr) return 0;
    Simulation *sim = &client->game->simulation;
    if (!sim->ent_exists(client->camera)) return 0;
    Entity &camera = sim->get_ent(client->camera);
    if (sim->ent_alive(camera.get_player())) {
        Entity &p = sim->get_ent(camera.get_player());
        x = p.get_x();
        y = p.get_y();
        return 1;
    }
    x = camera.get_camera_x();
    y = camera.get_camera_y();
    return 1;
}

static void trim_inplace(std::string &s) {
    while (!s.empty() && (s.front() == ' ' || s.front() == '\t')) s.erase(s.begin());
    while (!s.empty() && (s.back() == ' ' || s.back() == '\t')) s.pop_back();
}

static uint8_t cmd_match(std::string const &text, char const *cmd) {
    size_t n = std::strlen(cmd);
    if (text.size() < n) return 0;
    if (text.compare(0, n, cmd) != 0) return 0;
    return text.size() == n || text[n] == ' ' || text[n] == '\t';
}

static std::string cmd_args(std::string const &text, char const *cmd) {
    size_t n = std::strlen(cmd);
    if (text.size() <= n) return {};
    std::string a = text.substr(n);
    trim_inplace(a);
    return a;
}

static void split_first(std::string const &s, std::string &first, std::string &rest) {
    size_t sp = s.find_first_of(" \t");
    if (sp == std::string::npos) {
        first = s;
        rest.clear();
        return;
    }
    first = s.substr(0, sp);
    rest = s.substr(sp + 1);
    trim_inplace(rest);
}

void Chat::send_to(Client *client, uint8_t channel, std::string const &sender, std::string const &text,
                   std::string const &whisper_to) {
    if (client == nullptr) return;
    Writer writer(Server::OUTGOING_PACKET);
    writer.write<uint8_t>(Clientbound::kChatMessage);
    writer.write<uint8_t>(channel);
    writer.write<std::string>(sender);
    writer.write<std::string>(text);
    writer.write<std::string>(whisper_to);
    client->send_packet(writer.packet, writer.at - writer.packet);
}

void Chat::log_chat(uint8_t channel, std::string const &from, std::string const &to,
                    std::string const &text, uint8_t map_id) {
    if (text.empty()) return;
    // Chat is never evicted; at hard cap stop accepting new lines.
    if (g_chat_log.size() >= Chat::CHAT_LOG_CAP) return;
    Chat::ChatLogEntry e;
    e.id = g_chat_next_id++;
    if (g_chat_next_id == 0) g_chat_next_id = 1;
    e.channel = channel;
    e.map_id = map_id;
    e.ts_ms = wall_ms();
    e.from = from;
    e.to = to;
    e.text = text;
    g_chat_log.push_back(std::move(e));
    g_chat_dirty = 1;
}

void Chat::broadcast_server(std::string const &sender, std::string const &text,
                            std::string const &whisper_to, LiveBcKind kind,
                            std::string const &player, uint8_t map_id) {
    std::string who = player;
    if (who.empty() && sender != "系统" && sender != "公告") who = sender;
    push_broadcast(kind, sender, who, text, map_id);
    if (Server::games == nullptr) return;
    for (uint8_t mi = 0; mi < MapID::kNumMaps; ++mi) {
        for (Client *c : Server::games[mi].clients) {
            if (c != nullptr)
                Chat::send_to(c, ChatChannel::kChatServer, sender, text, whisper_to);
        }
    }
}

void Chat::broadcast_server_banner(std::string const &sender, std::string const &text,
                                   std::string const &player, uint8_t map_id) {
    std::string who = player;
    if (who.empty() && sender != "系统" && sender != "公告") who = sender;
    push_broadcast(Chat::kAnnounce, sender, who, text, map_id);
    if (Server::games == nullptr) return;
    for (uint8_t mi = 0; mi < MapID::kNumMaps; ++mi) {
        for (Client *c : Server::games[mi].clients) {
            if (c != nullptr)
                Chat::send_to(c, ChatChannel::kChatServer, sender, text, "banner");
        }
    }
}

void Chat::recent_broadcasts(std::vector<Chat::LiveBroadcast> &out) {
    out.clear();
    size_t n = g_live_bc.size();
    size_t start = n > Chat::LIVE_BROADCAST_FEED ? n - Chat::LIVE_BROADCAST_FEED : 0;
    out.assign(g_live_bc.begin() + (std::ptrdiff_t) start, g_live_bc.end());
}

void Chat::load() {
    g_live_bc.clear();
    g_chat_log.clear();
    g_bc_dirty = 0;
    g_chat_dirty = 0;
    g_live_bc_next_id = 1;
    g_chat_next_id = 1;
    {
        std::istringstream in(read_file(BC_FILE));
        std::string line;
        while (std::getline(in, line)) {
            if (line.empty() || line[0] == '#') continue;
            // TSV: id kind ts [map] sender player text  (map optional for old files)
            std::vector<std::string> cols;
            {
                std::string cur;
                for (char c : line) {
                    if (c == '\t') { cols.push_back(cur); cur.clear(); }
                    else cur += c;
                }
                cols.push_back(cur);
            }
            if (cols.size() < 6) continue;
            size_t i = 0;
            Chat::LiveBroadcast e;
            e.id = (uint32_t) std::strtoul(cols[i++].c_str(), nullptr, 10);
            e.kind = (uint8_t) std::atoi(cols[i++].c_str());
            e.ts_ms = (uint64_t) std::strtoull(cols[i++].c_str(), nullptr, 10);
            if (cols.size() >= 7) {
                e.map_id = (uint8_t) std::atoi(cols[i++].c_str());
            } else {
                e.map_id = Chat::MAP_ALL;
            }
            e.sender = unesc_field(cols[i++]);
            e.player = unesc_field(cols[i++]);
            if (e.player == "-") e.player.clear();
            e.text = unesc_field(cols[i]);
            if (e.id == 0) continue;
            g_live_bc.push_back(std::move(e));
            if (e.id >= g_live_bc_next_id) g_live_bc_next_id = e.id + 1;
        }
        enforce_bc_cap();
    }
    {
        std::istringstream in(read_file(CHAT_FILE));
        std::string line;
        while (std::getline(in, line)) {
            if (line.empty() || line[0] == '#') continue;
            std::vector<std::string> cols;
            {
                std::string cur;
                for (char c : line) {
                    if (c == '\t') { cols.push_back(cur); cur.clear(); }
                    else cur += c;
                }
                cols.push_back(cur);
            }
            if (cols.size() < 6) continue;
            size_t i = 0;
            Chat::ChatLogEntry e;
            e.id = (uint32_t) std::strtoul(cols[i++].c_str(), nullptr, 10);
            e.channel = (uint8_t) std::atoi(cols[i++].c_str());
            e.ts_ms = (uint64_t) std::strtoull(cols[i++].c_str(), nullptr, 10);
            if (cols.size() >= 7) {
                e.map_id = (uint8_t) std::atoi(cols[i++].c_str());
            } else {
                e.map_id = Chat::MAP_ALL;
            }
            e.from = unesc_field(cols[i++]);
            e.to = unesc_field(cols[i++]);
            e.text = unesc_field(cols[i]);
            if (e.to == "-") e.to.clear();
            if (e.id == 0) continue;
            g_chat_log.push_back(std::move(e));
            if (e.id >= g_chat_next_id) g_chat_next_id = e.id + 1;
        }
        // Never trim chat on load (announce/chat are keep-forever within hard cap).
    }
    if (g_live_bc_next_id == 0) g_live_bc_next_id = 1;
    if (g_chat_next_id == 0) g_chat_next_id = 1;
}

void Chat::flush() {
    if (g_bc_dirty) {
        std::ostringstream out;
        out << "# ashflor broadcasts TSV: id kind ts_ms map sender player text\n";
        for (Chat::LiveBroadcast const &e : g_live_bc) {
            out << e.id << '\t' << (int) e.kind << '\t' << e.ts_ms << '\t'
                << (int) e.map_id << '\t'
                << esc_field(e.sender) << '\t' << esc_field(e.player.empty() ? "-" : e.player)
                << '\t' << esc_field(e.text) << '\n';
        }
        write_file(BC_FILE, out.str());
        g_bc_dirty = 0;
    }
    if (g_chat_dirty) {
        std::ostringstream out;
        out << "# ashflor chatlog TSV: id channel ts_ms map from to text\n";
        for (Chat::ChatLogEntry const &e : g_chat_log) {
            out << e.id << '\t' << (int) e.channel << '\t' << e.ts_ms << '\t'
                << (int) e.map_id << '\t'
                << esc_field(e.from) << '\t' << esc_field(e.to.empty() ? "-" : e.to)
                << '\t' << esc_field(e.text) << '\n';
        }
        write_file(CHAT_FILE, out.str());
        g_chat_dirty = 0;
    }
}

static uint8_t match_player(std::string const &needle, std::string const &a, std::string const &b) {
    if (needle.empty()) return 1;
    std::string n = needle;
    for (char &c : n) if (c >= 'A' && c <= 'Z') c = (char) (c - 'A' + 'a');
    auto has = [&](std::string s) {
        for (char &c : s) if (c >= 'A' && c <= 'Z') c = (char) (c - 'A' + 'a');
        return s.find(n) != std::string::npos;
    };
    return has(a) || has(b);
}

std::string Chat::query_broadcasts_json(uint32_t before_id, uint32_t limit,
                                        std::string const &day, std::string const &player,
                                        int kind_filter, int map_filter) {
    if (limit < 1) limit = 50;
    if (limit > 200) limit = 200;
    std::vector<Chat::LiveBroadcast const *> hits;
    hits.reserve(limit);
    // Walk newest→oldest, collect, then reverse to oldest→newest for UI.
    for (auto it = g_live_bc.rbegin(); it != g_live_bc.rend(); ++it) {
        if (before_id && it->id >= before_id) continue;
        if (kind_filter >= 0 && (int) it->kind != kind_filter) continue;
        if (map_filter >= 0 && (int) it->map_id != map_filter) continue;
        if (!day.empty() && day_of(it->ts_ms) != day) continue;
        if (!match_player(player, it->player, it->sender)) continue;
        hits.push_back(&(*it));
        if (hits.size() >= limit) break;
    }
    std::reverse(hits.begin(), hits.end());
    std::string out = "{\"rows\":[";
    for (size_t i = 0; i < hits.size(); ++i) {
        Chat::LiveBroadcast const &e = *hits[i];
        if (i) out += ',';
        out += "{\"id\":";
        out += std::to_string(e.id);
        out += ",\"kind\":";
        out += std::to_string(e.kind);
        out += ",\"map\":";
        out += std::to_string(e.map_id);
        out += ",\"ts\":";
        out += std::to_string(e.ts_ms);
        out += ",\"sender\":\"";
        json_escape_append(out, e.sender);
        out += "\",\"player\":\"";
        json_escape_append(out, e.player);
        out += "\",\"text\":\"";
        json_escape_append(out, e.text);
        out += "\"}";
    }
    out += "],\"total\":";
    out += std::to_string(g_live_bc.size());
    out += '}';
    return out;
}

std::string Chat::query_chatlog_json(uint32_t before_id, uint32_t limit,
                                     std::string const &day, std::string const &player,
                                     int channel_filter, int map_filter) {
    if (limit < 1) limit = 50;
    if (limit > 200) limit = 200;
    std::vector<Chat::ChatLogEntry const *> hits;
    hits.reserve(limit);
    for (auto it = g_chat_log.rbegin(); it != g_chat_log.rend(); ++it) {
        if (before_id && it->id >= before_id) continue;
        if (channel_filter >= 0 && (int) it->channel != channel_filter) continue;
        if (map_filter >= 0 && (int) it->map_id != map_filter) continue;
        if (!day.empty() && day_of(it->ts_ms) != day) continue;
        if (!match_player(player, it->from, it->to)) continue;
        hits.push_back(&(*it));
        if (hits.size() >= limit) break;
    }
    std::reverse(hits.begin(), hits.end());
    std::string out = "{\"rows\":[";
    for (size_t i = 0; i < hits.size(); ++i) {
        Chat::ChatLogEntry const &e = *hits[i];
        if (i) out += ',';
        out += "{\"id\":";
        out += std::to_string(e.id);
        out += ",\"channel\":";
        out += std::to_string(e.channel);
        out += ",\"map\":";
        out += std::to_string(e.map_id);
        out += ",\"ts\":";
        out += std::to_string(e.ts_ms);
        out += ",\"from\":\"";
        json_escape_append(out, e.from);
        out += "\",\"to\":\"";
        json_escape_append(out, e.to);
        out += "\",\"text\":\"";
        json_escape_append(out, e.text);
        out += "\"}";
    }
    out += "],\"total\":";
    out += std::to_string(g_chat_log.size());
    out += '}';
    return out;
}

static void send_nearby(Client *client, std::string const &name, std::string const &text) {
    float mx = 0.f, my = 0.f;
    if (!chat_xy(client, mx, my)) return;
    Chat::log_chat(ChatChannel::kChatNearby, name, "", text, map_of(client));
    for (Client *c : client->game->clients) {
        if (c == nullptr) continue;
        float ox = 0.f, oy = 0.f;
        if (!chat_xy(c, ox, oy)) continue;
        float dx = ox - mx;
        float dy = oy - my;
        if (dx * dx + dy * dy > Chat::NEARBY_RADIUS * Chat::NEARBY_RADIUS) continue;
        Chat::send_to(c, ChatChannel::kChatNearby, name, text);
    }
}

static void send_party(Client *client, std::string const &name, std::string const &text) {
    if (client->party_id == 0) {
        Chat::send_to(client, ChatChannel::kChatSystem, "", "你还没有队伍。");
        return;
    }
    Chat::log_chat(ChatChannel::kChatParty, name, "", text, map_of(client));
    for (Client *c : client->game->clients) {
        if (c != nullptr && c->party_id == client->party_id)
            Chat::send_to(c, ChatChannel::kChatParty, name, text);
    }
}

static void send_guild(Client *client, std::string const &name, std::string const &text) {
    if (client->guild_name.empty()) {
        Chat::send_to(client, ChatChannel::kChatSystem, "", "你还没有公会。");
        return;
    }
    Chat::log_chat(ChatChannel::kChatGuild, name, client->guild_name, text, map_of(client));
    Guild::broadcast_chat(client, name, text);
}

static void send_server(Client *client, std::string const &name, std::string const &text, uint64_t now) {
    if (client->last_server_chat_ms != 0
        && now - client->last_server_chat_ms < SERVER_CHAT_COOLDOWN_MS) {
        uint64_t left = (SERVER_CHAT_COOLDOWN_MS
            - (now - client->last_server_chat_ms) + 999) / 1000;
        Chat::send_to(client, ChatChannel::kChatSystem, "",
            "全服频道冷却中，请 " + std::to_string(left) + " 秒后再试。");
        return;
    }
    client->last_server_chat_ms = now;
    Chat::log_chat(ChatChannel::kChatServer, name, "", text, map_of(client));
    for (uint8_t mi = 0; mi < MapID::kNumMaps; ++mi) {
        for (Client *c : Server::games[mi].clients) {
            if (c != nullptr)
                Chat::send_to(c, ChatChannel::kChatServer, name, text);
        }
    }
}

static void send_whisper(Client *from, std::string const &from_name,
                         std::string const &target_name, std::string const &msg) {
    if (target_name.empty()) {
        Chat::send_to(from, ChatChannel::kChatSystem, "",
            "用法: /w 玩家名 内容  （或 /whisper）");
        return;
    }
    if (msg.empty()) {
        Chat::send_to(from, ChatChannel::kChatSystem, "", "请输入私聊内容。");
        return;
    }
    if (target_name == from_name) {
        Chat::send_to(from, ChatChannel::kChatSystem, "", "不能私聊自己。");
        return;
    }
    Client *other = find_online_by_name(target_name);
    if (other == nullptr) {
        Chat::send_to(from, ChatChannel::kChatSystem, "", "找不到玩家：" + target_name);
        return;
    }
    Chat::log_chat(ChatChannel::kChatWhisper, from_name, target_name, msg, map_of(from));
    // Both sides get the same whisper_to so UI can show "aqa → Bob"
    Chat::send_to(from, ChatChannel::kChatWhisper, from_name, msg, target_name);
    Chat::send_to(other, ChatChannel::kChatWhisper, from_name, msg, target_name);
    other->last_whisper_from = from_name;
}

static void send_help(Client *client) {
    // Keep lines short for the mobile chat box (MAX_CHAT_LENGTH).
    char const *lines[] = {
        "—— 聊天指令（移动端）——",
        "/local|/附近 内容 — 附近频道",
        "/squad|/party|/组队聊 内容 — 队伍",
        "/guild|/公会 内容 — 公会频道",
        "/server|/全服 内容 — 全服(15s)",
        "/w|/whisper|/密语 名 内容 — 私聊",
        "/r|/reply|/回复 内容 — 回最近私聊",
        "/invite|/组队|/squad-invite 名 — 邀请入队",
        "/leave|/离队|/squad-leave — 离开队伍",
        "（附近自动组队；超出范围自动离队）",
        "/guild-create|/创建公会 名",
        "/guild-invite|/公会邀请 名",
        "/guild-leave|/退会 — 离开公会",
        "/guild-motd|/公告 内容 — 会长公告",
        "/help|/帮助 — 本列表",
    };
    for (char const *line : lines)
        Chat::send_to(client, ChatChannel::kChatSystem, "", line);
}

// Returns 1 if the line was consumed as a slash command (do not treat as normal chat).
static uint8_t handle_slash(Client *client, std::string const &text, uint64_t now) {
    std::string name = display_name(client);

    if (cmd_match(text, "/help") || cmd_match(text, "/帮助") || text == "/?") {
        send_help(client);
        return 1;
    }

    // —— Party (florr: squad) ——
    if (cmd_match(text, "/leave") || cmd_match(text, "/离队")
        || cmd_match(text, "/squad-leave") || cmd_match(text, "/party-leave")) {
        Party::leave(client);
        return 1;
    }
    if (cmd_match(text, "/invite") || cmd_match(text, "/组队")
        || cmd_match(text, "/squad-invite") || cmd_match(text, "/party-invite")) {
        std::string arg;
        if (text.rfind("/squad-invite", 0) == 0) arg = cmd_args(text, "/squad-invite");
        else if (text.rfind("/party-invite", 0) == 0) arg = cmd_args(text, "/party-invite");
        else if (text.rfind("/invite", 0) == 0) arg = cmd_args(text, "/invite");
        else arg = cmd_args(text, "/组队");
        Party::invite(client, arg);
        return 1;
    }
    if (cmd_match(text, "/squad") || cmd_match(text, "/party") || cmd_match(text, "/组队聊")) {
        std::string msg;
        if (text.rfind("/squad", 0) == 0) msg = cmd_args(text, "/squad");
        else if (text.rfind("/party", 0) == 0) msg = cmd_args(text, "/party");
        else msg = cmd_args(text, "/组队聊");
        if (msg.empty()) {
            Chat::send_to(client, ChatChannel::kChatSystem, "",
                "用法: /squad 内容  （队伍频道）");
            return 1;
        }
        send_party(client, name, msg);
        if (client->stat_messages < 4000000000u) ++client->stat_messages;
        return 1;
    }

    // —— Guild ——
    if (cmd_match(text, "/guild-create") || cmd_match(text, "/创建公会")) {
        std::string arg = text.rfind("/guild-create", 0) == 0
            ? cmd_args(text, "/guild-create")
            : cmd_args(text, "/创建公会");
        if (arg.empty()) {
            Chat::send_to(client, ChatChannel::kChatSystem, "",
                "用法: /guild-create 公会名");
            return 1;
        }
        Guild::create(client, arg);
        return 1;
    }
    if (cmd_match(text, "/guild-invite") || cmd_match(text, "/公会邀请")) {
        std::string arg = text.rfind("/guild-invite", 0) == 0
            ? cmd_args(text, "/guild-invite")
            : cmd_args(text, "/公会邀请");
        if (arg.empty()) {
            Chat::send_to(client, ChatChannel::kChatSystem, "",
                "用法: /guild-invite 玩家名");
            return 1;
        }
        Guild::invite(client, arg);
        return 1;
    }
    if (cmd_match(text, "/guild-leave") || cmd_match(text, "/退会")
        || cmd_match(text, "/离开公会")) {
        Guild::leave(client);
        return 1;
    }
    if (cmd_match(text, "/guild-motd") || cmd_match(text, "/guild-anno")
        || cmd_match(text, "/公告")) {
        std::string arg;
        if (text.rfind("/guild-motd", 0) == 0) arg = cmd_args(text, "/guild-motd");
        else if (text.rfind("/guild-anno", 0) == 0) arg = cmd_args(text, "/guild-anno");
        else arg = cmd_args(text, "/公告");
        if (arg.empty()) {
            Chat::send_to(client, ChatChannel::kChatSystem, "",
                "用法: /guild-motd 公告内容  （会长）");
            return 1;
        }
        Guild::set_announcement(client, arg);
        return 1;
    }
    // /guild message — must be after /guild-* prefixes
    if (cmd_match(text, "/guild") || cmd_match(text, "/公会")) {
        // Avoid matching /guild-create etc. — already handled above.
        if (text.rfind("/guild-", 0) == 0) return 0;
        std::string msg = text.rfind("/guild", 0) == 0
            ? cmd_args(text, "/guild")
            : cmd_args(text, "/公会");
        if (msg.empty()) {
            Chat::send_to(client, ChatChannel::kChatSystem, "",
                "用法: /guild 内容  （公会频道）");
            return 1;
        }
        send_guild(client, name, msg);
        if (client->stat_messages < 4000000000u) ++client->stat_messages;
        return 1;
    }

    // —— Whisper ——
    if (cmd_match(text, "/whisper") || cmd_match(text, "/w")
        || cmd_match(text, "/密语") || cmd_match(text, "/私聊")) {
        std::string arg;
        if (text.rfind("/whisper", 0) == 0) arg = cmd_args(text, "/whisper");
        else if (text.rfind("/密语", 0) == 0) arg = cmd_args(text, "/密语");
        else if (text.rfind("/私聊", 0) == 0) arg = cmd_args(text, "/私聊");
        else arg = cmd_args(text, "/w");
        std::string target, msg;
        split_first(arg, target, msg);
        send_whisper(client, name, target, msg);
        if (client->stat_messages < 4000000000u) ++client->stat_messages;
        return 1;
    }
    if (cmd_match(text, "/reply") || cmd_match(text, "/r") || cmd_match(text, "/回复")) {
        std::string msg;
        if (text.rfind("/reply", 0) == 0) msg = cmd_args(text, "/reply");
        else if (text.rfind("/回复", 0) == 0) msg = cmd_args(text, "/回复");
        else msg = cmd_args(text, "/r");
        if (client->last_whisper_from.empty()) {
            Chat::send_to(client, ChatChannel::kChatSystem, "", "没有可回复的私聊。");
            return 1;
        }
        if (msg.empty()) {
            Chat::send_to(client, ChatChannel::kChatSystem, "",
                "用法: /r 内容  （回复 " + client->last_whisper_from + "）");
            return 1;
        }
        send_whisper(client, name, client->last_whisper_from, msg);
        if (client->stat_messages < 4000000000u) ++client->stat_messages;
        return 1;
    }

    // —— Channel shortcuts (florr /local /guild /squad style) ——
    if (cmd_match(text, "/local") || cmd_match(text, "/附近")) {
        std::string msg = text.rfind("/local", 0) == 0
            ? cmd_args(text, "/local")
            : cmd_args(text, "/附近");
        if (msg.empty()) {
            Chat::send_to(client, ChatChannel::kChatSystem, "",
                "用法: /local 内容  （附近频道）");
            return 1;
        }
        send_nearby(client, name, msg);
        if (client->stat_messages < 4000000000u) ++client->stat_messages;
        return 1;
    }
    if (cmd_match(text, "/server") || cmd_match(text, "/全服")) {
        std::string msg = text.rfind("/server", 0) == 0
            ? cmd_args(text, "/server")
            : cmd_args(text, "/全服");
        if (msg.empty()) {
            Chat::send_to(client, ChatChannel::kChatSystem, "",
                "用法: /server 内容  （全服，15秒冷却）");
            return 1;
        }
        send_server(client, name, msg, now);
        if (client->stat_messages < 4000000000u) ++client->stat_messages;
        return 1;
    }

    if (!text.empty() && text.front() == '/') {
        Chat::send_to(client, ChatChannel::kChatSystem, "",
            "未知指令。输入 /help 查看列表。");
        return 1;
    }
    return 0;
}

void Chat::handle(Client *client, uint8_t channel, std::string const &target, std::string const &raw) {
    if (client == nullptr || client->game == nullptr) return;
    if (!client->game->simulation.ent_exists(client->camera)) return;
    std::string text = raw;
    trim_inplace(text);
    if (text.empty()) return;
    // Allow slightly longer slash lines so "/w Name hello…" fits; still cap hard.
    uint32_t cap = text.front() == '/' ? (MAX_CHAT_LENGTH + 32) : MAX_CHAT_LENGTH;
    text = UTF8Parser::trunc_string(text, cap);

    uint64_t now = (uint64_t) std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();

    // Slash commands first (mobile-friendly; no channel Tab needed).
    if (handle_slash(client, text, now))
        return;

    if (client->last_chat_ms != 0 && now - client->last_chat_ms < 800) return;
    client->last_chat_ms = now;

    std::string name = display_name(client);

    if (channel == ChatChannel::kChatServer) {
        send_server(client, name, text, now);
        if (client->stat_messages < 4000000000u) ++client->stat_messages;
        return;
    }

    if (channel == ChatChannel::kChatWhisper) {
        if (target.empty()) {
            send_to(client, ChatChannel::kChatSystem, "",
                "私聊对象无效。移动端可用 /w 玩家名 内容");
            return;
        }
        send_whisper(client, name, target, text);
        if (client->stat_messages < 4000000000u) ++client->stat_messages;
        return;
    }

    if (channel == ChatChannel::kChatParty) {
        send_party(client, name, text);
        if (client->stat_messages < 4000000000u) ++client->stat_messages;
        return;
    }

    if (channel == ChatChannel::kChatGuild) {
        send_guild(client, name, text);
        if (client->stat_messages < 4000000000u) ++client->stat_messages;
        return;
    }

    send_nearby(client, name, text);
    if (client->stat_messages < 4000000000u)
        ++client->stat_messages;
}
