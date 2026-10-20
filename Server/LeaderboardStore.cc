#include <Server/LeaderboardStore.hh>

#include <Server/Client.hh>
#include <Server/Game.hh>
#include <Server/NodeFs.hh>
#include <Server/Server.hh>

#include <Shared/StaticDefinitions.hh>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <unordered_map>
#include <vector>

static char const *LB_FILE = "leaderboard.dat";
static std::unordered_map<std::string, LeaderboardEntry> ENTRIES;
static uint8_t DIRTY = 0;

static std::string read_file(char const *path) {
#ifdef WASM_SERVER
    char *ptr = node_fs_read_alloc(path);
    if (ptr == nullptr) return {};
    std::string out(ptr);
    free(ptr);
    return out;
#else
    FILE *f = fopen(path, "rb");
    if (!f) return {};
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz <= 0) { fclose(f); return {}; }
    std::string out((size_t) sz, '\0');
    fread(out.data(), 1, (size_t) sz, f);
    fclose(f);
    return out;
#endif
}

static void write_file(char const *path, std::string const &data) {
#ifdef WASM_SERVER
    node_fs_write_atomic(path, data.data(), (int) data.size());
#else
    FILE *f = fopen(path, "wb");
    if (!f) return;
    fwrite(data.data(), 1, data.size(), f);
    fclose(f);
#endif
}

// Escape so names may contain spaces: replace \ and space
static std::string escape_name(std::string const &s) {
    std::string out;
    out.reserve(s.size());
    for (unsigned char c : s) {
        if (c == '\\') out += "\\\\";
        else if (c == ' ') out += "\\s";
        else if (c == '\n' || c == '\r') continue;
        else out += (char) c;
    }
    return out;
}

static std::string unescape_name(std::string const &s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '\\' && i + 1 < s.size()) {
            char n = s[++i];
            if (n == '\\') out += '\\';
            else if (n == 's') out += ' ';
            else out += n;
        } else out += s[i];
    }
    return out;
}

void LeaderboardStore::load() {
    ENTRIES.clear();
    DIRTY = 0;
    std::string raw = read_file(LB_FILE);
    if (raw.empty()) return;
    // One entry per line: name score color [playtime level messages kills crafts].
    // Older files only carry the first three fields; the rest stay zero.
    std::istringstream in(raw);
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        std::istringstream ls(line);
        std::string enc;
        if (!(ls >> enc)) continue;
        std::string name = unescape_name(enc);
        if (name.empty()) continue;
        LeaderboardEntry e;
        e.name = name;
        // NOTE: uint8_t is read as a *character* by operator>>, so every
        // numeric field goes through a uint32_t and is narrowed afterwards.
        uint32_t score = 0, color = 0, playtime = 0, level = 0;
        uint32_t messages = 0, kills = 0, crafts = 0;
        if (!(ls >> score)) continue;
        if (!(ls >> color)) color = 0;
        if (!(ls >> playtime)) playtime = 0;
        if (!(ls >> level)) level = 0;
        if (!(ls >> messages)) messages = 0;
        if (!(ls >> kills)) kills = 0;
        if (!(ls >> crafts)) crafts = 0;
        e.score = score;
        e.color = (uint8_t) (color & 0xff);
        e.playtime = playtime;
        e.level = level;
        e.messages = messages;
        e.kills = kills;
        e.crafts = crafts;
        ENTRIES[name] = e;
    }
}

void LeaderboardStore::save() {
    if (!DIRTY) return;
    flush();
}

void LeaderboardStore::flush() {
    if (!DIRTY) return;
    DIRTY = 0;
    std::ostringstream out;
    for (auto const &kv : ENTRIES) {
        out << escape_name(kv.second.name) << ' '
            << kv.second.score << ' '
            << (uint32_t) kv.second.color << ' '
            << kv.second.playtime << ' '
            << kv.second.level << ' '
            << kv.second.messages << ' '
            << kv.second.kills << ' '
            << kv.second.crafts << '\n';
    }
    write_file(LB_FILE, out.str());
}

uint32_t LeaderboardStore::recorded_count() {
    return (uint32_t) ENTRIES.size();
}

void LeaderboardStore::record(std::string const &name, uint32_t score, uint8_t color) {
    if (name.empty()) return;
    auto it = ENTRIES.find(name);
    if (it == ENTRIES.end()) {
        LeaderboardEntry e;
        e.name = name;
        e.score = score;
        e.color = color;
        ENTRIES[name] = e;
        DIRTY = 1;
        return;
    }
    if (score > it->second.score) {
        it->second.score = score;
        it->second.color = color;
        DIRTY = 1;
    } else if (it->second.color != color) {
        it->second.color = color;
        DIRTY = 1;
    }
}

void LeaderboardStore::fill_top(std::vector<LeaderboardEntry> &out) {
    out.clear();
    out.reserve(ENTRIES.size());
    for (auto const &kv : ENTRIES)
        out.push_back(kv.second);
    std::stable_sort(out.begin(), out.end(), [](LeaderboardEntry const &a, LeaderboardEntry const &b) {
        return a.score > b.score;
    });
    if (out.size() > LEADERBOARD_SIZE)
        out.resize(LEADERBOARD_SIZE);
}

uint8_t LeaderboardStore::name_taken(std::string const &name, Client *self) {
    if (name.empty()) return 0;
    for (uint8_t i = 0; i < MapID::kNumMaps; ++i) {
        for (Client *c : Server::games[i].clients) {
            if (c == nullptr || c == self) continue;
            if (!c->play_name.empty() && c->play_name == name) return 1;
        }
    }
    return 0;
}

// --- Stats board -----------------------------------------------------------

uint64_t LeaderboardStore::stat_value(LeaderboardEntry const &entry, uint8_t category) {
    switch (category) {
        case kLbPlayTime: return entry.playtime;
        case kLbLevel: return entry.level;
        case kLbMessages: return entry.messages;
        case kLbKills: return entry.kills;
        case kLbCrafts: return entry.crafts;
        case kLbScore: return entry.score;
        default: return 0;
    }
}

void LeaderboardStore::add_stat(std::string const &name, uint8_t category, uint32_t amount) {
    if (name.empty() || category >= kLbCategoryCount) return;
    auto it = ENTRIES.find(name);
    if (it == ENTRIES.end()) {
        if (amount == 0 && category != kLbLevel) return;
        LeaderboardEntry e;
        e.name = name;
        ENTRIES[name] = e;
        it = ENTRIES.find(name);
    }
    LeaderboardEntry &e = it->second;
    uint32_t before = 0;
    switch (category) {
        case kLbPlayTime: before = e.playtime; break;
        case kLbLevel: before = e.level; break;
        case kLbMessages: before = e.messages; break;
        case kLbKills: before = e.kills; break;
        case kLbCrafts: before = e.crafts; break;
        case kLbScore: before = e.score; break;
        default: return;
    }
    uint32_t after = before;
    if (category == kLbLevel || category == kLbScore)
        after = amount > before ? amount : before; // level and best score only go up
    else
        after = before + amount; // counters accumulate across sessions
    if (after == before) return;
    switch (category) {
        case kLbPlayTime: e.playtime = after; break;
        case kLbLevel: e.level = after; break;
        case kLbMessages: e.messages = after; break;
        case kLbKills: e.kills = after; break;
        case kLbCrafts: e.crafts = after; break;
        case kLbScore: e.score = after; break;
        default: break;
    }
    DIRTY = 1;
}

void LeaderboardStore::fill_top_stats(std::vector<LeaderboardStat> &out, uint8_t category, uint32_t limit) {
    out.clear();
    if (category >= kLbCategoryCount) return;
    for (auto const &kv : ENTRIES) {
        uint64_t value = stat_value(kv.second, category);
        if (value == 0) continue;
        out.push_back(LeaderboardStat{kv.second.name, value});
    }
    std::stable_sort(out.begin(), out.end(), [](LeaderboardStat const &a, LeaderboardStat const &b) {
        if (a.value != b.value) return a.value > b.value;
        return a.name < b.name;
    });
    if (out.size() > limit) out.resize(limit);
}

uint32_t LeaderboardStore::stat_rank(std::string const &name, uint8_t category, uint64_t &value) {
    value = 0;
    if (name.empty() || category >= kLbCategoryCount) return 0;
    auto it = ENTRIES.find(name);
    if (it == ENTRIES.end()) return 0;
    uint64_t mine = stat_value(it->second, category);
    value = mine;
    if (mine == 0) return 0; // unranked until the player records something
    uint32_t rank = 1;
    for (auto const &kv : ENTRIES) {
        if (kv.first == name) continue;
        if (stat_value(kv.second, category) > mine) ++rank;
    }
    return rank;
}
