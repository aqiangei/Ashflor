#include <Shared/Craft.hh>

#include <Shared/Recipe.hh>
#include <Shared/StaticData.hh>

#include <Helpers/Math.hh>

#include <cmath>
#include <cstring>
#include <vector>

namespace Craft {
    static float const SUCCESS[RarityID::kNumRarities - 1] = {
        0.64f,
        0.36f,
        0.18f,
        0.09f,
        0.04f,
        0.015f
    };

    static float const FARM_SEC[RarityID::kNumRarities] = {
        8.f, 28.f, 140.f, 700.f, 2800.f, 9000.f, 28000.f
    };

    static uint32_t const COIN_BASE[RarityID::kNumRarities] = {
        1, 4, 12, 35, 90, 220, 550
    };
    static uint32_t const COIN_SPREAD[RarityID::kNumRarities] = {
        2, 4, 10, 25, 60, 140, 350
    };

    static uint32_t const BUY_RARITY[RarityID::kNumRarities] = {
        8, 25, 90, 320, 1200, 4800, 18000
    };

    float success_chance(uint8_t from_rarity) {
        if (from_rarity >= RarityID::kNumRarities - 1) return 0;
        return SUCCESS[from_rarity];
    }

    uint32_t exchange_cost(uint8_t from_rarity) {
        float p = success_chance(from_rarity);
        if (p <= 0.0001f) return 999999;
        return (uint32_t) std::ceil((double) kCraftBatch / (double) p);
    }

    float farm_seconds(uint8_t rarity) {
        if (rarity >= RarityID::kNumRarities) return 0;
        return FARM_SEC[rarity];
    }

    PetalID::T random_petal_of_rarity(uint8_t rarity) {
        static std::vector<PetalID::T> pools[RarityID::kNumRarities];
        static uint8_t inited = 0;
        if (!inited) {
            for (uint32_t id = 1; id < (uint32_t) PetalID::kNumPetals; ++id) {
                if (Recipe::is_bag_only((PetalID::T) id)) continue;
                uint8_t r = PETAL_DATA[id].rarity;
                if (r < RarityID::kNumRarities)
                    pools[r].push_back((PetalID::T) id);
            }
            inited = 1;
        }
        if (rarity >= RarityID::kNumRarities || pools[rarity].empty())
            return PetalID::kBasic;
        uint32_t idx = (uint32_t) (frand() * pools[rarity].size());
        if (idx >= pools[rarity].size()) idx = (uint32_t) pools[rarity].size() - 1;
        return pools[rarity][idx];
    }

    PetalID::T first_petal_of_rarity(uint8_t rarity) {
        static PetalID::T first[RarityID::kNumRarities];
        static uint8_t inited = 0;
        if (!inited) {
            for (uint8_t r = 0; r < RarityID::kNumRarities; ++r)
                first[r] = PetalID::kNone;
            for (uint32_t id = 1; id < (uint32_t) PetalID::kNumPetals; ++id) {
                uint8_t r = PETAL_DATA[id].rarity;
                if (r < RarityID::kNumRarities && first[r] == PetalID::kNone)
                    first[r] = (PetalID::T) id;
            }
            inited = 1;
        }
        if (rarity >= RarityID::kNumRarities) return PetalID::kNone;
        return first[rarity];
    }

    static PetalID::T find_same_name_at_rarity(PetalID::T from, uint8_t rarity) {
        if (from == PetalID::kNone || from >= PetalID::kNumPetals) return PetalID::kNone;
        char const *name = PETAL_DATA[from].name;
        for (uint32_t id = 1; id < (uint32_t) PetalID::kNumPetals; ++id) {
            if (PETAL_DATA[id].rarity != rarity) continue;
            if (std::strcmp(PETAL_DATA[id].name, name) == 0)
                return (PetalID::T) id;
        }
        return PetalID::kNone;
    }

    uint8_t can_upgrade_petal(PetalID::T id) {
        if (id == PetalID::kNone || id >= PetalID::kNumPetals) return 0;
        uint8_t r = PETAL_DATA[id].rarity;
        if (r >= RarityID::kUnique - 1) return 0;
        return find_same_name_at_rarity(id, (uint8_t) (r + 1)) != PetalID::kNone;
    }

    PetalID::T craft_result(PetalID::T from) {
        if (!can_upgrade_petal(from)) return PetalID::kNone;
        uint8_t next_r = (uint8_t) (PETAL_DATA[from].rarity + 1);
        // Same English name only — never random into another petal family.
        return find_same_name_at_rarity(from, next_r);
    }

    uint32_t coins_from_mob(uint8_t mob_rarity) {
        if (mob_rarity >= RarityID::kNumRarities) mob_rarity = 0;
        uint32_t base = COIN_BASE[mob_rarity];
        uint32_t spread = COIN_SPREAD[mob_rarity];
        return base + (uint32_t) (frand() * (spread + 1));
    }

    uint32_t buy_rarity_cost(uint8_t rarity) {
        if (rarity >= RarityID::kNumRarities) return 999999;
        return BUY_RARITY[rarity];
    }

    uint8_t is_craftable_special(PetalID::T id) {
        return id == PetalID::kAntennae
            || id == PetalID::kAccelerator
            || id == PetalID::kMagnet;
    }

    uint32_t buy_special_cost(PetalID::T id) {
        switch (id) {
            case PetalID::kAccelerator: return 120;
            case PetalID::kMagnet: return 180;
            case PetalID::kAntennae: return 900;
            default: return 999999;
        }
    }
}
