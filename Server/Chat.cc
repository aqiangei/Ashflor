#include <Server/Chat.hh>

#include <Server/Client.hh>
#include <Server/Game.hh>
#include <Server/Guild.hh>
#include <Server/Party.hh>
#include <Server/Server.hh>

#include <Shared/Config.hh>
#include <Shared/Entity.hh>
#include <Shared/Simulation.hh>

#include <Helpers/UTF8.hh>

#include <chrono>

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

void Chat::send_to(Client *client, uint8_t channel, std::string const &sender, std::string const &text) {
    if (client == nullptr) return;
    Writer writer(Server::OUTGOING_PACKET);
    writer.write<uint8_t>(Clientbound::kChatMessage);
    writer.write<uint8_t>(channel);
    writer.write<std::string>(sender);
    writer.write<std::string>(text);
    client->send_packet(writer.packet, writer.at - writer.packet);
}

void Chat::handle(Client *client, uint8_t channel, std::string const &target, std::string const &raw) {
    if (client == nullptr || !client->alive() || client->game == nullptr) return;
    std::string text = raw;
    while (!text.empty() && (text.front() == ' ' || text.front() == '\t')) text.erase(text.begin());
    while (!text.empty() && (text.back() == ' ' || text.back() == '\t')) text.pop_back();
    if (text.empty()) return;
    text = UTF8Parser::trunc_string(text, MAX_CHAT_LENGTH);

    if (text == "/leave" || text == "/离队") {
        Party::leave(client);
        return;
    }
    if (text.rfind("/invite", 0) == 0 || text.rfind("/组队", 0) == 0) {
        std::string arg;
        size_t sp = text.find(' ');
        if (sp != std::string::npos) {
            arg = text.substr(sp + 1);
            while (!arg.empty() && (arg.front() == ' ' || arg.front() == '\t')) arg.erase(arg.begin());
        }
        Party::invite(client, arg);
        return;
    }

    uint64_t now = (uint64_t) std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    if (client->last_chat_ms != 0 && now - client->last_chat_ms < 800) return;
    client->last_chat_ms = now;

    std::string name = display_name(client);
    Simulation *sim = &client->game->simulation;
    Entity &me = sim->get_ent(sim->get_ent(client->camera).get_player());

    if (channel == ChatChannel::kChatWhisper) {
        if (target.empty()) {
            send_to(client, ChatChannel::kChatSystem, "", "私聊对象无效。");
            return;
        }
        Client *other = find_alive_by_name(client->game, target);
        if (other == nullptr) {
            send_to(client, ChatChannel::kChatSystem, "", "找不到玩家：" + target);
            return;
        }
        send_to(client, ChatChannel::kChatWhisper, name, text);
        send_to(other, ChatChannel::kChatWhisper, name, text);
        if (client->stat_messages < 4000000000u)
            ++client->stat_messages;
        return;
    }

    if (channel == ChatChannel::kChatParty) {
        if (client->party_id == 0) {
            send_to(client, ChatChannel::kChatSystem, "", "你还没有队伍。");
            return;
        }
        for (Client *c : client->game->clients) {
            if (c != nullptr && c->party_id == client->party_id)
                send_to(c, ChatChannel::kChatParty, name, text);
        }
        if (client->stat_messages < 4000000000u)
            ++client->stat_messages;
        return;
    }

    if (channel == ChatChannel::kChatGuild) {
        if (client->guild_name.empty()) {
            send_to(client, ChatChannel::kChatSystem, "", "你还没有公会。");
            return;
        }
        Guild::broadcast_chat(client, name, text);
        if (client->stat_messages < 4000000000u)
            ++client->stat_messages;
        return;
    }

    // Nearby
    for (Client *c : client->game->clients) {
        if (c == nullptr || !c->alive()) continue;
        Entity &other = sim->get_ent(sim->get_ent(c->camera).get_player());
        float dx = other.get_x() - me.get_x();
        float dy = other.get_y() - me.get_y();
        if (dx * dx + dy * dy > NEARBY_RADIUS * NEARBY_RADIUS) continue;
        send_to(c, ChatChannel::kChatNearby, name, text);
    }
    if (client->stat_messages < 4000000000u)
        ++client->stat_messages;
}
