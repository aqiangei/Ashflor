#include <Server/Client.hh>
#include <Server/DevCheats.hh>

#include <Server/AccountStore.hh>
#include <Server/Chat.hh>
#include <Server/EntityFunctions.hh>
#include <Server/Game.hh>
#include <Server/FeedbackStore.hh>
#include <Server/LeaderboardStore.hh>
#include <Server/Guild.hh>
#include <Server/Party.hh>
#include <Server/PetalTracker.hh>
#include <Server/WorldEvent.hh>
#include <Server/WorldExplore.hh>
#include <Server/RateLimit.hh>
#include <Server/Server.hh>
#include <Server/Spawn.hh>

#include <Helpers/UTF8.hh>

#include <Shared/Binary.hh>
#include <Shared/Config.hh>
#include <Shared/Craft.hh>
#include <Shared/Map.hh>
#include <Shared/Recipe.hh>
#include <Shared/StaticData.hh>
#include <Shared/StaticDefinitions.hh>
#include <Shared/Talent.hh>

#include <Helpers/Math.hh>

#include <algorithm>
#include <array>
#include <chrono>
#include <iostream>

constexpr std::array<uint32_t, RarityID::kNumRarities> RARITY_TO_XP = {
    2, 10, 50, 200, 1000, 5000, 0, 12000, 30000
};

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

static void write_sparse_petal_levels(Writer &writer,
    std::array<uint32_t, PetalID::kNumPetals> const &levels) {
    uint16_t n_levels = 0;
    for (uint32_t i = 1; i < PetalID::kNumPetals; ++i)
        if (levels[i] > 0) ++n_levels;
    writer.write<uint16_t>(n_levels);
    for (uint32_t i = 1; i < PetalID::kNumPetals; ++i) {
        if (levels[i] == 0) continue;
        writer.write<PetalID::T>((PetalID::T) i);
        writer.write<uint32_t>(levels[i]);
    }
}

static void send_craft_update(Client *client, uint8_t result, PetalID::T petal,
                              uint16_t amount = 0) {
    Writer writer(Server::OUTGOING_PACKET);
    writer.write<uint8_t>(Clientbound::kCraftUpdate);
    writer.write<uint8_t>(result);
    writer.write<PetalID::T>(petal);
    // Win/buy count (or miss loss). 0 for bank sync / errors.
    writer.write<uint16_t>(amount);
    for (uint32_t i = 0; i < PetalID::kNumPetals; ++i)
        writer.write<uint32_t>(client->craft_bank[i]);
    writer.write<uint32_t>(client->coins);
    write_sparse_petal_levels(writer, client->petal_levels);
    client->send_packet(writer.packet, writer.at - writer.packet);
}

static void send_talent_update(Client *client) {
    Writer writer(Server::OUTGOING_PACKET);
    writer.write<uint8_t>(Clientbound::kTalentUpdate);
    for (uint8_t i = 0; i < TalentID::kNum; ++i)
        writer.write<uint8_t>(client->talent_ranks[i]);
    client->send_packet(writer.packet, writer.at - writer.packet);
}

static uint32_t client_slot_count(Client *client) {
    if (client == nullptr) return Talent::kBaseSlots;
    return Talent::slot_count_from_ranks(client->talent_ranks);
}

static void send_loadout_preset_update(Client *client, uint8_t result, uint8_t index = 0) {
    Writer writer(Server::OUTGOING_PACKET);
    writer.write<uint8_t>(Clientbound::kLoadoutPresetSync);
    writer.write<uint8_t>(result);
    writer.write<uint8_t>(index);
    for (LoadoutPreset const &preset : client->loadout_presets) {
        writer.write<uint8_t>(preset.slot_count);
        for (PetalID::T id : preset.petals)
            writer.write<PetalID::T>(id);
        for (uint8_t i = 0; i < TalentID::kNum; ++i)
            writer.write<uint8_t>(preset.talent_ranks[i]);
    }
    // Append to the legacy prefix so cached clients can still read primaries.
    writer.write<uint8_t>(1);
    for (LoadoutPreset const &preset : client->loadout_presets)
        for (PetalID::T id : preset.secondary) writer.write<PetalID::T>(id);
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
    uint32_t lv = camera.get_respawn_level();
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

// Unique craft / Unique level-up only: 全服播报 + 排行榜「高级合成」
static constexpr uint8_t kHighCraftRarity = RarityID::kUnique;

static char const *rarity_zh(uint8_t rarity) {
    static char const *const names[RarityID::kNumRarities] = {
        "普通", "罕见", "稀有", "史诗", "传奇", "神话", "究极", "超凡", "唯一"
    };
    if (rarity >= RarityID::kNumRarities) return "?";
    return names[rarity];
}

static std::string craft_announce_name(Client *client) {
    if (client == nullptr) return "无名";
    if (client->game != nullptr) {
        Simulation *sim = &client->game->simulation;
        if (sim->ent_exists(client->camera)) {
            Entity &camera = sim->get_ent(client->camera);
            if (sim->ent_alive(camera.get_player())) {
                std::string n = sim->get_ent(camera.get_player()).get_name();
                if (!n.empty()) return n;
            }
        }
    }
    if (!client->account_name.empty()) return client->account_name;
    if (!client->play_name.empty()) return client->play_name;
    return "无名";
}

static void credit_high_craft(Client *client, PetalID::T got) {
    if (client == nullptr || got == PetalID::kNone || got >= PetalID::kNumPetals) return;
    uint8_t r = PETAL_DATA[got].rarity;
    if (r < kHighCraftRarity) return;
    if (client->stat_crafts < 4000000000u) ++client->stat_crafts;
    char const *pname = PETAL_DATA[got].name;
    if (pname == nullptr || pname[0] == 0) pname = "花瓣";
    // whisper_to "craft:N" → client paints the line in that rarity color
    std::string who = craft_announce_name(client);
    std::string msg = who + " 合成了【" + rarity_zh(r) + "】" + pname + "！";
    uint8_t mid = Chat::MAP_UNKNOWN;
    if (client->game != nullptr) mid = client->game->map_id;
    Chat::broadcast_server("系统", msg, "craft:" + std::to_string((unsigned) r),
                           Chat::kCraft, who, mid);
}

static void credit_unique_level(Client *client, PetalID::T id, uint32_t lv) {
    if (client == nullptr || id == PetalID::kNone || id >= PetalID::kNumPetals) return;
    if (PETAL_DATA[id].rarity < RarityID::kUnique) return;
    if (client->stat_crafts < 4000000000u) ++client->stat_crafts;
    char const *pname = PETAL_DATA[id].name;
    if (pname == nullptr || pname[0] == 0) pname = "花瓣";
    std::string who = craft_announce_name(client);
    std::string msg = who + " 将【唯一】" + pname + " 升级到 Lv." + std::to_string(lv) + "！";
    uint8_t mid = Chat::MAP_UNKNOWN;
    if (client->game != nullptr) mid = client->game->map_id;
    Chat::broadcast_server("系统", msg,
        "craft:" + std::to_string((unsigned) RarityID::kUnique),
        Chat::kCraft, who, mid);
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

// Send the multi-category leaderboard board (top rows + the player's own row)
static void send_leaderboard_stats(Client *client) {
    if (client == nullptr) return;
    client_flush_stats(client);
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
    uint8_t max_r = Talent::max_rank_for(talent_id, level);
    uint8_t &rank = client->talent_ranks[talent_id];
    if (rank >= max_r) return 0;
    uint8_t next = (uint8_t) (rank + 1);
    uint32_t free = Talent::points_free(level, client->talent_ranks);
    if (free < Talent::rank_cost(next)) return 0;
    rank = next;
    return 1;
}

static uint8_t try_refund_talent(Client *client, uint8_t talent_id) {
    if (client == nullptr || client->game == nullptr
        || !client->game->simulation.ent_exists(client->camera)) return 0;
    if (talent_id == 0xFF) {
        for (uint8_t i = 0; i < TalentID::kNum; ++i)
            client->talent_ranks[i] = 0;
        return 1;
    }
    if (talent_id >= TalentID::kNum) return 0;
    if (client->talent_ranks[talent_id] == 0) return 0;
    --client->talent_ranks[talent_id];
    return 1;
}

// Jump / rollback one branch to an absolute rank (tree node click).
static uint8_t try_set_talent_rank(Client *client, uint8_t talent_id, uint8_t target) {
    if (client == nullptr || client->game == nullptr
        || !client->game->simulation.ent_exists(client->camera)) return 0;
    if (talent_id >= TalentID::kNum) return 0;
    uint32_t level = client_level(client);
    if (!Talent::can_set_rank(talent_id, target, level, client->talent_ranks))
        return 0;
    uint8_t &rank = client->talent_ranks[talent_id];
    if (rank == target) return 0;
    rank = target;
    return 1;
}

// Memory snapshot only ??never sync-write accounts.dat on the tick hot path.
// Periodic Server flush (~5s) and disconnect/drain call AccountStore::save().
static void persist_account(Client *client) {
    AccountStore::capture_nosave(client);
}

void client_sync_craft(Client *client) {
    if (client == nullptr) return;
    send_craft_update(client, CraftResultCode::kCraftBankSync, PetalID::kNone);
}

void client_clear_life_stats(Client *client) {
    if (client == nullptr) return;
    client->life_mob_kills.fill(0);
    client->life_petal_picks.fill(0);
}

void client_credit_mob_kill(Client *client, MobID::T id) {
    if (client == nullptr || id >= MobID::kNumMobs) return;
    if (client->life_mob_kills[id] < 65535)
        ++client->life_mob_kills[id];
}

void client_credit_petal_pick(Client *client, PetalID::T id) {
    if (client == nullptr || id == PetalID::kNone || id >= PetalID::kNumPetals) return;
    if (client->life_petal_picks[id] < 65535)
        ++client->life_petal_picks[id];
}

void client_send_life_summary(Client *client) {
    if (client == nullptr) return;
    Writer writer(Server::OUTGOING_PACKET);
    writer.write<uint8_t>(Clientbound::kLifeSummary);
    uint8_t mob_n = 0;
    for (uint32_t i = 0; i < MobID::kNumMobs; ++i)
        if (client->life_mob_kills[i] > 0) ++mob_n;
    writer.write<uint8_t>(mob_n);
    for (uint32_t i = 0; i < MobID::kNumMobs; ++i) {
        if (client->life_mob_kills[i] == 0) continue;
        writer.write<uint8_t>((uint8_t) i);
        writer.write<uint16_t>(client->life_mob_kills[i]);
    }
    uint16_t petal_n = 0;
    for (uint32_t i = 0; i < PetalID::kNumPetals; ++i)
        if (client->life_petal_picks[i] > 0) ++petal_n;
    writer.write<uint16_t>(petal_n);
    for (uint32_t i = 0; i < PetalID::kNumPetals; ++i) {
        if (client->life_petal_picks[i] == 0) continue;
        writer.write<PetalID::T>((PetalID::T) i);
        writer.write<uint16_t>(client->life_petal_picks[i]);
    }
    client->send_packet(writer.packet, writer.at - writer.packet);
}

void client_award_coins(Client *client, uint32_t amount) {
    if (client == nullptr || amount == 0) return;
    uint32_t given = 0;
    for (uint32_t i = 0; i < amount; ++i) {
        if (!try_give_petal(client, PetalID::kCoin)) break;
        ++given;
    }
    for (uint32_t i = 0; i < given; ++i)
        client_credit_petal_pick(client, PetalID::kCoin);
    persist_account(client);
    send_craft_update(client, CraftResultCode::kCraftBankSync, PetalID::kCoin);
}

uint8_t client_award_petal(Client *client, PetalID::T id) {
    if (client == nullptr || id == PetalID::kNone) return 0;
    if (!try_give_petal(client, id)) return 0;
    client_credit_petal_pick(client, id);
    persist_account(client);
    send_craft_update(client, CraftResultCode::kCraftBankSync, id);
    return 1;
}

void client_send_loot_toast(Client *client, PetalID::T id, uint16_t amount,
                            std::string const &title) {
    if (client == nullptr) return;
    Writer writer(Server::OUTGOING_PACKET);
    writer.write<uint8_t>(Clientbound::kLootToast);
    writer.write<PetalID::T>(id);
    writer.write<uint16_t>(amount);
    writer.write<std::string>(title);
    client->send_packet(writer.packet, writer.at - writer.packet);
}

// Secondary + pure bag slots (not primary): [loadout, loadout+MAX).
// Dead players keep inventory on the camera ??same as bank withdraw.
static uint32_t count_bag_mats(Client *client, PetalID::T id) {
    if (client == nullptr || id == PetalID::kNone || id >= PetalID::kNumPetals) return 0;
    if (client->game == nullptr || !client->game->simulation.ent_exists(client->camera)) return 0;
    Simulation *simulation = &client->game->simulation;
    Entity &camera = simulation->get_ent(client->camera);
    Entity *player = client->alive() ? &simulation->get_ent(camera.get_player()) : nullptr;
    uint32_t loadout = player != nullptr ? player->get_loadout_count()
        : (uint32_t) client_slot_count(client);
    uint32_t end = loadout + MAX_SLOT_COUNT;
    uint32_t n = 0;
    for (uint32_t i = loadout; i < end; ++i) {
        PetalID::T slot = player != nullptr ? player->get_loadout_ids(i)
            : camera.get_inventory(i);
        if (slot == id) ++n;
    }
    return n;
}

static uint32_t craft_owned(Client *client, PetalID::T id) {
    return count_bag_mats(client, id) + client->craft_bank[id];
}

// Consume from backpack (secondary + pure bag), high index first.
static uint8_t consume_craft_mats(Client *client, PetalID::T id, uint32_t amount) {
    if (amount == 0) return 1;
    if (id == PetalID::kNone || id >= PetalID::kNumPetals) return 0;
    if (craft_owned(client, id) < amount) return 0;
    if (client->game == nullptr || !client->game->simulation.ent_exists(client->camera)) return 0;
    Simulation *simulation = &client->game->simulation;
    Entity &camera = simulation->get_ent(client->camera);
    Entity *player = client->alive() ? &simulation->get_ent(camera.get_player()) : nullptr;
    uint32_t loadout = player != nullptr ? player->get_loadout_count()
        : (uint32_t) client_slot_count(client);
    uint32_t end = loadout + MAX_SLOT_COUNT;
    for (uint32_t i = end; i-- > loadout && amount > 0;) {
        PetalID::T cur = player != nullptr ? player->get_loadout_ids(i)
            : camera.get_inventory(i);
        if (cur != id) continue;
        if (player != nullptr) player->set_loadout_ids(i, PetalID::kNone);
        else camera.set_inventory(i, PetalID::kNone);
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
    if (client->game == nullptr || !client->game->simulation.ent_exists(client->camera)) return 0;
    Simulation *simulation = &client->game->simulation;
    Entity &camera = simulation->get_ent(client->camera);
    Entity *player = client->alive() ? &simulation->get_ent(camera.get_player()) : nullptr;
    uint32_t loadout = player != nullptr ? player->get_loadout_count()
        : (uint32_t) client_slot_count(client);
    auto place = [&](uint32_t i) -> uint8_t {
        PetalID::T cur = player != nullptr ? player->get_loadout_ids(i)
            : camera.get_inventory(i);
        if (cur != PetalID::kNone) return 0;
        if (player != nullptr) player->set_loadout_ids(i, id);
        else camera.set_inventory(i, id);
        PetalTracker::add_petal(simulation, id);
        return 1;
    };
    for (uint32_t i = loadout; i < loadout + MAX_SLOT_COUNT; ++i)
        if (place(i)) return 1;
    // Primary after backpack/secondary — coins equip like any other petal.
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
    client->last_rx_ms = (uint64_t) std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
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
        case Serverbound::kTrainingReset:
            TrainingDummy::reset(client);
            break;
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
            bool has_view = validator.at < validator.end;
            if (has_view && client->check_invalid(
                validator.validate_uint16() && validator.validate_uint16())) return;
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
            if (has_view) {
                client->view_half_w = reader.read<uint16_t>();
                client->view_half_h = reader.read<uint16_t>();
            }
            break;
        }
        case Serverbound::kClientSpawn: {
            // Client thinks it is dead but server still has a live flower
            // (desync / watchdog). Flush loadout onto the camera FIRST — while
            // alive the camera inventory is empty, so deleting without a flush
            // respawns into all-Basics and can wipe the account on capture.
            if (client->alive() && client->game != nullptr) {
                Simulation *stale_sim = &client->game->simulation;
                Entity &stale_cam = stale_sim->get_ent(client->camera);
                Entity &stale_player = stale_sim->get_ent(stale_cam.get_player());
                flush_flower_loadout_to_camera(stale_sim, stale_cam, stale_player);
                AccountStore::capture_nosave(client);
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
            uint8_t map_id = clamp_to_open_map(reader.read<uint8_t>());
            uint8_t guest_id_reassigned = 0;
            if (!client->account_name.empty()) {
                name = client->account_name;
            } else {
                // Guests: fixed G#XXXXXX only — ignore free-form names.
                // Collisions (same localStorage id in two tabs, ghost socket still
                // holding play_name) must remint — never reject with NameTaken.
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
                auto mint_unique_guest = [client]() -> std::string {
                    static uint32_t next_guest = 100000;
                    char buf[16];
                    static char const alphabet[] = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";
                    std::string out;
                    do {
                        uint32_t n = next_guest++;
                        buf[0] = 'G'; buf[1] = '#';
                        for (int i = 0; i < 6; ++i) {
                            buf[2 + i] = alphabet[n % 32];
                            n /= 32;
                        }
                        buf[8] = 0;
                        out = buf;
                    } while (LeaderboardStore::name_taken(out, client)
                        || AccountStore::name_reserved_for_guest(out));
                    return out;
                };
                if (!guest_tag_ok(name)
                    || AccountStore::name_reserved_for_guest(name)
                    || LeaderboardStore::name_taken(name, client)) {
                    std::string const prev = name;
                    name = mint_unique_guest();
                    guest_id_reassigned = 1;
                    if (!prev.empty() && prev != name)
                        std::cout << "[spawn] guest id " << prev << " → " << name << std::endl;
                }
            }
            // Accounts only: unique play names (guests already reminted above).
            if (!client->account_name.empty() && !name.empty()
                && LeaderboardStore::name_taken(name, client)) {
                std::cout << "[spawn] name taken: " << name << std::endl;
                Writer writer(Server::OUTGOING_PACKET);
                writer.write<uint8_t>(Clientbound::kSpawnResult);
                writer.write<uint8_t>(SpawnResultCode::kSpawnNameTaken);
                client->send_packet(writer.packet, writer.at - writer.packet);
                break;
            }
            // Continue after death ??near death site on same map; first enter / other map ??hub
            uint8_t near_death = client->has_death_pos && client->death_map_id == map_id;
            float near_x = client->death_x;
            float near_y = client->death_y;
            client->has_death_pos = 0;
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
            if (near_death) {
                float sx = near_x + (frand() - 0.5f) * 320.f;
                float sy = near_y + (frand() - 0.5f) * 320.f;
                Map::resolve_circle_terrain(simulation->map_id, sx, sy, BASE_FLOWER_RADIUS + 8);
                sx = fclamp(sx, BASE_FLOWER_RADIUS, ARENA_WIDTH - BASE_FLOWER_RADIUS);
                sy = fclamp(sy, BASE_FLOWER_RADIUS, ARENA_HEIGHT - BASE_FLOWER_RADIUS);
                player_spawn_at(simulation, camera, player, sx, sy);
            } else {
                player_spawn(simulation, camera, player);
            }
            player.set_name(name);
            player.set_guild(client->guild_name);
            client->play_name = name;
            client_clear_life_stats(client);
            if (!name.empty())
                LeaderboardStore::record(name, player.get_score(), player.get_color());
            if (guest_id_reassigned) {
                Writer writer(Server::OUTGOING_PACKET);
                writer.write<uint8_t>(Clientbound::kSpawnResult);
                writer.write<uint8_t>(SpawnResultCode::kSpawnGuestIdAssigned);
                writer.write<std::string>(name);
                client->send_packet(writer.packet, writer.at - writer.packet);
            }
            Party::apply_team(client);
            Party::send_update(client);
            Guild::send_update(client);
            WorldExplore::on_player_spawned(client);
            WorldEvent::clear_client(client);
            // Re-push bank + Unique levels after every spawn (WS/deploy drop
            // clears client petal_levels; login-only sync was easy to miss).
            client_sync_craft(client);
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
            uint8_t result = RateLimit::precheck_auth(client->remote_ip, username, mode);
            if (result == AccountResultCode::kAccountOk) {
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
            }
            if (result == AccountResultCode::kAccountOk)
                RateLimit::note_auth_ok(client->remote_ip, username);
            else
                RateLimit::note_auth_fail(client->remote_ip, username, mode);
            // Local QA kit must land before AccountResult so the client snapshot is full.
            if (result == AccountResultCode::kAccountOk)
                DevCheats::apply_kit(client);
            Writer writer(Server::OUTGOING_PACKET);
            writer.write<uint8_t>(Clientbound::kAccountResult);
            writer.write<uint8_t>(result);
            Account *acc = AccountStore::find(client->account_name);
            writer.write<uint32_t>(acc == nullptr ? 1u : acc->level);
            for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i)
                writer.write<PetalID::T>(acc == nullptr ? PetalID::kNone : acc->petals[i]);
            for (uint32_t i = 0; i < PetalID::kNumPetals; ++i)
                writer.write<uint32_t>(acc == nullptr ? 0 : acc->craft_bank[i]);
            writer.write<uint32_t>(acc == nullptr ? 0 : acc->coins);
            // Unique levels in AccountResult so UI badges survive reconnect
            // without waiting for a later craft sync (deploy / WS drop).
            if (acc != nullptr)
                write_sparse_petal_levels(writer, acc->petal_levels);
            else {
                static std::array<uint32_t, PetalID::kNumPetals> const kEmptyLevels = {};
                write_sparse_petal_levels(writer, kEmptyLevels);
            }
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
        case Serverbound::kFeedbackPost: {
            if (client->check_invalid(validator.validate_string(960))) return;
            std::string text;
            reader.read<std::string>(text);
            uint32_t id = 0;
            uint8_t result = FeedbackStore::post(client->account_name, text, id, client->remote_ip);
            Writer writer(Server::OUTGOING_PACKET);
            writer.write<uint8_t>(Clientbound::kFeedbackResult);
            writer.write<uint8_t>(result);
            writer.write<uint32_t>(id);
            client->send_packet(writer.packet, writer.at - writer.packet);
            break;
        }
        case Serverbound::kFeedbackSetResolved: {
            if (client->check_invalid(validator.validate_uint32() && validator.validate_uint8())) return;
            uint32_t id = reader.read<uint32_t>();
            uint8_t resolved = reader.read<uint8_t>() ? 1 : 0;
            uint8_t result = FeedbackStore::set_resolved(client->account_name, id, resolved);
            Writer writer(Server::OUTGOING_PACKET);
            writer.write<uint8_t>(Clientbound::kFeedbackResolveResult);
            writer.write<uint8_t>(result);
            writer.write<uint32_t>(id);
            writer.write<uint8_t>(result == kFeedbackPosted ? resolved : (uint8_t) 0);
            client->send_packet(writer.packet, writer.at - writer.packet);
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
            if (try_spend_talent(client, talent_id))
                AccountStore::capture_nosave(client);
            send_talent_update(client);
            break;
        }
        case Serverbound::kTalentRefund: {
            if (client->check_invalid(validator.validate_uint8())) return;
            uint8_t talent_id = reader.read<uint8_t>();
            if (try_refund_talent(client, talent_id))
                AccountStore::capture_nosave(client);
            send_talent_update(client);
            break;
        }
        case Serverbound::kTalentSetRank: {
            if (client->check_invalid(validator.validate_uint8())) return;
            if (client->check_invalid(validator.validate_uint8())) return;
            uint8_t talent_id = reader.read<uint8_t>();
            uint8_t target = reader.read<uint8_t>();
            if (try_set_talent_rank(client, talent_id, target))
                AccountStore::capture_nosave(client);
            send_talent_update(client);
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
            // Tracker: leave loadout ??world drop (alloc_drop adds)
            PetalTracker::remove_petal(simulation, old_id);
            Entity &drop = alloc_drop(simulation, old_id, NULL_ENTITY);
            drop.set_x(player.get_x());
            drop.set_y(player.get_y());
            drop.velocity.unit_normal(frand() * 2 * (float) M_PI).set_magnitude(40);
            // Brief global immunity so dropper does not instantly re-pick
            drop.immunity_ticks = TPS;
            AccountStore::capture_nosave(client);
            break;
        }
        case Serverbound::kPetalCraft: {
            // Allow death-screen craft (inventory lives on camera while down)
            if (client->game == nullptr
                || !client->game->simulation.ent_exists(client->camera)) break;
            if (client->check_invalid(validator.validate_uint8() && validator.validate_uint16())) return;
            uint8_t mode = reader.read<uint8_t>();
            uint16_t arg = reader.read<uint16_t>();
            if (mode == CraftMode::kCraftBuyAnnounce) {
                (void) arg;
                if (client->check_invalid(validator.validate_string(MAX_CHAT_LENGTH))) return;
                std::string text;
                reader.read<std::string>(text);
                // Trim
                while (!text.empty() && (text.back() == ' ' || text.back() == '\t'
                    || text.back() == '\n' || text.back() == '\r'))
                    text.pop_back();
                size_t start = 0;
                while (start < text.size() && (text[start] == ' ' || text[start] == '\t'))
                    ++start;
                if (start > 0) text = text.substr(start);
                if (text.empty()) {
                    send_craft_update(client, CraftResultCode::kCraftNoMats, PetalID::kNone);
                    break;
                }
                uint32_t cost = Craft::kShopAnnounceCost;
                if (craft_owned(client, PetalID::kCoin) < cost) {
                    send_craft_update(client, CraftResultCode::kCraftNoCoins, PetalID::kNone);
                    break;
                }
                if (!consume_craft_mats(client, PetalID::kCoin, cost)) {
                    send_craft_update(client, CraftResultCode::kCraftNoCoins, PetalID::kNone);
                    break;
                }
                std::string who = craft_announce_name(client);
                // Body always carries the payer name so chat + banner stay clear
                // even if the title line is missed (small UI / faded alpha).
                std::string body = who + "：" + text;
                uint8_t mid = Chat::MAP_UNKNOWN;
                if (client->game != nullptr) mid = client->game->map_id;
                Chat::broadcast_server_banner(who, body, who, mid);
                persist_account(client);
                send_craft_update(client, CraftResultCode::kCraftBought, PetalID::kNone);
                break;
            }
            if (mode == CraftMode::kCraftBuyRarity) {
                uint8_t rarity = (uint8_t) arg;
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
                    // Refund materials path already banked; still spent coins ??OK (got material)
                    persist_account(client);
                    send_craft_update(client, CraftResultCode::kCraftInvFull, got);
                } else {
                    persist_account(client);
                    send_craft_update(client, CraftResultCode::kCraftBought, got, 1);
                }
                break;
            }
            if (mode == CraftMode::kCraftBuySpecial) {
                PetalID::T id = (PetalID::T) arg;
                if (!Craft::is_shop_purchasable(id)) {
                    send_craft_update(client, CraftResultCode::kCraftNoMats, PetalID::kNone);
                    break;
                }
                uint32_t cost = Craft::shop_price(id);
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
                    send_craft_update(client, CraftResultCode::kCraftBought, id, 1);
                }
                break;
            }
            if (mode == CraftMode::kCraftRecipe) {
                uint16_t idx = (uint16_t) arg;
                if (client->check_invalid(validator.validate_uint8())) return;
                uint8_t rarity = reader.read<uint8_t>();
                if (rarity >= RarityID::kNumRarities) rarity = RarityID::kCommon;
                if (idx >= Recipe::recipe_count()) {
                    send_craft_update(client, CraftResultCode::kCraftNoMats, PetalID::kNone);
                    break;
                }
                Recipe::Def const &rec = Recipe::recipe_at(idx);
                uint8_t coin_shop = 0;
                for (uint8_t i = 0; i < rec.ingredient_count; ++i)
                    if (rec.ingredients[i].id == PetalID::kCoin) { coin_shop = 1; break; }
                // Coin-shop rows: baked IDs only (no rarity ladder exploit).
                PetalID::T result = coin_shop
                    ? rec.result
                    : Craft::petal_at_rarity(rec.result, rarity);
                if (result == PetalID::kNone || result >= PetalID::kNumPetals) {
                    send_craft_update(client, CraftResultCode::kCraftNoMats, PetalID::kNone);
                    break;
                }
                PetalID::T mats[Recipe::kMaxIngredients] = {};
                uint8_t ok = 1;
                for (uint8_t i = 0; i < rec.ingredient_count; ++i) {
                    mats[i] = coin_shop
                        ? rec.ingredients[i].id
                        : Craft::petal_at_rarity(rec.ingredients[i].id, rarity);
                    if (mats[i] == PetalID::kNone || mats[i] >= PetalID::kNumPetals) {
                        ok = 0;
                        break;
                    }
                }
                if (!ok) {
                    send_craft_update(client, CraftResultCode::kCraftNoMats, PetalID::kNone);
                    break;
                }
                for (uint8_t i = 0; i < rec.ingredient_count; ++i) {
                    if (craft_owned(client, mats[i]) < rec.ingredients[i].count) {
                        ok = 0;
                        break;
                    }
                }
                if (!ok) {
                    send_craft_update(client, CraftResultCode::kCraftNoMats, PetalID::kNone);
                    break;
                }
                for (uint8_t i = 0; i < rec.ingredient_count; ++i) {
                    if (!consume_craft_mats(client, mats[i], rec.ingredients[i].count)) {
                        ok = 0;
                        break;
                    }
                }
                if (!ok) {
                    send_craft_update(client, CraftResultCode::kCraftNoMats, PetalID::kNone);
                    break;
                }
                if (!try_give_petal(client, result)) {
                    persist_account(client);
                    send_craft_update(client, CraftResultCode::kCraftInvFull, result);
                } else {
                    credit_high_craft(client, result);
                    persist_account(client);
                    send_craft_update(client, CraftResultCode::kCraftBought, result, 1);
                }
                break;
            }
            PetalID::T from = (PetalID::T) arg;
            if (from == PetalID::kNone || from >= PetalID::kNumPetals
                || !Craft::can_craft_petal(from)) {
                send_craft_update(client, CraftResultCode::kCraftNoMats, PetalID::kNone);
                break;
            }
            if (Craft::can_level_petal(from)) {
                // Unique: infinite level ladder instead of a rarity rung.
                uint32_t &lv = client->petal_levels[from];
                if (mode == CraftMode::kCraftExchange) {
                    uint32_t cost = Craft::level_exchange_cost(lv);
                    if (lv >= Craft::kMaxPetalLevel || !consume_craft_mats(client, from, cost)) {
                        send_craft_update(client, CraftResultCode::kCraftNoMats, PetalID::kNone);
                        break;
                    }
                    if (client->craft_attempts < 4000000000u)
                        ++client->craft_attempts;
                    ++lv;
                    credit_unique_level(client, from, lv);
                    persist_account(client);
                    send_craft_update(client, CraftResultCode::kCraftLevelWin, from,
                        (uint16_t) lv);
                    break;
                }
                uint32_t max_batches = 1u;
                if (mode == CraftMode::kCraftGamble || mode == CraftMode::kCraftGambleAll) {
                    if (client->check_invalid(validator.validate_uint16())) return;
                    uint16_t n = reader.read<uint16_t>();
                    if (n > 0) max_batches = n;
                }
                if (mode == CraftMode::kCraftGambleAll || max_batches > 1u) {
                    uint32_t owned0 = craft_owned(client, from);
                    if (owned0 > max_batches) max_batches = owned0;
                    if (max_batches > 65535u) max_batches = 65535u;
                }
                uint32_t done = 0, wins = 0, last_lost = 0;
                while (done < max_batches && lv < Craft::kMaxPetalLevel
                    && craft_owned(client, from) >= Craft::kCraftBatch) {
                    if (!consume_craft_mats(client, from, Craft::kCraftBatch)) break;
                    ++done;
                    if (client->craft_attempts < 4000000000u)
                        ++client->craft_attempts;
                    if (frand() < Craft::level_success_chance(lv)) {
                        ++lv;
                        ++wins;
                    } else {
                        uint32_t lost = Craft::craft_fail_loss();
                        if (lost > Craft::kCraftBatch) lost = Craft::kCraftBatch;
                        last_lost = lost;
                        for (uint32_t i = lost; i < Craft::kCraftBatch; ++i)
                            try_give_petal(client, from);
                    }
                }
                persist_account(client);
                if (done == 0)
                    send_craft_update(client, CraftResultCode::kCraftNoMats, PetalID::kNone);
                else if (wins > 0) {
                    credit_unique_level(client, from, lv);
                    send_craft_update(client, CraftResultCode::kCraftLevelWin, from,
                        (uint16_t) (lv > 65535u ? 65535u : lv));
                } else {
                    uint16_t lost = (uint16_t) (last_lost > 0 ? last_lost : Craft::kCraftFailLossMin);
                    send_craft_update(client, CraftResultCode::kCraftMiss,
                        (PetalID::T) lost, lost);
                }
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
                credit_high_craft(client, got);
                persist_account(client);
                send_craft_update(client,
                    placed ? CraftResultCode::kCraftExchanged : CraftResultCode::kCraftInvFull,
                    got, placed ? 1 : 0);
                break;
            }
            uint32_t max_batches = (mode == CraftMode::kCraftGambleAll) ? 65535u : 1u;
            if (mode == CraftMode::kCraftGamble || mode == CraftMode::kCraftGambleAll) {
                if (client->check_invalid(validator.validate_uint16())) return;
                uint16_t n = reader.read<uint16_t>();
                if (n > 0) max_batches = n;
                if (max_batches > 65535u) max_batches = 65535u;
            }
            // Multi-batch / All: miss refunds 1–4 back into the pool — keep
            // rolling until fewer than a full batch remain. Each attempt loses
            // ≥1 so initial owned is a safe iteration bound.
            if (mode == CraftMode::kCraftGambleAll || max_batches > 1u) {
                uint32_t owned0 = craft_owned(client, from);
                if (owned0 > max_batches) max_batches = owned0;
                if (max_batches > 65535u) max_batches = 65535u;
            }
            uint32_t done = 0;
            uint32_t wins = 0;
            uint8_t last_placed = 0;
            PetalID::T last_got = PetalID::kNone;
            uint32_t last_lost = 0;
            while (done < max_batches && craft_owned(client, from) >= Craft::kCraftBatch) {
                if (!consume_craft_mats(client, from, Craft::kCraftBatch)) break;
                ++done;
                if (client->craft_attempts < 4000000000u)
                    ++client->craft_attempts;
                if (frand() < Craft::success_chance(rarity)) {
                    last_got = Craft::craft_result(from);
                    if (last_got == PetalID::kNone) {
                        // Should not happen for upgradeable petals; refund batch.
                        for (uint32_t i = 0; i < Craft::kCraftBatch; ++i)
                            try_give_petal(client, from);
                        continue;
                    }
                    last_placed = try_give_petal(client, last_got);
                    ++wins;
                    credit_high_craft(client, last_got);
                } else {
                    // Miss: destroy 1–4 of the batch, refund the rest.
                    uint32_t lost = Craft::craft_fail_loss();
                    if (lost > Craft::kCraftBatch) lost = Craft::kCraftBatch;
                    last_lost = lost;
                    uint32_t refund = Craft::kCraftBatch - lost;
                    for (uint32_t i = 0; i < refund; ++i)
                        try_give_petal(client, from);
                }
            }
            persist_account(client);
            if (done == 0)
                send_craft_update(client, CraftResultCode::kCraftNoMats, PetalID::kNone);
            else if (wins > 0)
                send_craft_update(client,
                    last_placed ? CraftResultCode::kCraftWin : CraftResultCode::kCraftInvFull,
                    last_got, (uint16_t) (wins > 65535u ? 65535u : wins));
            else {
                uint16_t lost = (uint16_t) (last_lost > 0 ? last_lost : Craft::kCraftFailLossMin);
                // Petal field still encodes loss for older clients; amount is canonical.
                send_craft_update(client, CraftResultCode::kCraftMiss,
                    (PetalID::T) lost, lost);
            }
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
                // Same rule as client: bag-only barred from primary; empty OK.
                if ((pos1 < primary && Recipe::is_bag_only(player.get_loadout_ids(pos2)))
                    || (pos2 < primary && Recipe::is_bag_only(player.get_loadout_ids(pos1))))
                    break;
                PetalID::T tmp = player.get_loadout_ids(pos1);
                player.set_loadout_ids(pos1, player.get_loadout_ids(pos2));
                player.set_loadout_ids(pos2, tmp);
                // Do not flush craft_bank into empty secondary/bag here — that
                // silently auto-fills other empty secondary slots on every swap.
            } else {
                uint32_t primary = client_slot_count(client);
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
                : (uint8_t) client_slot_count(client);
            std::array<PetalID::T, 2 * MAX_SLOT_COUNT> inventory = {};
            for (uint8_t i = 0; i < slots + MAX_SLOT_COUNT; ++i)
                inventory[i] = player != nullptr ? player->get_loadout_ids(i) : camera.get_inventory(i);
            auto write_inventory = [&](std::array<PetalID::T, 2 * MAX_SLOT_COUNT> const &inv) {
                for (uint8_t i = 0; i < slots + MAX_SLOT_COUNT; ++i) {
                    if (player != nullptr) player->set_loadout_ids(i, inv[i]);
                    else camera.set_inventory(i, inv[i]);
                }
            };
            auto save_preset_from_inventory = [&](uint8_t dest) {
                if (dest >= LOADOUT_PRESET_COUNT) return;
                client->loadout_presets[dest] = current_loadout_preset(inventory, slots);
            };
            if (message_type == Serverbound::kLoadoutPresetSave) {
                save_preset_from_inventory(index);
                client->active_loadout_preset = index;
                client->last_applied_loadout = current_loadout_preset(inventory, slots);
                AccountStore::capture_nosave(client);
                send_loadout_preset_update(client, kPresetSaved, index);
                break;
            }
            // Capture edits to the outgoing preset, irrespective of K/L bank.
            // Never overwrite K1/L1 merely because the target is in that bank.
            AccountStore::capture_nosave(client);
            LoadoutPreset &preset = client->loadout_presets[index];
            bool const empty_preset = preset.slot_count == 0;
            auto const previous = inventory;
            auto const previous_bank = client->craft_bank;
            uint8_t fallback_count = 0;
            if (!arrange_loadout_preset(inventory, slots, preset, client->craft_bank, fallback_count)) {
                send_loadout_preset_update(client, kPresetEmpty, index);
                break;
            }
            for (uint8_t i = 0; i < slots + MAX_SLOT_COUNT; ++i) {
                PetalTracker::remove_petal(simulation, previous[i]);
                PetalTracker::add_petal(simulation, inventory[i]);
            }
            write_inventory(inventory);
            TrainingDummy::reset(client);
            if (empty_preset) preset = current_loadout_preset(inventory, slots);
            client->active_loadout_preset = index;
            client->last_applied_loadout = current_loadout_preset(inventory, slots);
            // Petals only — talents stay on the shared account tree.
            AccountStore::capture_nosave(client);
            if (client->craft_bank != previous_bank)
                send_craft_update(client, CraftResultCode::kCraftBankSync, PetalID::kNone);
            send_loadout_preset_update(client,
                empty_preset ? kPresetAppliedBanked
                    : (fallback_count ? kPresetAppliedAdjusted : kPresetApplied), index);
            break;
        }
        case Serverbound::kBankWithdraw: {
            // Equip one copy from backpack stacks onto the loadout bar.
            if (client->check_invalid(validator.validate_uint16()
                && validator.validate_uint8())) return;
            PetalID::T id = reader.read<PetalID::T>();
            uint8_t want_dest = reader.read<uint8_t>();
            if (id == PetalID::kNone || id >= PetalID::kNumPetals
                || client->craft_bank[id] == 0) break;
            if (!client->game || !client->game->simulation.ent_exists(client->camera)) break;
            Simulation *simulation = &client->game->simulation;
            Entity &camera = simulation->get_ent(client->camera);
            Entity *player = client->alive() ? &simulation->get_ent(camera.get_player()) : nullptr;
            uint8_t n = player != nullptr ? player->get_loadout_count()
                : (uint8_t) client_slot_count(client);
            if (n == 0) break;
            auto slot_at = [&](uint8_t i) -> PetalID::T {
                return player != nullptr ? player->get_loadout_ids(i)
                    : camera.get_inventory(i);
            };
            auto set_slot = [&](uint8_t i, PetalID::T v) {
                if (player != nullptr) player->set_loadout_ids(i, v);
                else camera.set_inventory(i, v);
            };
            if (Recipe::is_bag_only(id)) break;
            uint8_t lim = (uint8_t) (n + MAX_SLOT_COUNT);
            if (lim > 2 * MAX_SLOT_COUNT) lim = 2 * MAX_SLOT_COUNT;
            uint8_t dest = want_dest;
            if (dest >= lim) {
                // Auto (tap): empty → Basic → empty secondary. Never displace.
                dest = (uint8_t) -1;
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
            }
            // Explicit dest from bag drag: empty / Basic / replace (old → bank).
            if (dest == (uint8_t) -1 || dest >= lim) break;
            if (dest < n && Recipe::is_bag_only(id)) break;
            PetalID::T old = slot_at(dest);
            if (old == PetalID::kBasic) {
                PetalTracker::remove_petal(simulation, old);
            } else if (old != PetalID::kNone && old < PetalID::kNumPetals) {
                // At stack cap: still clear the displaced petal off the bar.
                PetalTracker::remove_petal(simulation, old);
                uint32_t cap = Recipe::bank_stack_cap(old);
                if (client->craft_bank[old] < cap) ++client->craft_bank[old];
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
                : (uint8_t) client_slot_count(client);
            if (n == 0 || pos >= 2 * n) break;
            PetalID::T id = player != nullptr ? player->get_loadout_ids(pos)
                : camera.get_inventory(pos);
            if (id == PetalID::kNone || id >= PetalID::kNumPetals) break;
            // Always unequip. At stack cap, clear the slot without bumping bank
            // (coins/petals maxed must not freeze the loadout bar).
            uint32_t cap = Recipe::bank_stack_cap(id);
            if (client->craft_bank[id] < cap) ++client->craft_bank[id];
            PetalTracker::remove_petal(simulation, id);
            if (player != nullptr) player->set_loadout_ids(pos, PetalID::kNone);
            else camera.set_inventory(pos, PetalID::kNone);
            AccountStore::capture_nosave(client);
            send_craft_update(client, CraftResultCode::kCraftBankSync, id);
            break;
        }
        case Serverbound::kChat: {
            // Death screen may still chat (camera stays in map); soft-leave has no game.
            if (client->game == nullptr || !client->game->simulation.ent_exists(client->camera))
                break;
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
            // System is server-only; Server (全服) is a valid player channel.
            if (channel == ChatChannel::kChatSystem
                || channel > ChatChannel::kChatServer)
                channel = ChatChannel::kChatNearby;
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
        case Serverbound::kGuildSetAnno: {
            if (client->check_invalid(validator.validate_string(Guild::kAnnoMax))) return;
            std::string text;
            reader.read<std::string>(text);
            Guild::set_announcement(client, text);
            break;
        }
        case Serverbound::kClientLeave: {
            // Return to title without closing the WebSocket
            if (client->game == nullptr) break;
            Simulation *simulation = &client->game->simulation;
            if (!simulation->ent_exists(client->camera)) break;
            Entity &camera = simulation->get_ent(client->camera);
            client->has_death_pos = 0;
            Party::clear_corpse(client);
            // Capture while the flower is still alive (copy_progress prefers
            // flower loadout). Deleting first then capturing used to wipe
            // accounts because camera inventory is empty mid-life.
            if (simulation->ent_alive(camera.get_player())) {
                Entity &player = simulation->get_ent(camera.get_player());
                // Title return is not a disconnect — keep memory dirty; ~15s
                // flush / WS close still sync-write. Avoids hitch on every leave.
                AccountStore::capture_nosave(client);
                BitMath::set(player.flags, EntityFlags::kNoDrops);
                simulation->request_delete(player.id);
            } else {
                AccountStore::capture_nosave(client);
            }
            // Leaving the match always turns drop-bonus off (must re-enable next spawn).
            if (client->bonus_enabled) {
                client->bonus_enabled = 0;
                send_boost_update(client);
            }
            Party::leave(client);
            client->play_name.clear();
            // Next spawn (esp. another map) must be a full snapshot, not a
            // stale in_view delta from this instance.
            client->in_view.clear();
            client->seen_arena = 0;
            Party::send_update(client);
            break;
        }
        case Serverbound::kAccountLogout: {
            if (client->account_name.empty()) break;
            // Persist from the live flower first; never delete the account row.
            AccountStore::capture(client);
            if (client->alive()) {
                Simulation *simulation = &client->game->simulation;
                Entity &camera = simulation->get_ent(client->camera);
                BitMath::set(simulation->get_ent(camera.get_player()).flags, EntityFlags::kNoDrops);
                simulation->request_delete(camera.get_player());
            }
            client->bonus_enabled = 0;
            AccountStore::logout(client, 1);
            client->craft_bank = {};
            client->coins = 0;
            client->craft_attempts = 0;
            client->petal_levels = {};
            for (uint8_t i = 0; i < TalentID::kNum; ++i)
                client->talent_ranks[i] = 0;
            client->loadout_presets = {};
            client->active_loadout_preset = LOADOUT_PRESET_COUNT;
            Party::leave(client);
            Guild::leave(client);
            Writer writer(Server::OUTGOING_PACKET);
            writer.write<uint8_t>(Clientbound::kAccountResult);
            writer.write<uint8_t>(AccountResultCode::kAccountLoggedOut);
            // Must match login AccountResult: level is uint32_t (varint).
            writer.write<uint32_t>(1);
            for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i)
                writer.write<PetalID::T>(PetalID::kNone);
            for (uint32_t i = 0; i < PetalID::kNumPetals; ++i)
                writer.write<uint32_t>(0);
            writer.write<uint32_t>(0);
            {
                static std::array<uint32_t, PetalID::kNumPetals> const kEmptyLevels = {};
                write_sparse_petal_levels(writer, kEmptyLevels);
            }
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
                    client->pending_score = 0;
                    for (uint8_t ti = 0; ti < TalentID::kNum; ++ti)
                        client->talent_ranks[ti] = 0;
                    uint32_t slots = client_slot_count(client);
                    for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i) {
                        PetalID::T id = i < slots ? PetalID::kBasic : PetalID::kNone;
                        camera.set_inventory(i, id);
                        PetalTracker::add_petal(simulation, id);
                    }
                }
                client->craft_bank.fill(0);
                client->coins = 0;
                client->petal_levels = {};
                for (uint8_t i = 0; i < TalentID::kNum; ++i)
                    client->talent_ranks[i] = 0;
                send_craft_update(client, CraftResultCode::kCraftBankSync, PetalID::kNone);
                send_talent_update(client);
                break;
            }
            // level + score + inventory
            if (client->check_invalid(validator.validate_uint32())) return; // level
            if (client->check_invalid(validator.validate_uint32())) return; // score
            for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i)
                if (client->check_invalid(validator.validate_uint16())) return;
            // Trailing: coins + talents (+ optional sparse Unique levels).
            uint32_t bank_n = 0;
            uint8_t talent_n = TalentID::kNum;
            {
                auto levels_fit = [](uint8_t const *at, uint8_t const *end) -> uint8_t {
                    if (at == end) return 1;
                    Reader r(at);
                    if (r.at >= end) return 0;
                    uint16_t n = r.read<uint16_t>();
                    if (r.at > end || n > PetalID::kNumPetals) return 0;
                    for (uint16_t i = 0; i < n; ++i) {
                        (void) r.read<PetalID::T>();
                        (void) r.read<uint32_t>();
                        if (r.at > end) return 0;
                    }
                    return r.at == end;
                };
                auto try_tail = [&levels_fit](uint8_t const *at, uint8_t const *end, uint8_t tn) -> uint8_t {
                    Validator rest(at, end);
                    if (!rest.validate_uint32()) return 0;
                    for (uint8_t i = 0; i < tn; ++i)
                        if (!rest.validate_uint8()) return 0;
                    return levels_fit(rest.at, end);
                };
                Validator probe(validator.at, validator.end);
                while (bank_n < (uint32_t) PetalID::kNumPetals) {
                    Validator trial(probe.at, probe.end);
                    if (!trial.validate_uint32()) break;
                    if (try_tail(trial.at, trial.end, TalentID::kNum)) {
                        talent_n = TalentID::kNum;
                        probe.at = trial.at;
                        ++bank_n;
                        continue;
                    }
                    if (try_tail(trial.at, trial.end, 2)) {
                        talent_n = 2;
                        probe.at = trial.at;
                        ++bank_n;
                        continue;
                    }
                    break;
                }
            }
            for (uint32_t i = 0; i < bank_n; ++i)
                if (client->check_invalid(validator.validate_uint32())) return;
            if (client->check_invalid(validator.validate_uint32())) return; // coins
            for (uint8_t i = 0; i < talent_n; ++i)
                if (client->check_invalid(validator.validate_uint8())) return;
            uint8_t const have_levels = validator.at < validator.end ? 1 : 0;
            if (have_levels) {
                Reader probe(validator.at);
                uint16_t n = probe.read<uint16_t>();
                if (n > PetalID::kNumPetals) return;
                for (uint16_t i = 0; i < n; ++i) {
                    (void) probe.read<PetalID::T>();
                    (void) probe.read<uint32_t>();
                    if (probe.at > validator.end) return;
                }
                if (probe.at != validator.end) return;
            }
            uint32_t level = reader.read<uint32_t>();
            if (level < 1) level = 1;
            uint32_t score = reader.read<uint32_t>();
            Simulation *simulation = &client->game->simulation;
            if (!simulation->ent_exists(client->camera)) break;
            Entity &camera = simulation->get_ent(client->camera);
            if (level < camera.get_respawn_level())
                level = camera.get_respawn_level();
            PetalID::T petals_tmp[2 * MAX_SLOT_COUNT];
            for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i) {
                PetalID::T id = reader.read<PetalID::T>();
                if (id >= PetalID::kNumPetals) id = PetalID::kNone;
                petals_tmp[i] = id;
            }
            for (uint32_t i = 0; i < PetalID::kNumPetals; ++i)
                client->craft_bank[i] = 0;
            for (uint32_t i = 0; i < bank_n; ++i)
                client->craft_bank[i] = reader.read<uint32_t>();
            client->coins = reader.read<uint32_t>();
            for (uint8_t i = 0; i < TalentID::kNum; ++i)
                client->talent_ranks[i] = 0;
            for (uint8_t i = 0; i < talent_n; ++i) {
                uint8_t r = reader.read<uint8_t>();
                if (i < TalentID::kNum) client->talent_ranks[i] = r;
            }
            client->petal_levels = {};
            if (have_levels) {
                uint16_t n_levels = reader.read<uint16_t>();
                for (uint16_t i = 0; i < n_levels; ++i) {
                    PetalID::T lid = reader.read<PetalID::T>();
                    uint32_t lv = reader.read<uint32_t>();
                    if (lid < PetalID::kNumPetals) client->petal_levels[lid] = lv;
                }
            }
            Talent::clamp_ranks(client->talent_ranks, level);
            for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i)
                PetalTracker::remove_petal(simulation, camera.get_inventory(i));
            camera.set_respawn_level(level);
            {
                uint32_t floor = level_to_score(level);
                if (score < floor) score = floor;
                client->pending_score = score;
            }
            uint32_t slots = client_slot_count(client);
            for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i) {
                PetalID::T id = petals_tmp[i];
                if (i < slots && id == PetalID::kNone) id = PetalID::kBasic;
                camera.set_inventory(i, id);
                PetalTracker::add_petal(simulation, id);
            }
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
        case Serverbound::kAutoPartyOpt: {
            if (client->check_invalid(validator.validate_uint8())) return;
            client->disable_auto_party = reader.read<uint8_t>() ? 1 : 0;
            break;
        }
    }
}

void Client::on_disconnect(WebSocket *ws, int code, std::string_view message) {
    Client *client = ws->getUserData();
    {
        uint64_t now_ms = (uint64_t) std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
        unsigned long long life_s = (client && client->open_ms && now_ms > client->open_ms)
            ? (now_ms - client->open_ms) / 1000ull : 0ull;
        unsigned long long idle_s = (client && client->last_rx_ms && now_ms > client->last_rx_ms)
            ? (now_ms - client->last_rx_ms) / 1000ull : 0ull;
        // idle ~= 104s with our idleTimeout=120 => uWS idle timeout (no close frame,
        // browser sees 1006). Small idle => peer/edge/network dropped the tunnel.
        std::printf("disconnect: [%d] life=%llus idle=%llus name=%s\n", code,
            life_s, idle_s, client ? client->play_name.c_str() : "");
    }
    if (client == nullptr) return;
    if (client->conn_counted) {
        RateLimit::on_disconnect(client->remote_ip);
        client->conn_counted = 0;
    }
    client->remove();
}

bool Client::check_invalid(bool valid) {
    if (valid) return false;
    std::cout << "client sent an invalid packet\n";
    //optional
    disconnect();

    return true;
}
