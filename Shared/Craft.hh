#pragma once

#include <Shared/StaticDefinitions.hh>

#include <array>
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
    // True gambling EV petals/success: accounts for miss refunding (lose 1–4 only).
    // For p=0.8% Super→Unique ≈315, not the exchange sticker of 625.
    uint32_t expected_gamble_cost(float chance);
    uint32_t exchange_cost(uint8_t from_rarity);

    // Infinite petal levels: Unique petals merge 5 → +1 Lv and return 1 Unique
    // (net −4 on success), same shape as rarity craft. Level is per account per
    // family; only the first primary loadout copy of that ID gets the stat bonus.
    constexpr uint32_t kMaxPetalLevel = 65535;
    constexpr float kLevelStatPerLevel = 0.12f; // +12% combat / additive attrs per level
    // Global petal CD after Golden Leaf × talent: never faster than this fraction
    // of base reload (99% CDR cap). Flower still clamps reload to ≥2 ticks for art.
    constexpr float kReloadFactorFloor = 0.01f;
    constexpr float kReloadFactorCeil = 4.f;
    // Unique Observer design FOV. level_scale_factor's 1% CDR floor must NOT apply to
    // vision — Unique Lv≥2 was hitting 0.01 → FOV×100 / tooltip "10000%".
    constexpr float kVisionFactorFloor = 0.18f;
    // Multiplicative attrs (speed, CDR, vision…): scale the deviation from 1.
    // CDR softens toward kReloadFactorFloor (99% cap); no per-leaf 40% ceiling.
    inline float level_scale_factor(float factor, float lv_mult) {
        if (lv_mult <= 1.0001f) return factor;
        if (factor >= 1.f)
            return 1.f + (factor - 1.f) * lv_mult;
        float depth = (1.f - factor) * lv_mult;
        if (depth > 1.f - kReloadFactorFloor) depth = 1.f - kReloadFactorFloor;
        float out = 1.f - depth;
        return out < kReloadFactorFloor ? kReloadFactorFloor : out;
    }
    // Vision FOV multiplier (lower = see more). Cap at Unique Observer design value.
    inline float level_scale_vision(float factor, float lv_mult) {
        float out = level_scale_factor(factor, lv_mult);
        if (factor > 0.f && factor < kVisionFactorFloor) return factor;
        return out < kVisionFactorFloor ? kVisionFactorFloor : out;
    }
    uint8_t can_level_petal(PetalID::T id);
    // Tray accepts it (rarity upgrade or level-up).
    uint8_t can_craft_petal(PetalID::T id);
    float level_success_chance(uint32_t level);
    uint32_t level_exchange_cost(uint32_t level);
    uint32_t level_expected_gamble_cost(uint32_t level);
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
    // Stable Unique rung for a family (highest id with that English name).
    // Used so Unique Lv survives PetalID reshuffles (Grapes was losing Lv alone).
    // Names match with spaces/_ equivalent (accounts.dat l: keys use _).
    PetalID::T unique_id_for_name(char const *name);
    PetalID::T canonical_unique_id(PetalID::T from);
    // Fold every non-zero entry onto the family's current Unique PetalID.
    // Call after load / network apply so stale ids from enum reshuffles still count.
    void petal_levels_canonicalize(std::array<uint32_t, PetalID::kNumPetals> &levels);
    // Read Unique Lv for any family member (resolves via canonical_unique_id).
    uint32_t petal_level_get(std::array<uint32_t, PetalID::kNumPetals> const &levels,
                             PetalID::T id);
    // True only for Unique petals with no Super same-name (world singleton).
    // Ladder Uniques (Peas/Grapes/…) may drop even if one already exists.
    uint8_t unique_is_world_singleton(PetalID::T id);
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
