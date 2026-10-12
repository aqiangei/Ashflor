#pragma once

#include <Shared/StaticDefinitions.hh>

#include <array>
#include <cstdint>

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

extern std::array<struct PetalData, PetalID::kNumPetals> const PETAL_DATA;
extern std::array<struct MobData, MobID::kNumMobs> const MOB_DATA;

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
            .difficulty = 3, .color = 0xff6c757d, .name = "Polar"
        },
        {
            .left = 2200, .top = 2200, .right = 15800, .bottom = 15800,
            .density = 2.5f, .drop_multiplier = 0.06,
            .spawns = {
                { MobID::kDarkLadybug, 130000 }, { MobID::kHornet, 120000 }, { MobID::kEvilCentipede, 20000 },
                { MobID::kMassiveBeetle, 2600 }, { MobID::kSquare, 1 }
            },
            .difficulty = 2, .color = 0xff868e96, .name = "Polar"
        },
        {
            .left = 4800, .top = 4800, .right = 13200, .bottom = 13200,
            .density = 2.2f, .drop_multiplier = 0.12,
            .spawns = {
                { MobID::kBoulder, 160000 }, { MobID::kSpider, 80000 }, { MobID::kBee, 70000 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 1, .color = 0xff9aa3ab, .name = "Polar"
        },
        {
            .left = 7200, .top = 7200, .right = 10800, .bottom = 10800,
            .density = 0.2f, .drop_multiplier = 0.25,
            .spawns = {
                { MobID::kRock, 600000 }, { MobID::kLadybug, 200000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 0, .color = MapTheme::POLAR_GROUND, .name = "Polar"
        }
    })
});

// Implemented in MapGen.cc — deterministic per-map zones + impassable terrain.
MapZones const &map_zones(uint8_t map_id);
MapTerrain const &map_terrain(uint8_t map_id);

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
