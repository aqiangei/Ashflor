#include <Server/AccountStore.hh>

#include <Server/Client.hh>
#include <Server/Game.hh>
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
    if (!client->account_name.empty())
        ACTIVE_SESSIONS.erase(client->account_name);
    client->account_name = username;
    client->craft_bank = acc->craft_bank;
    client->coins = acc->coins;
    client->craft_attempts = acc->craft_attempts;
    uint8_t max_r = Talent::max_rank(acc->level);
    client->talent_hp = acc->talent_hp > max_r ? max_r : acc->talent_hp;
    client->talent_rot = acc->talent_rot > max_r ? max_r : acc->talent_rot;
    ACTIVE_SESSIONS.insert(username);
    if (client->game != nullptr && client->game->simulation.ent_exists(client->camera) && !client->alive())
        AccountStore::apply_to_camera(&client->game->simulation, client->game->simulation.get_ent(client->camera), *acc);
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
        ls >> acc.username >> acc.pass_hash >> acc.level;
        if (acc.username.empty() || !valid_username(acc.username)) continue;
        if (acc.level < 1) acc.level = 1;
        if (acc.level > MAX_LEVEL) acc.level = MAX_LEVEL;
        acc.petals.fill(PetalID::kNone);
        acc.craft_bank.fill(0);
        acc.coins = 0;
        acc.craft_attempts = 0;
        acc.talent_hp = 0;
        acc.talent_rot = 0;
        for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i) {
            unsigned int v = PetalID::kNone;
            if (!(ls >> v)) break;
            if (v >= PetalID::kNumPetals) v = PetalID::kNone;
            acc.petals[i] = (PetalID::T) v;
        }
        {
            std::vector<uint64_t> rest;
            uint64_t v = 0;
            while (ls >> v) rest.push_back(v);
            // craft_bank grew when Royal petals were appended; accept prior length.
            uint32_t bank_n = PetalID::kNumPetals;
            // Accept saves from before rarity ladders (+43) and/or Royal petals (+12)
            constexpr uint32_t kLadderAdded = 43;
            constexpr uint32_t kRoyalAdded = 12;
            uint32_t const prev_ladder = PetalID::kNumPetals > kLadderAdded
                ? (uint32_t)(PetalID::kNumPetals - kLadderAdded) : PetalID::kNumPetals;
            uint32_t const prev_royal = prev_ladder > kRoyalAdded
                ? prev_ladder - kRoyalAdded : prev_ladder;
            if (rest.size() < bank_n && rest.size() >= prev_ladder)
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
            } else if (rest.size() >= RarityID::kNumRarities) {
                // Legacy rarity buckets → park counts on first petal of that rarity
                for (uint8_t r = 0; r < RarityID::kNumRarities; ++r) {
                    PetalID::T id = Craft::first_petal_of_rarity(r);
                    if (id != PetalID::kNone)
                        acc.craft_bank[id] = (uint32_t) rest[r];
                }
                if (rest.size() > RarityID::kNumRarities)
                    acc.coins = (uint32_t) rest[RarityID::kNumRarities];
            }
        }
        ACCOUNTS[acc.username] = acc;
    }
    std::cout << "[accounts] loaded " << ACCOUNTS.size() << " accounts" << std::endl;
    // Ensure accounts.dat always exists on disk after boot (empty file OK)
    save();
}

void AccountStore::save() {
    std::ostringstream out;
    out << "# ashflor accounts  fields: user hash level petals[16] craft_bank[N] coins craft_attempts talent_hp talent_rot session_hash\n";
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
        out << '\n';
    }
    write_file_atomic(ACCOUNT_FILE, out.str());
    maybe_write_snapshot(ACCOUNT_FILE);
}

uint32_t AccountStore::registered_count() {
    return (uint32_t) ACCOUNTS.size();
}

static void normalize_petals(Account &acc) {
    uint32_t slots = loadout_slots_at_level(acc.level);
    for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i) {
        if (acc.petals[i] >= PetalID::kNumPetals)
            acc.petals[i] = PetalID::kNone;
    }
    for (uint32_t i = 0; i < slots; ++i) {
        if (acc.petals[i] == PetalID::kNone)
            acc.petals[i] = PetalID::kBasic;
    }
}

static void copy_progress(Account &acc, Client *client) {
    if (client == nullptr || client->game == nullptr) return;
    Simulation *sim = &client->game->simulation;
    if (!sim->ent_exists(client->camera)) return;
    Entity &camera = sim->get_ent(client->camera);
    acc.petals.fill(PetalID::kNone);
    if (sim->ent_alive(camera.get_player())) {
        Entity &player = sim->get_ent(camera.get_player());
        uint32_t level = score_to_level(player.get_score());
        if (level < 1) level = 1;
        if (level > MAX_LEVEL) level = MAX_LEVEL;
        acc.level = (uint8_t) level;
        for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i)
            acc.petals[i] = player.get_loadout_ids(i);
    } else {
        uint8_t level = camera.get_respawn_level();
        if (level < 1) level = 1;
        if (level > MAX_LEVEL) level = MAX_LEVEL;
        acc.level = level;
        for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i)
            acc.petals[i] = camera.get_inventory(i);
    }
    normalize_petals(acc);
    acc.craft_bank = client->craft_bank;
    acc.coins = client->coins;
    acc.craft_attempts = client->craft_attempts;
    uint8_t max_r = Talent::max_rank(acc.level);
    acc.talent_hp = client->talent_hp > max_r ? max_r : client->talent_hp;
    acc.talent_rot = client->talent_rot > max_r ? max_r : client->talent_rot;
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
