#include <Server/Guild.hh>

#include <Server/Chat.hh>
#include <Server/Client.hh>
#include <Server/Game.hh>
#include <Server/Server.hh>

#include <Shared/Binary.hh>
#include <Shared/Config.hh>
#include <Shared/Entity.hh>
#include <Shared/Simulation.hh>
#include <Shared/StaticData.hh>

#include <Helpers/UTF8.hh>

#include <algorithm>
#include <unordered_map>
#include <vector>

struct GuildMemberRec {
    std::string name;
    std::string account;
    uint8_t leader = 0;
    uint32_t level = 1;
};

struct GuildInfo {
    std::string name;
    std::string leader_account;
    std::string leader_play;
    std::string announcement;
    std::vector<GuildMemberRec> members;
};

static std::unordered_map<std::string, GuildInfo> GUILDS;

static std::string player_name(Client *client) {
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

static uint32_t client_level(Client *client) {
    if (client == nullptr || client->game == nullptr) return 1;
    Simulation *sim = &client->game->simulation;
    if (!sim->ent_exists(client->camera)) return 1;
    Entity &camera = sim->get_ent(client->camera);
    if (sim->ent_alive(camera.get_player())) {
        uint32_t lv = score_to_level(sim->get_ent(camera.get_player()).get_score());
        return lv < 1 ? 1 : lv;
    }
    uint32_t lv = camera.get_respawn_level();
    return lv < 1 ? 1 : lv;
}

static Client *find_by_name(GameInstance *game, std::string const &name) {
    if (game == nullptr || name.empty()) return nullptr;
    for (Client *c : game->clients) {
        if (c == nullptr || !c->alive()) continue;
        if (player_name(c) == name) return c;
    }
    return nullptr;
}

// Any connected client in this game with matching guild identity (online for roster).
static Client *find_connected_member(GameInstance *game, GuildMemberRec const &rec) {
    if (game == nullptr) return nullptr;
    for (Client *c : game->clients) {
        if (c == nullptr || c->guild_name.empty()) continue;
        if (!rec.account.empty() && !c->account_name.empty()
            && c->account_name == rec.account)
            return c;
        if (player_name(c) == rec.name) return c;
    }
    return nullptr;
}

static uint8_t valid_guild_name(std::string const &name) {
    if (name.empty() || name.size() > Guild::kNameMax) return 0;
    for (unsigned char c : name) {
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') return 0;
    }
    return UTF8Parser::is_valid_utf8(name) ? 1 : 0;
}

static int roster_index(GuildInfo &info, Client *client) {
    if (client == nullptr) return -1;
    for (size_t i = 0; i < info.members.size(); ++i) {
        GuildMemberRec &m = info.members[i];
        if (!client->account_name.empty() && !m.account.empty()
            && m.account == client->account_name)
            return (int) i;
        if (m.name == player_name(client))
            return (int) i;
    }
    return -1;
}

static void roster_upsert(GuildInfo &info, Client *client, uint8_t as_leader) {
    if (client == nullptr) return;
    int idx = roster_index(info, client);
    if (idx < 0) {
        if (info.members.size() >= Guild::MAX_SIZE) return;
        GuildMemberRec rec;
        rec.name = player_name(client);
        rec.account = client->account_name;
        rec.leader = as_leader ? 1 : 0;
        rec.level = client_level(client);
        info.members.push_back(rec);
        return;
    }
    GuildMemberRec &m = info.members[(size_t) idx];
    m.name = player_name(client);
    if (!client->account_name.empty()) m.account = client->account_name;
    m.level = client_level(client);
    if (as_leader) m.leader = 1;
}

static void roster_remove(GuildInfo &info, Client *client) {
    int idx = roster_index(info, client);
    if (idx < 0) return;
    info.members.erase(info.members.begin() + idx);
}

static void roster_set_leader(GuildInfo &info, Client *leader) {
    if (leader == nullptr) return;
    for (GuildMemberRec &m : info.members) m.leader = 0;
    int idx = roster_index(info, leader);
    if (idx >= 0) {
        info.members[(size_t) idx].leader = 1;
        info.members[(size_t) idx].name = player_name(leader);
        info.leader_account = leader->account_name;
        info.leader_play = player_name(leader);
    }
}

void Guild::notify(Client *client, std::string const &line) {
    Chat::send_to(client, ChatChannel::kChatSystem, "", line);
}

void Guild::sync_nametag(Client *client) {
    if (client == nullptr || client->game == nullptr) return;
    Simulation *sim = &client->game->simulation;
    if (!sim->ent_exists(client->camera)) return;
    Entity &camera = sim->get_ent(client->camera);
    if (!sim->ent_alive(camera.get_player())) return;
    Entity &player = sim->get_ent(camera.get_player());
    if (!player.has_component(kName)) return;
    player.set_guild(client->guild_name);
}

void Guild::send_update(Client *client) {
    if (client == nullptr || client->game == nullptr) return;
    Writer writer(Server::OUTGOING_PACKET);
    writer.write<uint8_t>(Clientbound::kGuildUpdate);
    writer.write<std::string>(client->guild_name);

    std::string anno;
    std::vector<GuildMemberRec> roster;
    if (!client->guild_name.empty()) {
        auto it = GUILDS.find(client->guild_name);
        if (it != GUILDS.end()) {
            anno = it->second.announcement;
            // Refresh live fields for connected members
            for (GuildMemberRec &m : it->second.members) {
                Client *live = find_connected_member(client->game, m);
                if (live != nullptr) {
                    m.name = player_name(live);
                    m.level = client_level(live);
                    if (live->guild_leader) m.leader = 1;
                }
            }
            roster = it->second.members;
        }
    }
    // Leader first, then online, then offline; stable by name within group
    std::stable_sort(roster.begin(), roster.end(),
        [&](GuildMemberRec const &a, GuildMemberRec const &b) {
            if (a.leader != b.leader) return a.leader > b.leader;
            Client *ca = find_connected_member(client->game, a);
            Client *cb = find_connected_member(client->game, b);
            uint8_t oa = ca != nullptr;
            uint8_t ob = cb != nullptr;
            if (oa != ob) return oa > ob;
            return a.name < b.name;
        });

    writer.write<std::string>(anno);
    writer.write<uint8_t>((uint8_t) roster.size());
    for (GuildMemberRec const &m : roster) {
        writer.write<std::string>(m.name);
        uint8_t flags = 0;
        if (m.leader) flags |= Guild::kFlagLeader;
        Client *live = find_connected_member(client->game, m);
        if (live != nullptr) flags |= Guild::kFlagOnline;
        if (live == client) flags |= Guild::kFlagSelf;
        else if (live == nullptr && !client->account_name.empty()
            && !m.account.empty() && m.account == client->account_name)
            flags |= Guild::kFlagSelf;
        else if (live == nullptr && m.name == player_name(client))
            flags |= Guild::kFlagSelf;
        writer.write<uint8_t>(flags);
        writer.write<uint32_t>(m.level);
    }
    client->send_packet(writer.packet, writer.at - writer.packet);
}

static void broadcast_guild(Client *any) {
    if (any == nullptr || any->game == nullptr || any->guild_name.empty()) return;
    for (Client *c : any->game->clients) {
        if (c != nullptr && c->guild_name == any->guild_name)
            Guild::send_update(c);
    }
}

void Guild::leave(Client *client) {
    if (client == nullptr) return;
    std::string gname = client->guild_name;
    if (gname.empty()) {
        send_update(client);
        return;
    }
    uint8_t was_leader = client->guild_leader;
    auto git = GUILDS.find(gname);
    if (git != GUILDS.end())
        roster_remove(git->second, client);

    client->guild_name.clear();
    client->guild_leader = 0;
    sync_nametag(client);
    client->pending_guild_from.clear();
    client->pending_guild_ttl = 0;
    send_update(client);
    notify(client, "[公会] 你已离开公会。");

    GameInstance *game = client->game;
    if (game == nullptr) return;
    std::vector<Client *> rest;
    for (Client *c : game->clients) {
        if (c != nullptr && c->guild_name == gname)
            rest.push_back(c);
    }
    git = GUILDS.find(gname);
    if (rest.empty() && (git == GUILDS.end() || git->second.members.empty())) {
        GUILDS.erase(gname);
        return;
    }
    if (was_leader) {
        Client *next = rest.empty() ? nullptr : rest[0];
        if (next != nullptr) {
            next->guild_leader = 1;
            if (git != GUILDS.end())
                roster_set_leader(git->second, next);
            for (Client *c : rest)
                notify(c, "[公会] " + player_name(next) + " 成为会长。");
        } else if (git != GUILDS.end() && !git->second.members.empty()) {
            // No online heir — promote first roster entry until they reconnect
            for (GuildMemberRec &m : git->second.members) m.leader = 0;
            git->second.members[0].leader = 1;
            git->second.leader_account = git->second.members[0].account;
            git->second.leader_play = git->second.members[0].name;
        }
    }
    for (Client *c : rest)
        send_update(c);
}

uint8_t Guild::create(Client *client, std::string const &raw_name) {
    if (client == nullptr || !client->alive()) return 0;
    if (!client->guild_name.empty()) {
        notify(client, "[公会] 你已在公会中，请先离开。");
        return 0;
    }
    std::string name = UTF8Parser::trunc_string(raw_name, kNameMax);
    while (!name.empty() && (name.front() == ' ' || name.back() == ' ')) {
        if (name.front() == ' ') name.erase(name.begin());
        else name.pop_back();
    }
    if (!valid_guild_name(name)) {
        notify(client, "[公会] 名称无效（1–12 字，无空格）。");
        return 0;
    }
    if (GUILDS.contains(name)) {
        notify(client, "[公会] 该名称已被占用。");
        return 0;
    }
    GuildInfo info;
    info.name = name;
    info.leader_account = client->account_name;
    info.leader_play = player_name(client);
    roster_upsert(info, client, 1);
    GUILDS[name] = info;
    client->guild_name = name;
    client->guild_leader = 1;
    sync_nametag(client);
    notify(client, "[公会] 已创建公会「" + name + "」。");
    send_update(client);
    return 1;
}

uint8_t Guild::invite(Client *from, std::string const &target_name) {
    if (from == nullptr || !from->alive() || from->game == nullptr) return 0;
    if (from->guild_name.empty()) {
        notify(from, "[公会] 你还不在公会中，请先创建。");
        return 0;
    }
    Client *target = find_by_name(from->game, target_name);
    if (target == nullptr) {
        notify(from, "[公会] 找不到玩家。");
        return 0;
    }
    if (target == from) {
        notify(from, "[公会] 不能邀请自己。");
        return 0;
    }
    if (!target->guild_name.empty()) {
        notify(from, "[公会] 对方已在公会中。");
        return 0;
    }
    auto it = GUILDS.find(from->guild_name);
    uint32_t count = it != GUILDS.end() ? (uint32_t) it->second.members.size() : 0;
    if (count == 0) {
        for (Client *c : from->game->clients)
            if (c != nullptr && c->guild_name == from->guild_name) ++count;
    }
    if (count >= MAX_SIZE) {
        notify(from, "[公会] 公会已满。");
        return 0;
    }
    target->pending_guild_from = player_name(from);
    target->pending_guild_ttl = INVITE_TTL_TICKS;
    Writer writer(Server::OUTGOING_PACKET);
    writer.write<uint8_t>(Clientbound::kInvitePrompt);
    writer.write<uint8_t>(InviteType::kInviteGuild);
    writer.write<std::string>(target->pending_guild_from);
    target->send_packet(writer.packet, writer.at - writer.packet);
    notify(from, "[公会] 已向 " + player_name(target) + " 发出邀请。");
    notify(target, "[公会] " + player_name(from) + " 邀请你加入「" + from->guild_name + "」。");
    return 1;
}

uint8_t Guild::respond(Client *target, std::string const &from_name, uint8_t accept) {
    if (target == nullptr || !target->alive() || target->game == nullptr) return 0;
    if (target->pending_guild_from.empty() || target->pending_guild_from != from_name) {
        notify(target, "[公会] 邀请已失效。");
        return 0;
    }
    target->pending_guild_from.clear();
    target->pending_guild_ttl = 0;
    if (!accept) {
        notify(target, "[公会] 已拒绝邀请。");
        Client *from = find_by_name(target->game, from_name);
        if (from != nullptr)
            notify(from, "[公会] " + player_name(target) + " 拒绝了公会邀请。");
        send_update(target);
        return 1;
    }
    if (!target->guild_name.empty()) {
        notify(target, "[公会] 你已在公会中。");
        return 0;
    }
    Client *from = find_by_name(target->game, from_name);
    if (from == nullptr || from->guild_name.empty()) {
        notify(target, "[公会] 邀请方已离线或不在公会。");
        return 0;
    }
    auto it = GUILDS.find(from->guild_name);
    if (it == GUILDS.end()) {
        notify(target, "[公会] 公会不存在。");
        return 0;
    }
    if (it->second.members.size() >= MAX_SIZE) {
        notify(target, "[公会] 公会已满。");
        return 0;
    }
    target->guild_name = from->guild_name;
    target->guild_leader = 0;
    roster_upsert(it->second, target, 0);
    sync_nametag(target);
    std::string msg = "[公会] " + player_name(target) + " 加入了公会。";
    for (Client *c : from->game->clients) {
        if (c != nullptr && c->guild_name == from->guild_name) {
            notify(c, msg);
            send_update(c);
        }
    }
    return 1;
}

uint8_t Guild::set_announcement(Client *client, std::string const &raw) {
    if (client == nullptr || client->guild_name.empty()) return 0;
    if (!client->guild_leader) {
        notify(client, "[公会] 只有会长可以修改公告。");
        return 0;
    }
    auto it = GUILDS.find(client->guild_name);
    if (it == GUILDS.end()) return 0;
    std::string text = UTF8Parser::trunc_string(raw, kAnnoMax);
    while (!text.empty() && (text.back() == '\n' || text.back() == '\r'))
        text.pop_back();
    if (!text.empty() && !UTF8Parser::is_valid_utf8(text)) {
        notify(client, "[公会] 公告包含无效字符。");
        return 0;
    }
    it->second.announcement = text;
    notify(client, "[公会] 公告已更新。");
    broadcast_guild(client);
    return 1;
}

uint8_t Guild::same_guild(Client *a, Client *b) {
    return a != nullptr && b != nullptr
        && !a->guild_name.empty() && a->guild_name == b->guild_name;
}

void Guild::on_login(Client *client) {
    if (client == nullptr) return;
    if (!client->guild_name.empty()) {
        auto it = GUILDS.find(client->guild_name);
        if (it == GUILDS.end()) {
            GuildInfo info;
            info.name = client->guild_name;
            info.leader_account = client->account_name;
            info.leader_play = player_name(client);
            roster_upsert(info, client, 1);
            GUILDS[client->guild_name] = info;
            client->guild_leader = 1;
        } else {
            // Restore leader flag from roster
            int idx = roster_index(it->second, client);
            if (idx >= 0 && it->second.members[(size_t) idx].leader)
                client->guild_leader = 1;
            roster_upsert(it->second, client, client->guild_leader);
        }
    }
    send_update(client);
    if (!client->guild_name.empty())
        broadcast_guild(client);
}

void Guild::tick(GameInstance *game) {
    if (game == nullptr) return;
    for (Client *c : game->clients) {
        if (c == nullptr) continue;
        if (c->pending_guild_ttl > 0) {
            --c->pending_guild_ttl;
            if (c->pending_guild_ttl == 0 && !c->pending_guild_from.empty()) {
                notify(c, "[公会] 邀请已过期。");
                c->pending_guild_from.clear();
            }
        }
    }
}

void Guild::broadcast_chat(Client *from, std::string const &sender, std::string const &text) {
    if (from == nullptr || from->game == nullptr || from->guild_name.empty()) return;
    for (Client *c : from->game->clients) {
        if (c != nullptr && c->guild_name == from->guild_name)
            Chat::send_to(c, ChatChannel::kChatGuild, sender, text);
    }
}
