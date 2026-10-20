#include <Server/AccountStore.hh>

#include <Server/Client.hh>
#include <Server/Game.hh>
#include <Server/LeaderboardStore.hh>
#include <Server/NodeFs.hh>
#include <Server/PetalTracker.hh>
#include <Server/Server.hh>

#include <Shared/Binary.hh>
#include <Shared/Config.hh>
#include <Shared/Craft.hh>
#include <Shared/Entity.hh>
#include <Shared/Simulation.hh>
#include <Shared/StaticData.hh>
#include <Shared/Talent.hh>

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

static char const *ACCOUNT_FILE = "accounts.dat";
static std::unordered_map<std::string, Account> ACCOUNTS;
static std::unordered_set<std::string> ACTIVE_SESSIONS;
static std::unordered_map<std::string, uint64_t> SESSION_TOKENS;
static std::chrono::steady_clock::time_point LAST_SNAPSHOT =
    std::chrono::steady_clock::now() - std::chrono::hours(1);

static uint64_t wall_seconds() {
    return (uint64_t) std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

static uint32_t beijing_day(uint64_t epoch) {
    return (uint32_t)((epoch + 8 * 3600) / 86400);
}

static uint64_t issue_session_token(std::string const &username) {
    uint64_t t = (uint64_t) std::chrono::high_resolution_clock::now().time_since_epoch().count();
    t ^= AccountStore::hash_password(username, std::to_string(t ^ 0x9e3779b97f4a7c15ull));
    t ^= (uint64_t) reinterpret_cast<uintptr_t>(&SESSION_TOKENS) << 13;
    if (t == 0) t = 1;
    SESSION_TOKENS[username] = t;
    if (Account *acc = AccountStore::find(username)) {
        acc->session_hash = AccountStore::hash_password(username, "session:" + std::to_string(t));
        AccountStore::save();
    }
    return t;
}

static uint8_t bind_session(Client *client, std::string const &username, Account *acc) {
    if (ACTIVE_SESSIONS.contains(username) && client->account_name != username)
        return AccountResultCode::kAccountAlreadyIn;
    bool continuing_session = client->account_name == username;
    if (!client->account_name.empty())
        ACTIVE_SESSIONS.erase(client->account_name);
    uint64_t now = wall_seconds();
    uint32_t day = beijing_day(now);
    if (acc->bonus_day != day) {
        acc->bonus_day = day;
        acc->daily_bonus_seconds = 3600;
    }
    if (!continuing_session && acc->last_seen_epoch > 0 && now > acc->last_seen_epoch) {
        uint64_t earned = (now - acc->last_seen_epoch) / 10;
        acc->offline_bonus_seconds = (uint32_t) std::min<uint64_t>(10800,
            (uint64_t)acc->offline_bonus_seconds + earned);
    }
    // Fold legacy overflow into actual backpack slots before sending the
    // account snapshot. Keep any remainder in the old save fields until a
    // slot is freed, so migration never destroys owned petals.
    bool migrated = false;
    uint32_t bag_start = loadout_slots_at_level(acc->level);
    for (uint32_t id = 1; id < PetalID::kNumPetals; ++id) {
        while (acc->craft_bank[id] > 0) {
            uint32_t slot = bag_start;
            while (slot < bag_start + MAX_SLOT_COUNT
                && acc->petals[slot] != PetalID::kNone) ++slot;
            if (slot >= bag_start + MAX_SLOT_COUNT) break;
            acc->petals[slot] = (PetalID::T)id;
            --acc->craft_bank[id];
            migrated = true;
        }
    }
    acc->last_seen_epoch = now;
    client->account_name = username;
    client->craft_bank = acc->craft_bank;
    client->coins = acc->coins;
    client->craft_attempts = acc->craft_attempts;
    uint8_t max_r = Talent::max_rank(acc->level);
    client->talent_hp = acc->talent_hp > max_r ? max_r : acc->talent_hp;
    client->talent_rot = acc->talent_rot > max_r ? max_r : acc->talent_rot;
    client->loadout_presets = acc->loadout_presets;
    client->bonus_day = acc->bonus_day;
    client->daily_bonus_seconds = acc->daily_bonus_seconds;
    client->offline_bonus_seconds = acc->offline_bonus_seconds;
    client->last_bonus_tick = now;
    client->last_bonus_sync = 0;
    ACTIVE_SESSIONS.insert(username);
    if (client->game != nullptr && client->game->simulation.ent_exists(client->camera) && !client->alive())
        AccountStore::apply_to_camera(&client->game->simulation, client->game->simulation.get_ent(client->camera), *acc);
    if (migrated) AccountStore::save();
    return AccountResultCode::kAccountOk;
}

static std::string read_file(char const *path) {
#ifdef WASM_SERVER
    char *ptr = node_fs_read_alloc(path);
    if (ptr == nullptr) return {};
    std::string out(ptr);
    free(ptr);
    return out;
#else
    std::ifstream in(path, std::ios::binary);
    if (!in) return {};
    return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
#endif
}

static void write_file_atomic(char const *path, std::string const &data) {
#ifdef WASM_SERVER
    int ok = node_fs_write_atomic(path, data.data(), (int) data.size());
    if (!ok) std::cerr << "[accounts] write failed: " << path << std::endl;
#else
    std::string tmp = std::string(path) + ".tmp";
    std::string bak = std::string(path) + ".bak";
    {
        std::ofstream out(tmp, std::ios::binary | std::ios::trunc);
        out.write(data.data(), (std::streamsize) data.size());
    }
    {
        std::ifstream in(path, std::ios::binary);
        if (in) {
            std::ofstream out(bak, std::ios::binary | std::ios::trunc);
            out << in.rdbuf();
        }
    }
    std::remove(path);
    std::rename(tmp.c_str(), path);
#endif
}

static void maybe_write_snapshot(char const *path) {
    auto now = std::chrono::steady_clock::now();
    if (now - LAST_SNAPSHOT < std::chrono::minutes(5)) return;
    LAST_SNAPSHOT = now;
#ifdef WASM_SERVER
    node_fs_snapshot(path);
#endif
}

uint64_t AccountStore::hash_password(std::string const &username, std::string const &password) {
    uint64_t h = 14695981039346656037ull;
    auto mix = [&](unsigned char c) {
        h ^= c;
        h *= 1099511628211ull;
    };
    for (unsigned char c : username) mix(c);
    mix(':');
    for (unsigned char c : password) mix(c);
    return h;
}

uint8_t AccountStore::valid_username(std::string const &s) {
    if (s.size() < 3 || s.size() > MAX_USERNAME_LENGTH) return 0;
    // Guest ids (G#XXXXXX) are reserved — cannot register as account names
    if (s.size() >= 2 && s[0] == 'G' && s[1] == '#') return 0;
    for (unsigned char c : s) {
        if (!(std::isalnum(c) || c == '_')) return 0;
    }
    return 1;
}

uint8_t AccountStore::valid_password(std::string const &s) {
    return s.size() >= 4 && s.size() <= MAX_PASSWORD_LENGTH;
}

Account *AccountStore::find(std::string const &username) {
    auto it = ACCOUNTS.find(username);
    if (it == ACCOUNTS.end()) return nullptr;
    return &it->second;
}

uint8_t AccountStore::name_reserved_for_guest(std::string const &name) {
    if (name.empty()) return 0;
    for (auto const &entry : ACCOUNTS) {
        std::string const &registered = entry.first;
        if (registered.size() != name.size()) continue;
        bool same = true;
        for (size_t i = 0; i < name.size(); ++i) {
            if (std::tolower((unsigned char) name[i]) !=
                std::tolower((unsigned char) registered[i])) {
                same = false;
                break;
            }
        }
        if (same) return 1;
    }
    return 0;
}

void AccountStore::load() {
    ACCOUNTS.clear();
    SESSION_TOKENS.clear();
    std::string raw = read_file(ACCOUNT_FILE);
    if (raw.empty()) return;
    std::istringstream in(raw);
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream ls(line);
        Account acc;
        // uint8_t is read as a *character* by operator>>, so the level goes
        // through a uint32_t and is narrowed afterwards (a saved level of 53
        // used to come back as 5).
        uint64_t pass_hash = 0;
        uint32_t level = 0;
        ls >> acc.username >> pass_hash >> level;
        if (acc.username.empty() || !valid_username(acc.username)) continue;
        acc.pass_hash = pass_hash;
        acc.level = level < 1 ? 1 : level;
        acc.petals.fill(PetalID::kNone);
        acc.craft_bank.fill(0);
        acc.coins = 0;
        acc.craft_attempts = 0;
        acc.talent_hp = 0;
        acc.talent_rot = 0;
        // Read remaining numbers first, then split petals vs craft_bank.
        // MAX_SLOT_COUNT grew 8→10 (petals 16→20, presets width 9→11).
        std::vector<uint64_t> fields;
        {
            uint64_t fv = 0;
            while (ls >> fv) fields.push_back(fv);
        }
        {
            constexpr uint32_t kPetalsNew = 2 * MAX_SLOT_COUNT;
            constexpr uint32_t kPetalsLegacy8 = 16;
            constexpr uint32_t kPresetNew = 1 + MAX_SLOT_COUNT;
            constexpr uint32_t kPresetLegacy8 = 1 + 8;
            uint32_t const n = (uint32_t) PetalID::kNumPetals;
            constexpr uint32_t kCommonLadderAdded = 14;
            constexpr uint32_t kStingerLegAdded = 1;
            constexpr uint32_t kJungleAdded = 12;
            constexpr uint32_t kLadderAdded = 43;
            constexpr uint32_t kRoyalAdded = 12;
            constexpr uint32_t kCoinAdded = 1;
            uint32_t const prev_coin = n > kCoinAdded ? n - kCoinAdded : n;
            uint32_t const prev_common = n > kCommonLadderAdded ? n - kCommonLadderAdded : n;
            uint32_t const prev_stinger = prev_common > kStingerLegAdded ? prev_common - kStingerLegAdded : prev_common;
            uint32_t const prev_jungle = prev_stinger > kJungleAdded ? prev_stinger - kJungleAdded : prev_stinger;
            uint32_t const prev_ladder = prev_jungle > kLadderAdded ? prev_jungle - kLadderAdded : prev_jungle;
            uint32_t const prev_royal = prev_ladder > kRoyalAdded ? prev_ladder - kRoyalAdded : prev_ladder;
            uint32_t const bank_candidates[] = {
                n, prev_coin, prev_common, prev_stinger, prev_jungle, prev_ladder, prev_royal
            };

            auto layout_ok = [&](uint32_t petal_n, uint32_t preset_w) -> bool {
                if (fields.size() < petal_n) return false;
                size_t rem = fields.size() - petal_n;
                for (uint32_t bank_n : bank_candidates) {
                    size_t const full = bank_n + 5 + LOADOUT_PRESET_COUNT * preset_w + 4;
                    size_t const presets = bank_n + 5 + LOADOUT_PRESET_COUNT * preset_w;
                    if (rem == full || rem == presets) return true;
                    if (rem >= bank_n && rem <= bank_n + 5) return true;
                }
                // Legacy rarity-bucket saves were frozen at 7 tiers (pre-Super/Ultra).
                if (petal_n == kPetalsLegacy8 && rem >= 7 && rem < n)
                    return true;
                return false;
            };

            uint32_t petal_n = kPetalsNew;
            uint32_t preset_w = kPresetNew;
            if (layout_ok(kPetalsNew, kPresetNew)) {
                petal_n = kPetalsNew;
                preset_w = kPresetNew;
            } else if (layout_ok(kPetalsLegacy8, kPresetLegacy8)) {
                petal_n = kPetalsLegacy8;
                preset_w = kPresetLegacy8;
            } else {
                petal_n = (uint32_t) std::min<size_t>(fields.size(), kPetalsNew);
                preset_w = kPresetNew;
            }

            for (uint32_t i = 0; i < petal_n && i < 2 * MAX_SLOT_COUNT; ++i) {
                uint64_t v = fields[i];
                if (v >= PetalID::kNumPetals) v = PetalID::kNone;
                acc.petals[i] = (PetalID::T) v;
            }

            std::vector<uint64_t> rest(
                fields.begin() + (std::ptrdiff_t) petal_n, fields.end());

            uint32_t bank_n = PetalID::kNumPetals;
            constexpr uint32_t kAccountTail = 5 + LOADOUT_PRESET_COUNT * (1 + MAX_SLOT_COUNT) + 4;
            if (rest.size() == prev_coin + kAccountTail
                || rest.size() == prev_coin + 5 + LOADOUT_PRESET_COUNT * preset_w + 4)
                bank_n = prev_coin;
            if (rest.size() < bank_n && rest.size() >= prev_common)
                bank_n = prev_common;
            else if (rest.size() < bank_n && rest.size() >= prev_stinger)
                bank_n = prev_stinger;
            else if (rest.size() < bank_n && rest.size() >= prev_jungle)
                bank_n = prev_jungle;
            else if (rest.size() < bank_n && rest.size() >= prev_ladder)
                bank_n = prev_ladder;
            else if (rest.size() < bank_n && rest.size() >= prev_royal)
                bank_n = prev_royal;
            if (rest.size() >= bank_n) {
                for (uint32_t i = 0; i < bank_n; ++i)
                    acc.craft_bank[i] = (uint32_t) rest[i];
                if (rest.size() > bank_n)
                    acc.coins = (uint32_t) rest[bank_n];
                if (rest.size() > bank_n + 1)
                    acc.craft_attempts = (uint32_t) rest[bank_n + 1];
                if (rest.size() > bank_n + 2)
                    acc.talent_hp = (uint8_t) std::min<uint64_t>(rest[bank_n + 2], 99);
                if (rest.size() > bank_n + 3)
                    acc.talent_rot = (uint8_t) std::min<uint64_t>(rest[bank_n + 3], 99);
                if (rest.size() > bank_n + 4)
                    acc.session_hash = rest[bank_n + 4];
                uint32_t const preset_petal_n = preset_w > 0 ? preset_w - 1 : 0;
                if (rest.size() >= bank_n + 5 + LOADOUT_PRESET_COUNT * preset_w) {
                    uint32_t at = bank_n + 5;
                    for (LoadoutPreset &preset : acc.loadout_presets) {
                        uint64_t slots = rest[at++];
                        preset.slot_count = slots <= MAX_SLOT_COUNT ? (uint8_t) slots : 0;
                        preset.petals.fill(PetalID::kNone);
                        for (uint32_t i = 0; i < preset_petal_n && i < MAX_SLOT_COUNT; ++i) {
                            uint64_t value = rest[at++];
                            preset.petals[i] = value < PetalID::kNumPetals
                                ? (PetalID::T) value : PetalID::kNone;
                        }
                    }
                    if (rest.size() >= at + 4) {
                        acc.bonus_day = (uint32_t)rest[at++];
                        acc.daily_bonus_seconds = (uint32_t)std::min<uint64_t>(rest[at++], 3600);
                        acc.offline_bonus_seconds = (uint32_t)std::min<uint64_t>(rest[at++], 10800);
                        acc.last_seen_epoch = rest[at++];
                    }
                }
            } else if (rest.size() >= 7) {
                // Legacy rarity buckets (7 tiers) → park counts on first petal of that rarity
                for (uint8_t r = 0; r < 7; ++r) {
                    PetalID::T id = Craft::first_petal_of_rarity(r);
                    if (id != PetalID::kNone)
                        acc.craft_bank[id] = (uint32_t) rest[r];
                }
                if (rest.size() > 7)
                    acc.coins = (uint32_t) rest[7];
            }
        }
        if (acc.coins > 0) {
            uint64_t sum = (uint64_t) acc.craft_bank[PetalID::kCoin] + acc.coins;
            if (sum > 1000000000ull) sum = 1000000000ull;
            acc.craft_bank[PetalID::kCoin] = (uint32_t) sum;
            acc.coins = 0;
        }
        ACCOUNTS[acc.username] = acc;
    }
    std::cout << "[accounts] loaded " << ACCOUNTS.size() << " accounts" << std::endl;
    // Ensure accounts.dat always exists on disk after boot (empty file OK)
    save();
}

void AccountStore::save() {
    std::ostringstream out;
    out << "# ashflor accounts  fields: user hash level petals[20] craft_bank[N] coins craft_attempts talent_hp talent_rot session_hash presets[20][slots+10 petals] bonus_day daily_bonus offline_bonus last_seen\n";
    for (auto const &kv : ACCOUNTS) {
        Account const &acc = kv.second;
        out << acc.username << ' ' << acc.pass_hash << ' ' << (uint32_t) acc.level;
        for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i)
            out << ' ' << (uint32_t) acc.petals[i];
        for (uint32_t i = 0; i < PetalID::kNumPetals; ++i)
            out << ' ' << acc.craft_bank[i];
        out << ' ' << acc.coins << ' ' << acc.craft_attempts
            << ' ' << (uint32_t) acc.talent_hp << ' ' << (uint32_t) acc.talent_rot
            << ' ' << acc.session_hash;
        for (LoadoutPreset const &preset : acc.loadout_presets) {
            out << ' ' << (uint32_t) preset.slot_count;
            for (PetalID::T id : preset.petals)
                out << ' ' << (uint32_t) id;
        }
        out << ' ' << acc.bonus_day << ' ' << acc.daily_bonus_seconds
            << ' ' << acc.offline_bonus_seconds << ' ' << acc.last_seen_epoch;
        out << '\n';
    }
    write_file_atomic(ACCOUNT_FILE, out.str());
    maybe_write_snapshot(ACCOUNT_FILE);
}

uint32_t AccountStore::registered_count() {
    return (uint32_t) ACCOUNTS.size();
}

void AccountStore::seed_leaderboard_levels() {
    for (auto const &kv : ACCOUNTS) {
        if (kv.first.empty()) continue;
        LeaderboardStore::add_stat(kv.first, kLbLevel, kv.second.level);
    }
}

static void normalize_petals(Account &acc) {
    for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i) {
        if (acc.petals[i] >= PetalID::kNumPetals)
            acc.petals[i] = PetalID::kNone;
    }
}

static void copy_progress(Account &acc, Client *client) {
    if (client == nullptr || client->game == nullptr) return;
    Simulation *sim = &client->game->simulation;
    if (!sim->ent_exists(client->camera)) return;
    Entity &camera = sim->get_ent(client->camera);
    acc.petals.fill(PetalID::kNone);
    // Level is a high-water mark — never write a lower value back to the account.
    uint32_t level = 1;
    if (sim->ent_alive(camera.get_player())) {
        Entity &player = sim->get_ent(camera.get_player());
        uint32_t lv = score_to_level(player.get_score());
        if (lv < 1) lv = 1;
        if (lv < camera.get_respawn_level()) lv = camera.get_respawn_level();
        level = lv;
        for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i)
            acc.petals[i] = player.get_loadout_ids(i);
    } else {
        level = camera.get_respawn_level();
        if (level < 1) level = 1;
        for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i)
            acc.petals[i] = camera.get_inventory(i);
    }
    if (level > acc.level) acc.level = level;
    normalize_petals(acc);
    acc.craft_bank = client->craft_bank;
    acc.coins = client->coins;
    acc.craft_attempts = client->craft_attempts;
    uint8_t max_r = Talent::max_rank(acc.level);
    acc.talent_hp = client->talent_hp > max_r ? max_r : client->talent_hp;
    acc.talent_rot = client->talent_rot > max_r ? max_r : client->talent_rot;
    acc.loadout_presets = client->loadout_presets;
    acc.bonus_day = client->bonus_day;
    acc.daily_bonus_seconds = client->daily_bonus_seconds;
    acc.offline_bonus_seconds = client->offline_bonus_seconds;
    acc.last_seen_epoch = wall_seconds();
}

uint8_t AccountStore::register_account(Client *client, std::string const &username, std::string const &password) {
    if (!valid_username(username)) return AccountResultCode::kAccountInvalidUser;
    if (!valid_password(password)) return AccountResultCode::kAccountInvalidPass;
    if (name_reserved_for_guest(username)) return AccountResultCode::kAccountExists;
    Account acc;
    acc.username = username;
    acc.pass_hash = hash_password(username, password);
    acc.level = 1;
    acc.petals.fill(PetalID::kNone);
    for (uint32_t i = 0; i < loadout_slots_at_level(acc.level); ++i)
        acc.petals[i] = PetalID::kBasic;
    acc.craft_bank.fill(0);
    acc.coins = 0;
    acc.craft_attempts = 0;
    normalize_petals(acc);
    copy_progress(acc, client);
    ACCOUNTS[username] = acc;
    save();
    return AccountResultCode::kAccountOk;
}

void AccountStore::apply_to_camera(Simulation *sim, Entity &camera, Account const &acc) {
    for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i)
        PetalTracker::remove_petal(sim, camera.get_inventory(i));
    camera.set_respawn_level(acc.level);
    for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i) {
        PetalID::T id = acc.petals[i];
        if (id >= PetalID::kNumPetals) id = PetalID::kNone;
        camera.set_inventory(i, id);
        PetalTracker::add_petal(sim, id);
    }
}

uint8_t AccountStore::login(Client *client, std::string const &username, std::string const &password) {
    if (!valid_username(username)) return AccountResultCode::kAccountInvalidUser;
    if (!valid_password(password)) return AccountResultCode::kAccountInvalidPass;
    Account *acc = find(username);
    if (acc == nullptr) return AccountResultCode::kAccountNotFound;
    if (acc->pass_hash != hash_password(username, password))
        return AccountResultCode::kAccountWrongPass;
    uint8_t result = bind_session(client, username, acc);
    if (result == AccountResultCode::kAccountOk)
        issue_session_token(username);
    return result;
}

uint8_t AccountStore::resume(Client *client, std::string const &username, uint64_t session_token) {
    if (!valid_username(username)) return AccountResultCode::kAccountInvalidUser;
    if (session_token == 0) return AccountResultCode::kAccountWrongPass;
    Account *acc = find(username);
    if (acc == nullptr) return AccountResultCode::kAccountNotFound;
    if (acc->session_hash == 0 ||
        acc->session_hash != hash_password(username, "session:" + std::to_string(session_token)))
        return AccountResultCode::kAccountWrongPass;
    uint8_t result = bind_session(client, username, acc);
    if (result == AccountResultCode::kAccountOk)
        SESSION_TOKENS[username] = session_token;
    return result;
}

uint64_t AccountStore::current_session(std::string const &username) {
    auto it = SESSION_TOKENS.find(username);
    if (it == SESSION_TOKENS.end()) return 0;
    return it->second;
}

void AccountStore::logout(Client *client, uint8_t revoke_session) {
    if (!client->account_name.empty()) {
        if (revoke_session) {
            SESSION_TOKENS.erase(client->account_name);
            if (Account *acc = find(client->account_name)) {
                acc->session_hash = 0;
                save();
            }
        }
        ACTIVE_SESSIONS.erase(client->account_name);
        client->account_name.clear();
    }
}

void AccountStore::capture_nosave(Client *client) {
    if (client == nullptr || client->account_name.empty() || client->game == nullptr) return;
    Account *acc = find(client->account_name);
    if (acc == nullptr) return;
    copy_progress(*acc, client);
}

void AccountStore::capture(Client *client) {
    capture_nosave(client);
    if (client != nullptr && !client->account_name.empty())
        save();
}
