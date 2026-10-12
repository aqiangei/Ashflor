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
#include <Shared/StaticData.hh>
#include <Shared/Talent.hh>

#include <Helpers/Math.hh>

#include <array>
#include <chrono>
#include <iostream>

constexpr std::array<uint32_t, RarityID::kNumRarities> RARITY_TO_XP = { 2, 10, 50, 200, 1000, 5000, 0 };

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

static uint8_t try_spend_talent(Client *client, uint8_t talent_id) {
    if (client == nullptr) return 0;
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

// Always snapshot full account (loadout + bank + coins + attempts) — never bank-only
static void persist_account(Client *client) {
    AccountStore::capture(client);
}

void client_sync_craft(Client *client) {
    if (client == nullptr) return;
    send_craft_update(client, CraftResultCode::kCraftBankSync, PetalID::kNone);
}

void client_award_coins(Client *client, uint32_t amount) {
    if (client == nullptr || amount == 0) return;
    uint64_t sum = (uint64_t) client->coins + amount;
    if (sum > 1000000000ull) sum = 1000000000ull;
    client->coins = (uint32_t) sum;
    persist_account(client);
    send_craft_update(client, CraftResultCode::kCraftBankSync, PetalID::kNone);
}

static void bank_petal(Client *client, PetalID::T id) {
    if (id == PetalID::kNone || id >= PetalID::kNumPetals) return;
    if (client->craft_bank[id] < 1000000)
        client->craft_bank[id] += 1;
    persist_account(client);
    send_craft_update(client, CraftResultCode::kCraftBankSync, PetalID::kNone);
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
    if (id == PetalID::kNone || id >= PetalID::kNumPetals) return 0;
    return client->craft_bank[id] + count_bag_mats(client, id);
}

// Consume from craft_bank first, then secondary/pure-bag slots (high index first).
static uint8_t consume_craft_mats(Client *client, PetalID::T id, uint32_t amount) {
    if (amount == 0) return 1;
    if (id == PetalID::kNone || id >= PetalID::kNumPetals) return 0;
    if (craft_owned(client, id) < amount) return 0;
    uint32_t from_bank = amount;
    if (from_bank > client->craft_bank[id]) from_bank = client->craft_bank[id];
    client->craft_bank[id] -= from_bank;
    amount -= from_bank;
    if (amount == 0) return 1;
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
    return amount == 0 ? 1 : 0;
}

// Prefer empty backpack slots, then empty primary; track uniques
static uint8_t try_give_petal(Client *client, PetalID::T id) {
    if (!client->alive()) {
        bank_petal(client, id);
        return 0;
    }
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
    for (uint32_t i = 0; i < loadout; ++i)
        if (place(i)) return 1;
    // No free slot — bank as material
    if (client->craft_bank[id] < 1000000)
        client->craft_bank[id] += 1;
    return 0;
}

Client::Client() : game(nullptr) {}

void Client::init() {
    DEBUG_ONLY(assert(game == nullptr);)
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
    switch (reader.read<uint8_t>()) {
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
            if (x == 0 && y == 0) player.acceleration.set(0,0);
            else {
                if (std::abs(x) > 5e3 || std::abs(y) > 5e3) break;
                Vector accel(x,y);
                float m = accel.magnitude();
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
            if (client->alive()) break;
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
            }
            break;
        }
        case Serverbound::kTalentSpend: {
            if (client->check_invalid(validator.validate_uint8())) return;
            uint8_t talent_id = reader.read<uint8_t>();
            if (try_spend_talent(client, talent_id)) {
                AccountStore::capture(client);
                send_talent_update(client);
            } else {
                send_talent_update(client);
            }
            break;
        }
        case Serverbound::kPetalDelete: {
            if (!client->alive()) break;
            Simulation *simulation = &client->game->simulation;
            Entity &camera = simulation->get_ent(client->camera);
            Entity &player = simulation->get_ent(camera.get_player());
            if (client->check_invalid(validator.validate_uint8())) return;
            uint8_t pos = reader.read<uint8_t>();
            if (pos >= MAX_SLOT_COUNT + player.get_loadout_count()) break;
            PetalID::T old_id = player.get_loadout_ids(pos);
            if (old_id != PetalID::kNone && old_id != PetalID::kBasic) {
                uint8_t rarity = PETAL_DATA[old_id].rarity;
                player.set_score(player.get_score() + RARITY_TO_XP[rarity]);
                if (player.deleted_petals.size() == player.deleted_petals.capacity())
                    PetalTracker::remove_petal(simulation, player.deleted_petals[0]);
                player.deleted_petals.push_back(old_id);
                player.set_loadout_ids(pos, PetalID::kNone);
                bank_petal(client, old_id);
                break;
            }
            player.set_loadout_ids(pos, PetalID::kNone);
            break;
        }
        case Serverbound::kPetalCraft: {
            if (!client->alive()) break;
            if (client->check_invalid(validator.validate_uint8() && validator.validate_uint8())) return;
            uint8_t mode = reader.read<uint8_t>();
            uint8_t arg = reader.read<uint8_t>();
            if (mode == CraftMode::kCraftBuyRarity) {
                uint8_t rarity = arg;
                if (rarity >= RarityID::kNumRarities) {
                    send_craft_update(client, CraftResultCode::kCraftNoMats, PetalID::kNone);
                    break;
                }
                uint32_t cost = Craft::buy_rarity_cost(rarity);
                if (client->coins < cost) {
                    send_craft_update(client, CraftResultCode::kCraftNoCoins, PetalID::kNone);
                    break;
                }
                PetalID::T got = Craft::random_petal_of_rarity(rarity);
                client->coins -= cost;
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
                if (client->coins < cost) {
                    send_craft_update(client, CraftResultCode::kCraftNoCoins, PetalID::kNone);
                    break;
                }
                client->coins -= cost;
                if (!try_give_petal(client, id)) {
                    persist_account(client);
                    send_craft_update(client, CraftResultCode::kCraftInvFull, id);
                } else {
                    persist_account(client);
                    send_craft_update(client, CraftResultCode::kCraftBought, id);
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
                try_give_petal(client, got);
                persist_account(client);
                send_craft_update(client, CraftResultCode::kCraftExchanged, got);
                break;
            }
            uint32_t max_batches = (mode == CraftMode::kCraftGambleAll) ? 200u : 1u;
            uint32_t done = 0;
            uint32_t wins = 0;
            PetalID::T last_got = PetalID::kNone;
            while (done < max_batches && craft_owned(client, from) >= Craft::kCraftBatch) {
                if (!consume_craft_mats(client, from, Craft::kCraftBatch)) break;
                ++done;
                if (client->craft_attempts < 4000000000u)
                    ++client->craft_attempts;
                if (frand() < Craft::success_chance(rarity)) {
                    last_got = Craft::craft_result(from);
                    try_give_petal(client, last_got);
                    ++wins;
                }
            }
            persist_account(client);
            if (done == 0)
                send_craft_update(client, CraftResultCode::kCraftNoMats, PetalID::kNone);
            else if (wins > 0)
                send_craft_update(client, CraftResultCode::kCraftWin, last_got);
            else
                send_craft_update(client, CraftResultCode::kCraftMiss, PetalID::kNone);
            break;
        }
        case Serverbound::kPetalSwap: {
            if (!client->alive()) break;
            Simulation *simulation = &client->game->simulation;
            Entity &camera = simulation->get_ent(client->camera);
            Entity &player = simulation->get_ent(camera.get_player());
            if (client->check_invalid(validator.validate_uint8() && validator.validate_uint8())) return;
            uint8_t pos1 = reader.read<uint8_t>();
            if (pos1 >= MAX_SLOT_COUNT + player.get_loadout_count()) break;
            uint8_t pos2 = reader.read<uint8_t>();
            if (pos2 >= MAX_SLOT_COUNT + player.get_loadout_count()) break;
            PetalID::T tmp = player.get_loadout_ids(pos1);
            player.set_loadout_ids(pos1, player.get_loadout_ids(pos2));
            player.set_loadout_ids(pos2, tmp);
            AccountStore::capture(client);
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
            break;
        }
        case Serverbound::kGuestProgress: {
            // Apply browser-cached guest progress when not logged in
            if (!client->account_name.empty()) break;
            if (client->alive()) break;
            if (client->check_invalid(validator.validate_uint8())) return;
            uint8_t has = reader.read<uint8_t>();
            if (!has) break;
            uint8_t need = 1 + 2 * MAX_SLOT_COUNT;
            for (uint32_t i = 0; i < need; ++i)
                if (client->check_invalid(validator.validate_uint8())) return;
            for (uint32_t i = 0; i < PetalID::kNumPetals; ++i)
                if (client->check_invalid(validator.validate_uint32())) return;
            if (client->check_invalid(validator.validate_uint32())) return; // coins
            if (client->check_invalid(validator.validate_uint8() && validator.validate_uint8())) return;
            uint8_t level = reader.read<uint8_t>();
            if (level < 1) level = 1;
            if (level > MAX_LEVEL) level = MAX_LEVEL;
            Simulation *simulation = &client->game->simulation;
            if (!simulation->ent_exists(client->camera)) break;
            Entity &camera = simulation->get_ent(client->camera);
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
