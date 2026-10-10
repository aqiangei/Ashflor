#include <Server/AccountStore.hh>

#include <Server/Client.hh>
#include <Server/Game.hh>
#include <Server/PetalTracker.hh>
#include <Server/Server.hh>

#include <Shared/Binary.hh>
#include <Shared/Config.hh>
#include <Shared/Entity.hh>
#include <Shared/Simulation.hh>

#include <cctype>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <vector>

#ifdef WASM_SERVER
#include <emscripten.h>
#endif

static char const *ACCOUNT_FILE = "accounts.dat";
static std::unordered_map<std::string, Account> ACCOUNTS;
static std::unordered_set<std::string> ACTIVE_SESSIONS;

static std::string read_file(char const *path) {
#ifdef WASM_SERVER
    char *ptr = (char *) EM_ASM_PTR({
        const fs = require("fs");
        const p = UTF8ToString($0);
        try {
            if (!fs.existsSync(p)) return 0;
            const buf = fs.readFileSync(p);
            const mem = _malloc(buf.length + 1);
            HEAPU8.set(buf, mem);
            HEAPU8[mem + buf.length] = 0;
            return mem;
        } catch (e) {
            return 0;
        }
    }, path);
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

static void write_file(char const *path, std::string const &data) {
#ifdef WASM_SERVER
    EM_ASM({
        const fs = require("fs");
        const p = UTF8ToString($0);
        const bytes = HEAPU8.subarray($1, $1 + $2);
        fs.writeFileSync(p, Buffer.from(bytes));
    }, path, data.data(), (int) data.size());
#else
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out.write(data.data(), (std::streamsize) data.size());
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

void AccountStore::load() {
    ACCOUNTS.clear();
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
        for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i) {
            unsigned int v = PetalID::kNone;
            if (!(ls >> v)) break;
            if (v >= PetalID::kNumPetals) v = PetalID::kNone;
            acc.petals[i] = (PetalID::T) v;
        }
        for (uint32_t i = 0; i < RarityID::kNumRarities; ++i) {
            unsigned int v = 0;
            if (!(ls >> v)) break;
            acc.craft_bank[i] = v;
        }
        {
            unsigned int v = 0;
            if (ls >> v) acc.coins = v;
        }
        ACCOUNTS[acc.username] = acc;
    }
    std::printf("loaded %zu accounts\n", ACCOUNTS.size());
}

void AccountStore::save() {
    std::ostringstream out;
    out << "# gardn accounts\n";
    for (auto const &kv : ACCOUNTS) {
        Account const &acc = kv.second;
        out << acc.username << ' ' << acc.pass_hash << ' ' << (uint32_t) acc.level;
        for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i)
            out << ' ' << (uint32_t) acc.petals[i];
        for (uint32_t i = 0; i < RarityID::kNumRarities; ++i)
            out << ' ' << acc.craft_bank[i];
        out << ' ' << acc.coins;
        out << '\n';
    }
    write_file(ACCOUNT_FILE, out.str());
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
}

uint8_t AccountStore::register_account(Client *client, std::string const &username, std::string const &password) {
    if (!valid_username(username)) return AccountResultCode::kAccountInvalidUser;
    if (!valid_password(password)) return AccountResultCode::kAccountInvalidPass;
    if (find(username) != nullptr) return AccountResultCode::kAccountExists;
    Account acc;
    acc.username = username;
    acc.pass_hash = hash_password(username, password);
    acc.level = 1;
    acc.petals.fill(PetalID::kNone);
    acc.craft_bank.fill(0);
    acc.coins = 0;
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
    if (ACTIVE_SESSIONS.contains(username) && client->account_name != username)
        return AccountResultCode::kAccountAlreadyIn;
    if (!client->account_name.empty())
        ACTIVE_SESSIONS.erase(client->account_name);
    client->account_name = username;
    client->craft_bank = acc->craft_bank;
    client->coins = acc->coins;
    ACTIVE_SESSIONS.insert(username);
    if (client->game != nullptr && client->game->simulation.ent_exists(client->camera) && !client->alive())
        apply_to_camera(&client->game->simulation, client->game->simulation.get_ent(client->camera), *acc);
    return AccountResultCode::kAccountOk;
}

void AccountStore::logout(Client *client) {
    if (!client->account_name.empty()) {
        ACTIVE_SESSIONS.erase(client->account_name);
        client->account_name.clear();
    }
}

void AccountStore::capture(Client *client) {
    if (client == nullptr || client->account_name.empty() || client->game == nullptr) return;
    Account *acc = find(client->account_name);
    if (acc == nullptr) return;
    copy_progress(*acc, client);
    save();
}
