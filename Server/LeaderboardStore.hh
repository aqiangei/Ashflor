#pragma once

#include <cstdint>
#include <string>
#include <vector>

class Client;

struct LeaderboardEntry {
    std::string name;
    uint32_t score = 0;
    uint8_t color = 0;
};

namespace LeaderboardStore {
    void load();
    void save(); // no-op unless dirty
    void flush(); // force write if dirty
    uint32_t recorded_count();
    // Upsert by name; keeps the higher score and refreshes color when score ties/raises.
    void record(std::string const &name, uint32_t score, uint8_t color);
    // Fill top LEADERBOARD_SIZE entries sorted by score desc.
    void fill_top(std::vector<LeaderboardEntry> &out);
    // True if another connected client already claimed this play name.
    uint8_t name_taken(std::string const &name, Client *self);
};
