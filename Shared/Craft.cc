#include <Shared/Craft.hh>

#include <Shared/StaticData.hh>

#include <Helpers/Math.hh>

#include <cmath>
#include <vector>

namespace Craft {
    // C→U, U→R, R→E, E→L, L→M, M→Unique
    static float const SUCCESS[RarityID::kNumRarities - 1] = {
        0.64f,  // EV≈7.8   commons → unusual (commons drop fast)
        0.36f,  // EV≈13.9
        0.18f,  // EV≈27.8  rare→epic (matches ~25× farm gap)
        0.09f,  // EV≈55.6
        0.04f,  // EV≈125
        0.015f  // EV≈333   mythic→unique
    };

    static float const FARM_SEC[RarityID::kNumRarities] = {
        8.f, 28.f, 140.f, 700.f, 2800.f, 9000.f, 28000.f
    };

    // Kill payout: common ants give pocket change; mythic/unique bosses pay big
    static uint32_t const COIN_BASE[RarityID::kNumRarities] = {
        1, 4, 12, 35, 90, 220, 550
    };
    static uint32_t const COIN_SPREAD[RarityID::kNumRarities] = {
        2, 4, 10, 25, 60, 140, 350
    };

    // Shop: random petal of rarity → backpack
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

    uint8_t can_upgrade(uint8_t from_rarity) {
        return from_rarity < RarityID::kUnique;
    }

    PetalID::T random_petal_of_rarity(uint8_t rarity) {
        static std::vector<PetalID::T> pools[RarityID::kNumRarities];
        static uint8_t inited = 0;
        if (!inited) {
            for (PetalID::T id = 1; id < PetalID::kNumPetals; ++id) {
                // Skip equipment-only specials from random rarity pools
                // (bought separately); still allow if they match rarity for exchange
                uint8_t r = PETAL_DATA[id].rarity;
                if (r < RarityID::kNumRarities)
                    pools[r].push_back(id);
            }
            inited = 1;
        }
        if (rarity >= RarityID::kNumRarities || pools[rarity].empty())
            return PetalID::kBasic;
        uint32_t idx = (uint32_t) (frand() * pools[rarity].size());
        if (idx >= pools[rarity].size()) idx = (uint32_t) pools[rarity].size() - 1;
        return pools[rarity][idx];
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
