#include <Server/Client.hh>

#include <Server/AccountStore.hh>
#include <Server/Chat.hh>
#include <Server/Game.hh>
#include <Server/LeaderboardStore.hh>
#include <Server/Guild.hh>
#include <Server/Party.hh>
#include <Server/PetalTracker.hh>
#include <Server/Server.hh>
#include <Server/Spawn.hh>

#include <Helpers/UTF8.hh>

#include <Shared/Binary.hh>
#include <Shared/Config.hh>
#include <Shared/Craft.hh>
#include <Shared/Recipe.hh>
#include <Shared/StaticData.hh>
#include <Shared/Talent.hh>

#include <Helpers/Math.hh>

#include <algorithm>
#include <array>
#include <chrono>
#include <iostream>

constexpr std::array<uint32_t, RarityID::kNumRarities> RARITY_TO_XP = { 2, 10, 50, 200, 1000, 5000, 0 };

static void send_boost_update(Client *client) {
    if (client == nullptr) return;
    Writer writer(Server::OUTGOING_PACKET);
    writer.write<uint8_t>(Clientbound::kBoostUpdate);
    writer.write<uint32_t>(client->daily_bonus_seconds);
    writer.write<uint32_t>(client->offline_bonus_seconds);
    writer.write<uint8_t>(client->bonus_enabled ? 1 : 0);
    client->send_packet(writer.packet, writer.at - writer.packet);
}

void client_tick_bonus(Client *client) {
    if (client == nullptr || client->account_name.empty()) return;
    uint64_t now = (uint64_t)std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    uint32_t day = (uint32_t)((now + 8 * 3600) / 86400);
    bool changed = false;
    if (client->bonus_day != day) {
        client->bonus_day = day;
        client->daily_bonus_seconds = 3600;
        changed = true;
    }
    if (client->last_bonus_tick == 0) client->last_bonus_tick = now;
    uint64_t elapsed = now > client->last_bonus_tick ? now - client->last_bonus_tick : 0;
    client->last_bonus_tick = now;
    // Only burn bonus seconds while the player has the toggle ON.
    if (client->bonus_enabled && client->alive() && elapsed > 0) {
        uint32_t spend = (uint32_t)std::min<uint64_t>(elapsed, 60);
        uint32_t daily = std::min(spend, client->daily_bonus_seconds);
        client->daily_bonus_seconds -= daily;
        spend -= daily;
        uint32_t offline = std::min(spend, client->offline_bonus_seconds);
        client->offline_bonus_seconds -= offline;
        changed = daily + offline > 0;
    }
    if (changed && (client->last_bonus_sync == 0 || now - client->last_bonus_sync >= 5)) {
        send_boost_update(client);
        client->last_bonus_sync = now;
    }
}

static uint8_t try_give_petal(Client *client, PetalID::T id);

static void send_craft_update(Client *client, uint8_t result, PetalID::T petal) {
    Writer writer(Server::OUTGOING_PACKET);
    writer.write<uint8_t>(Clientbound::kCraftUpdate);
    writer.write<uint8_t>(result);
    writer.write<uint8_t>(petal);
    for (uint32_t i = 0; i < PetalID::kNumPetals; ++i)
        writer.write<uint32_t>(client->craft_bank[i]);
    writer.write<uint32_t>(client->coins);
    client->send_packet(writer.packet, writer.at - writer.packet);
}

static void send_talent_update(Client *client) {
    Writer writer(Server::OUTGOING_PACKET);
    writer.write<uint8_t>(Clientbound::kTalentUpdate);
    writer.write<uint8_t>(client->talent_hp);
    writer.write<uint8_t>(client->talent_rot);
    client->send_packet(writer.packet, writer.at - writer.packet);
}

static void send_loadout_preset_update(Client *client, uint8_t result, uint8_t index = 0) {
    Writer writer(Server::OUTGOING_PACKET);
    writer.write<uint8_t>(Clientbound::kLoadoutPresetSync);
    writer.write<uint8_t>(result);
    writer.write<uint8_t>(index);
    for (LoadoutPreset const &preset : client->loadout_presets) {
        writer.write<uint8_t>(preset.slot_count);
        for (PetalID::T id : preset.petals)
            writer.write<uint8_t>(id);
    }
    client->send_packet(writer.packet, writer.at - writer.packet);
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
    uint8_t lv = camera.get_respawn_level();
    return lv < 1 ? 1 : lv;
}

// Level shown on the leaderboard board: the account level for signed-in
// players, the stored progress level for guests.
static uint32_t client_stats_level(Client *client) {
    if (client == nullptr) return 0;
    if (!client->account_name.empty()) {
        Account *acc = AccountStore::find(client->account_name);
        if (acc != nullptr) return acc->level;
    }
    if (client->game != nullptr && client->game->simulation.ent_exists(client->camera))
        return client->game->simulation.get_ent(client->camera).get_respawn_level();
    return 0;
}

void client_flush_stats(Client *client) {
    if (client == nullptr || client->play_name.empty()) return;
    uint32_t level = client_stats_level(client);
    if (level > 0)
        LeaderboardStore::add_stat(client->play_name, kLbLevel, level);
    if (client->stat_playtime > 0) {
        LeaderboardStore::add_stat(client->play_name, kLbPlayTime, client->stat_playtime);
        client->stat_playtime = 0;
    }
    if (client->stat_messages > 0) {
        LeaderboardStore::add_stat(client->play_name, kLbMessages, client->stat_messages);
        client->stat_messages = 0;
    }
    if (client->stat_kills > 0) {
        LeaderboardStore::add_stat(client->play_name, kLbKills, client->stat_kills);
        client->stat_kills = 0;
    }
    if (client->stat_crafts > 0) {
        LeaderboardStore::add_stat(client->play_name, kLbCrafts, client->stat_crafts);
        client->stat_crafts = 0;
    }
}

// Refresh the stored best score so the score column shows the live run
static void client_refresh_score(Client *client) {
    if (client == nullptr || client->game == nullptr || client->play_name.empty()) return;
    Simulation *sim = &client->game->simulation;
    if (!sim->ent_exists(client->camera)) return;
    Entity &cam = sim->get_ent(client->camera);
    if (!sim->ent_alive(cam.get_player())) return;
    Entity &player = sim->get_ent(cam.get_player());
    LeaderboardStore::record(client->play_name, player.get_score(), player.get_color());
}

// Send the multi-category leaderboard board (top rows + the player's own row)
static void send_leaderboard_stats(Client *client) {
    if (client == nullptr) return;
    client_flush_stats(client);
    client_refresh_score(client);
    Writer writer(Server::OUTGOING_PACKET);
    writer.write<uint8_t>(Clientbound::kLeaderboardStats);
    writer.write<uint32_t>(LeaderboardStore::recorded_count());
    for (uint8_t category = 0; category < kLbCategoryCount; ++category) {
        std::vector<LeaderboardStat> top;
        LeaderboardStore::fill_top_stats(top, category, LEADERBOARD_STAT_SIZE);
        writer.write<uint8_t>((uint8_t) top.size());
        for (LeaderboardStat const &stat : top) {
            writer.write<std::string>(stat.name);
            writer.write<uint64_t>(stat.value);
        }
        uint64_t value = 0;
        uint32_t rank = client->play_name.empty()
            ? 0 : LeaderboardStore::stat_rank(client->play_name, category, value);
        writer.write<uint8_t>(rank > 0 ? 1 : 0);
        writer.write<std::string>(client->play_name);
        writer.write<uint32_t>(rank);
        writer.write<uint64_t>(value);
    }
    client->send_packet(writer.packet, writer.at - writer.packet);
}

static uint8_t try_spend_talent(Client *client, uint8_t talent_id) {
    if (client == nullptr || client->game == nullptr
        || !client->game->simulation.ent_exists(client->camera)) return 0;
    if (talent_id >= TalentID::kNum) return 0;
    uint32_t level = client_level(client);
    uint8_t max_r = Talent::max_rank(level);
    uint8_t &rank = (talent_id == TalentID::kHealth) ? client->talent_hp : client->talent_rot;
    if (rank >= max_r) return 0;
    uint8_t next = (uint8_t) (rank + 1);
    uint32_t free = Talent::points_free(level, client->talent_hp, client->talent_rot);
    if (free < Talent::rank_cost(next)) return 0;
    rank = next;
    return 1;
}

// Memory snapshot only — never sync-write accounts.dat on the tick hot path.
// Periodic Server flush (~5s) and disconnect/drain call AccountStore::save().
static void persist_account(Client *client) {
    AccountStore::capture_nosave(client);
}

void client_sync_craft(Client *client) {
    if (client == nullptr) return;
    send_craft_update(client, CraftResultCode::kCraftBankSync, PetalID::kNone);
}

void client_award_coins(Client *client, uint32_t amount) {
    if (client == nullptr || amount == 0) return;
    for (uint32_t i = 0; i < amount; ++i) {
        if (!try_give_petal(client, PetalID::kCoin)) break;
    }
    persist_account(client);
    send_craft_update(client, CraftResultCode::kCraftBankSync, PetalID::kCoin);
}

// Secondary + pure bag slots (not primary): [loadout, loadout+MAX)
static uint32_t count_bag_mats(Client *client, PetalID::T id) {
    if (!client->alive() || id == PetalID::kNone || id >= PetalID::kNumPetals) return 0;
    Simulation *simulation = &client->game->simulation;
    Entity &camera = simulation->get_ent(client->camera);
    Entity &player = simulation->get_ent(camera.get_player());
    uint32_t loadout = player.get_loadout_count();
    uint32_t end = loadout + MAX_SLOT_COUNT;
    uint32_t n = 0;
    for (uint32_t i = loadout; i < end; ++i)
        if (player.get_loadout_ids(i) == id) ++n;
    return n;
}

static uint32_t craft_owned(Client *client, PetalID::T id) {
    return count_bag_mats(client, id) + client->craft_bank[id];
}

// Pull overflow bank into empty backpack slots so craft materials match the bag.
static void flush_craft_bank_to_bag(Client *client) {
    if (client == nullptr || !client->alive()) return;
    Simulation *simulation = &client->game->simulation;
    Entity &camera = simulation->get_ent(client->camera);
    Entity &player = simulation->get_ent(camera.get_player());
    uint32_t loadout = player.get_loadout_count();
    uint32_t end = loadout + MAX_SLOT_COUNT;
    uint8_t moved = 0;
    // Use uint32_t — PetalID::T is uint8_t; a naive uint8_t loop hangs at 256.
    for (uint32_t id = 1; id < (uint32_t) PetalID::kNumPetals; ++id) {
        while (client->craft_bank[id] > 0) {
            uint8_t placed = 0;
            for (uint32_t i = loadout; i < end; ++i) {
                if (player.get_loadout_ids(i) != PetalID::kNone) continue;
                player.set_loadout_ids(i, (PetalID::T) id);
                PetalTracker::add_petal(simulation, (PetalID::T) id);
                --client->craft_bank[id];
                placed = 1;
                moved = 1;
                break;
            }
            if (!placed) break;
        }
    }
    if (moved) {
        persist_account(client);
        send_craft_update(client, CraftResultCode::kCraftBankSync, PetalID::kNone);
    }
}

// Consume from backpack (secondary + pure bag), high index first.
static uint8_t consume_craft_mats(Client *client, PetalID::T id, uint32_t amount) {
    if (amount == 0) return 1;
    if (id == PetalID::kNone || id >= PetalID::kNumPetals) return 0;
    flush_craft_bank_to_bag(client);
    if (craft_owned(client, id) < amount) return 0;
    if (!client->alive()) return 0;
    Simulation *simulation = &client->game->simulation;
    Entity &camera = simulation->get_ent(client->camera);
    Entity &player = simulation->get_ent(camera.get_player());
    uint32_t loadout = player.get_loadout_count();
    uint32_t end = loadout + MAX_SLOT_COUNT;
    for (uint32_t i = end; i-- > loadout && amount > 0;) {
        if (player.get_loadout_ids(i) != id) continue;
        player.set_loadout_ids(i, PetalID::kNone);
        PetalTracker::remove_petal(simulation, id);
        --amount;
    }
    if (amount > 0) {
        if (client->craft_bank[id] < amount) return 0;
        client->craft_bank[id] -= amount;
        amount = 0;
    }
    return amount == 0 ? 1 : 0;
}

// Prefer empty backpack slots, then empty primary (never primary for bag-only).
// Returns 1 when the petal joins the backpack, including its stacked section.
static uint8_t try_give_petal(Client *client, PetalID::T id) {
    if (id == PetalID::kNone || id >= PetalID::kNumPetals) return 0;
    if (!client->alive()) return 0;
    Simulation *simulation = &client->game->simulation;
    Entity &camera = simulation->get_ent(client->camera);
    Entity &player = simulation->get_ent(camera.get_player());
    uint32_t loadout = player.get_loadout_count();
    auto place = [&](uint32_t i) -> uint8_t {
        if (player.get_loadout_ids(i) != PetalID::kNone) return 0;
        player.set_loadout_ids(i, id);
        PetalTracker::add_petal(simulation, id);
        return 1;
    };
    for (uint32_t i = loadout; i < loadout + MAX_SLOT_COUNT; ++i)
        if (place(i)) return 1;
    // Coins / craft mats never occupy primary slots
    if (!Recipe::is_bag_only(id)) {
        for (uint32_t i = 0; i < loadout; ++i)
            if (place(i)) return 1;
    }
    if (client->craft_bank[id] >= 1000000) return 0;
    ++client->craft_bank[id];
    return 1;
}

Client::Client() : game(nullptr) {}

void Client::init() {
    DEBUG_ONLY(assert(game == nullptr);)
    // Zero-downtime deploy: refuse new sessions while draining so nginx can flip to the new build
    if (Server::is_draining) {
        disconnect(CloseReason::kOutdated, "Outdated Version");
        return;
    }
    Server::games[MapID::kGarden].add_client(this);
}

void Client::remove() {
    if (game == nullptr) return;
    game->remove_client(this);
}

void Client::disconnect(int reason, std::string const &message) {
    if (ws == nullptr) return;
    remove();
    ws->end(reason, message);
}

uint8_t Client::alive() {
    if (game == nullptr) return false;
    Simulation *simulation = &game->simulation;
    if (!simulation->ent_exists(camera)) return false;
    // Must use ent_alive: pending-delete corpses still "exist" and would block respawn
    return simulation->ent_alive(simulation->get_ent(camera).get_player());
}

void Client::on_message(WebSocket *ws, std::string_view message, uint64_t code) {
    if (ws == nullptr) return;
    uint8_t const *data = reinterpret_cast<uint8_t const *>(message.data());
    Reader reader(data);
    Validator validator(data, data + message.size());
    Client *client = ws->getUserData();
    if (client == nullptr) {
        ws->end(CloseReason::kServer, "Server Error");
        return;
    }
    if (!client->verified) {
        if (client->check_invalid(validator.validate_uint8() && validator.validate_uint64())) return;
        if (reader.read<uint8_t>() != Serverbound::kVerify) {
            client->disconnect();
            return;
        }
        {
            uint64_t got = reader.read<uint64_t>();
            // Accept current hash, plus a recent window so cached tabs / SW
            // survive rapid deploys (not a reverse-proxy issue).
            uint8_t ok = (got == VERSION_HASH)
                || (got < VERSION_HASH && VERSION_HASH - got <= 300ull);
            if (!ok) {
                std::printf("outdated client hash=%llu expect=%llu\n",
                    (unsigned long long) got, (unsigned long long) VERSION_HASH);
                client->disconnect(CloseReason::kOutdated, "Outdated Version");
                return;
            }
            if (got != VERSION_HASH)
                std::printf("accepted legacy client hash=%llu\n", (unsigned long long) got);
        }
        client->verified = 1;
        client->init();
        return;
    }
    if (client->game == nullptr) {
        client->disconnect();
        return;
    }
    if (client->check_invalid(validator.validate_uint8())) return;
    uint8_t const message_type = reader.read<uint8_t>();
    switch (message_type) {
        case Serverbound::kVerify:
            client->disconnect();
            return;
        case Serverbound::kClientInput: {
            if (!client->alive()) break;
            Simulation *simulation = &client->game->simulation;
            Entity &camera = simulation->get_ent(client->camera);
            Entity &player = simulation->get_ent(camera.get_player());
            if (client->check_invalid(
                validator.validate_float() &&
                validator.validate_float() &&
                validator.validate_uint8() &&
                validator.validate_uint8()
            )) return;
            float x = reader.read<float>();
            float y = reader.read<float>();
            // Reject NaN/Inf: comparisons alone let NaN into acceleration and
            // poison motion until the whole arena freezes (esp. after mobile respawns).
            if (!is_finite(x) || !is_finite(y)) break;
            if (x == 0 && y == 0) player.acceleration.set(0,0);
            else {
                if (std::abs(x) > 5e3 || std::abs(y) > 5e3) break;
                Vector accel(x,y);
                float m = accel.magnitude();
                if (!is_finite(m)) break;
                if (m > 200) accel.set_magnitude(PLAYER_ACCELERATION);
                else accel.set_magnitude(m / 200 * PLAYER_ACCELERATION);
                player.acceleration = accel;
            }
            player.input = reader.read<uint8_t>();
            uint8_t rot_set = reader.read<uint8_t>();
            if (rot_set > Talent::kRotSettingMax) rot_set = Talent::kRotSettingMax;
            client->rot_speed_setting = rot_set;
            break;
        }
        case Serverbound::kClientSpawn: {
            // A spawn request means the client believes it is dead. If the
            // server still holds a live flower the client view is stale:
            // drop the old flower and honour the request. Silently ignoring
            // it leaves the client waiting for an update that never comes,
            // so the player sits on the death screen forever.
            if (client->alive() && client->game != nullptr) {
                Simulation *stale_sim = &client->game->simulation;
                Entity &stale_cam = stale_sim->get_ent(client->camera);
                Entity &stale_player = stale_sim->get_ent(stale_cam.get_player());
                BitMath::set(stale_player.flags, EntityFlags::kNoDrops);
                stale_sim->request_delete(stale_player.id);
                stale_cam.set_player(NULL_ENTITY);
                std::cout << "[spawn] replaced stale player for " << client->play_name << std::endl;
            }
            //check string length
            std::string name;
            if (client->check_invalid(
                validator.validate_string(MAX_NAME_LENGTH) &&
                validator.validate_uint8()
            )) return;
            reader.read<std::string>(name);
            if (client->check_invalid(UTF8Parser::is_valid_utf8(name))) return;
            uint8_t map_id = reader.read<uint8_t>();
            if (map_id >= MapID::kNumMaps) map_id = MapID::kGarden;
            if (!client->account_name.empty()) {
                name = client->account_name;
            } else {
                // Guests: fixed G#XXXXXX only — ignore free-form names
                auto guest_tag_ok = [](std::string const &s) -> uint8_t {
                    if (s.size() != 8 || s[0] != 'G' || s[1] != '#') return 0;
                    for (size_t i = 2; i < s.size(); ++i) {
                        char c = s[i];
                        if (c >= 'A' && c <= 'Z' && c != 'I' && c != 'O') continue;
                        if (c >= '2' && c <= '9') continue;
                        return 0;
                    }
                    return 1;
                };
                if (!guest_tag_ok(name)) {
                    // Assign a unique server guest id (does not occupy account names)
                    static uint32_t next_guest = 100000;
                    char buf[16];
                    static char const alphabet[] = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";
                    do {
                        uint32_t n = next_guest++;
                        buf[0] = 'G'; buf[1] = '#';
                        for (int i = 0; i < 6; ++i) {
                            buf[2 + i] = alphabet[n % 32];
                            n /= 32;
                        }
                        buf[8] = 0;
                        name = buf;
                    } while (LeaderboardStore::name_taken(name, client));
                }
            }
            if (client->account_name.empty() && AccountStore::name_reserved_for_guest(name)) {
                Writer writer(Server::OUTGOING_PACKET);
                writer.write<uint8_t>(Clientbound::kSpawnResult);
                writer.write<uint8_t>(SpawnResultCode::kSpawnNameTaken);
                client->send_packet(writer.packet, writer.at - writer.packet);
                break;
            }
            // Unique play names across all maps
            if (!name.empty() && LeaderboardStore::name_taken(name, client)) {
                std::cout << "[spawn] name taken: " << name << std::endl;
                Writer writer(Server::OUTGOING_PACKET);
                writer.write<uint8_t>(Clientbound::kSpawnResult);
                writer.write<uint8_t>(SpawnResultCode::kSpawnNameTaken);
                client->send_packet(writer.packet, writer.at - writer.packet);
                break;
            }
            Party::clear_corpse(client);
            if (client->game == nullptr)
                Server::games[map_id].add_client(client);
            else if (client->game->map_id != map_id)
                client->game->transfer_client(client, &Server::games[map_id]);
            Simulation *simulation = &client->game->simulation;
            Entity &camera = simulation->get_ent(client->camera);
            // Drop stale dead player so camera isn't stuck on a deleting entity
            if (simulation->ent_exists(camera.get_player())
                && !simulation->ent_alive(camera.get_player())) {
                simulation->request_delete(camera.get_player());
                camera.set_player(NULL_ENTITY);
            }
            Entity &player = alloc_player(simulation, camera.get_team());
            player_spawn(simulation, camera, player);
            player.set_name(name);
            player.set_guild(client->guild_name);
            client->play_name = name;
            if (!name.empty())
                LeaderboardStore::record(name, player.get_score(), player.get_color());
            Party::apply_team(client);
            Party::send_update(client);
            Guild::send_update(client);
            break;
        }
        case Serverbound::kAccountAuth: {
            if (client->check_invalid(
                validator.validate_uint8() &&
                validator.validate_string(MAX_USERNAME_LENGTH) &&
                validator.validate_string(MAX_PASSWORD_LENGTH)
            )) return;
            uint8_t mode = reader.read<uint8_t>();
            std::string username;
            std::string password;
            reader.read<std::string>(username);
            reader.read<std::string>(password);
            uint8_t result = AccountResultCode::kAccountInvalidUser;
            if (mode == AccountAuthMode::kAccountRegister) {
                result = AccountStore::register_account(client, username, password);
                if (result == AccountResultCode::kAccountOk)
                    result = AccountStore::login(client, username, password);
            } else if (mode == AccountAuthMode::kAccountResume) {
                uint64_t token = 0;
                for (unsigned char c : password) {
                    if (c < '0' || c > '9') { token = 0; break; }
                    token = token * 10 + (uint64_t) (c - '0');
                }
                result = AccountStore::resume(client, username, token);
            } else {
                result = AccountStore::login(client, username, password);
            }
            Writer writer(Server::OUTGOING_PACKET);
            writer.write<uint8_t>(Clientbound::kAccountResult);
            writer.write<uint8_t>(result);
            Account *acc = AccountStore::find(client->account_name);
            writer.write<uint8_t>(acc == nullptr ? 1 : acc->level);
            for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i)
                writer.write<uint8_t>(acc == nullptr ? PetalID::kNone : acc->petals[i]);
            for (uint32_t i = 0; i < PetalID::kNumPetals; ++i)
                writer.write<uint32_t>(acc == nullptr ? 0 : acc->craft_bank[i]);
            writer.write<uint32_t>(acc == nullptr ? 0 : acc->coins);
            writer.write<uint64_t>(
                result == AccountResultCode::kAccountOk
                    ? AccountStore::current_session(client->account_name)
                    : 0
            );
            client->send_packet(writer.packet, writer.at - writer.packet);
            if (result == AccountResultCode::kAccountOk) {
                send_craft_update(client, CraftResultCode::kCraftBankSync, PetalID::kNone);
                send_talent_update(client);
                send_loadout_preset_update(client, kPresetSynced);
                send_boost_update(client);
            }
            break;
        }
        case Serverbound::kBoostToggle: {
            if (client->check_invalid(validator.validate_uint8())) return;
            uint8_t on = reader.read<uint8_t>() ? 1 : 0;
            if (client->account_name.empty()) break;
            client->bonus_enabled = on;
            // Don't burn the pause gap after turning back on.
            client->last_bonus_tick = (uint64_t)std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
            send_boost_update(client);
            break;
        }
        case Serverbound::kTalentSpend: {
            if (client->check_invalid(validator.validate_uint8())) return;
            uint8_t talent_id = reader.read<uint8_t>();
            if (try_spend_talent(client, talent_id)) {
                AccountStore::capture_nosave(client);
                send_talent_update(client);
            } else {
                send_talent_update(client);
            }
            break;
        }
        case Serverbound::kPetalDelete: {
            // Wire name kept; behavior = drop petal on ground for others to pick up
            if (!client->alive()) break;
            Simulation *simulation = &client->game->simulation;
            Entity &camera = simulation->get_ent(client->camera);
            Entity &player = simulation->get_ent(camera.get_player());
            if (client->check_invalid(validator.validate_uint8())) return;
            uint8_t pos = reader.read<uint8_t>();
            if (pos >= MAX_SLOT_COUNT + player.get_loadout_count()) break;
            PetalID::T old_id = player.get_loadout_ids(pos);
            if (old_id == PetalID::kNone || old_id >= PetalID::kNumPetals) break;
            player.set_loadout_ids(pos, PetalID::kNone);
            // Tracker: leave loadout → world drop (alloc_drop adds)
            PetalTracker::remove_petal(simulation, old_id);
            Entity &drop = alloc_drop(simulation, old_id, NULL_ENTITY);
            drop.set_x(player.get_x());
            drop.set_y(player.get_y());
            drop.velocity.unit_normal(frand() * 2 * (float) M_PI).set_magnitude(40);
            // Brief global immunity so dropper does not instantly re-pick
            drop.immunity_ticks = TPS;
            flush_craft_bank_to_bag(client);
            AccountStore::capture_nosave(client);
            break;
        }
        case Serverbound::kPetalCraft: {
            if (!client->alive()) break;
            if (client->check_invalid(validator.validate_uint8() && validator.validate_uint8())) return;
            // Overflow bank → backpack first so craft list matches the bag UI.
            flush_craft_bank_to_bag(client);
            uint8_t mode = reader.read<uint8_t>();
            uint8_t arg = reader.read<uint8_t>();
            if (mode == CraftMode::kCraftBuyRarity) {
                uint8_t rarity = arg;
                if (rarity >= RarityID::kNumRarities) {
                    send_craft_update(client, CraftResultCode::kCraftNoMats, PetalID::kNone);
                    break;
                }
                uint32_t cost = Craft::buy_rarity_cost(rarity);
                if (craft_owned(client, PetalID::kCoin) < cost) {
                    send_craft_update(client, CraftResultCode::kCraftNoCoins, PetalID::kNone);
                    break;
                }
                PetalID::T got = Craft::random_petal_of_rarity(rarity);
                if (!consume_craft_mats(client, PetalID::kCoin, cost)) {
                    send_craft_update(client, CraftResultCode::kCraftNoCoins, PetalID::kNone);
                    break;
                }
                if (!try_give_petal(client, got)) {
                    // Refund materials path already banked; still spent coins — OK (got material)
                    persist_account(client);
                    send_craft_update(client, CraftResultCode::kCraftInvFull, got);
                } else {
                    persist_account(client);
                    send_craft_update(client, CraftResultCode::kCraftBought, got);
                }
                break;
            }
            if (mode == CraftMode::kCraftBuySpecial) {
                PetalID::T id = (PetalID::T) arg;
                if (!Craft::is_craftable_special(id)) {
                    send_craft_update(client, CraftResultCode::kCraftNoMats, PetalID::kNone);
                    break;
                }
                uint32_t cost = Craft::buy_special_cost(id);
                if (craft_owned(client, PetalID::kCoin) < cost) {
                    send_craft_update(client, CraftResultCode::kCraftNoCoins, PetalID::kNone);
                    break;
                }
                if (!consume_craft_mats(client, PetalID::kCoin, cost)) {
                    send_craft_update(client, CraftResultCode::kCraftNoCoins, PetalID::kNone);
                    break;
                }
                if (!try_give_petal(client, id)) {
                    persist_account(client);
                    send_craft_update(client, CraftResultCode::kCraftInvFull, id);
                } else {
                    persist_account(client);
                    send_craft_update(client, CraftResultCode::kCraftBought, id);
                }
                break;
            }
            if (mode == CraftMode::kCraftRecipe) {
                uint16_t idx = (uint16_t) arg;
                if (idx >= Recipe::recipe_count()) {
                    send_craft_update(client, CraftResultCode::kCraftNoMats, PetalID::kNone);
                    break;
                }
                Recipe::Def const &rec = Recipe::recipe_at(idx);
                flush_craft_bank_to_bag(client);
                uint8_t ok = 1;
                for (uint8_t i = 0; i < rec.ingredient_count; ++i) {
                    if (craft_owned(client, rec.ingredients[i].id) < rec.ingredients[i].count) {
                        ok = 0;
                        break;
                    }
                }
                if (!ok) {
                    send_craft_update(client, CraftResultCode::kCraftNoMats, PetalID::kNone);
                    break;
                }
                for (uint8_t i = 0; i < rec.ingredient_count; ++i) {
                    if (!consume_craft_mats(client, rec.ingredients[i].id, rec.ingredients[i].count)) {
                        ok = 0;
                        break;
                    }
                }
                if (!ok) {
                    send_craft_update(client, CraftResultCode::kCraftNoMats, PetalID::kNone);
                    break;
                }
                if (!try_give_petal(client, rec.result)) {
                    persist_account(client);
                    send_craft_update(client, CraftResultCode::kCraftInvFull, rec.result);
                } else {
                    if (client->stat_crafts < 4000000000u) ++client->stat_crafts;
                    persist_account(client);
                    send_craft_update(client, CraftResultCode::kCraftBought, rec.result);
                }
                break;
            }
            PetalID::T from = (PetalID::T) arg;
            if (from == PetalID::kNone || from >= PetalID::kNumPetals
                || !Craft::can_upgrade_petal(from)) {
                send_craft_update(client, CraftResultCode::kCraftNoMats, PetalID::kNone);
                break;
            }
            uint8_t rarity = PETAL_DATA[from].rarity;
            if (mode == CraftMode::kCraftExchange) {
                uint32_t cost = Craft::exchange_cost(rarity);
                if (!consume_craft_mats(client, from, cost)) {
                    send_craft_update(client, CraftResultCode::kCraftNoMats, PetalID::kNone);
                    break;
                }
                if (client->craft_attempts < 4000000000u)
                    ++client->craft_attempts;
                PetalID::T got = Craft::craft_result(from);
                if (got == PetalID::kNone) {
                    send_craft_update(client, CraftResultCode::kCraftNoMats, PetalID::kNone);
                    break;
                }
                uint8_t placed = try_give_petal(client, got);
                if (client->stat_crafts < 4000000000u)
                    ++client->stat_crafts;
                persist_account(client);
                send_craft_update(client,
                    placed ? CraftResultCode::kCraftExchanged : CraftResultCode::kCraftInvFull,
                    got);
                break;
            }
            uint32_t max_batches = (mode == CraftMode::kCraftGambleAll) ? 200u : 1u;
            uint32_t done = 0;
            uint32_t wins = 0;
            uint8_t last_placed = 0;
            PetalID::T last_got = PetalID::kNone;
            while (done < max_batches && craft_owned(client, from) >= Craft::kCraftBatch) {
                if (!consume_craft_mats(client, from, Craft::kCraftBatch)) break;
                ++done;
                if (client->craft_attempts < 4000000000u)
                    ++client->craft_attempts;
                if (frand() < Craft::success_chance(rarity)) {
                    last_got = Craft::craft_result(from);
                    if (last_got == PetalID::kNone) continue;
                    last_placed = try_give_petal(client, last_got);
                    ++wins;
                }
            }
            persist_account(client);
            if (done > 0 && client->stat_crafts < 4000000000u)
                client->stat_crafts += done;
            if (done == 0)
                send_craft_update(client, CraftResultCode::kCraftNoMats, PetalID::kNone);
            else if (wins > 0)
                send_craft_update(client,
                    last_placed ? CraftResultCode::kCraftWin : CraftResultCode::kCraftInvFull,
                    last_got);
            else
                send_craft_update(client, CraftResultCode::kCraftMiss, PetalID::kNone);
            break;
        }
        case Serverbound::kPetalSwap: {
            // Alive: flower loadout. Dead/title: camera inventory (respawn layout).
            if (client->check_invalid(validator.validate_uint8() && validator.validate_uint8())) return;
            uint8_t pos1 = reader.read<uint8_t>();
            uint8_t pos2 = reader.read<uint8_t>();
            if (!client->game || !client->game->simulation.ent_exists(client->camera)) break;
            Simulation *simulation = &client->game->simulation;
            Entity &camera = simulation->get_ent(client->camera);
            if (client->alive()) {
                Entity &player = simulation->get_ent(camera.get_player());
                if (pos1 >= MAX_SLOT_COUNT + player.get_loadout_count()) break;
                if (pos2 >= MAX_SLOT_COUNT + player.get_loadout_count()) break;
                uint8_t primary = player.get_loadout_count();
                if ((pos1 < primary && Recipe::is_bag_only(player.get_loadout_ids(pos2)))
                    || (pos2 < primary && Recipe::is_bag_only(player.get_loadout_ids(pos1))))
                    break;
                PetalID::T tmp = player.get_loadout_ids(pos1);
                player.set_loadout_ids(pos1, player.get_loadout_ids(pos2));
                player.set_loadout_ids(pos2, tmp);
                flush_craft_bank_to_bag(client);
            } else {
                uint32_t primary = loadout_slots_at_level(camera.get_respawn_level());
                uint32_t lim = primary + MAX_SLOT_COUNT;
                if (lim > 2 * MAX_SLOT_COUNT) lim = 2 * MAX_SLOT_COUNT;
                if (pos1 >= lim || pos2 >= lim) break;
                PetalID::T tmp = camera.get_inventory(pos1);
                camera.set_inventory(pos1, camera.get_inventory(pos2));
                camera.set_inventory(pos2, tmp);
            }
            // The periodic account flush and disconnect path persist this swap.
            // Synchronous full-file writes here stall the Node event loop.
            AccountStore::capture_nosave(client);
            break;
        }
        case Serverbound::kLoadoutPresetSave:
        case Serverbound::kLoadoutPresetApply: {
            if (client->check_invalid(validator.validate_uint8())) return;
            uint8_t index = reader.read<uint8_t>();
            if (index >= LOADOUT_PRESET_COUNT) break;
            if (client->account_name.empty()) {
                send_loadout_preset_update(client, kPresetNeedAccount, index);
                break;
            }
            if (!client->game || !client->game->simulation.ent_exists(client->camera)) break;
            Simulation *simulation = &client->game->simulation;
            Entity &camera = simulation->get_ent(client->camera);
            Entity *player = client->alive() ? &simulation->get_ent(camera.get_player()) : nullptr;
            uint8_t slots = player != nullptr ? player->get_loadout_count()
                : (uint8_t) loadout_slots_at_level(camera.get_respawn_level());
            std::array<PetalID::T, 2 * MAX_SLOT_COUNT> inventory = {};
            for (uint8_t i = 0; i < slots + MAX_SLOT_COUNT; ++i)
                inventory[i] = player != nullptr ? player->get_loadout_ids(i) : camera.get_inventory(i);
            LoadoutPreset &preset = client->loadout_presets[index];
            if (message_type == Serverbound::kLoadoutPresetSave) {
                preset.slot_count = slots;
                for (uint8_t i = 0; i < MAX_SLOT_COUNT; ++i)
                    preset.petals[i] = i < slots ? inventory[i] : PetalID::kNone;
                AccountStore::capture_nosave(client);
                send_loadout_preset_update(client, kPresetSaved, index);
                break;
            }
            if (preset.slot_count == 0) {
                send_loadout_preset_update(client, kPresetEmpty, index);
                break;
            }
            std::array<uint32_t, PetalID::kNumPetals> overflow = {};
            uint8_t fallback_count = 0;
            if (!arrange_loadout_preset(inventory, slots, preset, overflow, fallback_count)) {
                send_loadout_preset_update(client, kPresetEmpty, index);
                break;
            }
            for (uint8_t i = 0; i < slots + MAX_SLOT_COUNT; ++i) {
                if (player != nullptr) player->set_loadout_ids(i, inventory[i]);
                else camera.set_inventory(i, inventory[i]);
            }
            AccountStore::capture_nosave(client);
            send_loadout_preset_update(client,
                fallback_count ? kPresetAppliedAdjusted : kPresetApplied, index);
            break;
        }
        case Serverbound::kBankWithdraw: {
            // Equip one copy from backpack stacks onto the loadout bar.
            if (client->check_invalid(validator.validate_uint8())) return;
            PetalID::T id = reader.read<uint8_t>();
            if (id == PetalID::kNone || id >= PetalID::kNumPetals
                || client->craft_bank[id] == 0) break;
            if (!client->game || !client->game->simulation.ent_exists(client->camera)) break;
            Simulation *simulation = &client->game->simulation;
            Entity &camera = simulation->get_ent(client->camera);
            Entity *player = client->alive() ? &simulation->get_ent(camera.get_player()) : nullptr;
            uint8_t n = player != nullptr ? player->get_loadout_count()
                : (uint8_t) loadout_slots_at_level(camera.get_respawn_level());
            if (n == 0) break;
            auto slot_at = [&](uint8_t i) -> PetalID::T {
                return player != nullptr ? player->get_loadout_ids(i)
                    : camera.get_inventory(i);
            };
            auto set_slot = [&](uint8_t i, PetalID::T v) {
                if (player != nullptr) player->set_loadout_ids(i, v);
                else camera.set_inventory(i, v);
            };
            // Coins / craft mats stay in the backpack — never on primary.
            if (Recipe::is_bag_only(id)) break;
            // Same priority as client auto-equip: empty → Basic → empty secondary → swap.
            uint8_t dest = (uint8_t) -1;
            for (uint8_t i = 0; i < n; ++i)
                if (slot_at(i) == PetalID::kNone) { dest = i; break; }
            if (dest == (uint8_t) -1)
                for (uint8_t i = 0; i < n; ++i)
                    if (slot_at(i) == PetalID::kBasic) { dest = i; break; }
            if (dest == (uint8_t) -1)
                for (uint8_t i = 0; i < n; ++i) {
                    uint8_t s = (uint8_t) (n + i);
                    if (slot_at(s) == PetalID::kNone) { dest = s; break; }
                }
            if (dest == (uint8_t) -1)
                for (uint8_t i = 0; i < n; ++i)
                    if (slot_at(i) != id) { dest = i; break; }
            if (dest == (uint8_t) -1) break;
            PetalID::T old = slot_at(dest);
            if (old != PetalID::kNone && old != PetalID::kBasic) {
                if (client->craft_bank[old] >= 1000000) break;
                ++client->craft_bank[old];
                PetalTracker::remove_petal(simulation, old);
            } else if (old == PetalID::kBasic) {
                PetalTracker::remove_petal(simulation, old);
            }
            --client->craft_bank[id];
            PetalTracker::add_petal(simulation, id);
            set_slot(dest, id);
            AccountStore::capture_nosave(client);
            send_craft_update(client, CraftResultCode::kCraftBankSync, id);
            break;
        }
        case Serverbound::kBankDeposit: {
            // Unequip primary/secondary into backpack stacks (never secondary).
            if (client->check_invalid(validator.validate_uint8())) return;
            uint8_t pos = reader.read<uint8_t>();
            if (!client->game || !client->game->simulation.ent_exists(client->camera)) break;
            Simulation *simulation = &client->game->simulation;
            Entity &camera = simulation->get_ent(client->camera);
            Entity *player = client->alive() ? &simulation->get_ent(camera.get_player()) : nullptr;
            uint8_t n = player != nullptr ? player->get_loadout_count()
                : (uint8_t) loadout_slots_at_level(camera.get_respawn_level());
            if (n == 0 || pos >= 2 * n) break;
            PetalID::T id = player != nullptr ? player->get_loadout_ids(pos)
                : camera.get_inventory(pos);
            if (id == PetalID::kNone || id >= PetalID::kNumPetals) break;
            if (client->craft_bank[id] >= 1000000) break;
            ++client->craft_bank[id];
            PetalTracker::remove_petal(simulation, id);
            if (player != nullptr) player->set_loadout_ids(pos, PetalID::kNone);
            else camera.set_inventory(pos, PetalID::kNone);
            AccountStore::capture_nosave(client);
            send_craft_update(client, CraftResultCode::kCraftBankSync, id);
            break;
        }
        case Serverbound::kChat: {
            if (!client->alive()) break;
            if (client->check_invalid(
                validator.validate_uint8() &&
                validator.validate_string(MAX_NAME_LENGTH) &&
                validator.validate_string(MAX_CHAT_LENGTH)
            )) return;
            uint8_t channel = reader.read<uint8_t>();
            std::string target;
            std::string text;
            reader.read<std::string>(target);
            reader.read<std::string>(text);
            if (client->check_invalid(UTF8Parser::is_valid_utf8(target) && UTF8Parser::is_valid_utf8(text))) return;
            if (channel > ChatChannel::kChatWhisper) channel = ChatChannel::kChatNearby;
            Chat::handle(client, channel, target, text);
            break;
        }
        case Serverbound::kPartyInvite: {
            if (!client->alive()) break;
            if (client->check_invalid(validator.validate_string(MAX_NAME_LENGTH))) return;
            std::string target;
            reader.read<std::string>(target);
            Party::invite(client, target);
            break;
        }
        case Serverbound::kPartyLeave: {
            Party::leave(client);
            break;
        }
        case Serverbound::kInviteRespond: {
            if (!client->alive()) break;
            if (client->check_invalid(
                validator.validate_uint8() &&
                validator.validate_uint8() &&
                validator.validate_string(MAX_NAME_LENGTH)
            )) return;
            uint8_t type = reader.read<uint8_t>();
            uint8_t accept = reader.read<uint8_t>();
            std::string from_name;
            reader.read<std::string>(from_name);
            if (type == InviteType::kInviteGuild)
                Guild::respond(client, from_name, accept ? 1 : 0);
            else
                Party::respond(client, from_name, accept ? 1 : 0);
            break;
        }
        case Serverbound::kGuildCreate: {
            if (!client->alive()) break;
            if (client->check_invalid(validator.validate_string(Guild::kNameMax))) return;
            std::string name;
            reader.read<std::string>(name);
            Guild::create(client, name);
            break;
        }
        case Serverbound::kGuildInvite: {
            if (!client->alive()) break;
            if (client->check_invalid(validator.validate_string(MAX_NAME_LENGTH))) return;
            std::string target;
            reader.read<std::string>(target);
            Guild::invite(client, target);
            break;
        }
        case Serverbound::kGuildLeave: {
            Guild::leave(client);
            break;
        }
        case Serverbound::kClientLeave: {
            // Return to title without closing the WebSocket
            if (client->game == nullptr) break;
            Simulation *simulation = &client->game->simulation;
            if (!simulation->ent_exists(client->camera)) break;
            Entity &camera = simulation->get_ent(client->camera);
            Party::clear_corpse(client);
            if (simulation->ent_alive(camera.get_player())) {
                Entity &player = simulation->get_ent(camera.get_player());
                BitMath::set(player.flags, EntityFlags::kNoDrops);
                simulation->request_delete(player.id);
            }
            AccountStore::capture(client);
            Party::leave(client);
            client->play_name.clear();
            Party::send_update(client);
            break;
        }
        case Serverbound::kAccountLogout: {
            if (client->account_name.empty()) break;
            // Persist progress first; never delete the account row
            AccountStore::capture(client);
            if (client->alive()) {
                Simulation *simulation = &client->game->simulation;
                Entity &camera = simulation->get_ent(client->camera);
                simulation->request_delete(camera.get_player());
            }
            AccountStore::logout(client, 1);
            client->craft_bank = {};
            client->coins = 0;
            client->craft_attempts = 0;
            client->talent_hp = 0;
            client->talent_rot = 0;
            client->loadout_presets = {};
            Party::leave(client);
            Guild::leave(client);
            Writer writer(Server::OUTGOING_PACKET);
            writer.write<uint8_t>(Clientbound::kAccountResult);
            writer.write<uint8_t>(AccountResultCode::kAccountLoggedOut);
            writer.write<uint8_t>(1);
            for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i)
                writer.write<uint8_t>(PetalID::kNone);
            for (uint32_t i = 0; i < PetalID::kNumPetals; ++i)
                writer.write<uint32_t>(0);
            writer.write<uint32_t>(0);
            writer.write<uint64_t>(0);
            client->send_packet(writer.packet, writer.at - writer.packet);
            send_craft_update(client, CraftResultCode::kCraftBankSync, PetalID::kNone);
            send_loadout_preset_update(client, kPresetSynced);
            break;
        }
        case Serverbound::kGuestProgress: {
            // Apply browser-cached guest progress when not logged in
            if (!client->account_name.empty()) break;
            if (client->alive()) break;
            if (client->check_invalid(validator.validate_uint8())) return;
            uint8_t has = reader.read<uint8_t>();
            if (!has) {
                Simulation *simulation = &client->game->simulation;
                if (simulation->ent_exists(client->camera)) {
                    Entity &camera = simulation->get_ent(client->camera);
                    for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i)
                        PetalTracker::remove_petal(simulation, camera.get_inventory(i));
                    camera.set_respawn_level(1);
                    uint32_t slots = loadout_slots_at_level(1);
                    for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i) {
                        PetalID::T id = i < slots ? PetalID::kBasic : PetalID::kNone;
                        camera.set_inventory(i, id);
                        PetalTracker::add_petal(simulation, id);
                    }
                }
                client->craft_bank.fill(0);
                client->coins = 0;
                client->talent_hp = client->talent_rot = 0;
                send_craft_update(client, CraftResultCode::kCraftBankSync, PetalID::kNone);
                send_talent_update(client);
                break;
            }
            // level + inventory
            if (client->check_invalid(validator.validate_uint8())) return;
            for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i)
                if (client->check_invalid(validator.validate_uint8())) return;
            // Craft bank length follows PetalID::kNumPetals and grows over deploys.
            // Old cached clients send fewer u32s — accept any count that still
            // leaves room for trailing coins(u32) + talent_hp/rot (2×u8).
            uint32_t bank_n = 0;
            {
                Validator probe(validator.at, validator.end);
                while (bank_n < (uint32_t) PetalID::kNumPetals) {
                    Validator trial(probe.at, probe.end);
                    if (!trial.validate_uint32()) break;
                    Validator rest(trial.at, trial.end);
                    if (!(rest.validate_uint32() && rest.validate_uint8()
                          && rest.validate_uint8()))
                        break;
                    probe.at = trial.at;
                    ++bank_n;
                }
            }
            for (uint32_t i = 0; i < bank_n; ++i)
                if (client->check_invalid(validator.validate_uint32())) return;
            if (client->check_invalid(validator.validate_uint32())) return; // coins
            if (client->check_invalid(validator.validate_uint8() && validator.validate_uint8())) return;
            uint8_t level = reader.read<uint8_t>();
            if (level < 1) level = 1;
            if (level > MAX_LEVEL) level = MAX_LEVEL;
            Simulation *simulation = &client->game->simulation;
            if (!simulation->ent_exists(client->camera)) break;
            Entity &camera = simulation->get_ent(client->camera);
            // Stale browser cache must not downgrade an already higher server level.
            if (level < camera.get_respawn_level())
                level = camera.get_respawn_level();
            for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i)
                PetalTracker::remove_petal(simulation, camera.get_inventory(i));
            camera.set_respawn_level(level);
            uint32_t slots = loadout_slots_at_level(level);
            for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i) {
                PetalID::T id = reader.read<uint8_t>();
                if (id >= PetalID::kNumPetals) id = PetalID::kNone;
                if (i < slots && id == PetalID::kNone) id = PetalID::kBasic;
                camera.set_inventory(i, id);
                PetalTracker::add_petal(simulation, id);
            }
            for (uint32_t i = 0; i < PetalID::kNumPetals; ++i)
                client->craft_bank[i] = 0;
            for (uint32_t i = 0; i < bank_n; ++i)
                client->craft_bank[i] = reader.read<uint32_t>();
            client->coins = reader.read<uint32_t>();
            uint8_t th = reader.read<uint8_t>();
            uint8_t tr = reader.read<uint8_t>();
            uint8_t max_r = Talent::max_rank(level);
            client->talent_hp = th > max_r ? max_r : th;
            client->talent_rot = tr > max_r ? max_r : tr;
            send_craft_update(client, CraftResultCode::kCraftBankSync, PetalID::kNone);
            send_talent_update(client);
            break;
        }
        case Serverbound::kHeartbeat:
            // Keepalive from background tabs (rAF frozen). No payload.
            break;
        case Serverbound::kLeaderboardRequest:
            send_leaderboard_stats(client);
            break;
    }
}

void Client::on_disconnect(WebSocket *ws, int code, std::string_view message) {
    std::printf("disconnect: [%d]\n", code);
    Client *client = ws->getUserData();
    if (client == nullptr) return;
    client->remove();
}

bool Client::check_invalid(bool valid) {
    if (valid) return false;
    std::cout << "client sent an invalid packet\n";
    //optional
    disconnect();

    return true;
}
