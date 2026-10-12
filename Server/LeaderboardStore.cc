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
    std::istringstream in(raw);
    std::string enc;
    uint32_t score = 0;
    uint32_t color = 0;
    while (in >> enc >> score >> color) {
        std::string name = unescape_name(enc);
        if (name.empty()) continue;
        LeaderboardEntry e;
        e.name = name;
        e.score = score;
        e.color = (uint8_t) (color & 0xff);
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
            << (uint32_t) kv.second.color << '\n';
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
