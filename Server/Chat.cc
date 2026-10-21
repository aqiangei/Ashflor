#include <Server/Chat.hh>

#include <Server/Client.hh>
#include <Server/Game.hh>
#include <Server/Guild.hh>
#include <Server/Party.hh>
#include <Server/Server.hh>

#include <Shared/Binary.hh>
#include <Shared/Config.hh>
#include <Shared/Entity.hh>
#include <Shared/Simulation.hh>
#include <Shared/StaticDefinitions.hh>

#include <Helpers/UTF8.hh>

#include <chrono>
#include <cstring>
#include <deque>
#include <string>
#include <vector>

// 全服广播单独冷却，避免刷屏淹没附近/公会频道
static constexpr uint64_t SERVER_CHAT_COOLDOWN_MS = 15000;

// Single-threaded WASM server tick — no lock needed.
static uint32_t g_live_bc_next_id = 1;
static std::deque<Chat::LiveBroadcast> g_live_bc;

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

static Client *find_alive_by_name(GameInstance *game, std::string const &name) {
    if (game == nullptr || name.empty()) return nullptr;
    for (Client *c : game->clients) {
        if (c == nullptr || !c->alive()) continue;
        if (display_name(c) == name) return c;
    }
    return nullptr;
}

// Whisper / guild-invite targets: search every map room (florr-style).
static Client *find_online_by_name(std::string const &name) {
    if (name.empty() || Server::games == nullptr) return nullptr;
    for (uint8_t mi = 0; mi < MapID::kNumMaps; ++mi) {
        Client *c = find_alive_by_name(&Server::games[mi], name);
        if (c != nullptr) return c;
    }
    return nullptr;
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

void Chat::broadcast_server(std::string const &sender, std::string const &text) {
    {
        Chat::LiveBroadcast entry;
        entry.id = g_live_bc_next_id++;
        if (g_live_bc_next_id == 0) g_live_bc_next_id = 1;
        entry.sender = sender;
        entry.text = text;
        g_live_bc.push_front(std::move(entry));
        while (g_live_bc.size() > Chat::LIVE_BROADCAST_CAP)
            g_live_bc.pop_back();
    }
    if (Server::games == nullptr) return;
    for (uint8_t mi = 0; mi < MapID::kNumMaps; ++mi) {
        for (Client *c : Server::games[mi].clients) {
            if (c != nullptr)
                Chat::send_to(c, ChatChannel::kChatServer, sender, text);
        }
    }
}

void Chat::recent_broadcasts(std::vector<Chat::LiveBroadcast> &out) {
    out.clear();
    out.assign(g_live_bc.begin(), g_live_bc.end());
}

static void send_nearby(Client *client, std::string const &name, std::string const &text) {
    Simulation *sim = &client->game->simulation;
    Entity &me = sim->get_ent(sim->get_ent(client->camera).get_player());
    for (Client *c : client->game->clients) {
        if (c == nullptr || !c->alive()) continue;
        Entity &other = sim->get_ent(sim->get_ent(c->camera).get_player());
        float dx = other.get_x() - me.get_x();
        float dy = other.get_y() - me.get_y();
        if (dx * dx + dy * dy > Chat::NEARBY_RADIUS * Chat::NEARBY_RADIUS) continue;
        Chat::send_to(c, ChatChannel::kChatNearby, name, text);
    }
}

static void send_party(Client *client, std::string const &name, std::string const &text) {
    if (client->party_id == 0) {
        Chat::send_to(client, ChatChannel::kChatSystem, "", "你还没有队伍。");
        return;
    }
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
        "/invite|/组队 名 — 邀请入队",
        "/leave|/离队 — 离开队伍",
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
    if (client == nullptr || !client->alive() || client->game == nullptr) return;
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
