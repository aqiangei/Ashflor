#pragma once

#include <Shared/StaticDefinitions.hh>

#include <cstdint>

namespace Craft {
    // 5 of the same petal → attempt 1 higher-rarity result
    constexpr uint32_t kCraftBatch = 5;

    float success_chance(uint8_t from_rarity);
    uint32_t exchange_cost(uint8_t from_rarity);
    float farm_seconds(uint8_t rarity);

    uint8_t can_upgrade_petal(PetalID::T id);
    // Same English name at rarity+1 when it exists; else random of rarity+1
    PetalID::T craft_result(PetalID::T from);
    PetalID::T random_petal_of_rarity(uint8_t rarity);
    // First petal id that uses this rarity (legacy rarity-bank migration)
    PetalID::T first_petal_of_rarity(uint8_t rarity);

    uint32_t coins_from_mob(uint8_t mob_rarity);
    uint32_t buy_rarity_cost(uint8_t rarity);
    uint32_t buy_special_cost(PetalID::T id);
    uint8_t is_craftable_special(PetalID::T id);
}
