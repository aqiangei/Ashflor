#include <Server/Party.hh>

#include <Server/Chat.hh>
#include <Server/Client.hh>
#include <Server/Game.hh>
#include <Server/Guild.hh>
#include <Server/Server.hh>
#include <Server/Spawn.hh>

#include <Shared/Binary.hh>
#include <Shared/Config.hh>
#include <Shared/Entity.hh>
#include <Shared/Simulation.hh>
#include <Shared/StaticData.hh>

#include <Helpers/Math.hh>
#include <Helpers/UTF8.hh>

#include <cmath>
#include <vector>

static uint32_t NEXT_PARTY_ID = 1;

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

static void set_entity_team(Simulation *sim, Entity &player, EntityID team) {
    player.set_team(team);
    for (uint32_t i = 0; i < player.get_loadout_count(); ++i) {
        LoadoutSlot &slot = player.loadout[i];
        for (uint32_t j = 0; j < MAX_PETALS_IN_CLUMP; ++j) {
            EntityID id = slot.petals[j].ent_id;
            if (!sim->ent_alive(id)) continue;
            sim->get_ent(id).set_team(team);
        }
    }
}

void Party::apply_team(Client *client) {
    if (client == nullptr || client->game == nullptr || client->party_id == 0) return;
    Simulation *sim = &client->game->simulation;
    if (!sim->ent_exists(client->camera)) return;
    EntityID team = NULL_ENTITY;
    for (Client *c : client->game->clients) {
        if (c != nullptr && c->party_id == client->party_id && c->party_leader) {
            team = c->camera;
            break;
        }
    }
    if (team == NULL_ENTITY) team = client->camera;
    Entity &camera = sim->get_ent(client->camera);
    camera.set_team(team);
    if (sim->ent_alive(camera.get_player()))
        set_entity_team(sim, sim->get_ent(camera.get_player()), team);
}

void Party::notify(Client *client, std::string const &line) {
    Chat::send_to(client, ChatChannel::kChatSystem, "", line);
}

void Party::send_update(Client *client) {
    if (client == nullptr || client->game == nullptr) return;
    Simulation *sim = &client->game->simulation;
    Writer writer(Server::OUTGOING_PACKET);
    writer.write<uint8_t>(Clientbound::kPartyUpdate);
    std::vector<Client *> members;
    if (client->party_id != 0) {
        for (Client *c : client->game->clients) {
            if (c != nullptr && c->party_id == client->party_id)
                members.push_back(c);
        }
    }
    writer.write<uint8_t>((uint8_t) members.size());
    for (Client *c : members) {
        writer.write<std::string>(player_name(c));
        uint8_t flags = 0;
        if (c->party_leader) flags |= 1;
        if (c->awaiting_revive) flags |= 2;
        if (c == client) flags |= 4;
        writer.write<uint8_t>(flags);
        uint8_t hp = 0;
        float x = -1.f;
        float y = -1.f;
        if (c->awaiting_revive) {
            x = c->corpse_x;
            y = c->corpse_y;
        } else if (sim->ent_exists(c->camera)) {
            Entity &camera = sim->get_ent(c->camera);
            if (sim->ent_alive(camera.get_player())) {
                Entity &player = sim->get_ent(camera.get_player());
                if (player.max_health > 0.f)
                    hp = (uint8_t) fclamp(player.health / player.max_health * 100.f, 0.f, 100.f);
                x = player.get_x();
                y = player.get_y();
            }
        }
        writer.write<uint8_t>(hp);
        writer.write<float>(x);
        writer.write<float>(y);
    }
    writer.write<uint8_t>(client->revive_progress > REVIVE_TICKS ? 100 :
        (uint8_t) (client->revive_progress * 100 / REVIVE_TICKS));
    client->send_packet(writer.packet, writer.at - writer.packet);
}

static void broadcast_party(Client *any) {
    if (any == nullptr || any->game == nullptr || any->party_id == 0) return;
    for (Client *c : any->game->clients) {
        if (c != nullptr && c->party_id == any->party_id)
            Party::send_update(c);
    }
}

void Party::clear_corpse(Client *client) {
    if (client == nullptr) return;
    if (client->game != nullptr && !(client->corpse_id == NULL_ENTITY)) {
        Simulation *sim = &client->game->simulation;
        if (sim->ent_exists(client->corpse_id))
            sim->request_delete(client->corpse_id);
    }
    client->corpse_id = NULL_ENTITY;
    client->awaiting_revive = 0;
    client->corpse_x = 0;
    client->corpse_y = 0;
    client->corpse_ttl = 0;
    client->revive_progress = 0;
}

void Party::leave(Client *client) {
    if (client == nullptr) return;
    uint32_t pid = client->party_id;
    if (pid == 0) {
        clear_corpse(client);
        send_update(client);
        return;
    }
    GameInstance *game = client->game;
    uint8_t was_leader = client->party_leader;
    clear_corpse(client);
    client->party_id = 0;
    client->party_leader = 0;
    if (game != nullptr && game->simulation.ent_exists(client->camera)) {
        Entity &camera = game->simulation.get_ent(client->camera);
        camera.set_team(client->camera);
        if (game->simulation.ent_alive(camera.get_player()))
            set_entity_team(&game->simulation, game->simulation.get_ent(camera.get_player()), client->camera);
    }
    send_update(client);
    notify(client, "[队伍] 你已离队。");

    if (game == nullptr) return;
    std::vector<Client *> rest;
    for (Client *c : game->clients) {
        if (c != nullptr && c->party_id == pid)
            rest.push_back(c);
    }
    if (rest.empty()) return;
    if (was_leader) {
        rest[0]->party_leader = 1;
        apply_team(rest[0]);
        for (Client *c : rest) {
            apply_team(c);
            notify(c, "[队伍] " + player_name(rest[0]) + " 成为队长。");
        }
    }
    for (Client *c : rest)
        send_update(c);
    if (rest.size() == 1) {
        notify(rest[0], "[队伍] 队伍已解散。");
        leave(rest[0]);
    }
}

uint8_t Party::same_party(Client *a, Client *b) {
    return a != nullptr && b != nullptr && a->party_id != 0 && a->party_id == b->party_id;
}

static Client *find_by_name(GameInstance *game, std::string const &name) {
    if (game == nullptr || name.empty()) return nullptr;
    for (Client *c : game->clients) {
        if (c == nullptr || !c->alive()) continue;
        if (player_name(c) == name) return c;
    }
    return nullptr;
}

static Client *find_nearest(Client *from) {
    if (from == nullptr || !from->alive() || from->game == nullptr) return nullptr;
    Simulation *sim = &from->game->simulation;
    Entity &me = sim->get_ent(sim->get_ent(from->camera).get_player());
    Client *best = nullptr;
    float best_d = Party::INVITE_RADIUS * Party::INVITE_RADIUS;
    for (Client *c : from->game->clients) {
        if (c == nullptr || c == from || !c->alive()) continue;
        Entity &other = sim->get_ent(sim->get_ent(c->camera).get_player());
        float dx = other.get_x() - me.get_x();
        float dy = other.get_y() - me.get_y();
        float d = dx * dx + dy * dy;
        if (d < best_d) {
            best_d = d;
            best = c;
        }
    }
    return best;
}

uint8_t Party::invite(Client *from, std::string const &target_name) {
    if (from == nullptr || !from->alive() || from->game == nullptr) return 0;
    Client *target = target_name.empty() ? find_nearest(from) : find_by_name(from->game, target_name);
    if (target == nullptr) {
        notify(from, "[队伍] 附近没有可邀请的玩家。");
        return 0;
    }
    if (target == from) {
        notify(from, "[队伍] 不能邀请自己。");
        return 0;
    }
    if (target->party_id != 0) {
        notify(from, "[队伍] " + player_name(target) + " 已在其他队伍中。");
        return 0;
    }
    uint32_t count = 0;
    if (from->party_id != 0) {
        for (Client *c : from->game->clients)
            if (c != nullptr && c->party_id == from->party_id) ++count;
    } else {
        count = 1;
    }
    if (count >= MAX_SIZE) {
        notify(from, "[队伍] 队伍已满。");
        return 0;
    }
    // Ensure inviter has a party shell so accept can join
    if (from->party_id == 0) {
        from->party_id = NEXT_PARTY_ID++;
        from->party_leader = 1;
        apply_team(from);
        send_update(from);
    }
    target->pending_party_from = player_name(from);
    target->pending_party_ttl = INVITE_TTL_TICKS;
    Writer writer(Server::OUTGOING_PACKET);
    writer.write<uint8_t>(Clientbound::kInvitePrompt);
    writer.write<uint8_t>(InviteType::kInviteParty);
    writer.write<std::string>(target->pending_party_from);
    target->send_packet(writer.packet, writer.at - writer.packet);
    notify(from, "[队伍] 已向 " + player_name(target) + " 发出组队邀请。");
    notify(target, "[队伍] " + player_name(from) + " 邀请你组队。");
    return 1;
}

uint8_t Party::respond(Client *target, std::string const &from_name, uint8_t accept) {
    if (target == nullptr || !target->alive() || target->game == nullptr) return 0;
    if (target->pending_party_from.empty() || target->pending_party_from != from_name) {
        notify(target, "[队伍] 邀请已失效。");
        return 0;
    }
    target->pending_party_from.clear();
    target->pending_party_ttl = 0;
    if (!accept) {
        notify(target, "[队伍] 已拒绝组队邀请。");
        Client *from = find_by_name(target->game, from_name);
        if (from != nullptr)
            notify(from, "[队伍] " + player_name(target) + " 拒绝了组队邀请。");
        return 1;
    }
    if (target->party_id != 0) {
        notify(target, "[队伍] 你已在队伍中。");
        return 0;
    }
    Client *from = find_by_name(target->game, from_name);
    if (from == nullptr || from->party_id == 0) {
        notify(target, "[队伍] 邀请方已离队或不在线。");
        return 0;
    }
    uint32_t count = 0;
    for (Client *c : from->game->clients)
        if (c != nullptr && c->party_id == from->party_id) ++count;
    if (count >= MAX_SIZE) {
        notify(target, "[队伍] 队伍已满。");
        return 0;
    }
    target->party_id = from->party_id;
    target->party_leader = 0;
    apply_team(target);
    std::string msg = "[队伍] " + player_name(target) + " 加入了队伍。";
    for (Client *c : from->game->clients) {
        if (c != nullptr && c->party_id == from->party_id) {
            notify(c, msg);
            send_update(c);
        }
    }
    return 1;
}

void Party::on_death(Client *client, float x, float y) {
    if (client == nullptr || client->game == nullptr) return;
    clear_corpse(client);
    if (client->party_id == 0) {
        send_update(client);
        return;
    }
    client->awaiting_revive = 1;
    client->corpse_x = x;
    client->corpse_y = y;
    client->corpse_ttl = CORPSE_TTL;
    client->revive_progress = 0;
    Simulation *sim = &client->game->simulation;
    Entity &camera = sim->get_ent(client->camera);
    camera.set_camera_x(x);
    camera.set_camera_y(y);
    Entity &corpse = alloc_corpse(sim, camera.get_team(), x, y, player_name(client));
    client->corpse_id = corpse.id;
    std::string msg = "[队伍] " + player_name(client) + " 倒下了，站在尸体旁 3 秒可复活。";
    for (Client *c : client->game->clients) {
        if (c != nullptr && c->party_id == client->party_id) {
            notify(c, msg);
            send_update(c);
        }
    }
}

static void do_revive(Client *dead) {
    if (dead == nullptr || dead->game == nullptr || !dead->awaiting_revive) return;
    Simulation *sim = &dead->game->simulation;
    if (!sim->ent_exists(dead->camera)) return;
    Entity &camera = sim->get_ent(dead->camera);
    float x = dead->corpse_x;
    float y = dead->corpse_y;
    Party::clear_corpse(dead);
    // Same stale-player clear as kClientSpawn — corpse revive must not keep a deleting flower
    if (sim->ent_exists(camera.get_player()) && !sim->ent_alive(camera.get_player())) {
        sim->request_delete(camera.get_player());
        camera.set_player(NULL_ENTITY);
    }
    Entity &player = alloc_player(sim, camera.get_team());
    player_spawn_at(sim, camera, player, x, y);
    Guild::sync_nametag(dead);
    Party::apply_team(dead);
    std::string msg = "[队伍] " + player_name(dead) + " 已被复活！";
    for (Client *c : dead->game->clients) {
        if (c != nullptr && c->party_id == dead->party_id) {
            Party::notify(c, msg);
            Party::send_update(c);
        }
    }
}

void Party::tick(GameInstance *game) {
    if (game == nullptr) return;
    Simulation *sim = &game->simulation;
    for (Client *c : game->clients) {
        if (c == nullptr) continue;
        if (c->pending_party_ttl > 0) {
            --c->pending_party_ttl;
            if (c->pending_party_ttl == 0 && !c->pending_party_from.empty()) {
                notify(c, "[队伍] 组队邀请已过期。");
                c->pending_party_from.clear();
            }
        }
    }
    // Refresh teammate HP / positions for HUD + minimap (~4 Hz)
    static uint8_t party_hud_cd = 0;
    if (++party_hud_cd >= 5) {
        party_hud_cd = 0;
        for (Client *c : game->clients) {
            if (c != nullptr && c->party_id != 0)
                send_update(c);
        }
    }
    for (Client *dead : game->clients) {
        if (dead == nullptr || !dead->awaiting_revive) continue;
        if (dead->corpse_ttl > 0) --dead->corpse_ttl;
        if (dead->corpse_ttl == 0) {
            notify(dead, "[队伍] 复活时间已过。");
            clear_corpse(dead);
            send_update(dead);
            broadcast_party(dead);
            continue;
        }
        if (!sim->ent_exists(dead->camera)) continue;
        uint32_t best = 0;
        for (Client *reviver : game->clients) {
            if (reviver == nullptr || reviver == dead) continue;
            if (!same_party(dead, reviver) || !reviver->alive()) continue;
            Entity &player = sim->get_ent(sim->get_ent(reviver->camera).get_player());
            float dx = player.get_x() - dead->corpse_x;
            float dy = player.get_y() - dead->corpse_y;
            if (dx * dx + dy * dy > REVIVE_RADIUS * REVIVE_RADIUS) continue;
            best = dead->revive_progress + 1;
            break;
        }
        if (best == 0) {
            if (dead->revive_progress > 0) {
                dead->revive_progress = dead->revive_progress > 2 ? dead->revive_progress - 2 : 0;
                send_update(dead);
            }
            continue;
        }
        dead->revive_progress = best;
        if (dead->revive_progress % 5 == 0 || dead->revive_progress >= REVIVE_TICKS)
            send_update(dead);
        if (dead->revive_progress >= REVIVE_TICKS)
            do_revive(dead);
    }
}
