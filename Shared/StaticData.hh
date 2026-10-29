#pragma once

#include <Shared/StaticDefinitions.hh>
#include <Shared/RarityScale.hh>

#include <array>
#include <cstdint>
#include <cstring>

extern uint32_t const TPS;

extern float const PLAYER_ACCELERATION;
extern float const DEFAULT_FRICTION;
extern float const SUMMON_RETREAT_RADIUS;
extern float const DIGGER_SPAWN_CHANCE;
extern float const WILD_LEASH_MULT;
extern float const MOB_MOVE_SPEED_MULT;
extern float const MOB_AGGRO_RANGE_MULT;

extern float const BASE_FLOWER_RADIUS;
extern float const BASE_PETAL_ROTATION_SPEED;
extern float const BASE_FOV;
extern float const BASE_HEALTH;
extern float const BASE_BODY_DAMAGE;

// Petal orbit extras beyond flower/moon radius (world units)
extern float const PETAL_ORBIT_IDLE;
extern float const PETAL_ORBIT_ATTACK;
extern float const PETAL_ORBIT_DEFEND;
extern float const PETAL_WING_WAVE_EXTRA;

extern std::array<struct PetalData, PetalID::kNumPetals> const PETAL_DATA;
extern std::array<struct MobData, MobID::kNumMobs> const MOB_DATA;

// World mob size / population. Radius×2 + dense multi-segment packs (centipede/
// leech) blew spatial-hash collide to ~100�?00ms/tick �?keep mild scale only.
inline constexpr float MOB_RADIUS_MULTIPLIER = 1.25f;
// Zone dens × FOV target. HK 8c: aim clone ≈16/screen; empty-map drain + REST
// phase telemetry keep daytime overruns diagnosable.
inline constexpr float MOB_DENSITY_MULTIPLIER = 1.20f;

inline uint8_t petal_name_is(PetalID::T id, char const *name) {
    return id != PetalID::kNone && id < PetalID::kNumPetals
        && std::strcmp(PETAL_DATA[id].name, name) == 0;
}

// Hot-path ID checks (avoid strcmp in collide / petal ticks).
inline uint8_t petal_is_meteor(PetalID::T id) {
    return id == PetalID::kMeteor
        || (id >= PetalID::kMeteorCommon && id <= PetalID::kMeteorUltra);
}

inline uint8_t petal_is_honey(PetalID::T id) {
    return id == PetalID::kHoney
        || (id >= PetalID::kHoneyCommon && id <= PetalID::kHoneyUltra);
}

inline uint8_t petal_is_moon(PetalID::T id) {
    return id == PetalID::kMoon
        || (id >= PetalID::kMoonCommon && id <= PetalID::kMoonSuper);
}

inline uint8_t petal_is_bone(PetalID::T id) {
    return petal_name_is(id, "Bone");
}

// Wiki petal heals used √3 after Mythic while combat stayed ×3 — Ultra+ heals
// felt broken vs cactus HP / zone damage. Catch up to full ×3 from Mythic.
inline float heal_tier_catchup(uint8_t rarity) {
    if (rarity >= RarityID::kUnique) return 5.1961524f; // 6561 / (729*√3)
    if (rarity >= RarityID::kSuper) return 3.f;         // 2187 / 729
    if (rarity >= RarityID::kUltra) return 1.7320508f;  // 729 / (243*√3)
    return 1.f;
}

// Same √3→×3 catchup for petal armor (Bone / shells) so Ultra+ isn't paper.
inline float armor_tier_catchup(uint8_t rarity) {
    return heal_tier_catchup(rarity);
}

using MapZones = StaticArray<struct ZoneDefinition, MAX_ZONES_PER_MAP>;
using MapTerrain = StaticArray<struct TerrainFeature, MAX_TERRAIN_PER_MAP>;

// Spawn / difficulty templates. Geometry rewritten by MapGen (seeded per map_id).
// Outer �?harder. Difficulty 0 hub is fixed spawn for new players.
inline std::array const MAPS = std::to_array<MapZones>({
    // ---- Garden ----
    // Bees must appear on Mythic+ rings — they used to stop at Unusual (diff≤1),
    // while the outer cap was only Epic(+1=Legendary), so M+ bees never existed.
    MapZones({
        {
            .left = 0, .top = 0, .right = (float)ARENA_WIDTH, .bottom = (float)ARENA_HEIGHT,
            .density = 3.2f, .drop_multiplier = 0.58,
            .spawns = {
                { MobID::kBee, 110000 }, { MobID::kHornet, 110000 }, { MobID::kSpider, 95000 },
                { MobID::kDarkLadybug, 90000 }, { MobID::kEvilCentipede, 28000 },
                { MobID::kMassiveBeetle, 12000 }, { MobID::kBoulder, 9000 },
                { MobID::kSquare, 1 }
            },
            // Mythic primary; +1 can roll Ultra (cap Ultra). Hub stays soft below.
            .difficulty = RarityID::kMythic, .color = 0xff8FC777, .name = "Garden"
        },
        {
            .left = 2200, .top = 2200, .right = 15800, .bottom = 15800,
            .density = 2.7f, .drop_multiplier = 0.30,
            .spawns = {
                { MobID::kRock, 70000 }, { MobID::kBee, 95000 }, { MobID::kSpider, 90000 },
                { MobID::kHornet, 90000 }, { MobID::kBeetle, 65000 },
                { MobID::kEvilCentipede, 20000 }, { MobID::kMassiveBeetle, 8000 },
                { MobID::kSquare, 1 }
            },
            // Legendary primary; +1 Mythic so mid-garden can still see M bees.
            .difficulty = RarityID::kLegendary, .color = 0xffA8D484, .name = "Garden"
        },
        {
            .left = 4800, .top = 4800, .right = 13200, .bottom = 13200,
            .density = 2.65f, .drop_multiplier = 0.10,
            .spawns = {
                { MobID::kRock, 160000 }, { MobID::kBee, 90000 }, { MobID::kLadybug, 100000 },
                { MobID::kBeetle, 55000 }, { MobID::kCentipede, 15000 },
                { MobID::kShinyLadybug, 2000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 1, .color = MapTheme::GARDEN_GROUND, .name = "Garden"
        },
        {
            // Spawn hub — Rock-heavy stakes (~70% passive/stationary feel).
            // Template AABB only; MapGen recenters on MAP_SPAWN (arena mid).
            .left = ARENA_WIDTH * 0.45f, .top = ARENA_HEIGHT * 0.46f,
            .right = ARENA_WIDTH * 0.55f, .bottom = ARENA_HEIGHT * 0.54f,
            .density = 2.55f, .drop_multiplier = 0.02,
            .spawns = {
                { MobID::kRock, 320000 }, { MobID::kLadybug, 180000 },
                { MobID::kBee, 120000 }, { MobID::kBeetle, 80000 },
                { MobID::kCentipede, 30000 }, { MobID::kShinyLadybug, 6000 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 0, .color = MapTheme::GARDEN_GROUND, .name = "Garden"
        }
    }),
    // ---- Desert ----
    MapZones({
        {
            .left = 0, .top = 0, .right = (float)ARENA_WIDTH, .bottom = (float)ARENA_HEIGHT,
            .density = 3.2f, .drop_multiplier = 0.56,
            .spawns = {
                { MobID::kDuneSpider, 105000 }, { MobID::kSandstorm, 90000 },
                { MobID::kScorpion, 85000 }, { MobID::kDesertCentipede, 22000 },
                { MobID::kDuneTitan, 14000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 3, .color = 0xffb57a2a, .name = "Desert"
        },
        {
            .left = 2200, .top = 2200, .right = 15800, .bottom = 15800,
            .density = 2.7f, .drop_multiplier = 0.28,
            .spawns = {
                { MobID::kCactus, 160000 }, { MobID::kDuneStone, 80000 },
                { MobID::kDuneBeetle, 110000 }, { MobID::kDuneSpider, 90000 },
                { MobID::kDuneHornet, 80000 }, { MobID::kScorpion, 70000 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 2, .color = 0xffc9923c, .name = "Desert"
        },
        {
            .left = 4800, .top = 4800, .right = 13200, .bottom = 13200,
            .density = 2.65f, .drop_multiplier = 0.09,
            .spawns = {
                { MobID::kCactus, 180000 }, { MobID::kDuneStone, 140000 },
                { MobID::kSandMite, 140000 }, { MobID::kDuneBeetle, 90000 },
                { MobID::kScorpion, 60000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 1, .color = 0xffd4a84b, .name = "Desert"
        },
        {
            // Spawn hub — cactus / stone stakes.
            .left = 7200, .top = 7200, .right = 10800, .bottom = 10800,
            .density = 2.55f, .drop_multiplier = 0.02,
            .spawns = {
                { MobID::kCactus, 220000 }, { MobID::kDuneStone, 200000 },
                { MobID::kSandMite, 140000 }, { MobID::kDuneBeetle, 90000 },
                { MobID::kScorpion, 40000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 0, .color = MapTheme::DESERT_GROUND, .name = "Desert"
        }
    }),
    // ---- Ocean ----
    MapZones({
        {
            .left = 0, .top = 0, .right = (float)ARENA_WIDTH, .bottom = (float)ARENA_HEIGHT,
            .density = 3.2f, .drop_multiplier = 0.56,
            .spawns = {
                { MobID::kSeaLeech, 24000 }, { MobID::kMorayEel, 70000 },
                { MobID::kInkCuttle, 20000 }, { MobID::kAbyssShark, 14000 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 3, .color = 0xff023e8a, .name = "Ocean"
        },
        {
            .left = 2200, .top = 2200, .right = 15800, .bottom = 15800,
            .density = 2.7f, .drop_multiplier = 0.28,
            .spawns = {
                { MobID::kSpinePuffer, 105000 }, { MobID::kSeaLeech, 24000 },
                { MobID::kMorayEel, 70000 }, { MobID::kReefCrab, 100000 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 2, .color = 0xff005582, .name = "Ocean"
        },
        {
            .left = 4800, .top = 4800, .right = 13200, .bottom = 13200,
            .density = 2.65f, .drop_multiplier = 0.08,
            .spawns = {
                { MobID::kSponge, 140000 }, { MobID::kBubble, 130000 },
                { MobID::kReefStone, 100000 }, { MobID::kSeaFry, 120000 },
                { MobID::kCoralStar, 90000 }, { MobID::kMoonJelly, 70000 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 1, .color = 0xff00669c, .name = "Ocean"
        },
        {
            // Spawn hub — reef stone / fry stakes.
            .left = 7200, .top = 7200, .right = 10800, .bottom = 10800,
            .density = 2.55f, .drop_multiplier = 0.02,
            .spawns = {
                { MobID::kSponge, 200000 }, { MobID::kBubble, 180000 },
                { MobID::kReefStone, 140000 }, { MobID::kSeaFry, 140000 },
                { MobID::kCoralStar, 80000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 0, .color = MapTheme::OCEAN_GROUND, .name = "Ocean"
        }
    }),
    // ---- Polar ----
    MapZones({
        {
            .left = 0, .top = 0, .right = (float)ARENA_WIDTH, .bottom = (float)ARENA_HEIGHT,
            .density = 3.2f, .drop_multiplier = 0.56,
            .spawns = {
                { MobID::kRimeCentipede, 22000 }, { MobID::kBlizzardWisp, 110000 },
                { MobID::kGlacierBeast, 90000 }, { MobID::kFrostYeti, 18000 },
                { MobID::kIceGolem, 14000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 3, .color = 0xff6c757d, .name = "Snow"
        },
        {
            .left = 2200, .top = 2200, .right = 15800, .bottom = 15800,
            .density = 2.7f, .drop_multiplier = 0.28,
            .spawns = {
                { MobID::kRimeCentipede, 22000 }, { MobID::kBlizzardWisp, 110000 },
                { MobID::kGlacierBeast, 90000 }, { MobID::kIceSpider, 120000 },
                { MobID::kFrostHornet, 105000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 2, .color = 0xff868e96, .name = "Snow"
        },
        {
            .left = 4800, .top = 4800, .right = 13200, .bottom = 13200,
            .density = 2.65f, .drop_multiplier = 0.07,
            .spawns = {
                { MobID::kSnowHare, 135000 }, { MobID::kIceSpider, 120000 },
                { MobID::kFrostHornet, 105000 }, { MobID::kSnowMite, 155000 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 1, .color = 0xff9aa3ab, .name = "Snow"
        },
        {
            // Spawn hub �?rich low-tier snow pack.
            .left = 7200, .top = 7200, .right = 10800, .bottom = 10800,
            .density = 2.55f, .drop_multiplier = 0.02,
            .spawns = {
                { MobID::kFrostStone, 140000 }, { MobID::kSnowMite, 160000 },
                { MobID::kSnowHare, 140000 }, { MobID::kIceSpider, 110000 },
                { MobID::kFrostHornet, 70000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 0, .color = MapTheme::POLAR_GROUND, .name = "Snow"
        }
    }),
    // ---- Jungle ----
    // Template rings (rewritten by build_zones + eco pockets). Jungle Wasp in every pool.
    MapZones({
        {
            // hard / Deep
            .left = 0, .top = 0, .right = (float)ARENA_WIDTH, .bottom = (float)ARENA_HEIGHT,
            .density = 3.4f, .drop_multiplier = 0.58,
            .spawns = {
                { MobID::kEmeraldMantis, 120000 }, { MobID::kStoneGuardian, 80000 },
                { MobID::kCarnivorousPlant, 100000 }, { MobID::kVineBug, 90000 },
                { MobID::kJungleWasp, 110000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 3, .color = 0xff1b4332, .name = "Jungle Deep"
        },
        {
            // mid
            .left = 2200, .top = 2200, .right = 15800, .bottom = 15800,
            .density = 2.8f, .drop_multiplier = 0.28,
            .spawns = {
                { MobID::kJungleSpider, 130000 }, { MobID::kPoisonToad, 90000 },
                { MobID::kSporeMushroom, 100000 }, { MobID::kRiverCrab, 80000 },
                { MobID::kJungleWasp, 100000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 2, .color = 0xff2d6a4f, .name = "Jungle Mid"
        },
        {
            // soft / Edge
            .left = 4800, .top = 4800, .right = 13200, .bottom = 13200,
            .density = 2.7f, .drop_multiplier = 0.09,
            .spawns = {
                { MobID::kBush, 140000 }, { MobID::kLeafbug, 120000 },
                { MobID::kJungleLadybug, 100000 }, { MobID::kFirefly, 90000 },
                { MobID::kJungleStone, 70000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 1, .color = 0xff40916c, .name = "Jungle Edge"
        },
        {
            // hub — Bush / Leafbug stakes; no wasps (aggro on sight)
            .left = 7200, .top = 7200, .right = 10800, .bottom = 10800,
            .density = 2.55f, .drop_multiplier = 0.02,
            .spawns = {
                { MobID::kBush, 200000 }, { MobID::kLeafbug, 160000 },
                { MobID::kJungleLadybug, 120000 }, { MobID::kFirefly, 80000 },
                { MobID::kJungleStone, 100000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 0, .color = MapTheme::JUNGLE_GROUND, .name = "Jungle Hub"
        }
    }),
    // ---- Ant Hell ----
    MapZones({
        {
            .left = 0, .top = 0, .right = (float)ARENA_WIDTH, .bottom = (float)ARENA_HEIGHT,
            .density = 3.4f, .drop_multiplier = 0.58,
            .spawns = {
                { MobID::kSoldierAnt, 160000 }, { MobID::kWorkerAnt, 140000 },
                { MobID::kAntHole, 18000 }, { MobID::kQueenAnt, 8000 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 3, .color = 0xff582f0e, .name = "Ant Hell"
        },
        {
            .left = 2200, .top = 2200, .right = 15800, .bottom = 15800,
            .density = 2.8f, .drop_multiplier = 0.28,
            .spawns = {
                { MobID::kWorkerAnt, 150000 }, { MobID::kSoldierAnt, 120000 },
                { MobID::kBabyAnt, 100000 }, { MobID::kAntHole, 12000 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 2, .color = 0xff6f4518, .name = "Ant Hell"
        },
        {
            .left = 4800, .top = 4800, .right = 13200, .bottom = 13200,
            .density = 2.7f, .drop_multiplier = 0.10,
            .spawns = {
                { MobID::kBabyAnt, 220000 }, { MobID::kWorkerAnt, 110000 },
                { MobID::kAntHole, 3000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 1, .color = 0xffa98467, .name = "Ant Hell"
        },
        {
            // Spawn hub — Baby Ant heavy (Poo farm).
            .left = 7200, .top = 7200, .right = 10800, .bottom = 10800,
            .density = 2.55f, .drop_multiplier = 0.02,
            .spawns = {
                { MobID::kBabyAnt, 280000 }, { MobID::kWorkerAnt, 120000 },
                { MobID::kAntHole, 35000 }, { MobID::kSoldierAnt, 35000 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 0, .color = MapTheme::ANT_GROUND, .name = "Ant Hell"
        }
    }),
    // ---- Volcano ----
    MapZones({
        {
            .left = 0, .top = 0, .right = (float)ARENA_WIDTH, .bottom = (float)ARENA_HEIGHT,
            .density = 3.2f, .drop_multiplier = 0.56,
            .spawns = {
                { MobID::kMagmaCrab, 105000 }, { MobID::kFireSpider, 110000 },
                { MobID::kMagmaCentipede, 22000 }, { MobID::kObsidianGolem, 18000 },
                { MobID::kLavaColossus, 14000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 3, .color = 0xff1a1a2e, .name = "Volcano"
        },
        {
            .left = 2200, .top = 2200, .right = 15800, .bottom = 15800,
            .density = 2.7f, .drop_multiplier = 0.28,
            .spawns = {
                { MobID::kCinderMoth, 135000 }, { MobID::kLavaBeetle, 125000 },
                { MobID::kEmberWasp, 105000 }, { MobID::kMagmaCrab, 105000 },
                { MobID::kFireSpider, 110000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 2, .color = 0xff3d405b, .name = "Volcano"
        },
        {
            .left = 4800, .top = 4800, .right = 13200, .bottom = 13200,
            .density = 2.65f, .drop_multiplier = 0.08,
            .spawns = {
                { MobID::kAshMite, 155000 }, { MobID::kCinderMoth, 135000 },
                { MobID::kLavaBeetle, 125000 }, { MobID::kEmberWasp, 105000 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 1, .color = 0xff4a4e69, .name = "Volcano"
        },
        {
            // Spawn hub �?rich low-tier volcano pack.
            .left = 7200, .top = 7200, .right = 10800, .bottom = 10800,
            .density = 2.55f, .drop_multiplier = 0.02,
            .spawns = {
                { MobID::kEmberStone, 140000 }, { MobID::kAshMite, 160000 },
                { MobID::kCinderMoth, 130000 }, { MobID::kLavaBeetle, 120000 },
                { MobID::kEmberWasp, 60000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 0, .color = MapTheme::VOLCANO_GROUND, .name = "Volcano"
        }
    }),
    // ---- Royal Capital (exactly 6 zones; geometry rewritten by build_zones_royal) ----
    MapZones({
        {
            // 0 Outer Wall — rich low-tier capital pack
            .left = 0, .top = 0, .right = (float)ARENA_WIDTH, .bottom = (float)ARENA_HEIGHT,
            .density = 2.55f, .drop_multiplier = 0.02,
            .spawns = {
                { MobID::kRoyalBoulder, 140000 }, { MobID::kRoyalPebble, 150000 },
                { MobID::kRustMite, 160000 }, { MobID::kCoinBeetle, 120000 },
                { MobID::kMimicChest, 80000 }, { MobID::kWanderingGuard, 50000 },
                { MobID::kTrader, 12000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 0, .color = RARITY_COLORS[RarityID::kCommon], .name = "Outer Wall"
        },
        {
            // 1 Common Quarter — Epic band
            .left = 800, .top = 7500, .right = 12500, .bottom = 19200,
            .density = 3.50f, .drop_multiplier = 0.09,
            .spawns = {
                { MobID::kRustMite, 300000 }, { MobID::kMimicChest, 250000 },
                { MobID::kCoinBeetle, 200000 }, { MobID::kWanderingGuard, 250000 },
                { MobID::kTrader, 18000 }, { MobID::kSquare, 1 }
            },
            .difficulty = RarityID::kEpic, .color = RARITY_COLORS[RarityID::kEpic], .name = "Common Quarter"
        },
        {
            // 2 Royal Garden — Legendary
            .left = 15500, .top = 3500, .right = 27200, .bottom = 16500,
            .density = 2.4f, .drop_multiplier = 0.17,
            .spawns = {
                { MobID::kRoyalBee, 450000 }, { MobID::kThornBug, 350000 },
                { MobID::kCoinBeetle, 200000 }, { MobID::kSquare, 1 }
            },
            .difficulty = RarityID::kLegendary, .color = RARITY_COLORS[RarityID::kLegendary], .name = "Royal Garden"
        },
        {
            // 3 Barracks — Mythic
            .left = 5000, .top = 400, .right = 23000, .bottom = 7200,
            .density = 2.5f, .drop_multiplier = 0.19,
            .spawns = {
                { MobID::kWanderingGuard, 200000 }, { MobID::kShieldSentinel, 350000 },
                { MobID::kSpearSentinel, 300000 }, { MobID::kStoneBeetle, 150000 },
                { MobID::kSquare, 1 }
            },
            .difficulty = RarityID::kMythic, .color = RARITY_COLORS[RarityID::kMythic], .name = "Barracks"
        },
        {
            // 4 Ruined Quarter — Ultra (Oracle rare attach)
            .left = 600, .top = 800, .right = 10500, .bottom = 9500,
            .density = 3.2f, .drop_multiplier = 0.55,
            .spawns = {
                { MobID::kPhantomOrb, 400000 }, { MobID::kFallenKnight, 250000 },
                { MobID::kMimicChest, 200000 }, { MobID::kRustMite, 150000 },
                { MobID::kOracle, 10000 }, { MobID::kSquare, 1 }
            },
            .difficulty = RarityID::kUltra, .color = RARITY_COLORS[RarityID::kUltra], .name = "Ruined Quarter"
        },
        {
            // 5 Royal Palace — Super (Titan rare attach near throne)
            .left = 14500, .top = 4500, .right = 24500, .bottom = 11500,
            .density = 3.3f, .drop_multiplier = 0.58,
            .spawns = {
                { MobID::kShieldSentinel, 350000 }, { MobID::kSpearSentinel, 300000 },
                { MobID::kFallenKnight, 250000 }, { MobID::kWanderingGuard, 100000 },
                { MobID::kTitan, 8000 }, { MobID::kSquare, 1 }
            },
            .difficulty = RarityID::kSuper, .color = RARITY_COLORS[RarityID::kSuper], .name = "Royal Palace"
        }
    }),
    // ---- Ant Nest (player layout; geometry/spawns rewritten by build_zones_ant_nest) ----
    MapZones({
        {
            .left = 0, .top = 0, .right = (float)ARENA_WIDTH, .bottom = (float)ARENA_HEIGHT,
            .density = 3.4f, .drop_multiplier = 0.58,
            .spawns = {
                { MobID::kSoldierAnt, 160000 }, { MobID::kWorkerAnt, 140000 },
                { MobID::kAntHole, 18000 }, { MobID::kQueenAnt, 8000 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 3, .color = 0xff582f0e, .name = "Ant Nest"
        },
        {
            .left = 2200, .top = 2200, .right = 15800, .bottom = 15800,
            .density = 2.8f, .drop_multiplier = 0.28,
            .spawns = {
                { MobID::kWorkerAnt, 150000 }, { MobID::kSoldierAnt, 120000 },
                { MobID::kBabyAnt, 100000 }, { MobID::kAntHole, 12000 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 2, .color = 0xff6f4518, .name = "Ant Nest"
        },
        {
            .left = 4800, .top = 4800, .right = 13200, .bottom = 13200,
            .density = 2.7f, .drop_multiplier = 0.10,
            .spawns = {
                { MobID::kBabyAnt, 180000 }, { MobID::kWorkerAnt, 120000 },
                { MobID::kAntHole, 3000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 1, .color = 0xffa98467, .name = "Ant Nest"
        },
        {
            .left = 7200, .top = 7200, .right = 10800, .bottom = 10800,
            .density = 2.55f, .drop_multiplier = 0.02,
            .spawns = {
                { MobID::kBabyAnt, 200000 }, { MobID::kWorkerAnt, 160000 },
                { MobID::kAntHole, 40000 }, { MobID::kSoldierAnt, 50000 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 0, .color = MapTheme::ANT_GROUND, .name = "Ant Nest"
        }
    }),
    // ---- Wildwood (L2) — former Ashflor jungle dens ----
    MapZones({
        {
            .left = 0, .top = 0, .right = (float)ARENA_WIDTH, .bottom = (float)ARENA_HEIGHT,
            .density = 3.4f, .drop_multiplier = 0.58,
            .spawns = {
                { MobID::kEmeraldMantis, 120000 }, { MobID::kStoneGuardian, 80000 },
                { MobID::kCarnivorousPlant, 100000 }, { MobID::kVineBug, 90000 },
                { MobID::kJungleWasp, 110000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 3, .color = 0xff1b4332, .name = "Wildwood Deep"
        },
        {
            .left = 2200, .top = 2200, .right = 15800, .bottom = 15800,
            .density = 2.8f, .drop_multiplier = 0.28,
            .spawns = {
                { MobID::kJungleSpider, 130000 }, { MobID::kPoisonToad, 90000 },
                { MobID::kSporeMushroom, 100000 }, { MobID::kRiverCrab, 80000 },
                { MobID::kJungleWasp, 100000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 2, .color = 0xff2d6a4f, .name = "Wildwood Mid"
        },
        {
            .left = 4800, .top = 4800, .right = 13200, .bottom = 13200,
            .density = 2.7f, .drop_multiplier = 0.09,
            .spawns = {
                { MobID::kLeafBeetle, 140000 }, { MobID::kJungleMite, 130000 },
                { MobID::kVineBug, 90000 }, { MobID::kJungleWasp, 90000 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 1, .color = 0xff40916c, .name = "Wildwood Edge"
        },
        {
            .left = 7200, .top = 7200, .right = 10800, .bottom = 10800,
            .density = 2.55f, .drop_multiplier = 0.02,
            .spawns = {
                { MobID::kJungleStone, 160000 }, { MobID::kJungleCrawler, 160000 },
                { MobID::kLeafBeetle, 140000 }, { MobID::kJungleMite, 120000 },
                { MobID::kVineBug, 60000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 0, .color = MapTheme::JUNGLE_GROUND, .name = "Wildwood Hub"
        }
    }),
    // ---- Sewers (L2) ----
    MapZones({
        {
            .left = 0, .top = 0, .right = (float)ARENA_WIDTH, .bottom = (float)ARENA_HEIGHT,
            .density = 3.3f, .drop_multiplier = 0.55,
            .spawns = {
                { MobID::kRoach, 120000 }, { MobID::kSilverfish, 110000 },
                { MobID::kMoth, 100000 }, { MobID::kFly, 100000 },
                { MobID::kSpider, 70000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 3, .color = 0xff3A3E38, .name = "Deep Sewers"
        },
        {
            .left = 2200, .top = 2200, .right = 15800, .bottom = 15800,
            .density = 2.8f, .drop_multiplier = 0.28,
            .spawns = {
                { MobID::kRoach, 120000 }, { MobID::kSilverfish, 110000 },
                { MobID::kFly, 120000 }, { MobID::kGarbage, 70000 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 2, .color = 0xff4A5048, .name = "Sewer Mid"
        },
        {
            .left = 4800, .top = 4800, .right = 13200, .bottom = 13200,
            .density = 2.7f, .drop_multiplier = 0.10,
            .spawns = {
                { MobID::kGarbage, 150000 }, { MobID::kSilverfish, 120000 },
                { MobID::kFly, 130000 }, { MobID::kRoach, 100000 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 1, .color = 0xff5A6058, .name = "Sewer Edge"
        },
        {
            .left = 7200, .top = 7200, .right = 10800, .bottom = 10800,
            .density = 2.55f, .drop_multiplier = 0.02,
            .spawns = {
                { MobID::kGarbage, 200000 }, { MobID::kSilverfish, 100000 },
                { MobID::kFly, 140000 }, { MobID::kRoach, 120000 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 0, .color = MapTheme::SEWER_GROUND, .name = "Sewer Hub"
        }
    }),
    // ---- Prime Garden (L2 official garden) ----
    MapZones({
        {
            .left = 0, .top = 0, .right = (float)ARENA_WIDTH, .bottom = (float)ARENA_HEIGHT,
            .density = 3.2f, .drop_multiplier = 0.56,
            .spawns = {
                { MobID::kHornet, 110000 }, { MobID::kSpider, 110000 },
                { MobID::kDarkLadybug, 100000 }, { MobID::kEvilCentipede, 28000 },
                { MobID::kMassiveBeetle, 12000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 3, .color = 0xff6FA85A, .name = "Prime Deep"
        },
        {
            .left = 2200, .top = 2200, .right = 15800, .bottom = 15800,
            .density = 2.7f, .drop_multiplier = 0.30,
            .spawns = {
                { MobID::kRock, 80000 }, { MobID::kBee, 100000 }, { MobID::kSpider, 95000 },
                { MobID::kHornet, 95000 }, { MobID::kBeetle, 70000 },
                { MobID::kCentipede, 18000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 2, .color = 0xff8FC777, .name = "Prime Mid"
        },
        {
            .left = 4800, .top = 4800, .right = 13200, .bottom = 13200,
            .density = 2.65f, .drop_multiplier = 0.10,
            .spawns = {
                { MobID::kRock, 180000 }, { MobID::kLadybug, 140000 },
                { MobID::kBee, 120000 }, { MobID::kBeetle, 60000 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 1, .color = 0xffA8D484, .name = "Prime Edge"
        },
        {
            .left = 7200, .top = 7200, .right = 10800, .bottom = 10800,
            .density = 2.55f, .drop_multiplier = 0.02,
            .spawns = {
                { MobID::kRock, 280000 }, { MobID::kLadybug, 180000 },
                { MobID::kBee, 140000 }, { MobID::kBeetle, 70000 },
                { MobID::kShinyLadybug, 8000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 0, .color = MapTheme::GARDEN_GROUND, .name = "Prime Hub"
        }
    }),
    // ---- Prime Desert / 沙海 (L2) ----
    MapZones({
        {
            .left = 0, .top = 0, .right = (float)ARENA_WIDTH, .bottom = (float)ARENA_HEIGHT,
            .density = 3.2f, .drop_multiplier = 0.56,
            .spawns = {
                { MobID::kDuneSpider, 110000 }, { MobID::kSandstorm, 100000 },
                { MobID::kScorpion, 100000 }, { MobID::kDesertCentipede, 28000 },
                { MobID::kDuneTitan, 16000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 3, .color = 0xffb57a2a, .name = "Sand Seas Deep"
        },
        {
            .left = 2200, .top = 2200, .right = 15800, .bottom = 15800,
            .density = 2.7f, .drop_multiplier = 0.28,
            .spawns = {
                { MobID::kCactus, 150000 }, { MobID::kDuneBeetle, 120000 },
                { MobID::kDuneSpider, 100000 }, { MobID::kScorpion, 90000 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 2, .color = 0xffc9923c, .name = "Sand Seas Mid"
        },
        {
            .left = 4800, .top = 4800, .right = 13200, .bottom = 13200,
            .density = 2.65f, .drop_multiplier = 0.09,
            .spawns = {
                { MobID::kCactus, 180000 }, { MobID::kDuneStone, 150000 },
                { MobID::kSandMite, 130000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 1, .color = 0xffd4a84b, .name = "Sand Seas Edge"
        },
        {
            .left = 7200, .top = 7200, .right = 10800, .bottom = 10800,
            .density = 2.55f, .drop_multiplier = 0.02,
            .spawns = {
                { MobID::kCactus, 240000 }, { MobID::kDuneStone, 200000 },
                { MobID::kSandMite, 140000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 0, .color = MapTheme::DESERT_GROUND, .name = "Sand Seas Hub"
        }
    }),
    // ---- Prime Ocean / 深海 (L2) ----
    MapZones({
        {
            .left = 0, .top = 0, .right = (float)ARENA_WIDTH, .bottom = (float)ARENA_HEIGHT,
            .density = 3.2f, .drop_multiplier = 0.56,
            .spawns = {
                { MobID::kSeaLeech, 40000 }, { MobID::kMorayEel, 100000 },
                { MobID::kInkCuttle, 80000 }, { MobID::kAbyssShark, 40000 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 3, .color = 0xff023e8a, .name = "Deep Ocean"
        },
        {
            .left = 2200, .top = 2200, .right = 15800, .bottom = 15800,
            .density = 2.7f, .drop_multiplier = 0.28,
            .spawns = {
                { MobID::kSpinePuffer, 110000 }, { MobID::kMorayEel, 90000 },
                { MobID::kReefCrab, 100000 }, { MobID::kSeaLeech, 30000 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 2, .color = 0xff005582, .name = "Deep Mid"
        },
        {
            .left = 4800, .top = 4800, .right = 13200, .bottom = 13200,
            .density = 2.65f, .drop_multiplier = 0.08,
            .spawns = {
                { MobID::kSponge, 140000 }, { MobID::kBubble, 130000 },
                { MobID::kCoralStar, 110000 }, { MobID::kSeaFry, 120000 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 1, .color = 0xff00669c, .name = "Deep Edge"
        },
        {
            .left = 7200, .top = 7200, .right = 10800, .bottom = 10800,
            .density = 2.55f, .drop_multiplier = 0.02,
            .spawns = {
                { MobID::kSponge, 200000 }, { MobID::kBubble, 180000 },
                { MobID::kReefStone, 150000 }, { MobID::kSeaFry, 140000 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 0, .color = MapTheme::OCEAN_GROUND, .name = "Deep Hub"
        }
    }),
    // ---- Factory (L3) ----
    MapZones({
        {
            .left = 0, .top = 0, .right = (float)ARENA_WIDTH, .bottom = (float)ARENA_HEIGHT,
            .density = 3.3f, .drop_multiplier = 0.58,
            .spawns = {
                { MobID::kMechaFlower, 100000 }, { MobID::kMechaWasp, 110000 },
                { MobID::kMechaSpider, 90000 }, { MobID::kMechaCrab, 70000 },
                { MobID::kAssembler, 8000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 3, .color = 0xff3A4550, .name = "Factory Deep"
        },
        {
            .left = 2200, .top = 2200, .right = 15800, .bottom = 15800,
            .density = 2.8f, .drop_multiplier = 0.30,
            .spawns = {
                { MobID::kMechaSpider, 120000 }, { MobID::kMechaFlower, 100000 },
                { MobID::kBarrel, 90000 }, { MobID::kMechaCrab, 60000 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 2, .color = 0xff4A5560, .name = "Factory Mid"
        },
        {
            .left = 4800, .top = 4800, .right = 13200, .bottom = 13200,
            .density = 2.7f, .drop_multiplier = 0.10,
            .spawns = {
                { MobID::kBarrel, 160000 }, { MobID::kMechaSpider, 120000 },
                { MobID::kScrapHeap, 100000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 1, .color = 0xff5A6570, .name = "Factory Edge"
        },
        {
            .left = 7200, .top = 7200, .right = 10800, .bottom = 10800,
            .density = 2.55f, .drop_multiplier = 0.02,
            .spawns = {
                { MobID::kBarrel, 200000 }, { MobID::kScrapHeap, 140000 },
                { MobID::kCogBeetle, 80000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 0, .color = MapTheme::FACTORY_GROUND, .name = "Factory Hub"
        }
    }),
    // ---- Pyramid (L3) ----
    MapZones({
        {
            .left = 0, .top = 0, .right = (float)ARENA_WIDTH, .bottom = (float)ARENA_HEIGHT,
            .density = 3.3f, .drop_multiplier = 0.58,
            .spawns = {
                { MobID::kPharaohBeetle, 50000 }, { MobID::kMummyBeetle, 100000 },
                { MobID::kTombGuard, 90000 }, { MobID::kTomb, 20000 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 3, .color = 0xff8A6A3A, .name = "Pyramid Deep"
        },
        {
            .left = 2200, .top = 2200, .right = 15800, .bottom = 15800,
            .density = 2.8f, .drop_multiplier = 0.30,
            .spawns = {
                { MobID::kMummyBeetle, 110000 }, { MobID::kScarab, 120000 },
                { MobID::kTombGuard, 80000 }, { MobID::kTomb, 25000 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 2, .color = 0xffB08A4A, .name = "Pyramid Mid"
        },
        {
            .left = 4800, .top = 4800, .right = 13200, .bottom = 13200,
            .density = 2.7f, .drop_multiplier = 0.10,
            .spawns = {
                { MobID::kScarab, 140000 }, { MobID::kTomb, 40000 },
                { MobID::kDuneStone, 100000 }, { MobID::kSandMite, 80000 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 1, .color = 0xffD0A85A, .name = "Pyramid Edge"
        },
        {
            .left = 7200, .top = 7200, .right = 10800, .bottom = 10800,
            .density = 2.55f, .drop_multiplier = 0.02,
            .spawns = {
                { MobID::kScarab, 160000 }, { MobID::kTomb, 50000 },
                { MobID::kDuneStone, 140000 }, { MobID::kSandMite, 100000 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 0, .color = MapTheme::PYRAMID_GROUND, .name = "Pyramid Hub"
        }
    }),
    // ---- Hel (special L2 — Mark death) ----
    MapZones({
        {
            .left = 0, .top = 0, .right = (float)ARENA_WIDTH, .bottom = (float)ARENA_HEIGHT,
            .density = 3.4f, .drop_multiplier = 0.55,
            .spawns = {
                { MobID::kHelBeetle, 120000 }, { MobID::kHelSpider, 110000 },
                { MobID::kHelWasp, 90000 }, { MobID::kHelCentipede, 40000 },
                { MobID::kGambler, 30000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 3, .color = 0xff2A1018, .name = "Hel Deep"
        },
        {
            .left = 2200, .top = 2200, .right = 15800, .bottom = 15800,
            .density = 2.9f, .drop_multiplier = 0.28,
            .spawns = {
                { MobID::kHelSpider, 130000 }, { MobID::kHelBeetle, 100000 },
                { MobID::kHelWasp, 80000 }, { MobID::kGambler, 40000 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 2, .color = 0xff3A1820, .name = "Hel Mid"
        },
        {
            .left = 4800, .top = 4800, .right = 13200, .bottom = 13200,
            .density = 2.7f, .drop_multiplier = 0.10,
            .spawns = {
                { MobID::kHelBeetle, 140000 }, { MobID::kHelSpider, 120000 },
                { MobID::kGambler, 50000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 1, .color = 0xff4A2030, .name = "Hel Edge"
        },
        {
            .left = 7200, .top = 7200, .right = 10800, .bottom = 10800,
            .density = 2.55f, .drop_multiplier = 0.02,
            .spawns = {
                { MobID::kHelBeetle, 160000 }, { MobID::kHelSpider, 120000 },
                { MobID::kGambler, 60000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 0, .color = MapTheme::HEL_GROUND, .name = "Hel Hub"
        }
    })
});

// Implemented in MapGen.cc �?deterministic per-map zones + impassable terrain.
MapZones const &map_zones(uint8_t map_id);
MapTerrain const &map_terrain(uint8_t map_id);

// Field portals (Slice D) �?config table, not client-hardcoded destinations.
struct MapPortalDef {
    uint8_t from_map;
    uint8_t to_map;
    float x;
    float y;
    float radius;
    float dest_x;
    float dest_y;
};

// Portal pad offset from hub. L2/L3 exit gates sit HERE (not map edges) so
// florr tmj walls never bury them — transfer only, layouts stay untouched.
inline float const PORTAL_HUB_OFF = 560.f;

inline std::array const MAP_PORTALS = std::to_array<MapPortalDef>({
    // ---- L1 edge gates (ashflor own layouts; edges are walkable) ----
    // Garden east → Royal Capital
    { MapID::kGarden, MapID::kRoyalCapital,
      ARENA_WIDTH * 0.88f, ARENA_HEIGHT * 0.50f, 220.f,
      MAP_SPAWN_X, MAP_SPAWN_Y },
    // Garden north → Prime Garden (L2 florr hub)
    { MapID::kGarden, MapID::kPrimeGarden,
      ARENA_WIDTH * 0.50f, ARENA_HEIGHT * 0.18f, 220.f,
      10638.f, 30151.f },
    // Garden south → Sewers (L2 florr hub)
    { MapID::kGarden, MapID::kSewers,
      ARENA_WIDTH * 0.50f, ARENA_HEIGHT * 0.86f, 220.f,
      2433.f, 29233.f },
    // Garden NE → Ant Nest (L2)
    { MapID::kGarden, MapID::kAntNest,
      ARENA_WIDTH * 0.82f, ARENA_HEIGHT * 0.22f, 220.f,
      MAP_SPAWN_X, MAP_SPAWN_Y },
    // Garden SE → Ant Hell (L2 florr hub)
    { MapID::kGarden, MapID::kAntHell,
      ARENA_WIDTH * 0.82f, ARENA_HEIGHT * 0.78f, 220.f,
      15616.f, 14272.f },
    // Desert west → Royal Capital
    { MapID::kDesert, MapID::kRoyalCapital,
      ARENA_WIDTH * 0.12f, ARENA_HEIGHT * 0.50f, 220.f,
      MAP_SPAWN_X, MAP_SPAWN_Y },
    // Desert east → Prime Desert (L2 florr hub)
    { MapID::kDesert, MapID::kPrimeDesert,
      ARENA_WIDTH * 0.88f, ARENA_HEIGHT * 0.50f, 220.f,
      2176.f, 1081.f },
    // Ocean west → Royal Capital
    { MapID::kOcean, MapID::kRoyalCapital,
      ARENA_WIDTH * 0.12f, ARENA_HEIGHT * 0.50f, 220.f,
      MAP_SPAWN_X, MAP_SPAWN_Y },
    // Ocean east → Prime Ocean (L2 florr hub)
    { MapID::kOcean, MapID::kPrimeOcean,
      ARENA_WIDTH * 0.88f, ARENA_HEIGHT * 0.50f, 220.f,
      13489.f, 12889.f },
    // Jungle west → Wildwood (L2 florr hub)
    { MapID::kJungle, MapID::kWildwood,
      ARENA_WIDTH * 0.12f, ARENA_HEIGHT * 0.50f, 220.f,
      2183.f, 1942.f },
    // Royal gates
    { MapID::kRoyalCapital, MapID::kGarden,
      ARENA_WIDTH * 0.10f, ARENA_HEIGHT * 0.50f, 240.f,
      MAP_SPAWN_X, MAP_SPAWN_Y },
    { MapID::kRoyalCapital, MapID::kDesert,
      ARENA_WIDTH * 0.90f, ARENA_HEIGHT * 0.50f, 240.f,
      MAP_SPAWN_X, MAP_SPAWN_Y },
    { MapID::kRoyalCapital, MapID::kOcean,
      ARENA_WIDTH * 0.50f, ARENA_HEIGHT * 0.90f, 240.f,
      MAP_SPAWN_X, MAP_SPAWN_Y },
    { MapID::kRoyalCapital, MapID::kVolcano,
      ARENA_WIDTH * 0.50f, ARENA_HEIGHT * 0.10f, 240.f,
      MAP_SPAWN_X, MAP_SPAWN_Y },
    { MapID::kVolcano, MapID::kRoyalCapital,
      ARENA_WIDTH * 0.50f, ARENA_HEIGHT * 0.90f, 240.f,
      MAP_SPAWN_X, MAP_SPAWN_Y },

    // ---- L2/L3 hub gates (coords = florr player_spawns ± PORTAL_HUB_OFF) ----
    // Ant Nest (ashflor layout — arena hub)
    { MapID::kAntNest, MapID::kGarden,
      MAP_SPAWN_X - PORTAL_HUB_OFF, MAP_SPAWN_Y, 240.f,
      MAP_SPAWN_X, MAP_SPAWN_Y },
    // Ant Hell hub 15616,14272 → Garden
    { MapID::kAntHell, MapID::kGarden,
      15056.f, 14272.f, 240.f,
      MAP_SPAWN_X, MAP_SPAWN_Y },
    // Wildwood hub 2183,1942 → Jungle
    { MapID::kWildwood, MapID::kJungle,
      2743.f, 1942.f, 240.f,
      MAP_SPAWN_X, MAP_SPAWN_Y },
    // Sewers hub 2433,29233 → Garden
    { MapID::kSewers, MapID::kGarden,
      2433.f, 28673.f, 240.f,
      MAP_SPAWN_X, MAP_SPAWN_Y },
    // Prime Garden hub 10638,30151 → Garden (south)
    { MapID::kPrimeGarden, MapID::kGarden,
      10638.f, 30711.f, 240.f,
      MAP_SPAWN_X, MAP_SPAWN_Y },
    // Prime Desert hub 2176,1081 → Desert
    { MapID::kPrimeDesert, MapID::kDesert,
      1616.f, 1081.f, 240.f,
      MAP_SPAWN_X, MAP_SPAWN_Y },
    // Prime Ocean hub 13489,12889 → Ocean
    { MapID::kPrimeOcean, MapID::kOcean,
      12929.f, 12889.f, 240.f,
      MAP_SPAWN_X, MAP_SPAWN_Y },
    // L3 from Prime Garden hub
    { MapID::kPrimeGarden, MapID::kFactory,
      10078.f, 30151.f, 220.f,
      MAP_SPAWN_X, MAP_SPAWN_Y },
    { MapID::kPrimeGarden, MapID::kPyramid,
      11198.f, 30151.f, 220.f,
      MAP_SPAWN_X, MAP_SPAWN_Y },
    // L3 returns → Prime Garden hub
    { MapID::kFactory, MapID::kPrimeGarden,
      MAP_SPAWN_X + PORTAL_HUB_OFF, MAP_SPAWN_Y, 240.f,
      10638.f, 30151.f },
    { MapID::kPyramid, MapID::kPrimeGarden,
      MAP_SPAWN_X - PORTAL_HUB_OFF, MAP_SPAWN_Y, 240.f,
      10638.f, 30151.f },
    // Hel exit hub 1329,1964
    { MapID::kHel, MapID::kGarden,
      1329.f, 1404.f, 240.f,
      MAP_SPAWN_X, MAP_SPAWN_Y },
});

std::array const ANTHOLE_SPAWNS = std::to_array<StaticArray<MobID::T, 3>>({
    {MobID::kBabyAnt},
    {MobID::kWorkerAnt,MobID::kBabyAnt},
    {MobID::kWorkerAnt,MobID::kWorkerAnt},
    {MobID::kSoldierAnt,MobID::kWorkerAnt},
    {MobID::kBabyAnt,MobID::kWorkerAnt,MobID::kSoldierAnt},
    {MobID::kWorkerAnt,MobID::kSoldierAnt},
    {MobID::kSoldierAnt,MobID::kWorkerAnt,MobID::kWorkerAnt},
    {MobID::kSoldierAnt,MobID::kSoldierAnt},
    {MobID::kQueenAnt},
    {MobID::kSoldierAnt,MobID::kSoldierAnt},
    {MobID::kSoldierAnt,MobID::kSoldierAnt,MobID::kSoldierAnt}
});

extern std::array<StaticArray<float, MAX_DROPS_PER_MOB>, MobID::kNumMobs> const MOB_DROP_CHANCES;

// Band-only base shares vs the mob's live rarity. Normals: same 20% / −1 60% /
// −2 20% (renormalized among bands present). Chase specials (Moon / Ygg /
// Golden Leaf): flat 10% each. Seeds >2 below mob rarity get 0 here — live
// loot uses Craft::mob_loot_weights (0.15 floor + tilt) instead.
// Writes drops.size() entries; sums ≈ 1.
void compute_mob_drop_weights(MobID::T mob_id, uint8_t mob_rarity,
                              float out[MAX_DROPS_PER_MOB]);
uint8_t is_chase_loot_petal(PetalID::T id);

extern uint32_t score_to_pass_level(uint32_t);
extern uint32_t score_to_level(uint32_t);
extern uint32_t level_to_score(uint32_t);
// Soft XP ceiling (level_to_score(300)). Stops overflow pinning UINT32_MAX as #1.
extern uint32_t score_soft_cap();
extern uint32_t loadout_slots_at_level(uint32_t);

extern float hp_at_level(uint32_t);
extern float body_damage_at_level(uint32_t);

// Map-anchor bosses (one per biome). Used for screen boss HP bars, etc.
inline uint8_t is_boss_mob(MobID::T id) {
    switch (id) {
        case MobID::kFlowerGuardian:
        case MobID::kSandwormMonarch:
        case MobID::kAbyssMaw:
        case MobID::kWinterSovereign:
        case MobID::kAncientTreant:
        case MobID::kQueenAnt:
        case MobID::kMoltenOverlord:
        case MobID::kFallenKing:
            return 1;
        default:
            return 0;
    }
}
