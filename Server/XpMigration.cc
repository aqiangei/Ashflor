#include <Server/XpMigration.hh>

#include <Server/AccountStore.hh>
#include <Server/LeaderboardStore.hh>
#include <Server/NodeFs.hh>

#include <Shared/StaticData.hh>
#include <Shared/StaticDefinitions.hh>
#include <Shared/Talent.hh>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

// Thresholds — todo/xp-inflation-migration-plan.md
static uint32_t const kSafe = 100;   // at/below: account level never touched
static uint32_t const kSoft = 150;   // above: exploded
static uint32_t const kAnchor = 100; // compressed level band (101..kCap)
static uint32_t const kCap = 130;
// v2: score compressed proportionally (not snapped to level floor).
static char const *kSentinel = "xp_migration_v2.done";
static char const *kLogFile = "xp_migration_v2.log";

static uint8_t file_exists(char const *path) {
#ifdef WASM_SERVER
    char *ptr = node_fs_read_alloc(path);
    if (ptr == nullptr) return 0;
    free(ptr);
    return 1;
#else
    std::ifstream in(path, std::ios::binary);
    return in ? 1 : 0;
#endif
}

static void write_text(char const *path, std::string const &data) {
#ifdef WASM_SERVER
    node_fs_write_atomic(path, data.data(), (int) data.size());
#else
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out.write(data.data(), (std::streamsize) data.size());
#endif
}

static std::string read_text(char const *path) {
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

static void backup_file(char const *src, std::string const &dest) {
    std::string raw = read_text(src);
    if (raw.empty() && !file_exists(src)) return;
    write_text(dest.c_str(), raw);
}

// Score band for compressed levels [kAnchor+1, kCap].
static void score_band(uint32_t &s_lo, uint32_t &s_hi) {
    s_lo = level_to_score(kAnchor + 1); // first score that is level 101
    // Last score that is still level kCap:
    uint32_t next = level_to_score(kCap + 1);
    s_hi = next > 0 ? next - 1 : level_to_score(kCap);
    if (s_hi < s_lo) s_hi = s_lo;
}

// Log-compress inflated score into [s_lo, s_hi], order-preserving.
static uint32_t compress_score(uint32_t score, uint32_t s_soft, uint32_t s_ref, uint32_t s_lo, uint32_t s_hi) {
    if (score <= s_soft) return score;
    if (s_ref <= s_soft) s_ref = s_soft + 1;
    double denom = std::log(1.0 + (double) (s_ref - s_soft));
    if (denom < 1e-9) denom = 1.0;
    double t = std::log(1.0 + (double) (score - s_soft)) / denom;
    if (t < 0.0) t = 0.0;
    if (t > 1.0) t = 1.0;
    double span = (double) (s_hi - s_lo);
    uint32_t out = s_lo + (uint32_t) std::floor(t * span + 1e-9);
    if (out < s_lo) out = s_lo;
    if (out > s_hi) out = s_hi;
    return out;
}

uint32_t XpMigration::run_once() {
    if (file_exists(kSentinel)) {
        std::cout << "[xp-mig] sentinel present, skip" << std::endl;
        return 0;
    }

    backup_file("accounts.dat", "accounts.dat.xp-mig-bak");
    backup_file("leaderboard.dat", "leaderboard.dat.xp-mig-bak");

    uint32_t s_soft = level_to_score(kSoft);
    uint32_t s_lo = 0, s_hi = 0;
    score_band(s_lo, s_hi);

    // Snapshot leaderboard scores by name (play name ≈ account username on this server).
    std::unordered_map<std::string, uint32_t> lb_score;
    std::unordered_map<std::string, uint32_t> lb_level;
    uint32_t max_score = s_soft;
    uint32_t max_lv = 0;
    LeaderboardStore::for_each_entry_mutable([&](LeaderboardEntry &e) {
        lb_score[e.name] = e.score;
        lb_level[e.name] = e.level;
        if (e.score > max_score) max_score = e.score;
        if (e.level > max_lv) max_lv = e.level;
    });

    uint32_t safe_n = 0, gray_n = 0;
    struct Row {
        std::string name;
        uint32_t old_level = 0;
        uint32_t old_score = 0; // effective score used for compression
        uint8_t has_account = 0;
    };
    std::vector<Row> exploded;

    AccountStore::for_each_account([&](std::string const &user, Account &acc) {
        if (acc.level > max_lv) max_lv = acc.level;
        uint32_t floor_sc = level_to_score(acc.level);
        uint32_t sc = floor_sc;
        auto it = lb_score.find(user);
        if (it != lb_score.end() && it->second > sc) sc = it->second;
        if (sc > max_score) max_score = sc;

        if (acc.level <= kSafe) {
            ++safe_n;
            return;
        }
        if (acc.level <= kSoft) {
            ++gray_n;
            return;
        }
        Row r;
        r.name = user;
        r.old_level = acc.level;
        r.old_score = sc;
        r.has_account = 1;
        exploded.push_back(std::move(r));
    });

    // Leaderboard-only exploded names (no account, or account was gray/safe but we
    // still need boom rows that only exist on the board).
    LeaderboardStore::for_each_entry_mutable([&](LeaderboardEntry &e) {
        if (e.level <= kSoft && e.score <= s_soft) return;
        if (e.level <= kSoft) return; // gray/safe board rows: score fix later, not boom band
        for (auto const &r : exploded)
            if (r.name == e.name) return;
        Row r;
        r.name = e.name;
        r.old_level = e.level;
        r.old_score = e.score > level_to_score(e.level) ? e.score : level_to_score(e.level);
        r.has_account = AccountStore::find(e.name) != nullptr ? 1 : 0;
        if (r.old_score > max_score) max_score = r.old_score;
        exploded.push_back(std::move(r));
    });

    uint32_t s_ref = max_score;
    if (s_ref < s_soft + 1000000u) s_ref = s_soft + 1000000u;

    std::stable_sort(exploded.begin(), exploded.end(),
        [](Row const &a, Row const &b) {
            if (a.old_score != b.old_score) return a.old_score < b.old_score;
            return a.name < b.name;
        });

    std::ostringstream log;
    log << "# xp_migration_v2  score-proportional compress\n";
    log << "# safe<=" << kSafe << " soft<=" << kSoft
        << " score_band=[" << s_lo << "," << s_hi << "]"
        << " s_soft=" << s_soft << " s_ref=" << s_ref << "\n";
    log << "# untouched_safe=" << safe_n << " untouched_gray=" << gray_n
        << " exploded=" << exploded.size() << "\n";

    uint32_t changed = 0;
    for (auto const &row : exploded) {
        uint32_t new_sc = compress_score(row.old_score, s_soft, s_ref, s_lo, s_hi);
        uint32_t new_lv = score_to_level(new_sc);
        if (new_lv < kAnchor + 1) new_lv = kAnchor + 1;
        if (new_lv > kCap) new_lv = kCap;
        // Keep score inside the declared level (fractional progress preserved).
        uint32_t lo = level_to_score(new_lv);
        uint32_t hi = level_to_score(new_lv + 1);
        if (hi > lo) hi -= 1;
        if (new_sc < lo) new_sc = lo;
        if (new_sc > hi) new_sc = hi;

        if (row.has_account) {
            Account *acc = AccountStore::find(row.name);
            if (acc != nullptr) {
                acc->score = new_sc;
                acc->level = new_lv;
                Talent::clamp_ranks(acc->talent_ranks, acc->level);
                AccountStore::mark_dirty();
            }
        }
        LeaderboardStore::force_set_level_score(row.name, new_lv, new_sc);
        log << row.name
            << " boom lv " << row.old_level << "->" << new_lv
            << " score " << row.old_score << "->" << new_sc << "\n";
        ++changed;
    }

    // Safe/gray: if board score alone exploded, snap down into *their* level band
    // (do not lift them into 101–130). No proportional boom-band mapping.
    uint32_t lb_align = 0;
    LeaderboardStore::for_each_entry_mutable([&](LeaderboardEntry &e) {
        if (e.level > kSoft) return; // already handled
        uint32_t lv = e.level > 0 ? e.level : 1;
        // Prefer account level when present (authoritative for safe/gray).
        if (Account *acc = AccountStore::find(e.name)) {
            if (acc->level >= 1 && acc->level <= kSoft)
                lv = acc->level;
        }
        uint32_t lo = level_to_score(lv);
        uint32_t hi = level_to_score(lv + 1);
        if (hi > lo) hi -= 1;
        uint32_t old_sc = e.score;
        uint32_t old_lv = e.level;
        if (old_sc <= hi && old_lv == lv) return;
        uint32_t new_sc = old_sc;
        if (new_sc > hi) new_sc = hi;
        if (new_sc < lo) new_sc = lo;
        if (new_sc == old_sc && old_lv == lv) return;
        LeaderboardStore::force_set_level_score(e.name, lv, new_sc);
        log << e.name << " align lv " << old_lv << "->" << lv
            << " score " << old_sc << "->" << new_sc << "\n";
        ++lb_align;
    });

    AccountStore::save();
    LeaderboardStore::flush();
    write_text(kLogFile, log.str());
    write_text(kSentinel, "ok\n");

    std::cout << "[xp-mig] v2 boom_rewritten=" << changed
              << " safe_gray_score_align=" << lb_align
              << " safe=" << safe_n << " gray=" << gray_n
              << " (petals/coins/crafts untouched)" << std::endl;
    return changed;
}

uint32_t XpMigration::scrub_overflow() {
    uint32_t cap = score_soft_cap(); // level_to_score(300)
    uint32_t s_soft = level_to_score(kSoft);
    uint32_t s_lo = 0, s_hi = 0;
    score_band(s_lo, s_hi);

    struct Hit {
        std::string name;
        uint32_t old_level = 0;
        uint32_t old_score = 0;
        uint8_t has_account = 0;
    };
    std::vector<Hit> hits;
    uint32_t max_score = s_soft;

    auto consider = [&](std::string const &name, uint32_t sc, uint32_t lv, uint8_t has_acc) {
        // Overflow pin, past soft XP ceiling, or stale level high-water
        // (older curves could leave level >> score_to_level(UINT32_MAX)=462).
        if (!(sc >= cap || sc == UINT32_MAX || lv > 300)) return;
        for (auto const &h : hits)
            if (h.name == name) return;
        Hit h;
        h.name = name;
        h.old_level = lv;
        h.old_score = sc > 0 ? sc : level_to_score(lv > 0 ? lv : 1);
        h.has_account = has_acc;
        if (h.old_score > max_score) max_score = h.old_score;
        hits.push_back(std::move(h));
    };

    AccountStore::for_each_account([&](std::string const &user, Account &acc) {
        consider(user, acc.score, acc.level, 1);
    });
    LeaderboardStore::for_each_entry_mutable([&](LeaderboardEntry &e) {
        consider(e.name, e.score, e.level, AccountStore::find(e.name) != nullptr ? 1 : 0);
    });

    if (hits.empty()) {
        std::cout << "[xp-mig] overflow scrub: clean" << std::endl;
        return 0;
    }

    uint32_t s_ref = max_score;
    if (s_ref < s_soft + 1000000u) s_ref = s_soft + 1000000u;

    std::ostringstream log;
    log << "# xp_overflow_scrub\n";
    uint32_t changed = 0;
    for (auto const &row : hits) {
        uint32_t new_sc = compress_score(row.old_score, s_soft, s_ref, s_lo, s_hi);
        uint32_t new_lv = score_to_level(new_sc);
        if (new_lv < kAnchor + 1) new_lv = kAnchor + 1;
        if (new_lv > kCap) new_lv = kCap;
        uint32_t lo = level_to_score(new_lv);
        uint32_t hi = level_to_score(new_lv + 1);
        if (hi > lo) hi -= 1;
        if (new_sc < lo) new_sc = lo;
        if (new_sc > hi) new_sc = hi;

        if (row.has_account) {
            Account *acc = AccountStore::find(row.name);
            if (acc != nullptr) {
                acc->score = new_sc;
                acc->level = new_lv;
                Talent::clamp_ranks(acc->talent_ranks, acc->level);
                AccountStore::mark_dirty();
            }
        }
        LeaderboardStore::force_set_level_score(row.name, new_lv, new_sc);
        log << row.name
            << " overflow lv " << row.old_level << "->" << new_lv
            << " score " << row.old_score << "->" << new_sc << "\n";
        ++changed;
    }

    AccountStore::save();
    LeaderboardStore::flush();
    write_text("xp_overflow_scrub.log", log.str());
    std::cout << "[xp-mig] overflow scrub rewritten=" << changed << std::endl;
    return changed;
}
