#pragma once

#include <Shared/StaticData.hh>

#include <cstdint>

// Talent trees: invest points for permanent combat buffs.
// +1 talent point per level gained (points_earned == current level).
// Rank 1 at level 1; each 10 levels unlocks another max rank.
namespace TalentID {
    enum : uint8_t {
        kHealth = 0,
        kRotSpeed = 1,
        kNum = 2
    };
}

namespace Talent {
    // ~2 soldier-ant hits of body HP per rank (soldier ≈40 dmg)
    inline float health_bonus(uint8_t rank) { return 20.f * (float) rank; }
    // Noticeable spin; stacks with Faster petal (~1 rad/s)
    inline float rot_bonus(uint8_t rank) { return 0.4f * (float) rank; }

    // Incremental: each level grants exactly one talent point.
    inline uint32_t points_earned(uint32_t level) {
        if (level < 1) level = 1;
        if (level > MAX_LEVEL) level = MAX_LEVEL;
        return level;
    }

    // Every 10 levels unlocks +1 max rank in each talent tree
    inline uint8_t max_rank(uint32_t level) {
        if (level < 1) level = 1;
        if (level > MAX_LEVEL) level = MAX_LEVEL;
        return (uint8_t) (1u + level / 10u);
    }

    // Odd costs: rank-r costs (2r-1). Maxing both trees at 99 needs 200 pts
    // vs 99 earned — still a real allocation choice.
    inline uint32_t rank_cost(uint8_t next_rank) {
        if (next_rank == 0) return 0;
        return 2u * (uint32_t) next_rank - 1u; // 1,3,5,...
    }

    inline uint32_t points_spent(uint8_t hp_rank, uint8_t rot_rank) {
        uint32_t s = 0;
        for (uint8_t r = 1; r <= hp_rank; ++r) s += rank_cost(r);
        for (uint8_t r = 1; r <= rot_rank; ++r) s += rank_cost(r);
        return s;
    }

    inline uint32_t points_free(uint32_t level, uint8_t hp_rank, uint8_t rot_rank) {
        uint32_t earned = points_earned(level);
        uint32_t spent = points_spent(hp_rank, rot_rank);
        return earned > spent ? earned - spent : 0;
    }

    // Q/E wheel: 0..10 → multiplier on (base + talent + petal) spin
    inline constexpr uint8_t kRotSettingMax = 10;
    inline constexpr uint8_t kRotSettingDefault = 7;
    inline float rot_setting_mult(uint8_t setting) {
        if (setting > kRotSettingMax) setting = kRotSettingMax;
        return 0.35f + 0.15f * (float) setting; // 0.35× … 1.85×
    }
}
