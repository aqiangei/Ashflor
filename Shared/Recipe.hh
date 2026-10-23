#pragma once

#include <Shared/StaticDefinitions.hh>

#include <cstdint>

namespace Recipe {
    constexpr uint8_t kMaxIngredients = 4;

    struct Ingredient {
        PetalID::T id;
        uint16_t count;
    };

    // tier: 0 = map base · 1 = cross-map · 2 = legendary · 3 = special (coin shop)
    struct Def {
        PetalID::T result;
        uint8_t tier;
        uint8_t ingredient_count;
        Ingredient ingredients[kMaxIngredients];
    };

    uint16_t recipe_count();
    Def const &recipe_at(uint16_t index);
    // Returns recipe index or (uint16_t)-1
    uint16_t find_by_result(PetalID::T result);

    // True for ids that must never sit in a primary loadout slot.
    // Coins are equippable (combat petal); only invalid ids are barred.
    uint8_t is_bag_only(PetalID::T id);

    // Soft stack ceiling for craft_bank / bag deposits. Coins allow 1e9; others 1e6.
    // At/over cap, unequip still clears the loadout slot (do not block the bar).
    uint32_t bank_stack_cap(PetalID::T id);
}
