#pragma once

#include <Shared/StaticDefinitions.hh>

#include <array>
#include <cstdint>
#include <cstring>

extern uint32_t const MAX_LEVEL;
extern uint32_t const TPS;

extern float const PETAL_DISABLE_DELAY;
extern float const PLAYER_ACCELERATION;
extern float const DEFAULT_FRICTION;
extern float const SUMMON_RETREAT_RADIUS;
extern float const DIGGER_SPAWN_CHANCE;

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

inline uint8_t petal_name_is(PetalID::T id, char const *name) {
    return id != PetalID::kNone && id < PetalID::kNumPetals
        && std::strcmp(PETAL_DATA[id].name, name) == 0;
}

using MapZones = StaticArray<struct ZoneDefinition, MAX_ZONES_PER_MAP>;
using MapTerrain = StaticArray<struct TerrainFeature, MAX_TERRAIN_PER_MAP>;

// Spawn / difficulty templates. Geometry rewritten by MapGen (seeded per map_id).
// Outer → harder. Difficulty 0 hub is fixed spawn for new players.
inline std::array const MAPS = std::to_array<MapZones>({
    // ---- Garden ----
    MapZones({
        {
            .left = 0, .top = 0, .right = (float)ARENA_WIDTH, .bottom = (float)ARENA_HEIGHT,
            .density = 2.6f, .drop_multiplier = 0.05,
            .spawns = {
                { MobID::kDarkLadybug, 120000 }, { MobID::kHornet, 120000 }, { MobID::kSpider, 100000 },
                { MobID::kEvilCentipede, 18000 }, { MobID::kMassiveBeetle, 2800 }, { MobID::kAntHole, 2500 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 3, .color = 0xff246b3d, .name = "Garden"
        },
        {
            .left = 2200, .top = 2200, .right = 15800, .bottom = 15800,
            .density = 2.5f, .drop_multiplier = 0.12,
            .spawns = {
                { MobID::kSpider, 100000 }, { MobID::kHornet, 100000 }, { MobID::kBeetle, 80000 },
                { MobID::kEvilCentipede, 12000 }, { MobID::kMassiveBeetle, 2000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 2, .color = 0xff2d8a50, .name = "Garden"
        },
        {
            .left = 4800, .top = 4800, .right = 13200, .bottom = 13200,
            .density = 2.2f, .drop_multiplier = 0.22,
            .spawns = {
                { MobID::kBee, 100000 }, { MobID::kLadybug, 90000 }, { MobID::kBeetle, 60000 },
                { MobID::kCentipede, 15000 }, { MobID::kAntHole, 2000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 1, .color = 0xff349f5c, .name = "Garden"
        },
        {
            // Spawn hub — sparse passive rocks + ladybugs for a soft buffer
            .left = 7200, .top = 7200, .right = 10800, .bottom = 10800,
            .density = 0.2f, .drop_multiplier = 0.35,
            .spawns = {
                { MobID::kRock, 600000 }, { MobID::kLadybug, 200000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 0, .color = MapTheme::GARDEN_GROUND, .name = "Garden"
        }
    }),
    // ---- Desert ----
    MapZones({
        {
            .left = 0, .top = 0, .right = (float)ARENA_WIDTH, .bottom = (float)ARENA_HEIGHT,
            .density = 2.6f, .drop_multiplier = 0.04,
            .spawns = {
                { MobID::kScorpion, 140000 }, { MobID::kBeetle, 120000 }, { MobID::kSandstorm, 100000 },
                { MobID::kDesertCentipede, 22000 }, { MobID::kMassiveBeetle, 3000 }, { MobID::kAntHole, 2800 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 3, .color = 0xffb57a2a, .name = "Desert"
        },
        {
            .left = 2200, .top = 2200, .right = 15800, .bottom = 15800,
            .density = 2.5f, .drop_multiplier = 0.1,
            .spawns = {
                { MobID::kBeetle, 120000 }, { MobID::kScorpion, 100000 }, { MobID::kSandstorm, 90000 },
                { MobID::kDesertCentipede, 20000 }, { MobID::kMassiveBeetle, 2200 }, { MobID::kSquare, 1 }
            },
            .difficulty = 2, .color = 0xffc9923c, .name = "Desert"
        },
        {
            .left = 4800, .top = 4800, .right = 13200, .bottom = 13200,
            .density = 2.2f, .drop_multiplier = 0.18,
            .spawns = {
                { MobID::kCactus, 280000 }, { MobID::kBeetle, 80000 }, { MobID::kSandstorm, 50000 },
                { MobID::kLadybug, 40000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 1, .color = 0xffd4a84b, .name = "Desert"
        },
        {
            .left = 7200, .top = 7200, .right = 10800, .bottom = 10800,
            .density = 0.2f, .drop_multiplier = 0.3,
            .spawns = {
                { MobID::kRock, 550000 }, { MobID::kLadybug, 250000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 0, .color = MapTheme::DESERT_GROUND, .name = "Desert"
        }
    }),
    // ---- Ocean ----
    MapZones({
        {
            .left = 0, .top = 0, .right = (float)ARENA_WIDTH, .bottom = (float)ARENA_HEIGHT,
            .density = 2.6f, .drop_multiplier = 0.035,
            .spawns = {
                { MobID::kHornet, 140000 }, { MobID::kEvilCentipede, 25000 }, { MobID::kMassiveBeetle, 3200 },
                { MobID::kBoulder, 120000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 3, .color = 0xff023e8a, .name = "Ocean"
        },
        {
            .left = 2200, .top = 2200, .right = 15800, .bottom = 15800,
            .density = 2.5f, .drop_multiplier = 0.08,
            .spawns = {
                { MobID::kHornet, 120000 }, { MobID::kBeetle, 100000 }, { MobID::kEvilCentipede, 18000 },
                { MobID::kMassiveBeetle, 2400 }, { MobID::kSquare, 1 }
            },
            .difficulty = 2, .color = 0xff005582, .name = "Ocean"
        },
        {
            .left = 4800, .top = 4800, .right = 13200, .bottom = 13200,
            .density = 2.2f, .drop_multiplier = 0.14,
            .spawns = {
                { MobID::kRock, 160000 }, { MobID::kBoulder, 120000 }, { MobID::kBee, 70000 },
                { MobID::kSandstorm, 60000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 1, .color = 0xff00669c, .name = "Ocean"
        },
        {
            .left = 7200, .top = 7200, .right = 10800, .bottom = 10800,
            .density = 0.2f, .drop_multiplier = 0.28,
            .spawns = {
                { MobID::kRock, 600000 }, { MobID::kLadybug, 200000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 0, .color = MapTheme::OCEAN_GROUND, .name = "Ocean"
        }
    }),
    // ---- Polar ----
    MapZones({
        {
            .left = 0, .top = 0, .right = (float)ARENA_WIDTH, .bottom = (float)ARENA_HEIGHT,
            .density = 2.6f, .drop_multiplier = 0.03,
            .spawns = {
                { MobID::kDarkLadybug, 140000 }, { MobID::kSpider, 130000 }, { MobID::kHornet, 120000 },
                { MobID::kEvilCentipede, 25000 }, { MobID::kMassiveBeetle, 3200 }, { MobID::kAntHole, 2800 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 3, .color = 0xff6c757d, .name = "Snow"
        },
        {
            .left = 2200, .top = 2200, .right = 15800, .bottom = 15800,
            .density = 2.5f, .drop_multiplier = 0.06,
            .spawns = {
                { MobID::kDarkLadybug, 130000 }, { MobID::kHornet, 120000 }, { MobID::kEvilCentipede, 20000 },
                { MobID::kMassiveBeetle, 2600 }, { MobID::kSquare, 1 }
            },
            .difficulty = 2, .color = 0xff868e96, .name = "Snow"
        },
        {
            .left = 4800, .top = 4800, .right = 13200, .bottom = 13200,
            .density = 2.2f, .drop_multiplier = 0.12,
            .spawns = {
                { MobID::kBoulder, 160000 }, { MobID::kSpider, 80000 }, { MobID::kBee, 70000 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 1, .color = 0xff9aa3ab, .name = "Snow"
        },
        {
            .left = 7200, .top = 7200, .right = 10800, .bottom = 10800,
            .density = 0.2f, .drop_multiplier = 0.25,
            .spawns = {
                { MobID::kRock, 600000 }, { MobID::kLadybug, 200000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 0, .color = MapTheme::POLAR_GROUND, .name = "Snow"
        }
    }),
    // ---- Jungle ----
    // Identity vs Garden: spider/centipede/hornet heavy → Web/Stinger/Iris/Leaf gear
    MapZones({
        {
            .left = 0, .top = 0, .right = (float)ARENA_WIDTH, .bottom = (float)ARENA_HEIGHT,
            .density = 2.8f, .drop_multiplier = 0.035,
            .spawns = {
                { MobID::kSpider, 160000 }, { MobID::kHornet, 130000 }, { MobID::kEvilCentipede, 28000 },
                { MobID::kDarkLadybug, 90000 }, { MobID::kMassiveBeetle, 3200 },
                { MobID::kQueenAnt, 1800 }, { MobID::kAntHole, 3500 }, { MobID::kSquare, 1 }
            },
            .difficulty = 3, .color = 0xff1b4332, .name = "Jungle Deep"
        },
        {
            .left = 2200, .top = 2200, .right = 15800, .bottom = 15800,
            .density = 2.6f, .drop_multiplier = 0.08,
            .spawns = {
                { MobID::kSpider, 130000 }, { MobID::kHornet, 110000 }, { MobID::kBeetle, 70000 },
                { MobID::kCentipede, 22000 }, { MobID::kEvilCentipede, 12000 },
                { MobID::kAntHole, 2800 }, { MobID::kSquare, 1 }
            },
            .difficulty = 2, .color = 0xff2d6a4f, .name = "Jungle Mid"
        },
        {
            .left = 4800, .top = 4800, .right = 13200, .bottom = 13200,
            .density = 2.3f, .drop_multiplier = 0.15,
            .spawns = {
                { MobID::kBee, 120000 }, { MobID::kLadybug, 90000 }, { MobID::kSpider, 70000 },
                { MobID::kCentipede, 18000 }, { MobID::kBeetle, 50000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 1, .color = 0xff40916c, .name = "Jungle Edge"
        },
        {
            .left = 7200, .top = 7200, .right = 10800, .bottom = 10800,
            .density = 0.22f, .drop_multiplier = 0.28,
            .spawns = {
                { MobID::kRock, 450000 }, { MobID::kLadybug, 350000 }, { MobID::kBee, 120000 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 0, .color = MapTheme::JUNGLE_GROUND, .name = "Jungle Hub"
        }
    }),
    // ---- Ant Hell ----
    MapZones({
        {
            .left = 0, .top = 0, .right = (float)ARENA_WIDTH, .bottom = (float)ARENA_HEIGHT,
            .density = 2.8f, .drop_multiplier = 0.05,
            .spawns = {
                { MobID::kSoldierAnt, 160000 }, { MobID::kWorkerAnt, 140000 }, { MobID::kAntHole, 8000 },
                { MobID::kMassiveBeetle, 2800 }, { MobID::kSquare, 1 }
            },
            .difficulty = 3, .color = 0xff582f0e, .name = "Ant Hell"
        },
        {
            .left = 2200, .top = 2200, .right = 15800, .bottom = 15800,
            .density = 2.6f, .drop_multiplier = 0.1,
            .spawns = {
                { MobID::kWorkerAnt, 150000 }, { MobID::kSoldierAnt, 120000 }, { MobID::kBabyAnt, 100000 },
                { MobID::kAntHole, 5000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 2, .color = 0xff6f4518, .name = "Ant Hell"
        },
        {
            .left = 4800, .top = 4800, .right = 13200, .bottom = 13200,
            .density = 2.3f, .drop_multiplier = 0.18,
            .spawns = {
                { MobID::kBabyAnt, 180000 }, { MobID::kWorkerAnt, 120000 }, { MobID::kAntHole, 3000 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 1, .color = 0xffa98467, .name = "Ant Hell"
        },
        {
            .left = 7200, .top = 7200, .right = 10800, .bottom = 10800,
            .density = 0.25f, .drop_multiplier = 0.28,
            .spawns = {
                { MobID::kBabyAnt, 400000 }, { MobID::kRock, 400000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 0, .color = MapTheme::ANT_GROUND, .name = "Ant Hell"
        }
    }),
    // ---- Volcano ----
    MapZones({
        {
            .left = 0, .top = 0, .right = (float)ARENA_WIDTH, .bottom = (float)ARENA_HEIGHT,
            .density = 2.6f, .drop_multiplier = 0.035,
            .spawns = {
                { MobID::kSoldierAnt, 140000 }, { MobID::kBeetle, 100000 }, { MobID::kHornet, 90000 },
                { MobID::kMassiveBeetle, 3200 }, { MobID::kSquare, 1 }
            },
            .difficulty = 3, .color = 0xff1a1a2e, .name = "Volcano"
        },
        {
            .left = 2200, .top = 2200, .right = 15800, .bottom = 15800,
            .density = 2.5f, .drop_multiplier = 0.08,
            .spawns = {
                { MobID::kSoldierAnt, 120000 }, { MobID::kBeetle, 100000 }, { MobID::kSpider, 80000 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 2, .color = 0xff3d405b, .name = "Volcano"
        },
        {
            .left = 4800, .top = 4800, .right = 13200, .bottom = 13200,
            .density = 2.2f, .drop_multiplier = 0.14,
            .spawns = {
                { MobID::kRock, 160000 }, { MobID::kBoulder, 120000 }, { MobID::kBeetle, 80000 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 1, .color = 0xff4a4e69, .name = "Volcano"
        },
        {
            .left = 7200, .top = 7200, .right = 10800, .bottom = 10800,
            .density = 0.2f, .drop_multiplier = 0.26,
            .spawns = {
                { MobID::kRock, 550000 }, { MobID::kLadybug, 250000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 0, .color = MapTheme::VOLCANO_GROUND, .name = "Volcano"
        }
    }),
    // ---- Royal Capital (exactly 6 zones; geometry rewritten by build_zones_royal) ----
    MapZones({
        {
            // 0 Outer Wall — spawn-safe base (covers arena; later zones overlay)
            .left = 0, .top = 0, .right = (float)ARENA_WIDTH, .bottom = (float)ARENA_HEIGHT,
            .density = 0.35f, .drop_multiplier = 0.30,
            .spawns = {
                { MobID::kWanderingGuard, 450000 }, { MobID::kStoneBeetle, 300000 },
                { MobID::kRustMite, 250000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 0, .color = MapTheme::ROYAL_GROUND, .name = "Outer Wall"
        },
        {
            // 1 Common Quarter — west / south
            .left = 800, .top = 7500, .right = 12500, .bottom = 19200,
            .density = 2.3f, .drop_multiplier = 0.18,
            .spawns = {
                { MobID::kRustMite, 300000 }, { MobID::kMimicChest, 250000 },
                { MobID::kCoinBeetle, 200000 }, { MobID::kWanderingGuard, 250000 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 1, .color = 0xffa8a194, .name = "Common Quarter"
        },
        {
            // 2 Royal Garden — east
            .left = 15500, .top = 3500, .right = 27200, .bottom = 16500,
            .density = 2.4f, .drop_multiplier = 0.12,
            .spawns = {
                { MobID::kRoyalBee, 450000 }, { MobID::kThornBug, 350000 },
                { MobID::kCoinBeetle, 200000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 2, .color = 0xff5a9e6a, .name = "Royal Garden"
        },
        {
            // 3 Barracks — north
            .left = 5000, .top = 400, .right = 23000, .bottom = 7200,
            .density = 2.5f, .drop_multiplier = 0.10,
            .spawns = {
                { MobID::kWanderingGuard, 200000 }, { MobID::kShieldSentinel, 350000 },
                { MobID::kSpearSentinel, 300000 }, { MobID::kStoneBeetle, 150000 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 2, .color = 0xff6d7580, .name = "Barracks"
        },
        {
            // 4 Ruined Quarter — northwest
            .left = 600, .top = 800, .right = 10500, .bottom = 9500,
            .density = 2.6f, .drop_multiplier = 0.06,
            .spawns = {
                { MobID::kPhantomOrb, 400000 }, { MobID::kFallenKnight, 250000 },
                { MobID::kMimicChest, 200000 }, { MobID::kRustMite, 150000 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 3, .color = 0xff5a4a68, .name = "Ruined Quarter"
        },
        {
            // 5 Royal Palace — NE of center (leave spawn plaza at MAP_SPAWN clear)
            .left = 14500, .top = 4500, .right = 24500, .bottom = 11500,
            .density = 2.7f, .drop_multiplier = 0.04,
            .spawns = {
                { MobID::kShieldSentinel, 350000 }, { MobID::kSpearSentinel, 300000 },
                { MobID::kFallenKnight, 250000 }, { MobID::kWanderingGuard, 100000 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 3, .color = 0xffc4a35a, .name = "Royal Palace"
        }
    })
});

// Implemented in MapGen.cc — deterministic per-map zones + impassable terrain.
MapZones const &map_zones(uint8_t map_id);
MapTerrain const &map_terrain(uint8_t map_id);

// Field portals (Slice D) — config table, not client-hardcoded destinations.
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
    // Garden east → Royal Capital outer plaza (center, away from royal side gates)
    { MapID::kGarden, MapID::kRoyalCapital,
      ARENA_WIDTH * 0.88f, ARENA_HEIGHT * 0.50f, 220.f,
      MAP_SPAWN_X, MAP_SPAWN_Y },
    // Desert west → Royal Capital plaza
    { MapID::kDesert, MapID::kRoyalCapital,
      ARENA_WIDTH * 0.12f, ARENA_HEIGHT * 0.50f, 220.f,
      MAP_SPAWN_X, MAP_SPAWN_Y },
    // Royal west gate → Garden hub (center, not near Garden east portal)
    { MapID::kRoyalCapital, MapID::kGarden,
      ARENA_WIDTH * 0.10f, ARENA_HEIGHT * 0.50f, 240.f,
      MAP_SPAWN_X, MAP_SPAWN_Y },
    // Royal east gate → Desert hub (center, not near Desert west portal)
    { MapID::kRoyalCapital, MapID::kDesert,
      ARENA_WIDTH * 0.90f, ARENA_HEIGHT * 0.50f, 240.f,
      MAP_SPAWN_X, MAP_SPAWN_Y },
    // Royal north → Volcano hub
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

extern uint32_t score_to_pass_level(uint32_t);
extern uint32_t score_to_level(uint32_t);
extern uint32_t level_to_score(uint32_t);
extern uint32_t loadout_slots_at_level(uint32_t);

extern float hp_at_level(uint32_t);
