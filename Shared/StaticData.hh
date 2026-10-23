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
inline constexpr float MOB_DENSITY_MULTIPLIER = 1.0f;

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

using MapZones = StaticArray<struct ZoneDefinition, MAX_ZONES_PER_MAP>;
using MapTerrain = StaticArray<struct TerrainFeature, MAX_TERRAIN_PER_MAP>;

// Spawn / difficulty templates. Geometry rewritten by MapGen (seeded per map_id).
// Outer �?harder. Difficulty 0 hub is fixed spawn for new players.
inline std::array const MAPS = std::to_array<MapZones>({
    // ---- Garden ----
    MapZones({
        {
            .left = 0, .top = 0, .right = (float)ARENA_WIDTH, .bottom = (float)ARENA_HEIGHT,
            .density = 3.2f, .drop_multiplier = 0.58,
            .spawns = {
                { MobID::kDarkLadybug, 120000 }, { MobID::kHornet, 120000 }, { MobID::kSpider, 100000 },
                { MobID::kEvilCentipede, 28000 }, { MobID::kMassiveBeetle, 12000 },
                { MobID::kBoulder, 9000 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 3, .color = 0xff8FC777, .name = "Garden"
        },
        {
            .left = 2200, .top = 2200, .right = 15800, .bottom = 15800,
            .density = 2.7f, .drop_multiplier = 0.30,
            .spawns = {
                { MobID::kSpider, 100000 }, { MobID::kHornet, 100000 }, { MobID::kBeetle, 80000 },
                { MobID::kEvilCentipede, 20000 }, { MobID::kMassiveBeetle, 8000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 2, .color = 0xffA8D484, .name = "Garden"
        },
        {
            .left = 4800, .top = 4800, .right = 13200, .bottom = 13200,
            .density = 2.65f, .drop_multiplier = 0.10,
            .spawns = {
                { MobID::kBee, 100000 }, { MobID::kLadybug, 90000 }, { MobID::kBeetle, 60000 },
                { MobID::kCentipede, 15000 }, { MobID::kShinyLadybug, 2000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 1, .color = MapTheme::GARDEN_GROUND, .name = "Garden"
        },
        {
            // Spawn hub �?rich low-tier mix (periphery packs).
            .left = 7200, .top = 7200, .right = 10800, .bottom = 10800,
            .density = 2.55f, .drop_multiplier = 0.02,
            .spawns = {
                { MobID::kRock, 220000 }, { MobID::kLadybug, 200000 },
                { MobID::kBee, 180000 }, { MobID::kBeetle, 140000 },
                { MobID::kCentipede, 50000 }, { MobID::kShinyLadybug, 8000 },
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
                { MobID::kCactus, 130000 }, { MobID::kDuneBeetle, 130000 },
                { MobID::kDuneSpider, 105000 }, { MobID::kDuneHornet, 95000 },
                { MobID::kScorpion, 85000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 2, .color = 0xffc9923c, .name = "Desert"
        },
        {
            .left = 4800, .top = 4800, .right = 13200, .bottom = 13200,
            .density = 2.65f, .drop_multiplier = 0.09,
            .spawns = {
                { MobID::kCactus, 130000 }, { MobID::kDuneBeetle, 130000 },
                { MobID::kSandMite, 155000 }, { MobID::kScorpion, 85000 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 1, .color = 0xffd4a84b, .name = "Desert"
        },
        {
            // Spawn hub �?rich low-tier desert pack.
            .left = 7200, .top = 7200, .right = 10800, .bottom = 10800,
            .density = 2.55f, .drop_multiplier = 0.02,
            .spawns = {
                { MobID::kDuneStone, 160000 }, { MobID::kSandMite, 180000 },
                { MobID::kCactus, 150000 }, { MobID::kDuneBeetle, 140000 },
                { MobID::kScorpion, 60000 }, { MobID::kSquare, 1 }
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
                { MobID::kMoonJelly, 110000 }, { MobID::kCoralStar, 120000 },
                { MobID::kReefCrab, 100000 }, { MobID::kSeaFry, 150000 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 1, .color = 0xff00669c, .name = "Ocean"
        },
        {
            // Spawn hub �?rich low-tier ocean pack.
            .left = 7200, .top = 7200, .right = 10800, .bottom = 10800,
            .density = 2.55f, .drop_multiplier = 0.02,
            .spawns = {
                { MobID::kReefStone, 140000 }, { MobID::kSeaFry, 160000 },
                { MobID::kMoonJelly, 130000 }, { MobID::kCoralStar, 130000 },
                { MobID::kReefCrab, 90000 }, { MobID::kSquare, 1 }
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
                { MobID::kLeafBeetle, 140000 }, { MobID::kJungleMite, 130000 },
                { MobID::kVineBug, 90000 }, { MobID::kJungleWasp, 90000 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 1, .color = 0xff40916c, .name = "Jungle Edge"
        },
        {
            // hub �?rich low-tier jungle; no wasps (aggro on sight)
            .left = 7200, .top = 7200, .right = 10800, .bottom = 10800,
            .density = 2.55f, .drop_multiplier = 0.02,
            .spawns = {
                { MobID::kJungleStone, 130000 }, { MobID::kJungleCrawler, 150000 },
                { MobID::kLeafBeetle, 140000 }, { MobID::kJungleMite, 140000 },
                { MobID::kVineBug, 70000 }, { MobID::kSquare, 1 }
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
                { MobID::kBabyAnt, 180000 }, { MobID::kWorkerAnt, 120000 },
                { MobID::kAntHole, 3000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 1, .color = 0xffa98467, .name = "Ant Hell"
        },
        {
            // Spawn hub �?rich low-tier ant pack.
            .left = 7200, .top = 7200, .right = 10800, .bottom = 10800,
            .density = 2.55f, .drop_multiplier = 0.02,
            .spawns = {
                { MobID::kBabyAnt, 200000 }, { MobID::kWorkerAnt, 160000 },
                { MobID::kAntHole, 40000 }, { MobID::kSoldierAnt, 50000 },
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
            // 0 Outer Wall �?rich low-tier capital pack
            .left = 0, .top = 0, .right = (float)ARENA_WIDTH, .bottom = (float)ARENA_HEIGHT,
            .density = 2.55f, .drop_multiplier = 0.02,
            .spawns = {
                { MobID::kRoyalBoulder, 140000 }, { MobID::kRoyalPebble, 150000 },
                { MobID::kRustMite, 160000 }, { MobID::kCoinBeetle, 120000 },
                { MobID::kMimicChest, 80000 }, { MobID::kWanderingGuard, 50000 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 0, .color = RARITY_COLORS[RarityID::kCommon], .name = "Outer Wall"
        },
        {
            // 1 Common Quarter �?Unusual band
            .left = 800, .top = 7500, .right = 12500, .bottom = 19200,
            .density = 3.50f, .drop_multiplier = 0.09,
            .spawns = {
                { MobID::kRustMite, 300000 }, { MobID::kMimicChest, 250000 },
                { MobID::kCoinBeetle, 200000 }, { MobID::kWanderingGuard, 250000 },
                { MobID::kSquare, 1 }
            },
            .difficulty = RarityID::kUnusual, .color = RARITY_COLORS[RarityID::kUnusual], .name = "Common Quarter"
        },
        {
            // 2 Royal Garden �?Rare
            .left = 15500, .top = 3500, .right = 27200, .bottom = 16500,
            .density = 2.4f, .drop_multiplier = 0.17,
            .spawns = {
                { MobID::kRoyalBee, 450000 }, { MobID::kThornBug, 350000 },
                { MobID::kCoinBeetle, 200000 }, { MobID::kSquare, 1 }
            },
            .difficulty = RarityID::kRare, .color = RARITY_COLORS[RarityID::kRare], .name = "Royal Garden"
        },
        {
            // 3 Barracks �?Epic
            .left = 5000, .top = 400, .right = 23000, .bottom = 7200,
            .density = 2.5f, .drop_multiplier = 0.19,
            .spawns = {
                { MobID::kWanderingGuard, 200000 }, { MobID::kShieldSentinel, 350000 },
                { MobID::kSpearSentinel, 300000 }, { MobID::kStoneBeetle, 150000 },
                { MobID::kSquare, 1 }
            },
            .difficulty = RarityID::kEpic, .color = RARITY_COLORS[RarityID::kEpic], .name = "Barracks"
        },
        {
            // 4 Ruined Quarter �?Legendary
            .left = 600, .top = 800, .right = 10500, .bottom = 9500,
            .density = 3.2f, .drop_multiplier = 0.55,
            .spawns = {
                { MobID::kPhantomOrb, 400000 }, { MobID::kFallenKnight, 250000 },
                { MobID::kMimicChest, 200000 }, { MobID::kRustMite, 150000 },
                { MobID::kSquare, 1 }
            },
            .difficulty = RarityID::kLegendary, .color = RARITY_COLORS[RarityID::kLegendary], .name = "Ruined Quarter"
        },
        {
            // 5 Royal Palace �?Mythic
            .left = 14500, .top = 4500, .right = 24500, .bottom = 11500,
            .density = 3.3f, .drop_multiplier = 0.58,
            .spawns = {
                { MobID::kShieldSentinel, 350000 }, { MobID::kSpearSentinel, 300000 },
                { MobID::kFallenKnight, 250000 }, { MobID::kWanderingGuard, 100000 },
                { MobID::kSquare, 1 }
            },
            .difficulty = RarityID::kMythic, .color = RARITY_COLORS[RarityID::kMythic], .name = "Royal Palace"
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

inline std::array const MAP_PORTALS = std::to_array<MapPortalDef>({
    // Garden east �?Royal Capital outer plaza (center, away from royal side gates)
    { MapID::kGarden, MapID::kRoyalCapital,
      ARENA_WIDTH * 0.88f, ARENA_HEIGHT * 0.50f, 220.f,
      MAP_SPAWN_X, MAP_SPAWN_Y },
    // Desert west �?Royal Capital plaza
    { MapID::kDesert, MapID::kRoyalCapital,
      ARENA_WIDTH * 0.12f, ARENA_HEIGHT * 0.50f, 220.f,
      MAP_SPAWN_X, MAP_SPAWN_Y },
    // Royal west gate �?Garden hub (center, not near Garden east portal)
    { MapID::kRoyalCapital, MapID::kGarden,
      ARENA_WIDTH * 0.10f, ARENA_HEIGHT * 0.50f, 240.f,
      MAP_SPAWN_X, MAP_SPAWN_Y },
    // Royal east gate �?Desert hub (center, not near Desert west portal)
    { MapID::kRoyalCapital, MapID::kDesert,
      ARENA_WIDTH * 0.90f, ARENA_HEIGHT * 0.50f, 240.f,
      MAP_SPAWN_X, MAP_SPAWN_Y },
    // Royal north �?Volcano hub
    { MapID::kRoyalCapital, MapID::kVolcano,
      ARENA_WIDTH * 0.50f, ARENA_HEIGHT * 0.10f, 240.f,
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

// Per-kill drop weights vs the mob's live rarity. Normals: same 20% / −1 60% /
// −2 20% (renormalized among bands present). Chase specials (Moon / Ygg /
// Golden Leaf): flat 10% each. Writes drops.size() entries; sums ≈ 1.
void compute_mob_drop_weights(MobID::T mob_id, uint8_t mob_rarity,
                              float out[MAX_DROPS_PER_MOB]);
uint8_t is_chase_loot_petal(PetalID::T id);

extern uint32_t score_to_pass_level(uint32_t);
extern uint32_t score_to_level(uint32_t);
extern uint32_t level_to_score(uint32_t);
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
