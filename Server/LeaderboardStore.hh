#pragma once

#include <Shared/Binary.hh>

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

class Client;

struct LeaderboardEntry {
    std::string name;
    uint32_t score = 0;
    uint8_t color = 0;
    // Stats board counters (see LeaderboardCategory)
    uint32_t playtime = 0; // seconds online
    uint32_t level = 0;    // account level
    uint32_t messages = 0; // chat messages sent
    uint32_t kills = 0;    // players killed
    uint32_t crafts = 0;   // craft attempts
};

struct LeaderboardStat {
    std::string name;
    uint64_t value = 0;
};

namespace LeaderboardStore {
    void load();
    void save(); // no-op unless dirty
    void flush(); // force write if dirty
    uint32_t recorded_count();
    // Upsert by name; keeps the higher score and refreshes color when score ties/raises.
    void record(std::string const &name, uint32_t score, uint8_t color);
    // Absolute set (may lower). Used by XP inflation migration only.
    void force_set_level_score(std::string const &name, uint32_t level, uint32_t score);
    void for_each_entry_mutable(std::function<void(LeaderboardEntry &)> const &fn);
    // Fill top LEADERBOARD_SIZE entries sorted by score desc.
    void fill_top(std::vector<LeaderboardEntry> &out);
    // True if another connected client already claimed this play name.
    uint8_t name_taken(std::string const &name, Client *self);

    // --- Stats board -------------------------------------------------
    // Adds to a counter category; the level category keeps the highest value.
    void add_stat(std::string const &name, uint8_t category, uint32_t amount);
    // Current value of one category for a stored player.
    uint64_t stat_value(LeaderboardEntry const &entry, uint8_t category);
    // Top `limit` players of one category, best first (entries with 0 are skipped).
    void fill_top_stats(std::vector<LeaderboardStat> &out, uint8_t category, uint32_t limit);
    // 1-based rank of a player inside a category; 0 when the player is unranked.
    uint32_t stat_rank(std::string const &name, uint8_t category, uint64_t &value);
}
