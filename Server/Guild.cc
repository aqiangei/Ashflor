#include <Server/Guild.hh>

#include <Server/Chat.hh>
#include <Server/Client.hh>
#include <Server/Game.hh>
#include <Server/Server.hh>

#include <Shared/Binary.hh>
#include <Shared/Config.hh>
#include <Shared/Entity.hh>
#include <Shared/Simulation.hh>

#include <Helpers/UTF8.hh>

#include <unordered_map>
#include <vector>

struct GuildInfo {
    std::string name;
    std::string leader_account; // account_name of leader (empty if guest leader)
    std::string leader_play;    // play name at create time
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

static Client *find_by_name(GameInstance *game, std::string const &name) {
    if (game == nullptr || name.empty()) return nullptr;
    for (Client *c : game->clients) {
        if (c == nullptr || !c->alive()) continue;
        if (player_name(c) == name) return c;
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
    std::vector<Client *> members;
    if (!client->guild_name.empty()) {
        for (Client *c : client->game->clients) {
            if (c != nullptr && c->guild_name == client->guild_name)
                members.push_back(c);
        }
    }
    writer.write<uint8_t>((uint8_t) members.size());
    for (Client *c : members) {
        writer.write<std::string>(player_name(c));
        uint8_t flags = 0;
        if (c->guild_leader) flags |= 1;
        if (c == client) flags |= 4;
        writer.write<uint8_t>(flags);
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
    if (rest.empty()) {
        GUILDS.erase(gname);
        return;
    }
    if (was_leader) {
        rest[0]->guild_leader = 1;
        auto it = GUILDS.find(gname);
        if (it != GUILDS.end()) {
            it->second.leader_account = rest[0]->account_name;
            it->second.leader_play = player_name(rest[0]);
        }
        for (Client *c : rest)
            notify(c, "[公会] " + player_name(rest[0]) + " 成为会长。");
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
    uint32_t count = 0;
    for (Client *c : from->game->clients)
        if (c != nullptr && c->guild_name == from->guild_name) ++count;
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
    uint32_t count = 0;
    for (Client *c : from->game->clients)
        if (c != nullptr && c->guild_name == from->guild_name) ++count;
    if (count >= MAX_SIZE) {
        notify(target, "[公会] 公会已满。");
        return 0;
    }
    target->guild_name = from->guild_name;
    target->guild_leader = 0;
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

uint8_t Guild::same_guild(Client *a, Client *b) {
    return a != nullptr && b != nullptr
        && !a->guild_name.empty() && a->guild_name == b->guild_name;
}

void Guild::on_login(Client *client) {
    if (client == nullptr) return;
    // In-memory only for now; guild_name may be restored from account later
    if (!client->guild_name.empty() && !GUILDS.contains(client->guild_name)) {
        GuildInfo info;
        info.name = client->guild_name;
        info.leader_account = client->account_name;
        info.leader_play = player_name(client);
        GUILDS[client->guild_name] = info;
        client->guild_leader = 1;
    }
    send_update(client);
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
