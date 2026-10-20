#include <Shared/Recipe.hh>

#include <Shared/StaticData.hh>

namespace Recipe {
    static Def const kRecipes[] = {
        // ---- Garden base ----
        { PetalID::kLeafBlade, 0, 2, {
            { PetalID::kMatLeaf, 5 }, { PetalID::kMatTwig, 2 }, {}, {}
        }},
        { PetalID::kBeeSting, 0, 2, {
            { PetalID::kMatBeeBarb, 3 }, { PetalID::kMatPollenDust, 5 }, {}, {}
        }},
        { PetalID::kHealingBloom, 0, 3, {
            { PetalID::kMatPollenDust, 8 }, { PetalID::kMatRoyalJelly, 1 },
            { PetalID::kMatLeaf, 3 }, {}
        }},
        // ---- Desert ----
        { PetalID::kSandSpike, 0, 2, {
            { PetalID::kMatSand, 6 }, { PetalID::kMatVenomBarb, 2 }, {}, {}
        }},
        { PetalID::kCraftCactus, 0, 2, {
            { PetalID::kMatCactusSap, 4 }, { PetalID::kMatCarapace, 3 }, {}, {}
        }},
        { PetalID::kQuicksandOrb, 0, 2, {
            { PetalID::kMatSand, 10 }, { PetalID::kMatSandstormEye, 1 }, {}, {}
        }},
        // ---- Ocean ----
        { PetalID::kWaterBubble, 0, 2, {
            { PetalID::kMatWaterDrop, 6 }, { PetalID::kMatSeaSalt, 2 }, {}, {}
        }},
        { PetalID::kStarfish, 0, 2, {
            { PetalID::kMatSeashell, 4 }, { PetalID::kMatCoral, 3 }, {}, {}
        }},
        { PetalID::kTidePearl, 0, 2, {
            { PetalID::kMatWaterDrop, 8 }, { PetalID::kMatAbyssPearl, 1 }, {}, {}
        }},
        // ---- Snow ----
        { PetalID::kIceSpike, 0, 2, {
            { PetalID::kMatIceCrystal, 5 }, { PetalID::kMatColdIron, 2 }, {}, {}
        }},
        { PetalID::kSnowball, 0, 2, {
            { PetalID::kMatSnowFluff, 6 }, { PetalID::kMatIceCrystal, 3 }, {}, {}
        }},
        { PetalID::kIceShield, 0, 2, {
            { PetalID::kMatColdIron, 5 }, { PetalID::kMatIceScale, 4 }, {}, {}
        }},
        // ---- Jungle ----
        { PetalID::kThornVine, 0, 2, {
            { PetalID::kMatVine, 6 }, { PetalID::kMatResin, 2 }, {}, {}
        }},
        { PetalID::kPoisonSpore, 0, 2, {
            { PetalID::kMatVenomSac, 4 }, { PetalID::kMatSpore, 6 }, {}, {}
        }},
        { PetalID::kRotwood, 0, 2, {
            { PetalID::kMatResin, 5 }, { PetalID::kMatVine, 4 }, {}, {}
        }},
        // ---- Ant Hell ----
        { PetalID::kAntFang, 0, 2, {
            { PetalID::kMatAntShell, 4 }, { PetalID::kMatAntenna, 3 }, {}, {}
        }},
        { PetalID::kAcidFlask, 0, 2, {
            { PetalID::kMatFormicAcid, 6 }, { PetalID::kMatAntShell, 3 }, {}, {}
        }},
        { PetalID::kSoldierEgg, 0, 2, {
            { PetalID::kMatAntRoe, 8 }, { PetalID::kMatQueenPheromone, 1 }, {}, {}
        }},
        // ---- Volcano ----
        { PetalID::kFireball, 0, 2, {
            { PetalID::kMatFireCrystal, 5 }, { PetalID::kMatSulfur, 3 }, {}, {}
        }},
        { PetalID::kMagmaCore, 0, 2, {
            { PetalID::kMatLavaRock, 6 }, { PetalID::kMatObsidian, 3 }, {}, {}
        }},
        { PetalID::kObsidianBlade, 0, 2, {
            { PetalID::kMatObsidian, 7 }, { PetalID::kMatFireCrystal, 3 }, {}, {}
        }},
        // ---- Royal Capital ----
        { PetalID::kSteelSword, 0, 2, {
            { PetalID::kMatSteel, 6 }, { PetalID::kMatGear, 3 }, {}, {}
        }},
        { PetalID::kHolyOrb, 0, 2, {
            { PetalID::kMatHolyShard, 6 }, { PetalID::kMatCapitalBadge, 2 }, {}, {}
        }},
        { PetalID::kMechGear, 0, 2, {
            { PetalID::kMatGear, 8 }, { PetalID::kMatSteel, 3 }, {}, {}
        }},
        // ---- Special shop (coin petals) ----
        { PetalID::kAccelerator, 3, 1, {
            { PetalID::kCoin, 120 }, {}, {}, {}
        }},
        { PetalID::kMagnet, 3, 1, {
            { PetalID::kCoin, 180 }, {}, {}, {}
        }},
        { PetalID::kAntennae, 3, 1, {
            { PetalID::kCoin, 900 }, {}, {}, {}
        }},
        // ---- Cross-map (tier 1) + Legendary (tier 2) ----
        { PetalID::kThornPalm, 1, 3, {
            { PetalID::kLeafBlade, 2 }, { PetalID::kCraftCactus, 1 }, { PetalID::kMatVenomBarb, 5 }, {}
        }},
        { PetalID::kPollenSandstorm, 1, 3, {
            { PetalID::kHealingBloom, 1 }, { PetalID::kQuicksandOrb, 1 }, { PetalID::kMatPollenDust, 12 }, {}
        }},
        { PetalID::kSandLeafDart, 1, 2, {
            { PetalID::kLeafBlade, 2 }, { PetalID::kSandSpike, 2 }, {}, {}
        }},
        { PetalID::kDesertRoot, 1, 3, {
            { PetalID::kCraftCactus, 2 }, { PetalID::kMatTwig, 10 }, { PetalID::kMatSandstormEye, 1 }, {}
        }},
        { PetalID::kFrostBubble, 1, 3, {
            { PetalID::kWaterBubble, 2 }, { PetalID::kIceSpike, 1 }, { PetalID::kMatIceCrystal, 6 }, {}
        }},
        { PetalID::kFrostStarfish, 1, 2, {
            { PetalID::kStarfish, 2 }, { PetalID::kSnowball, 2 }, {}, {}
        }},
        { PetalID::kIceTideRing, 1, 3, {
            { PetalID::kTidePearl, 1 }, { PetalID::kIceShield, 1 }, { PetalID::kMatPermafrostHeart, 1 }, {}
        }},
        { PetalID::kDeepFrostPearl, 1, 2, {
            { PetalID::kTidePearl, 2 }, { PetalID::kMatColdIron, 8 }, {}, {}
        }},
        { PetalID::kPoisonAntNest, 1, 3, {
            { PetalID::kPoisonSpore, 2 }, { PetalID::kSoldierEgg, 1 }, { PetalID::kMatFormicAcid, 8 }, {}
        }},
        { PetalID::kCorrodeVine, 1, 2, {
            { PetalID::kThornVine, 2 }, { PetalID::kAcidFlask, 2 }, {}, {}
        }},
        { PetalID::kColonyCore, 1, 3, {
            { PetalID::kRotwood, 1 }, { PetalID::kPoisonSpore, 2 }, { PetalID::kMatQueenPheromone, 1 }, {}
        }},
        { PetalID::kSymbioteShell, 1, 2, {
            { PetalID::kRotwood, 2 }, { PetalID::kMatAntShell, 12 }, {}, {}
        }},
        { PetalID::kForgedHolySword, 1, 3, {
            { PetalID::kSteelSword, 1 }, { PetalID::kMagmaCore, 1 }, { PetalID::kMatCapitalBadge, 2 }, {}
        }},
        { PetalID::kFlameGear, 1, 2, {
            { PetalID::kMechGear, 2 }, { PetalID::kFireball, 2 }, {}, {}
        }},
        { PetalID::kObsidianKingShield, 1, 3, {
            { PetalID::kObsidianBlade, 1 }, { PetalID::kMechGear, 1 }, { PetalID::kMatSteel, 8 }, {}
        }},
        { PetalID::kHolyFlameCore, 1, 3, {
            { PetalID::kHolyOrb, 2 }, { PetalID::kMatFireCrystal, 10 }, { PetalID::kMatLeylineCore, 1 }, {}
        }},
        { PetalID::kAnemoneBloom, 1, 3, {
            { PetalID::kHealingBloom, 1 }, { PetalID::kStarfish, 1 }, { PetalID::kMatCoral, 8 }, {}
        }},
        { PetalID::kWetLeaf, 1, 2, {
            { PetalID::kLeafBlade, 2 }, { PetalID::kTidePearl, 1 }, {}, {}
        }},
        { PetalID::kCoralHive, 1, 2, {
            { PetalID::kBeeSting, 2 }, { PetalID::kMatCoral, 10 }, {}, {}
        }},
        { PetalID::kLifeLotus, 1, 2, {
            { PetalID::kHealingBloom, 2 }, { PetalID::kMatAbyssPearl, 1 }, {}, {}
        }},
        { PetalID::kMoltenSandBlade, 1, 2, {
            { PetalID::kSandSpike, 2 }, { PetalID::kObsidianBlade, 1 }, {}, {}
        }},
        { PetalID::kVolcanoWorm, 1, 3, {
            { PetalID::kQuicksandOrb, 1 }, { PetalID::kMagmaCore, 1 }, { PetalID::kMatSandwormCore, 1 }, {}
        }},
        { PetalID::kScorchPalm, 1, 2, {
            { PetalID::kCraftCactus, 2 }, { PetalID::kFireball, 2 }, {}, {}
        }},
        { PetalID::kGlassStorm, 1, 2, {
            { PetalID::kQuicksandOrb, 2 }, { PetalID::kMatFireCrystal, 8 }, {}, {}
        }},
        { PetalID::kFrostKnightSword, 1, 2, {
            { PetalID::kSteelSword, 1 }, { PetalID::kIceSpike, 2 }, {}, {}
        }},
        { PetalID::kHolySnowflake, 1, 2, {
            { PetalID::kHolyOrb, 2 }, { PetalID::kSnowball, 2 }, {}, {}
        }},
        { PetalID::kPermafrostGear, 1, 2, {
            { PetalID::kMechGear, 2 }, { PetalID::kMatPermafrostHeart, 1 }, {}, {}
        }},
        { PetalID::kFrostCrown, 1, 3, {
            { PetalID::kHolyOrb, 1 }, { PetalID::kIceShield, 2 }, { PetalID::kMatRoyalSeal, 1 }, {}
        }},
        { PetalID::kRoyalThorn, 1, 2, {
            { PetalID::kThornVine, 2 }, { PetalID::kSteelSword, 1 }, {}, {}
        }},
        { PetalID::kMechPoisonBee, 1, 3, {
            { PetalID::kBeeSting, 2 }, { PetalID::kMechGear, 1 }, { PetalID::kMatVenomSac, 6 }, {}
        }},
        { PetalID::kHolySpore, 1, 2, {
            { PetalID::kPoisonSpore, 2 }, { PetalID::kHolyOrb, 1 }, {}, {}
        }},
        { PetalID::kTreantGuard, 1, 3, {
            { PetalID::kRotwood, 2 }, { PetalID::kMatCapitalBadge, 3 }, { PetalID::kMatHeartwoodChip, 1 }, {}
        }},
        { PetalID::kHeartOfNature, 2, 4, {
            { PetalID::kLifeLotus, 1 }, { PetalID::kMatAncientHeart, 1 }, { PetalID::kMatHeartwoodChip, 1 }, { PetalID::kMatRoyalJelly, 3 }
        }},
        { PetalID::kAbyssalTide, 2, 4, {
            { PetalID::kIceTideRing, 1 }, { PetalID::kMatTideCore, 1 }, { PetalID::kMatBlizzardEye, 1 }, { PetalID::kMatAbyssPearl, 3 }
        }},
        { PetalID::kInfernalCore, 2, 4, {
            { PetalID::kHolyFlameCore, 1 }, { PetalID::kMatLeylineCore, 2 }, { PetalID::kMatPhoenixEmber, 1 }, { PetalID::kMatObsidian, 15 }
        }},
        { PetalID::kChronoVortex, 2, 4, {
            { PetalID::kPermafrostGear, 1 }, { PetalID::kMatSandstormEye, 1 }, { PetalID::kMatSovereignCore, 1 }, { PetalID::kMatTideCore, 1 }
        }},
        { PetalID::kAntKing, 2, 3, {
            { PetalID::kPoisonAntNest, 2 }, { PetalID::kMatHiveHeart, 1 }, { PetalID::kMatRoyalSeal, 1 }, {}
        }},
        { PetalID::kWorldTreeSeed, 2, 3, {
            { PetalID::kHeartOfNature, 1 }, { PetalID::kMatAncientHeart, 2 }, { PetalID::kMatAbyssPearl, 2 }, {}
        }},
        { PetalID::kSkyblade, 2, 3, {
            { PetalID::kForgedHolySword, 1 }, { PetalID::kFrostKnightSword, 1 }, { PetalID::kMatRoyalSeal, 2 }, {}
        }},
        { PetalID::kChaosPrism, 2, 3, {
            { PetalID::kChronoVortex, 1 }, { PetalID::kInfernalCore, 1 }, { PetalID::kAbyssalTide, 1 }, {}
        }},
    };

    uint16_t recipe_count() {
        return (uint16_t) (sizeof(kRecipes) / sizeof(kRecipes[0]));
    }

    Def const &recipe_at(uint16_t index) {
        return kRecipes[index % recipe_count()];
    }

    uint16_t find_by_result(PetalID::T result) {
        for (uint16_t i = 0; i < recipe_count(); ++i)
            if (kRecipes[i].result == result) return i;
        return (uint16_t) -1;
    }

    uint8_t is_bag_only(PetalID::T id) {
        if (id == PetalID::kNone || id >= PetalID::kNumPetals) return 1;
        // Currency + craft mats stay in the backpack (never primary combat slots).
        if (id == PetalID::kCoin) return 1;
        if (id >= PetalID::kMatLeaf && id <= PetalID::kMatRoyalSeal) return 1;
        return 0;
    }
}
