#include <Server/DevCheats.hh>

#include <Server/AccountStore.hh>
#include <Server/Client.hh>
#include <Server/Server.hh>

#include <Shared/Entity.hh>
#include <Shared/Simulation.hh>
#include <Shared/StaticData.hh>
#include <Shared/Talent.hh>

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace DevCheats {
namespace {

uint8_t g_enabled = 0;
uint8_t g_logged = 0;

// points_earned(level)==level; full Ultra talents ≈511 pts → lv600 leaves headroom.
constexpr uint32_t kKitLevel = 600;
constexpr uint32_t kKitCoins = 99999999u;
constexpr uint32_t kBankStock = 99999u;
constexpr uint32_t kBankFloor = 1000u;

uint8_t name_is_dev(std::string const &name) {
    if (name.size() != 3) return 0;
    return (char) std::tolower((unsigned char) name[0]) == 'd'
        && (char) std::tolower((unsigned char) name[1]) == 'e'
        && (char) std::tolower((unsigned char) name[2]) == 'v';
}

void ensure_boot_from_env() {
    if (g_logged) return;
    g_logged = 1;
    char const *env = std::getenv("ASHFLOR_DEV_CHEATS");
    if (env != nullptr && env[0] == '1' && env[1] == '\0')
        g_enabled = 1;
    if (g_enabled)
        std::printf("[DevCheats] enabled (ASHFLOR_DEV_CHEATS=1) — account \"dev\" gets godmode + infinite petals\n");
}

void max_talents(uint8_t *ranks, uint32_t level) {
    for (uint8_t id = 0; id < TalentID::kNum; ++id)
        ranks[id] = Talent::max_rank_for(id, level);
}

void fill_bank(std::array<uint32_t, PetalID::kNumPetals> &bank) {
    for (uint32_t id = 1; id < PetalID::kNumPetals; ++id)
        bank[id] = kBankStock;
}

} // namespace

void set_enabled(uint8_t on) {
    g_enabled = on ? 1 : 0;
    g_logged = 1;
    if (g_enabled)
        std::printf("[DevCheats] enabled — account \"dev\" gets godmode + infinite petals\n");
    else
        std::printf("[DevCheats] disabled\n");
}

uint8_t enabled() {
    ensure_boot_from_env();
    return g_enabled;
}

uint8_t active(Client const *client) {
    if (!enabled() || client == nullptr) return 0;
    return name_is_dev(client->account_name);
}

void apply_kit(Client *client) {
    if (!active(client)) return;
    Account *acc = AccountStore::find(client->account_name);
    if (acc == nullptr) return;

    acc->level = kKitLevel;
    acc->score = level_to_score(kKitLevel);
    acc->coins = kKitCoins;
    fill_bank(acc->craft_bank);
    max_talents(acc->talent_ranks, kKitLevel);

    // Ensure primary slots have basics so spawn is playable before bag edit.
    uint32_t slots = Talent::slot_count_from_ranks(acc->talent_ranks);
    for (uint32_t i = 0; i < slots; ++i) {
        if (acc->petals[i] == PetalID::kNone || acc->petals[i] >= PetalID::kNumPetals)
            acc->petals[i] = PetalID::kBasic;
    }

    client->craft_bank = acc->craft_bank;
    client->coins = acc->coins;
    client->pending_score = acc->score;
    for (uint8_t i = 0; i < TalentID::kNum; ++i)
        client->talent_ranks[i] = acc->talent_ranks[i];

    if (client->game != nullptr && client->game->simulation.ent_exists(client->camera)) {
        Entity &cam = client->game->simulation.get_ent(client->camera);
        AccountStore::apply_to_camera(&client->game->simulation, cam, *acc);
    }

    AccountStore::capture(client);
    std::printf("[DevCheats] kit applied to \"%s\" (lv%d, bank×%u, coins=%u)\n",
        client->account_name.c_str(), kKitLevel, kBankStock, kKitCoins);
}

void tick_client(Client *client) {
    if (!active(client)) return;
    if (client->game == nullptr) return;
    Simulation *sim = &client->game->simulation;
    if (!sim->ent_exists(client->camera)) return;
    Entity &cam = sim->get_ent(client->camera);
    EntityID pid = cam.get_player();
    if (sim->ent_alive(pid)) {
        Entity &pl = sim->get_ent(pid);
        // Godmode without immunity_ticks — portals still work.
        if (pl.has_component(kHealth)) {
            pl.health = pl.max_health;
            pl.poison_ticks = 0;
            pl.slow_ticks = 0;
        }
    }
    // Top up bank so craft / delete never runs dry.
    uint8_t dirty = 0;
    for (uint32_t id = 1; id < PetalID::kNumPetals; ++id) {
        if (client->craft_bank[id] < kBankFloor) {
            client->craft_bank[id] = kBankStock;
            dirty = 1;
        }
    }
    if (client->coins < kKitCoins / 2) {
        client->coins = kKitCoins;
        dirty = 1;
    }
    if (dirty) {
        AccountStore::capture_nosave(client);
        client_sync_craft(client);
    }
}

} // namespace DevCheats
