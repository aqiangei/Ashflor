#pragma once

#include <Shared/StaticDefinitions.hh>

#include <cstdint>

namespace Craft {
    // 5 petals of rarity R → attempt 1 petal of rarity R+1
    constexpr uint32_t kCraftBatch = 5;

    // Success chance tuned vs farm time (RARITY_MULT ratios + craft tax)
    // EV cost = 5 / P ; exchange_cost = ceil(EV)
    float success_chance(uint8_t from_rarity);
    uint32_t exchange_cost(uint8_t from_rarity);

    // Approx seconds to farm one petal of this rarity (for UI hints)
    float farm_seconds(uint8_t rarity);

    uint8_t can_upgrade(uint8_t from_rarity);
    PetalID::T random_petal_of_rarity(uint8_t rarity);

    // --- Coins ---
    // Mob kill coin payout by mob rarity (base + random spread)
    uint32_t coins_from_mob(uint8_t mob_rarity);
    // Coin cost to buy 1 random petal of this rarity (into backpack)
    uint32_t buy_rarity_cost(uint8_t rarity);
    // Coin cost to buy a fixed special petal
    uint32_t buy_special_cost(PetalID::T id);
    uint8_t is_craftable_special(PetalID::T id);
}
