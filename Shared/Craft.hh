#pragma once

#include <Shared/StaticDefinitions.hh>

#include <cstdint>

namespace Craft {
    // 5 of the same petal → attempt 1 higher-rarity result.
    // On miss: lose craft_fail_loss() petals (1–4), refund the rest.
    constexpr uint32_t kCraftBatch = 5;
    constexpr uint32_t kCraftFailLossMin = 1;
    constexpr uint32_t kCraftFailLossMax = 4;

    // Shop panel tabs (client + shared catalog)
    enum ShopTab : uint8_t {
        kShopSpecials = 0, // unique mechanics / chase petals
        kShopMaterials = 1, // biome mats (Common–Unusual)
        kShopRareMats = 2, // Rare+ mats for assemble
        kShopCombos = 3, // base-tier assemble results
        kShopRandom = 4, // random petal of a rarity
        kShopTabCount = 5
    };

    struct ShopOffer {
        PetalID::T id;       // petal to buy (ignored if random / announce)
        uint8_t tab;
        uint8_t random_rarity; // 1 = buy_rarity offer; id unused
        uint8_t rarity;        // for random_rarity offers
        uint8_t announce;      // 1 = paid full-server screen banner
    };

    // Coin cost for one shop screen-announce purchase
    constexpr uint32_t kShopAnnounceCost = 2500;

    float success_chance(uint8_t from_rarity);
    uint32_t exchange_cost(uint8_t from_rarity);
    float farm_seconds(uint8_t rarity);
    // Uniform roll in [kCraftFailLossMin, kCraftFailLossMax] for a missed gamble.
    uint32_t craft_fail_loss();

    // Any petal below Unique (except coin) can attempt an upgrade.
    uint8_t can_upgrade_petal(PetalID::T id);
    // Same English name at rarity+1 when it exists; else random of rarity+1
    PetalID::T craft_result(PetalID::T from);
    // Same English name at exact rarity; kNone if that rung is missing
    PetalID::T petal_at_rarity(PetalID::T from, uint8_t rarity);
    PetalID::T random_petal_of_rarity(uint8_t rarity);
    // First petal id that uses this rarity (legacy rarity-bank migration)
    PetalID::T first_petal_of_rarity(uint8_t rarity);

    uint32_t coins_from_mob(uint8_t mob_rarity);
    uint32_t buy_rarity_cost(uint8_t rarity);
    uint32_t buy_special_cost(PetalID::T id);
    uint8_t is_craftable_special(PetalID::T id);

    // Material / combo shop (also accepted by kCraftBuySpecial)
    uint8_t is_shop_material(PetalID::T id);
    uint8_t is_shop_combo(PetalID::T id);
    uint8_t is_shop_purchasable(PetalID::T id); // special | material | combo
    uint32_t buy_material_cost(PetalID::T id);
    uint32_t buy_combo_cost(PetalID::T id);
    uint32_t shop_price(PetalID::T id); // unified price for any purchasable petal

    uint16_t shop_offer_count();
    ShopOffer const &shop_offer_at(uint16_t index);
    uint32_t shop_offer_cost(ShopOffer const &offer);
    uint8_t is_shop_announce(ShopOffer const &offer);

    // Drop-table tilt: shop chase petals down (mats are normal petals)
    float drop_weight_mult(PetalID::T id);
}
