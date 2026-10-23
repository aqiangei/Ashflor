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

    // Coin cost for one shop screen-announce purchase (~Rare mid-chase)
    constexpr uint32_t kShopAnnounceCost = 5000;

    float success_chance(uint8_t from_rarity);
    // Guaranteed-exchange price: ceil(5 / chance) copies (ignores miss refunds,
    // so it costs a bit more than gambling on average — convenience tax).
    uint32_t expected_cost(float chance);
    uint32_t exchange_cost(uint8_t from_rarity);

    // Infinite petal levels: Unique petals keep merging 5 → Lv.L+1. Level is
    // per account per PetalID; only the first primary loadout copy of that ID
    // gets the stat bonus (other copies stay at base stats).
    constexpr uint32_t kMaxPetalLevel = 65535;
    constexpr float kLevelStatPerLevel = 0.03f; // +3% damage & health per level
    uint8_t can_level_petal(PetalID::T id);
    // Tray accepts it (rarity upgrade or level-up).
    uint8_t can_craft_petal(PetalID::T id);
    float level_success_chance(uint32_t level);
    uint32_t level_exchange_cost(uint32_t level);
    float level_stat_mult(uint32_t level);
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

    // Live loot weights for a mob kill (same path Death rolls). Applies
    // compute_mob_drop_weights + drop_weight_mult + out-of-band 0.15 floor.
    // Writes drops.size() entries; returns sum (0 if empty / invalid).
    float mob_loot_weights(MobID::T mob_id, uint8_t mob_rarity,
                           float out[MAX_DROPS_PER_MOB]);
}
