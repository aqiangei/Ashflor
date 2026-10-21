#include <Shared/Craft.hh>

#include <Shared/Recipe.hh>
#include <Shared/StaticData.hh>

#include <Helpers/Math.hh>

#include <cmath>
#include <cstring>
#include <vector>

namespace Craft {
    // Indexed by from-rarity. Ladder: …Mythic → Ultra → Super → Unique.
    static float const SUCCESS[RarityID::kNumRarities - 1] = {
        0.64f,   // Common → Unusual
        0.36f,   // Unusual → Rare
        0.18f,   // Rare → Epic
        0.09f,   // Epic → Legendary
        0.04f,   // Legendary → Mythic
        0.015f,  // Mythic → Ultra
        0.008f,  // Ultra → Super
        0.004f   // Super → Unique
    };

    // Expected seconds to meaningfully progress a rarity via kill→drop→craft.
    static float const FARM_SEC[RarityID::kNumRarities] = {
        8.f, 28.f, 140.f, 700.f, 2800.f, 9000.f, 28000.f, 70000.f, 160000.f
    };

    // Coin per kill (before zone_coin_mult). Tuned so ~3–6 min hard-zone grind
    // buys a mid special; Unique chase petals stay long sinks.
    static uint32_t const COIN_BASE[RarityID::kNumRarities] = {
        2, 6, 18, 48, 120, 280, 700, 1800, 4500
    };
    static uint32_t const COIN_SPREAD[RarityID::kNumRarities] = {
        3, 6, 14, 32, 80, 160, 400, 900, 2200
    };

    // Random-rarity packs: intentionally worse EV than focused farming (~1.6–2×).
    static uint32_t const BUY_RARITY[RarityID::kNumRarities] = {
        40, 140, 480, 1600, 5200, 16000, 55000, 180000, 500000
    };

    // Material shop by petal rarity (convenience > farm EV for that mat).
    static uint32_t const MAT_COST[RarityID::kNumRarities] = {
        28, 65, 160, 420, 1100, 2800, 999999, 999999, 999999
    };

    static PetalID::T const kSpecials[] = {
        PetalID::kAccelerator,
        PetalID::kMagnet,
        PetalID::kFaster,
        PetalID::kBubble,
        PetalID::kWeb,
        PetalID::kYinYang,
        PetalID::kCutter,
        PetalID::kBone,
        PetalID::kSalt,
        PetalID::kObserver,
        PetalID::kMonstera,
        PetalID::kAntennae,
        PetalID::kHeaviest,
        PetalID::kMoon,
        PetalID::kGoldenLeaf,
        PetalID::kLotus,
        PetalID::kThirdEye,
        PetalID::kYggdrasil,
        PetalID::kHoney,
        PetalID::kMeteor,
    };

    static uint8_t is_honey_or_meteor_family(PetalID::T id) {
        if (id == PetalID::kNone || id >= PetalID::kNumPetals) return 0;
        char const *n = PETAL_DATA[id].name;
        if (n == nullptr) return 0;
        return std::strcmp(n, "Honey") == 0 || std::strcmp(n, "Meteor") == 0;
    }

    static uint32_t const HONEY_METEOR_COST[RarityID::kNumRarities] = {
        90, 220, 560, 1400, 3600, 9000, 24000, 70000, 180000
    };

    static PetalID::T const kCombos[] = {
        PetalID::kLeafBlade, PetalID::kBeeSting, PetalID::kHealingBloom,
        PetalID::kSandSpike, PetalID::kCraftCactus, PetalID::kQuicksandOrb,
        PetalID::kWaterBubble, PetalID::kStarfish, PetalID::kTidePearl,
        PetalID::kIceSpike, PetalID::kSnowball, PetalID::kIceShield,
        PetalID::kThornVine, PetalID::kPoisonSpore, PetalID::kRotwood,
        PetalID::kAntFang, PetalID::kAcidFlask, PetalID::kSoldierEgg,
        PetalID::kFireball, PetalID::kMagmaCore, PetalID::kObsidianBlade,
        PetalID::kSteelSword, PetalID::kHolyOrb, PetalID::kMechGear,
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
                // Coin only — mats/combos are normal petals.
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

    uint32_t craft_fail_loss() {
        uint32_t span = kCraftFailLossMax - kCraftFailLossMin + 1;
        uint32_t roll = (uint32_t) (frand() * (double) span);
        if (roll >= span) roll = span - 1;
        return kCraftFailLossMin + roll;
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
        if (name == nullptr) return PetalID::kNone;
        PetalID::T best = PetalID::kNone;
        for (uint32_t id = 1; id < (uint32_t) PetalID::kNumPetals; ++id) {
            if (PETAL_DATA[id].rarity != rarity) continue;
            if (PETAL_DATA[id].name == nullptr) continue;
            if (std::strcmp(PETAL_DATA[id].name, name) != 0) continue;
            if (best == PetalID::kNone || id > best) best = (PetalID::T) id;
        }
        return best;
    }

    uint8_t can_upgrade_petal(PetalID::T id) {
        if (id == PetalID::kNone || id >= PetalID::kNumPetals) return 0;
        if (Recipe::is_bag_only(id)) return 0; // coin only
        uint8_t r = PETAL_DATA[id].rarity;
        return r < RarityID::kNumRarities - 1 ? 1 : 0;
    }

    PetalID::T craft_result(PetalID::T from) {
        if (!can_upgrade_petal(from)) return PetalID::kNone;
        uint8_t next_r = (uint8_t) (PETAL_DATA[from].rarity + 1);
        PetalID::T same = find_same_name_at_rarity(from, next_r);
        if (same != PetalID::kNone) return same;
        return random_petal_of_rarity(next_r);
    }

    PetalID::T petal_at_rarity(PetalID::T from, uint8_t rarity) {
        return find_same_name_at_rarity(from, rarity);
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

    uint8_t is_shop_material(PetalID::T id) {
        return id >= PetalID::kMatLeaf && id <= PetalID::kMatRoyalSeal;
    }

    uint8_t is_shop_combo(PetalID::T id) {
        for (PetalID::T c : kCombos)
            if (c == id) return 1;
        return 0;
    }

    uint8_t is_craftable_special(PetalID::T id) {
        if (is_honey_or_meteor_family(id)) return 1;
        for (PetalID::T s : kSpecials)
            if (s == id) return 1;
        return 0;
    }

    uint8_t is_shop_purchasable(PetalID::T id) {
        return is_craftable_special(id) || is_shop_material(id) || is_shop_combo(id);
    }

    uint32_t buy_material_cost(PetalID::T id) {
        if (!is_shop_material(id) || id >= PetalID::kNumPetals) return 999999;
        uint8_t r = PETAL_DATA[id].rarity;
        if (r >= RarityID::kNumRarities) return 999999;
        return MAT_COST[r];
    }

    uint32_t buy_combo_cost(PetalID::T id) {
        if (!is_shop_combo(id)) return 999999;
        uint16_t idx = Recipe::find_by_result(id);
        if (idx == (uint16_t) -1) return 900;
        Recipe::Def const &rec = Recipe::recipe_at(idx);
        uint32_t sum = 0;
        for (uint8_t i = 0; i < rec.ingredient_count; ++i) {
            PetalID::T mid = rec.ingredients[i].id;
            uint32_t unit = is_shop_material(mid) ? buy_material_cost(mid)
                : (is_shop_combo(mid) ? buy_combo_cost(mid) : 80);
            if (unit >= 999999) unit = 200;
            sum += unit * (uint32_t) rec.ingredients[i].count;
        }
        // Convenience premium vs farming mats + hammer (~35%).
        return (uint32_t) std::lround((double) sum * 1.35) + 40;
    }

    uint32_t buy_special_cost(PetalID::T id) {
        switch (id) {
            // Entry utility — affordable after a short soft-zone run
            case PetalID::kAccelerator: return 150;
            case PetalID::kMagnet: return 220;
            case PetalID::kFaster: return 280;
            case PetalID::kBubble: return 320;
            case PetalID::kWeb: return 380;
            case PetalID::kYinYang: return 520;
            case PetalID::kCutter: return 600;
            case PetalID::kBone: return 720;
            case PetalID::kSalt: return 800;
            case PetalID::kObserver: return 1100;
            case PetalID::kMonstera: return 1400;
            case PetalID::kAntennae: return 1800;
            case PetalID::kHeaviest: return 2400;
            case PetalID::kMoon: return 3200;
            case PetalID::kGoldenLeaf: return 4000;
            case PetalID::kLotus: return 4800;
            case PetalID::kThirdEye: return 5600;
            case PetalID::kYggdrasil: return 7200;
            case PetalID::kHoney: return HONEY_METEOR_COST[RarityID::kRare];
            case PetalID::kMeteor: return HONEY_METEOR_COST[RarityID::kLegendary];
            default: break;
        }
        if (is_honey_or_meteor_family(id) && id < PetalID::kNumPetals) {
            uint8_t r = PETAL_DATA[id].rarity;
            if (r < RarityID::kNumRarities) return HONEY_METEOR_COST[r];
        }
        if (is_shop_material(id)) return buy_material_cost(id);
        if (is_shop_combo(id)) return buy_combo_cost(id);
        return 999999;
    }

    uint32_t shop_price(PetalID::T id) {
        return buy_special_cost(id);
    }

    float drop_weight_mult(PetalID::T id) {
        if (id == PetalID::kNone || id >= PetalID::kNumPetals) return 1.f;
        // Mats are normal petals (assemble ingredients only); no special drop tilt.
        if (is_craftable_special(id)) {
            // Chase petals: rarer → rarer in the wild (shop is the clean sink)
            uint8_t r = PETAL_DATA[id].rarity;
            if (r >= RarityID::kLegendary) return 0.22f;
            if (r >= RarityID::kEpic) return 0.35f;
            if (r >= RarityID::kRare) return 0.50f;
            return 0.70f;
        }
        // Filler combat petals that clog bags after long grinds
        if (id == PetalID::kBasic || id == PetalID::kLight || id == PetalID::kHeavy)
            return 0.55f;
        return 1.f;
    }

    static std::vector<ShopOffer> g_offers;
    static uint8_t g_offers_ready = 0;

    static void build_offers() {
        if (g_offers_ready) return;
        g_offers.clear();
        for (PetalID::T id : kSpecials)
            g_offers.push_back({ id, kShopSpecials, 0, 0, 0 });
        // Paid full-server top banner (aqa-style); prompt for text on buy.
        g_offers.push_back({ PetalID::kNone, kShopSpecials, 0, 0, 1 });
        for (uint32_t id = 1; id < (uint32_t) PetalID::kNumPetals; ++id) {
            if (!is_honey_or_meteor_family((PetalID::T) id)) continue;
            if (id == (uint32_t) PetalID::kHoney || id == (uint32_t) PetalID::kMeteor)
                continue;
            g_offers.push_back({ (PetalID::T) id, kShopSpecials, 0, 0, 0 });
        }
        for (uint32_t id = (uint32_t) PetalID::kMatLeaf;
             id <= (uint32_t) PetalID::kMatRoyalSeal; ++id) {
            uint8_t r = PETAL_DATA[id].rarity;
            uint8_t tab = (r <= RarityID::kUnusual) ? kShopMaterials : kShopRareMats;
            g_offers.push_back({ (PetalID::T) id, tab, 0, 0, 0 });
        }
        for (PetalID::T id : kCombos)
            g_offers.push_back({ id, kShopCombos, 0, 0, 0 });
        // Random packs: Common → Legendary (Mythic+ too swingy / Unique gated)
        for (uint8_t r = RarityID::kCommon; r <= RarityID::kLegendary; ++r)
            g_offers.push_back({ PetalID::kNone, kShopRandom, 1, r, 0 });
        g_offers_ready = 1;
    }

    uint16_t shop_offer_count() {
        build_offers();
        return (uint16_t) g_offers.size();
    }

    ShopOffer const &shop_offer_at(uint16_t index) {
        build_offers();
        static ShopOffer const kNone = { PetalID::kNone, 0, 0, 0, 0 };
        if (index >= g_offers.size()) return kNone;
        return g_offers[index];
    }

    uint8_t is_shop_announce(ShopOffer const &offer) {
        return offer.announce ? 1 : 0;
    }

    uint32_t shop_offer_cost(ShopOffer const &offer) {
        if (offer.announce) return kShopAnnounceCost;
        if (offer.random_rarity)
            return buy_rarity_cost(offer.rarity);
        return shop_price(offer.id);
    }
}
