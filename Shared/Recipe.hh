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

    // Currency (coins): stack in bag, never equip to primary.
    uint8_t is_bag_only(PetalID::T id);
}
