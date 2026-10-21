#include <Server/AccountStore.hh>

#include <Server/Client.hh>
#include <Server/Game.hh>
#include <Server/Guild.hh>
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

static void restore_primary_from_active_preset(Account &acc);

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

static Client *find_live_account_holder(std::string const &username) {
    if (Server::games == nullptr || username.empty()) return nullptr;
    for (uint8_t i = 0; i < MapID::kNumMaps; ++i) {
        for (Client *c : Server::games[i].clients) {
            if (c != nullptr && c->account_name == username)
                return c;
        }
    }
    return nullptr;
}

// New login/resume takes the account: persist + kick any other live holder,
// and clear ghost ACTIVE_SESSIONS locks from crashed tabs.
static void reclaim_account_lock(std::string const &username, Client *incoming) {
    Client *holder = find_live_account_holder(username);
    if (holder != nullptr && holder != incoming) {
        AccountStore::capture(holder);
        // Drop the old connection. Message is shown on that client; it must not
        // auto-resume with the same token and steal the account back.
        holder->disconnect(CloseReason::kProtocol, "Logged in elsewhere");
    }
    ACTIVE_SESSIONS.erase(username);
}

static uint8_t bind_session(Client *client, std::string const &username, Account *acc) {
    bool continuing_session = client->account_name == username;
    // Always displace any prior holder (or stale lock) unless this socket is
    // already bound to the same account.
    if (!continuing_session)
        reclaim_account_lock(username, client);
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
    uint32_t bag_start = Talent::slot_count_from_ranks(acc->talent_ranks);
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
    // Empty primary after a bad drain → rebuild from last K/L scheme using bag.
    client->active_loadout_preset = acc->active_loadout_preset;
    client->loadout_presets = acc->loadout_presets;
    restore_primary_from_active_preset(*acc);
    client->craft_bank = acc->craft_bank;
    client->coins = acc->coins;
    client->craft_attempts = acc->craft_attempts;
    for (uint8_t i = 0; i < TalentID::kNum; ++i)
        client->talent_ranks[i] = acc->talent_ranks[i];
    Talent::clamp_ranks(client->talent_ranks, acc->level);
    client->bonus_day = acc->bonus_day;
    client->daily_bonus_seconds = acc->daily_bonus_seconds;
    client->offline_bonus_seconds = acc->offline_bonus_seconds;
    client->last_bonus_tick = now;
    client->last_bonus_sync = 0;
    client->guild_name = acc->guild_name;
    client->guild_leader = acc->guild_leader ? 1 : 0;
    {
        uint32_t lv = acc->level < 1 ? 1 : acc->level;
        if (lv > MAX_PLAYER_LEVEL) lv = MAX_PLAYER_LEVEL;
        uint32_t floor = level_to_score(lv);
        uint32_t sc = acc->score;
        if (sc < floor) sc = floor;
        uint32_t const cap = level_to_score(MAX_PLAYER_LEVEL);
        if (sc > cap) sc = cap;
        // Restore full mid-level XP (not just level floor).
        client->pending_score = sc;
    }
    ACTIVE_SESSIONS.insert(username);
    if (client->game != nullptr && client->game->simulation.ent_exists(client->camera) && !client->alive())
        AccountStore::apply_to_camera(&client->game->simulation, client->game->simulation.get_ent(client->camera), *acc);
    if (migrated) AccountStore::save();
    Guild::on_login(client);
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
        // Do NOT clamp inflated levels here — XpMigration::run_once rewrites
        // only exploded accounts; safe/gray rows must stay byte-identical.
        if (level < 1) level = 1;
        acc.level = level;
        acc.score = 0; // filled from trailing s:TOKEN or derived after parse
        acc.petals.fill(PetalID::kNone);
        acc.craft_bank.fill(0);
        acc.coins = 0;
        acc.craft_attempts = 0;
        for (uint8_t i = 0; i < TalentID::kNum; ++i)
            acc.talent_ranks[i] = 0;
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
            // Subtract append-only waves newest→oldest so old craft_bank lengths still match.
            constexpr uint32_t kHoneyMeteorLadderAdded = 16;
            constexpr uint32_t kHoneyMeteorAdded = 2;
            constexpr uint32_t kMonsteraLadderAdded = 6;
            constexpr uint32_t kAntHellAdded = 12;
            constexpr uint32_t kSuperUltraAdded = 42;
            constexpr uint32_t kCrossLegendAdded = 40;
            constexpr uint32_t kBaseCraftAdded = 22;
            constexpr uint32_t kMatsAdded = 48;
            constexpr uint32_t kMonsteraAdded = 1;
            constexpr uint32_t kCoinAdded = 1;
            constexpr uint32_t kCommonLadderAdded = 14;
            constexpr uint32_t kStingerLegAdded = 1;
            constexpr uint32_t kJungleAdded = 12;
            constexpr uint32_t kLadderAdded = 43;
            constexpr uint32_t kRoyalAdded = 12;
            auto sub = [](uint32_t cur, uint32_t amt) -> uint32_t {
                return cur > amt ? cur - amt : cur;
            };
            uint32_t const prev_honey_meteor_ladder = sub(n, kHoneyMeteorLadderAdded);
            uint32_t const prev_honey_meteor = sub(prev_honey_meteor_ladder, kHoneyMeteorAdded);
            uint32_t const prev_monstera_ladder = sub(prev_honey_meteor, kMonsteraLadderAdded);
            uint32_t const prev_anthell = sub(prev_monstera_ladder, kAntHellAdded);
            uint32_t const prev_super = sub(prev_anthell, kSuperUltraAdded);
            uint32_t const prev_cross = sub(prev_super, kCrossLegendAdded);
            uint32_t const prev_basecraft = sub(prev_cross, kBaseCraftAdded);
            uint32_t const prev_mats = sub(prev_basecraft, kMatsAdded);
            uint32_t const prev_monstera = sub(prev_mats, kMonsteraAdded);
            uint32_t const prev_coin = sub(prev_monstera, kCoinAdded);
            uint32_t const prev_common = sub(prev_coin, kCommonLadderAdded);
            uint32_t const prev_stinger = sub(prev_common, kStingerLegAdded);
            uint32_t const prev_jungle = sub(prev_stinger, kJungleAdded);
            uint32_t const prev_ladder = sub(prev_jungle, kLadderAdded);
            uint32_t const prev_royal = sub(prev_ladder, kRoyalAdded);
            uint32_t const bank_candidates[] = {
                n, prev_honey_meteor_ladder, prev_honey_meteor, prev_monstera_ladder, prev_anthell, prev_super,
                prev_cross, prev_basecraft, prev_mats, prev_monstera, prev_coin,
                prev_common, prev_stinger, prev_jungle, prev_ladder, prev_royal
            };

            auto layout_ok = [&](uint32_t petal_n, uint32_t preset_w) -> bool {
                if (fields.size() < petal_n) return false;
                size_t rem = fields.size() - petal_n;
                // Mid block: coins + craft_attempts + talents + session.
                // Old saves: 2 talents → mid 5. New saves: 7 talents → mid 10.
                for (uint32_t bank_n : bank_candidates) {
                    for (uint32_t mid : {5u, 10u}) {
                        size_t const presets = bank_n + mid + LOADOUT_PRESET_COUNT * preset_w;
                        size_t const full = presets + 4;      // + bonus block
                        size_t const full_ap = full + 1;       // + active_preset
                        if (rem == full || rem == full_ap || rem == presets) return true;
                        if (rem >= bank_n && rem <= bank_n + mid) return true;
                    }
                }
                // True pre-per-petal banks: exactly 7 rarity buckets (+ optional coins).
                if (petal_n == kPetalsLegacy8 && (rem == 7 || rem == 8))
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
            // Pick the largest known historical bank length that still fits.
            for (uint32_t cand : bank_candidates) {
                if (rest.size() >= cand) {
                    bank_n = cand;
                    break;
                }
            }
            if (rest.size() >= bank_n && !(rest.size() == 7 || rest.size() == 8)) {
                for (uint32_t i = 0; i < bank_n; ++i)
                    acc.craft_bank[i] = (uint32_t) rest[i];
                if (rest.size() > bank_n)
                    acc.coins = (uint32_t) rest[bank_n];
                if (rest.size() > bank_n + 1)
                    acc.craft_attempts = (uint32_t) rest[bank_n + 1];
                // Talents before session_hash: legacy 2 (hp, rot) or new 7 ranks.
                uint8_t talent_n = 2;
                if (rest.size() > bank_n + 2) {
                    size_t const after = rest.size() - (bank_n + 2);
                    size_t const preset_block = (size_t) LOADOUT_PRESET_COUNT * preset_w;
                    auto fits_talent = [&](uint8_t tn) -> bool {
                        if (after == tn) return true;
                        if (after == tn + 1) return true; // + session
                        if (after == tn + 1 + preset_block) return true;
                        if (after == tn + 1 + preset_block + 4) return true; // + bonus
                        if (after == tn + 1 + preset_block + 5) return true; // + active_preset
                        return false;
                    };
                    if (fits_talent(TalentID::kNum))
                        talent_n = TalentID::kNum;
                    else if (fits_talent(2))
                        talent_n = 2;
                    else if (after >= TalentID::kNum)
                        talent_n = TalentID::kNum;
                }
                for (uint8_t i = 0; i < talent_n; ++i) {
                    if (rest.size() <= bank_n + 2 + i) break;
                    acc.talent_ranks[i] = (uint8_t) std::min<uint64_t>(rest[bank_n + 2 + i], 99);
                }
                uint32_t const after_talents = bank_n + 2 + talent_n;
                if (rest.size() > after_talents)
                    acc.session_hash = rest[after_talents];
                uint32_t const preset_petal_n = preset_w > 0 ? preset_w - 1 : 0;
                uint32_t const preset_at0 = after_talents + 1;
                if (rest.size() >= preset_at0 + LOADOUT_PRESET_COUNT * preset_w) {
                    uint32_t at = preset_at0;
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
                    // Optional active K/L preset index (absent on older saves).
                    if (rest.size() > at) {
                        uint64_t ap = rest[at++];
                        acc.active_loadout_preset = ap < LOADOUT_PRESET_COUNT
                            ? (uint8_t) ap : LOADOUT_PRESET_COUNT;
                    }
                }
            } else if (rest.size() == 7 || rest.size() == 8) {
                // Legacy rarity buckets only — never treat a truncated modern bank
                // as buckets (that parked huge counts on Basic + Stinger).
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
        // Trailing tokens after numeric fields (optional on old saves):
        //   g:<name> <leader>   s:<score>
        {
            ls.clear();
            std::string tok;
            while (ls >> tok) {
                if (tok.size() >= 2 && tok[0] == 'g' && tok[1] == ':') {
                    std::string gname = tok.substr(2);
                    if (gname != "-") acc.guild_name = gname;
                    uint32_t glead = 0;
                    if (ls >> glead) acc.guild_leader = glead ? 1 : 0;
                } else if (tok.size() >= 2 && tok[0] == 's' && tok[1] == ':') {
                    try {
                        unsigned long long v = std::stoull(tok.substr(2));
                        if (v > 0xffffffffull) v = 0xffffffffull;
                        acc.score = (uint32_t) v;
                    } catch (...) {
                        acc.score = 0;
                    }
                }
            }
        }
        // Legacy rows have no s: — start at level floor. Keep score ≥ floor.
        {
            uint32_t floor = level_to_score(acc.level < 1 ? 1 : acc.level);
            if (acc.score < floor) acc.score = floor;
            uint32_t implied = score_to_level(acc.score);
            if (implied > acc.level) acc.level = implied;
            if (acc.level > MAX_PLAYER_LEVEL) acc.level = MAX_PLAYER_LEVEL;
            uint32_t const cap = level_to_score(MAX_PLAYER_LEVEL);
            if (acc.score > cap) acc.score = cap;
        }
        ACCOUNTS[acc.username] = acc;
    }
    std::cout << "[accounts] loaded " << ACCOUNTS.size() << " accounts" << std::endl;
    // Ensure accounts.dat always exists on disk after boot (empty file OK)
    save();
}

void AccountStore::save() {
    std::ostringstream out;
    out << "# ashflor accounts  fields: user hash level petals[20] craft_bank[N] coins craft_attempts talent_ranks[7] session_hash presets[20][slots+10 petals] bonus_day daily_bonus offline_bonus last_seen active_preset g:guild leader s:score\n";
    for (auto const &kv : ACCOUNTS) {
        Account const &acc = kv.second;
        out << acc.username << ' ' << acc.pass_hash << ' ' << (uint32_t) acc.level;
        for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i)
            out << ' ' << (uint32_t) acc.petals[i];
        for (uint32_t i = 0; i < PetalID::kNumPetals; ++i)
            out << ' ' << acc.craft_bank[i];
        out << ' ' << acc.coins << ' ' << acc.craft_attempts;
        for (uint8_t i = 0; i < TalentID::kNum; ++i)
            out << ' ' << (uint32_t) acc.talent_ranks[i];
        out << ' ' << acc.session_hash;
        for (LoadoutPreset const &preset : acc.loadout_presets) {
            out << ' ' << (uint32_t) preset.slot_count;
            for (PetalID::T id : preset.petals)
                out << ' ' << (uint32_t) id;
        }
        out << ' ' << acc.bonus_day << ' ' << acc.daily_bonus_seconds
            << ' ' << acc.offline_bonus_seconds << ' ' << acc.last_seen_epoch
            << ' ' << (uint32_t) acc.active_loadout_preset;
        out << " g:" << (acc.guild_name.empty() ? "-" : acc.guild_name)
            << ' ' << (uint32_t) (acc.guild_leader ? 1 : 0);
        uint32_t sc = acc.score;
        uint32_t floor = level_to_score(acc.level < 1 ? 1 : acc.level);
        if (sc < floor) sc = floor;
        out << " s:" << sc;
        out << '\n';
    }
    write_file_atomic(ACCOUNT_FILE, out.str());
    maybe_write_snapshot(ACCOUNT_FILE);
}

uint32_t AccountStore::registered_count() {
    return (uint32_t) ACCOUNTS.size();
}

void AccountStore::for_each_account(std::function<void(std::string const &, Account &)> const &fn) {
    for (auto &kv : ACCOUNTS)
        fn(kv.first, kv.second);
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

static uint8_t petal_is_filler(PetalID::T id) {
    return id == PetalID::kNone || id == PetalID::kBasic || id == PetalID::kUniqueBasic;
}

static uint32_t count_valuable_petals(std::array<PetalID::T, 2 * MAX_SLOT_COUNT> const &p) {
    uint32_t n = 0;
    for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i)
        if (!petal_is_filler(p[i])) ++n;
    return n;
}

// Primary bar only — bag/secondary is a separate authority.
static uint8_t primary_is_trivial(
    std::array<PetalID::T, 2 * MAX_SLOT_COUNT> const &p, uint32_t primary_n
) {
    if (primary_n > MAX_SLOT_COUNT) primary_n = MAX_SLOT_COUNT;
    for (uint32_t i = 0; i < primary_n; ++i)
        if (!petal_is_filler(p[i])) return 0;
    return 1;
}

static uint32_t account_primary_slots(Account const &acc, Client *client) {
    uint32_t n = client != nullptr
        ? Talent::slot_count_from_ranks(client->talent_ranks)
        : Talent::slot_count_from_ranks(acc.talent_ranks);
    if (n < 5) n = 5;
    if (n > MAX_SLOT_COUNT) n = MAX_SLOT_COUNT;
    return n;
}

// If primary was wiped but an active K/L preset still names the gear, rebuild
// the bar from owned petals (bag + leftover primary). Never deletes bag items.
static void restore_primary_from_active_preset(Account &acc) {
    uint8_t idx = acc.active_loadout_preset;
    if (idx >= LOADOUT_PRESET_COUNT) return;
    LoadoutPreset const &preset = acc.loadout_presets[idx];
    if (preset.slot_count == 0) return;
    uint32_t primary_n = account_primary_slots(acc, nullptr);
    if (!primary_is_trivial(acc.petals, primary_n)) return;
    std::array<uint32_t, PetalID::kNumPetals> overflow = {};
    uint8_t fallback = 0;
    if (!arrange_loadout_preset(acc.petals, (uint8_t) primary_n, preset, overflow, fallback))
        return;
    for (uint32_t id = 0; id < PetalID::kNumPetals; ++id) {
        if (overflow[id] == 0) continue;
        uint64_t sum = (uint64_t) acc.craft_bank[id] + overflow[id];
        if (sum > 1000000ull) sum = 1000000ull;
        acc.craft_bank[id] = (uint32_t) sum;
    }
}

static void copy_progress(Account &acc, Client *client) {
    if (client == nullptr || client->game == nullptr) return;
    Simulation *sim = &client->game->simulation;
    if (!sim->ent_exists(client->camera)) return;
    Entity &camera = sim->get_ent(client->camera);
    std::array<PetalID::T, 2 * MAX_SLOT_COUNT> next = {};
    next.fill(PetalID::kNone);
    // Level + full score are high-water marks — never regress on capture.
    uint32_t level = 1;
    uint32_t score = 0;
    // Prefer flower loadout whenever the entity still exists — even if
    // pending_delete. While alive, camera inventory is cleared to None at spawn;
    // reading it after request_delete but before entity_on_death wipes accounts.
    EntityID pid = camera.get_player();
    if (sim->ent_exists(pid) && sim->get_ent(pid).has_component(kFlower)) {
        Entity &player = sim->get_ent(pid);
        score = player.get_score();
        if (client->pending_score > score) score = client->pending_score;
        uint32_t lv = score_to_level(score);
        if (lv < 1) lv = 1;
        if (lv < camera.get_respawn_level()) lv = camera.get_respawn_level();
        level = lv;
        for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i)
            next[i] = player.get_loadout_ids(i);
    } else {
        level = camera.get_respawn_level();
        if (level < 1) level = 1;
        score = level_to_score(level);
        if (client->pending_score > score) score = client->pending_score;
        uint32_t from_sc = score_to_level(score);
        if (from_sc > level) level = from_sc;
        for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i)
            next[i] = camera.get_inventory(i);
    }
    // Future soft ceiling only (post-migration grind). Never wipe inventory.
    if (level > MAX_PLAYER_LEVEL) level = MAX_PLAYER_LEVEL;
    uint32_t const cap = level_to_score(MAX_PLAYER_LEVEL);
    if (score > cap) score = cap;
    if (level > acc.level) acc.level = level;
    if (acc.level > MAX_PLAYER_LEVEL) acc.level = MAX_PLAYER_LEVEL;
    if (score > acc.score) acc.score = score;
    {
        uint32_t floor = level_to_score(acc.level);
        if (acc.score < floor) acc.score = floor;
        uint32_t implied = score_to_level(acc.score);
        if (implied > acc.level) acc.level = implied;
        if (acc.level > MAX_PLAYER_LEVEL) acc.level = MAX_PLAYER_LEVEL;
    }
    // Keep session pending in sync so the next spawn / reconnect matches disk.
    if (client->pending_score < acc.score)
        client->pending_score = acc.score;

    uint32_t const primary_n = account_primary_slots(acc, client);
    uint32_t const next_v = count_valuable_petals(next);
    uint32_t const acc_v = count_valuable_petals(acc.petals);
    // Primary and bag are separate authorities. A mid-drain snap with an empty
    // primary bar but intact bag used to overwrite and DELETE equipped petals.
    // Refuse that: keep previous primary when valuables were lost.
    if (primary_is_trivial(next, primary_n)
        && !primary_is_trivial(acc.petals, primary_n)
        && next_v < acc_v) {
        for (uint32_t i = primary_n; i < primary_n + MAX_SLOT_COUNT
            && i < 2 * MAX_SLOT_COUNT; ++i)
            acc.petals[i] = next[i];
    } else {
        acc.petals = next;
    }
    normalize_petals(acc);

    // Refresh the active K/L preset from the live primary so redeploy restores it.
    // Never used a scheme yet → park current bar on K1.
    if (!primary_is_trivial(acc.petals, primary_n)) {
        if (client->active_loadout_preset >= LOADOUT_PRESET_COUNT)
            client->active_loadout_preset = 0;
        LoadoutPreset &p = client->loadout_presets[client->active_loadout_preset];
        p.slot_count = (uint8_t) primary_n;
        p.petals.fill(PetalID::kNone);
        for (uint32_t i = 0; i < primary_n; ++i)
            p.petals[i] = acc.petals[i];
    }

    acc.craft_bank = client->craft_bank;
    acc.coins = client->coins;
    acc.craft_attempts = client->craft_attempts;
    for (uint8_t i = 0; i < TalentID::kNum; ++i)
        acc.talent_ranks[i] = client->talent_ranks[i];
    Talent::clamp_ranks(acc.talent_ranks, acc.level);
    acc.loadout_presets = client->loadout_presets;
    acc.active_loadout_preset = client->active_loadout_preset;
    acc.bonus_day = client->bonus_day;
    acc.daily_bonus_seconds = client->daily_bonus_seconds;
    acc.offline_bonus_seconds = client->offline_bonus_seconds;
    acc.last_seen_epoch = wall_seconds();
    acc.guild_name = client->guild_name;
    acc.guild_leader = client->guild_leader ? 1 : 0;
}

uint8_t AccountStore::register_account(Client *client, std::string const &username, std::string const &password) {
    if (!valid_username(username)) return AccountResultCode::kAccountInvalidUser;
    if (!valid_password(password)) return AccountResultCode::kAccountInvalidPass;
    if (name_reserved_for_guest(username)) return AccountResultCode::kAccountExists;
    Account acc;
    acc.username = username;
    acc.pass_hash = hash_password(username, password);
    acc.level = 1;
    acc.score = level_to_score(1);
    acc.petals.fill(PetalID::kNone);
    for (uint32_t i = 0; i < Talent::slot_count_from_ranks(acc.talent_ranks); ++i)
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
    {
        uint32_t lv = acc.level;
        if (lv < 1) lv = 1;
        if (lv > MAX_PLAYER_LEVEL) lv = MAX_PLAYER_LEVEL;
        camera.set_respawn_level(lv);
    }
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
    // Rotate even on resume so a kicked tab cannot reconnect with the old
    // token and bounce the new session (single active login).
    if (result == AccountResultCode::kAccountOk)
        issue_session_token(username);
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

void AccountStore::prune_stale_sessions() {
    if (ACTIVE_SESSIONS.empty()) return;
    std::vector<std::string> stale;
    stale.reserve(ACTIVE_SESSIONS.size());
    for (std::string const &name : ACTIVE_SESSIONS) {
        if (find_live_account_holder(name) == nullptr)
            stale.push_back(name);
    }
    for (std::string const &name : stale) {
        ACTIVE_SESSIONS.erase(name);
        std::cout << "[accounts] cleared stale session lock: " << name << std::endl;
    }
}

void AccountStore::capture_nosave(Client *client) {
    if (client == nullptr || client->account_name.empty()) return;
    Account *acc = find(client->account_name);
    if (acc == nullptr) return;
    // Always keep guild fields even if the camera is gone mid-teardown.
    acc->guild_name = client->guild_name;
    acc->guild_leader = client->guild_leader ? 1 : 0;
    if (client->game != nullptr)
        copy_progress(*acc, client);
}

void AccountStore::capture(Client *client) {
    capture_nosave(client);
    if (client != nullptr && !client->account_name.empty())
        save();
}
