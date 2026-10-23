#pragma once

#include <Shared/StaticData.hh>
#include <Shared/StaticDefinitions.hh>

#include <array>
#include <cstdint>

// Talent trees: invest points for permanent combat buffs.
// +1 talent point per level. Ranks tint like petal rarities (1=Common … 9=Ultra).
// Slots: base 5 + kSlots rank, hard-capped at MAX_SLOT_COUNT (10).
namespace TalentID {
    enum : uint8_t {
        kHealth = 0,
        kRotSpeed = 1,
        kAttackRange = 2,
        kCooldown = 3,
        kMagnet = 4,
        kPetalArmor = 5,
        kSlots = 6,
        kNum = 7
    };
}

namespace Talent {
    inline constexpr uint8_t kBaseSlots = 5;
    inline constexpr uint8_t kMaxGradeRank = 9; // Ultra
    inline constexpr uint8_t kMaxSlotRanks = 5; // 5 + 5 = 10

    // Was 20/rank (~+180 at max) — too weak vs level HP curve; 50 → +450 at r9.
    inline float health_bonus(uint8_t rank) { return 50.f * (float) rank; }
    inline float rot_bonus(uint8_t rank) { return 0.4f * (float) rank; }
    inline float range_bonus(uint8_t rank) { return 8.f * (float) rank; }
    // Stacking multiplier < 1 → shorter reload
    inline float reload_mult(uint8_t rank) {
        float m = 1.f;
        for (uint8_t i = 0; i < rank; ++i) m *= 0.97f;
        return m;
    }
    inline float magnet_bonus(uint8_t rank) { return 200.f * (float) rank; }
    inline float petal_health_mult(uint8_t rank) {
        return 1.f + 0.08f * (float) rank;
    }
    inline uint8_t slot_count(uint8_t slot_rank) {
        uint32_t s = (uint32_t) kBaseSlots + slot_rank;
        if (s > MAX_SLOT_COUNT) s = MAX_SLOT_COUNT;
        return (uint8_t) s;
    }
    inline uint8_t slot_count_from_ranks(uint8_t const *ranks) {
        return slot_count(ranks[TalentID::kSlots]);
    }

    // Rank 1 → Common (0), … Rank 9 → Ultra (8)
    inline uint8_t rank_grade(uint8_t rank) {
        if (rank == 0) return 0;
        uint8_t g = (uint8_t) (rank - 1);
        return g >= RarityID::kNumRarities ? (uint8_t) (RarityID::kNumRarities - 1) : g;
    }

    inline uint32_t points_earned(uint32_t level) {
        if (level < 1) level = 1;
        return level;
    }

    inline uint8_t max_rank_for(uint8_t talent_id, uint32_t level) {
        if (talent_id == TalentID::kSlots) return kMaxSlotRanks;
        if (level < 1) level = 1;
        uint32_t r = 1u + level / 10u;
        if (r > kMaxGradeRank) r = kMaxGradeRank;
        return (uint8_t) r;
    }

    // Legacy alias used by rot/health caps elsewhere
    inline uint8_t max_rank(uint32_t level) {
        return max_rank_for(TalentID::kHealth, level);
    }

    inline uint32_t rank_cost(uint8_t next_rank) {
        if (next_rank == 0) return 0;
        return 2u * (uint32_t) next_rank - 1u;
    }

    // Cost to move one talent from `from_rank` up to `to_rank` (to > from).
    inline uint32_t cost_to_reach(uint8_t from_rank, uint8_t to_rank) {
        if (to_rank <= from_rank) return 0;
        uint32_t s = 0;
        for (uint8_t r = (uint8_t) (from_rank + 1); r <= to_rank; ++r)
            s += rank_cost(r);
        return s;
    }

    inline uint32_t points_spent(uint8_t const *ranks) {
        uint32_t s = 0;
        for (uint8_t id = 0; id < TalentID::kNum; ++id)
            for (uint8_t r = 1; r <= ranks[id]; ++r)
                s += rank_cost(r);
        return s;
    }

    inline uint32_t points_free(uint32_t level, uint8_t const *ranks) {
        uint32_t earned = points_earned(level);
        uint32_t spent = points_spent(ranks);
        return earned > spent ? earned - spent : 0;
    }

    // Whether setting ranks[id] = target is legal for this level / free points.
    inline uint8_t can_set_rank(uint8_t talent_id, uint8_t target, uint32_t level,
                                uint8_t const *ranks) {
        if (talent_id >= TalentID::kNum) return 0;
        if (target > max_rank_for(talent_id, level)) return 0;
        uint8_t cur = ranks[talent_id];
        if (target == cur) return 1;
        if (target < cur) return 1; // refund always free
        return points_free(level, ranks) >= cost_to_reach(cur, target) ? 1 : 0;
    }

    // Compat helpers for call sites still passing hp/rot only (migrate callers)
    inline uint32_t points_spent(uint8_t hp, uint8_t rot) {
        uint8_t ranks[TalentID::kNum] = {};
        ranks[TalentID::kHealth] = hp;
        ranks[TalentID::kRotSpeed] = rot;
        return points_spent(ranks);
    }
    inline uint32_t points_free(uint32_t level, uint8_t hp, uint8_t rot) {
        uint8_t ranks[TalentID::kNum] = {};
        ranks[TalentID::kHealth] = hp;
        ranks[TalentID::kRotSpeed] = rot;
        return points_free(level, ranks);
    }

    inline constexpr uint8_t kRotSettingMax = 10;
    inline constexpr uint8_t kRotSettingDefault = 4; // was 7 — slower default spin
    inline float rot_setting_mult(uint8_t setting) {
        if (setting > kRotSettingMax) setting = kRotSettingMax;
        return 0.35f + 0.15f * (float) setting;
    }

    inline void clamp_ranks(uint8_t *ranks, uint32_t level) {
        for (uint8_t id = 0; id < TalentID::kNum; ++id) {
            uint8_t m = max_rank_for(id, level);
            if (ranks[id] > m) ranks[id] = m;
        }
        while (points_spent(ranks) > points_earned(level)) {
            uint8_t best = TalentID::kNum;
            for (uint8_t id = 0; id < TalentID::kNum; ++id)
                if (ranks[id] > 0 && (best == TalentID::kNum || ranks[id] >= ranks[best]))
                    best = id;
            if (best >= TalentID::kNum) break;
            --ranks[best];
        }
    }

    inline uint8_t slot_count_from_ranks(std::array<uint8_t, TalentID::kNum> const &r) {
        return slot_count(r[TalentID::kSlots]);
    }
    inline uint32_t points_spent(std::array<uint8_t, TalentID::kNum> const &r) {
        return points_spent(r.data());
    }
    inline uint32_t points_free(uint32_t level, std::array<uint8_t, TalentID::kNum> const &r) {
        return points_free(level, r.data());
    }
    inline void clamp_ranks(std::array<uint8_t, TalentID::kNum> &r, uint32_t level) {
        clamp_ranks(r.data(), level);
    }
}
