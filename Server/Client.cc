#include <Server/Client.hh>

#include <Server/AccountStore.hh>
#include <Server/Chat.hh>
#include <Server/Game.hh>
#include <Server/Party.hh>
#include <Server/PetalTracker.hh>
#include <Server/Server.hh>
#include <Server/Spawn.hh>

#include <Helpers/UTF8.hh>

#include <Shared/Binary.hh>
#include <Shared/Config.hh>
#include <Shared/Craft.hh>
#include <Shared/StaticData.hh>

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
    for (uint32_t i = 0; i < RarityID::kNumRarities; ++i)
        writer.write<uint32_t>(client->craft_bank[i]);
    writer.write<uint32_t>(client->coins);
    client->send_packet(writer.packet, writer.at - writer.packet);
}

static void persist_craft_bank(Client *client) {
    if (client->account_name.empty()) return;
    Account *acc = AccountStore::find(client->account_name);
    if (acc == nullptr) return;
    acc->craft_bank = client->craft_bank;
    acc->coins = client->coins;
    AccountStore::save();
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
    persist_craft_bank(client);
    send_craft_update(client, CraftResultCode::kCraftBankSync, PetalID::kNone);
}

static void bank_petal(Client *client, PetalID::T id) {
    if (id == PetalID::kNone || id >= PetalID::kNumPetals) return;
    uint8_t rarity = PETAL_DATA[id].rarity;
    if (rarity >= RarityID::kNumRarities) return;
    if (client->craft_bank[rarity] < 1000000)
        client->craft_bank[rarity] += 1;
    persist_craft_bank(client);
    send_craft_update(client, CraftResultCode::kCraftBankSync, PetalID::kNone);
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
    uint8_t rarity = PETAL_DATA[id].rarity;
    if (rarity < RarityID::kNumRarities && client->craft_bank[rarity] < 1000000)
        client->craft_bank[rarity] += 1;
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
    return simulation->ent_exists(camera) 
    && simulation->ent_exists(simulation->get_ent(camera).get_player());
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
            // Temporarily accept recent hashes so cached tabs can get in;
            // new deploys still ship VERSION_HASH and cache-busted assets.
            uint8_t ok = (got == VERSION_HASH)
                || (got == 19235684321333ull)
                || (got == 19235684321332ull)
                || (got == 19235684321331ull)
                || (got == 19235684321330ull);
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
            Party::clear_corpse(client);
            if (client->game == nullptr)
                Server::games[map_id].add_client(client);
            else if (client->game->map_id != map_id)
                client->game->transfer_client(client, &Server::games[map_id]);
            Simulation *simulation = &client->game->simulation;
            Entity &camera = simulation->get_ent(client->camera);
            Entity &player = alloc_player(simulation, camera.get_team());
            player_spawn(simulation, camera, player);
            if (name.empty() && !client->account_name.empty()) name = client->account_name;
            player.set_name(name);
            Party::apply_team(client);
            Party::send_update(client);
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
            for (uint32_t i = 0; i < RarityID::kNumRarities; ++i)
                writer.write<uint32_t>(acc == nullptr ? 0 : acc->craft_bank[i]);
            writer.write<uint32_t>(acc == nullptr ? 0 : acc->coins);
            client->send_packet(writer.packet, writer.at - writer.packet);
            if (result == AccountResultCode::kAccountOk)
                send_craft_update(client, CraftResultCode::kCraftBankSync, PetalID::kNone);
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
                    persist_craft_bank(client);
                    send_craft_update(client, CraftResultCode::kCraftInvFull, got);
                } else {
                    persist_craft_bank(client);
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
                    persist_craft_bank(client);
                    send_craft_update(client, CraftResultCode::kCraftInvFull, id);
                } else {
                    persist_craft_bank(client);
                    send_craft_update(client, CraftResultCode::kCraftBought, id);
                }
                break;
            }
            uint8_t rarity = arg;
            if (!Craft::can_upgrade(rarity)) {
                send_craft_update(client, CraftResultCode::kCraftNoMats, PetalID::kNone);
                break;
            }
            if (mode == CraftMode::kCraftExchange) {
                uint32_t cost = Craft::exchange_cost(rarity);
                if (client->craft_bank[rarity] < cost) {
                    send_craft_update(client, CraftResultCode::kCraftNoMats, PetalID::kNone);
                    break;
                }
                client->craft_bank[rarity] -= cost;
                PetalID::T got = Craft::random_petal_of_rarity(rarity + 1);
                try_give_petal(client, got);
                persist_craft_bank(client);
                send_craft_update(client, CraftResultCode::kCraftExchanged, got);
            } else {
                if (client->craft_bank[rarity] < Craft::kCraftBatch) {
                    send_craft_update(client, CraftResultCode::kCraftNoMats, PetalID::kNone);
                    break;
                }
                client->craft_bank[rarity] -= Craft::kCraftBatch;
                if (frand() < Craft::success_chance(rarity)) {
                    PetalID::T got = Craft::random_petal_of_rarity(rarity + 1);
                    try_give_petal(client, got);
                    persist_craft_bank(client);
                    send_craft_update(client, CraftResultCode::kCraftWin, got);
                } else {
                    persist_craft_bank(client);
                    send_craft_update(client, CraftResultCode::kCraftMiss, PetalID::kNone);
                }
            }
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
            for (uint32_t i = 0; i < RarityID::kNumRarities; ++i)
                if (client->check_invalid(validator.validate_uint32())) return;
            if (client->check_invalid(validator.validate_uint32())) return; // coins
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
            for (uint32_t i = 0; i < RarityID::kNumRarities; ++i)
                client->craft_bank[i] = reader.read<uint32_t>();
            client->coins = reader.read<uint32_t>();
            send_craft_update(client, CraftResultCode::kCraftBankSync, PetalID::kNone);
            break;
        }
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