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

// Difficulty strips + high density. Colors from MapTheme (MAP_TERRAIN.md).
inline std::array const MAPS = std::to_array<MapZones>({
    MapZones({
        {
            .left = 0, .top = 0, .right = 10000, .bottom = (float)ARENA_HEIGHT,
            .density = 3.0f, .drop_multiplier = 0.3,
            .spawns = {
                { MobID::kRock, 500000 }, { MobID::kLadybug, 100000 }, { MobID::kBee, 100000 },
                { MobID::kBabyAnt, 25000 }, { MobID::kCentipede, 10000 }, { MobID::kBoulder, 10000 },
                { MobID::kMassiveLadybug, 200 }, { MobID::kSquare, 1 }
            },
            .difficulty = 0, .color = MapTheme::GARDEN_GROUND, .name = "Garden"
        },
        {
            .left = 10000, .top = 0, .right = 20000, .bottom = (float)ARENA_HEIGHT,
            .density = 3.0f, .drop_multiplier = 0.2,
            .spawns = {
                { MobID::kBee, 120000 }, { MobID::kLadybug, 100000 }, { MobID::kBeetle, 80000 },
                { MobID::kCentipede, 20000 }, { MobID::kAntHole, 3000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 1, .color = 0xff349f5c, .name = "Garden"
        },
        {
            .left = 20000, .top = 0, .right = 30000, .bottom = (float)ARENA_HEIGHT,
            .density = 2.9f, .drop_multiplier = 0.12,
            .spawns = {
                { MobID::kSpider, 100000 }, { MobID::kHornet, 100000 }, { MobID::kBeetle, 80000 },
                { MobID::kEvilCentipede, 12000 }, { MobID::kMassiveBeetle, 2000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 2, .color = 0xff2d8a50, .name = "Garden"
        },
        {
            .left = 30000, .top = 0, .right = (float)ARENA_WIDTH, .bottom = (float)ARENA_HEIGHT,
            .density = 2.8f, .drop_multiplier = 0.05,
            .spawns = {
                { MobID::kDarkLadybug, 120000 }, { MobID::kHornet, 120000 }, { MobID::kSpider, 100000 },
                { MobID::kEvilCentipede, 18000 }, { MobID::kMassiveBeetle, 2800 }, { MobID::kAntHole, 2500 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 3, .color = 0xff246b3d, .name = "Garden"
        }
    }),
    MapZones({
        {
            .left = 0, .top = 0, .right = 10000, .bottom = (float)ARENA_HEIGHT,
            .density = 3.0f, .drop_multiplier = 0.2,
            .spawns = {
                { MobID::kCactus, 400000 }, { MobID::kBeetle, 100000 }, { MobID::kSandstorm, 50000 },
                { MobID::kScorpion, 50000 }, { MobID::kLadybug, 40000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 0, .color = MapTheme::DESERT_GROUND, .name = "Desert"
        },
        {
            .left = 10000, .top = 0, .right = 20000, .bottom = (float)ARENA_HEIGHT,
            .density = 3.0f, .drop_multiplier = 0.15,
            .spawns = {
                { MobID::kCactus, 300000 }, { MobID::kSandstorm, 80000 }, { MobID::kScorpion, 80000 },
                { MobID::kDesertCentipede, 15000 }, { MobID::kAntHole, 2500 }, { MobID::kShinyLadybug, 800 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 1, .color = 0xffd4a84b, .name = "Desert"
        },
        {
            .left = 20000, .top = 0, .right = 30000, .bottom = (float)ARENA_HEIGHT,
            .density = 2.9f, .drop_multiplier = 0.1,
            .spawns = {
                { MobID::kBeetle, 120000 }, { MobID::kScorpion, 100000 }, { MobID::kSandstorm, 90000 },
                { MobID::kDesertCentipede, 20000 }, { MobID::kMassiveBeetle, 2200 }, { MobID::kSquare, 1 }
            },
            .difficulty = 2, .color = 0xffc9923c, .name = "Desert"
        },
        {
            .left = 30000, .top = 0, .right = (float)ARENA_WIDTH, .bottom = (float)ARENA_HEIGHT,
            .density = 2.8f, .drop_multiplier = 0.04,
            .spawns = {
                { MobID::kScorpion, 140000 }, { MobID::kBeetle, 120000 }, { MobID::kSandstorm, 100000 },
                { MobID::kDesertCentipede, 22000 }, { MobID::kMassiveBeetle, 3000 }, { MobID::kAntHole, 2800 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 3, .color = 0xffb57a2a, .name = "Desert"
        }
    }),
    MapZones({
        {
            .left = 0, .top = 0, .right = 10000, .bottom = (float)ARENA_HEIGHT,
            .density = 3.1f, .drop_multiplier = 0.15,
            .spawns = {
                { MobID::kRock, 200000 }, { MobID::kBoulder, 150000 }, { MobID::kSandstorm, 100000 },
                { MobID::kBee, 60000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 0, .color = MapTheme::OCEAN_GROUND, .name = "Ocean"
        },
        {
            .left = 10000, .top = 0, .right = 20000, .bottom = (float)ARENA_HEIGHT,
            .density = 3.0f, .drop_multiplier = 0.12,
            .spawns = {
                { MobID::kBoulder, 160000 }, { MobID::kHornet, 100000 }, { MobID::kBeetle, 90000 },
                { MobID::kSandstorm, 90000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 1, .color = 0xff00669c, .name = "Ocean"
        },
        {
            .left = 20000, .top = 0, .right = 30000, .bottom = (float)ARENA_HEIGHT,
            .density = 2.9f, .drop_multiplier = 0.08,
            .spawns = {
                { MobID::kHornet, 120000 }, { MobID::kBeetle, 100000 }, { MobID::kEvilCentipede, 18000 },
                { MobID::kMassiveBeetle, 2400 }, { MobID::kSquare, 1 }
            },
            .difficulty = 2, .color = 0xff005582, .name = "Ocean"
        },
        {
            .left = 30000, .top = 0, .right = (float)ARENA_WIDTH, .bottom = (float)ARENA_HEIGHT,
            .density = 2.85f, .drop_multiplier = 0.035,
            .spawns = {
                { MobID::kHornet, 140000 }, { MobID::kEvilCentipede, 25000 }, { MobID::kMassiveBeetle, 3200 },
                { MobID::kBoulder, 120000 }, { MobID::kSquare, 1 }
            },
            .difficulty = 3, .color = 0xff023e8a, .name = "Ocean"
        }
    }),
    MapZones({
        {
            .left = 0, .top = 0, .right = 10000, .bottom = (float)ARENA_HEIGHT,
            .density = 3.1f, .drop_multiplier = 0.12,
            .spawns = {
                { MobID::kBoulder, 180000 }, { MobID::kSpider, 100000 }, { MobID::kBee, 80000 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 0, .color = MapTheme::POLAR_GROUND, .name = "Polar"
        },
        {
            .left = 10000, .top = 0, .right = 20000, .bottom = (float)ARENA_HEIGHT,
            .density = 3.0f, .drop_multiplier = 0.1,
            .spawns = {
                { MobID::kSpider, 130000 }, { MobID::kDarkLadybug, 100000 }, { MobID::kHornet, 90000 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 1, .color = 0xff9aa3ab, .name = "Polar"
        },
        {
            .left = 20000, .top = 0, .right = 30000, .bottom = (float)ARENA_HEIGHT,
            .density = 2.9f, .drop_multiplier = 0.06,
            .spawns = {
                { MobID::kDarkLadybug, 130000 }, { MobID::kHornet, 120000 }, { MobID::kEvilCentipede, 20000 },
                { MobID::kMassiveBeetle, 2600 }, { MobID::kSquare, 1 }
            },
            .difficulty = 2, .color = 0xff868e96, .name = "Polar"
        },
        {
            .left = 30000, .top = 0, .right = (float)ARENA_WIDTH, .bottom = (float)ARENA_HEIGHT,
            .density = 2.85f, .drop_multiplier = 0.03,
            .spawns = {
                { MobID::kDarkLadybug, 140000 }, { MobID::kSpider, 130000 }, { MobID::kHornet, 120000 },
                { MobID::kEvilCentipede, 25000 }, { MobID::kMassiveBeetle, 3200 }, { MobID::kAntHole, 2800 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 3, .color = 0xff6c757d, .name = "Polar"
        }
    })
});

// Nature-inspired hills/water (impassable) + fast channels (walkable, no attack).
inline std::array const MAP_TERRAIN = std::to_array<MapTerrain>({
    // ---- Garden ----
    MapTerrain({
        // Hills (ellipses)
        { TerrainKind::kHill, TerrainShape::kEllipse, TerrainFlags::kImpassable, 2200, 900, 900, 620, MapTheme::GARDEN_HILL },
        { TerrainKind::kHill, TerrainShape::kEllipse, TerrainFlags::kImpassable, 4800, 3100, 1100, 700, MapTheme::GARDEN_HILL },
        { TerrainKind::kHill, TerrainShape::kEllipse, TerrainFlags::kImpassable, 7800, 700, 850, 550, MapTheme::GARDEN_HILL },
        { TerrainKind::kHill, TerrainShape::kEllipse, TerrainFlags::kImpassable, 12500, 2800, 1000, 750, MapTheme::GARDEN_HILL },
        { TerrainKind::kHill, TerrainShape::kEllipse, TerrainFlags::kImpassable, 16000, 900, 1200, 650, MapTheme::GARDEN_HILL },
        { TerrainKind::kHill, TerrainShape::kEllipse, TerrainFlags::kImpassable, 21000, 3200, 950, 600, MapTheme::GARDEN_HILL },
        { TerrainKind::kHill, TerrainShape::kEllipse, TerrainFlags::kImpassable, 24500, 1100, 1050, 700, MapTheme::GARDEN_HILL },
        { TerrainKind::kHill, TerrainShape::kEllipse, TerrainFlags::kImpassable, 29000, 2900, 1150, 680, MapTheme::GARDEN_HILL },
        { TerrainKind::kHill, TerrainShape::kEllipse, TerrainFlags::kImpassable, 34000, 800, 1000, 580, MapTheme::GARDEN_HILL },
        { TerrainKind::kHill, TerrainShape::kEllipse, TerrainFlags::kImpassable, 37500, 3000, 900, 650, MapTheme::GARDEN_HILL },
        // Creek segments
        { TerrainKind::kWater, TerrainShape::kAABB, TerrainFlags::kImpassable, 3500, 1700, 7200, 2050, MapTheme::GARDEN_WATER },
        { TerrainKind::kWater, TerrainShape::kAABB, TerrainFlags::kImpassable, 7000, 1900, 9800, 2300, MapTheme::GARDEN_WATER },
        { TerrainKind::kWater, TerrainShape::kEllipse, TerrainFlags::kImpassable, 10500, 2100, 700, 450, MapTheme::GARDEN_WATER },
        { TerrainKind::kWater, TerrainShape::kAABB, TerrainFlags::kImpassable, 18500, 1500, 23000, 1850, MapTheme::GARDEN_WATER },
        { TerrainKind::kWater, TerrainShape::kEllipse, TerrainFlags::kImpassable, 26800, 2000, 800, 500, MapTheme::GARDEN_WATER },
        // Leaf express channel Easy → ???
        { TerrainKind::kChannel, TerrainShape::kAABB, TerrainFlags::kNoAttack, 1500, 1850, 9000, 2150, MapTheme::GARDEN_CHANNEL },
        { TerrainKind::kChannel, TerrainShape::kAABB, TerrainFlags::kNoAttack, 8800, 1750, 16000, 2250, MapTheme::GARDEN_CHANNEL },
        { TerrainKind::kChannel, TerrainShape::kAABB, TerrainFlags::kNoAttack, 15800, 1650, 25000, 2150, MapTheme::GARDEN_CHANNEL },
        { TerrainKind::kChannel, TerrainShape::kAABB, TerrainFlags::kNoAttack, 24800, 1550, 33000, 2050, MapTheme::GARDEN_CHANNEL },
        { TerrainKind::kChannel, TerrainShape::kAABB, TerrainFlags::kNoAttack, 32800, 1450, 39500, 1950, MapTheme::GARDEN_CHANNEL }
    }),
    // ---- Desert ----
    MapTerrain({
        { TerrainKind::kHill, TerrainShape::kEllipse, TerrainFlags::kImpassable, 2500, 1200, 1000, 700, MapTheme::DESERT_HILL },
        { TerrainKind::kHill, TerrainShape::kEllipse, TerrainFlags::kImpassable, 6000, 2800, 1200, 650, MapTheme::DESERT_HILL },
        { TerrainKind::kHill, TerrainShape::kEllipse, TerrainFlags::kImpassable, 11000, 900, 900, 600, MapTheme::DESERT_HILL },
        { TerrainKind::kHill, TerrainShape::kEllipse, TerrainFlags::kImpassable, 15000, 3000, 1100, 700, MapTheme::DESERT_HILL },
        { TerrainKind::kHill, TerrainShape::kEllipse, TerrainFlags::kImpassable, 20000, 1000, 1000, 650, MapTheme::DESERT_HILL },
        { TerrainKind::kHill, TerrainShape::kEllipse, TerrainFlags::kImpassable, 25500, 2900, 1200, 720, MapTheme::DESERT_HILL },
        { TerrainKind::kHill, TerrainShape::kEllipse, TerrainFlags::kImpassable, 31000, 850, 950, 580, MapTheme::DESERT_HILL },
        { TerrainKind::kHill, TerrainShape::kEllipse, TerrainFlags::kImpassable, 36000, 2700, 1050, 680, MapTheme::DESERT_HILL },
        { TerrainKind::kWater, TerrainShape::kEllipse, TerrainFlags::kImpassable, 9000, 2000, 750, 500, MapTheme::DESERT_WATER },
        { TerrainKind::kWater, TerrainShape::kEllipse, TerrainFlags::kImpassable, 22000, 2100, 850, 520, MapTheme::DESERT_WATER },
        { TerrainKind::kWater, TerrainShape::kAABB, TerrainFlags::kImpassable, 28000, 1800, 32000, 2150, MapTheme::DESERT_WATER },
        { TerrainKind::kChannel, TerrainShape::kAABB, TerrainFlags::kNoAttack, 1200, 2400, 8000, 2750, MapTheme::DESERT_CHANNEL },
        { TerrainKind::kChannel, TerrainShape::kAABB, TerrainFlags::kNoAttack, 7800, 2300, 15500, 2800, MapTheme::DESERT_CHANNEL },
        { TerrainKind::kChannel, TerrainShape::kAABB, TerrainFlags::kNoAttack, 15200, 2200, 24000, 2700, MapTheme::DESERT_CHANNEL },
        { TerrainKind::kChannel, TerrainShape::kAABB, TerrainFlags::kNoAttack, 23800, 2100, 32000, 2600, MapTheme::DESERT_CHANNEL },
        { TerrainKind::kChannel, TerrainShape::kAABB, TerrainFlags::kNoAttack, 31800, 2000, 39200, 2500, MapTheme::DESERT_CHANNEL }
    }),
    // ---- Ocean ----
    MapTerrain({
        { TerrainKind::kHill, TerrainShape::kEllipse, TerrainFlags::kImpassable, 3000, 800, 950, 550, MapTheme::OCEAN_HILL },
        { TerrainKind::kHill, TerrainShape::kEllipse, TerrainFlags::kImpassable, 7000, 3000, 1100, 700, MapTheme::OCEAN_HILL },
        { TerrainKind::kHill, TerrainShape::kEllipse, TerrainFlags::kImpassable, 13000, 1000, 1000, 600, MapTheme::OCEAN_HILL },
        { TerrainKind::kHill, TerrainShape::kEllipse, TerrainFlags::kImpassable, 18000, 2900, 1150, 680, MapTheme::OCEAN_HILL },
        { TerrainKind::kHill, TerrainShape::kEllipse, TerrainFlags::kImpassable, 24000, 900, 900, 580, MapTheme::OCEAN_HILL },
        { TerrainKind::kHill, TerrainShape::kEllipse, TerrainFlags::kImpassable, 29000, 2800, 1050, 650, MapTheme::OCEAN_HILL },
        { TerrainKind::kHill, TerrainShape::kEllipse, TerrainFlags::kImpassable, 35000, 1100, 1000, 620, MapTheme::OCEAN_HILL },
        { TerrainKind::kWater, TerrainShape::kAABB, TerrainFlags::kImpassable, 4000, 1600, 9000, 2100, MapTheme::OCEAN_WATER },
        { TerrainKind::kWater, TerrainShape::kEllipse, TerrainFlags::kImpassable, 15000, 2000, 900, 600, MapTheme::OCEAN_WATER },
        { TerrainKind::kWater, TerrainShape::kAABB, TerrainFlags::kImpassable, 26000, 1700, 31000, 2200, MapTheme::OCEAN_WATER },
        { TerrainKind::kChannel, TerrainShape::kAABB, TerrainFlags::kNoAttack, 1000, 1750, 8500, 2100, MapTheme::OCEAN_CHANNEL },
        { TerrainKind::kChannel, TerrainShape::kAABB, TerrainFlags::kNoAttack, 8300, 1650, 16500, 2150, MapTheme::OCEAN_CHANNEL },
        { TerrainKind::kChannel, TerrainShape::kAABB, TerrainFlags::kNoAttack, 16300, 1550, 25500, 2050, MapTheme::OCEAN_CHANNEL },
        { TerrainKind::kChannel, TerrainShape::kAABB, TerrainFlags::kNoAttack, 25300, 1450, 34000, 1950, MapTheme::OCEAN_CHANNEL },
        { TerrainKind::kChannel, TerrainShape::kAABB, TerrainFlags::kNoAttack, 33800, 1350, 39500, 1850, MapTheme::OCEAN_CHANNEL }
    }),
    // ---- Polar ----
    MapTerrain({
        { TerrainKind::kHill, TerrainShape::kEllipse, TerrainFlags::kImpassable, 2800, 1000, 1000, 650, MapTheme::POLAR_HILL },
        { TerrainKind::kHill, TerrainShape::kEllipse, TerrainFlags::kImpassable, 6500, 2900, 1100, 700, MapTheme::POLAR_HILL },
        { TerrainKind::kHill, TerrainShape::kEllipse, TerrainFlags::kImpassable, 12000, 800, 950, 560, MapTheme::POLAR_HILL },
        { TerrainKind::kHill, TerrainShape::kEllipse, TerrainFlags::kImpassable, 17000, 3100, 1200, 720, MapTheme::POLAR_HILL },
        { TerrainKind::kHill, TerrainShape::kEllipse, TerrainFlags::kImpassable, 23000, 950, 1000, 600, MapTheme::POLAR_HILL },
        { TerrainKind::kHill, TerrainShape::kEllipse, TerrainFlags::kImpassable, 28000, 3000, 1100, 680, MapTheme::POLAR_HILL },
        { TerrainKind::kHill, TerrainShape::kEllipse, TerrainFlags::kImpassable, 34500, 900, 950, 580, MapTheme::POLAR_HILL },
        { TerrainKind::kHill, TerrainShape::kEllipse, TerrainFlags::kImpassable, 38000, 2800, 850, 620, MapTheme::POLAR_HILL },
        { TerrainKind::kWater, TerrainShape::kAABB, TerrainFlags::kImpassable, 5000, 1800, 10000, 2200, MapTheme::POLAR_WATER },
        { TerrainKind::kWater, TerrainShape::kEllipse, TerrainFlags::kImpassable, 20000, 2000, 800, 520, MapTheme::POLAR_WATER },
        { TerrainKind::kWater, TerrainShape::kAABB, TerrainFlags::kImpassable, 30000, 1900, 35000, 2300, MapTheme::POLAR_WATER },
        { TerrainKind::kChannel, TerrainShape::kAABB, TerrainFlags::kNoAttack, 1400, 2200, 9000, 2550, MapTheme::POLAR_CHANNEL },
        { TerrainKind::kChannel, TerrainShape::kAABB, TerrainFlags::kNoAttack, 8800, 2100, 17000, 2600, MapTheme::POLAR_CHANNEL },
        { TerrainKind::kChannel, TerrainShape::kAABB, TerrainFlags::kNoAttack, 16800, 2000, 26000, 2500, MapTheme::POLAR_CHANNEL },
        { TerrainKind::kChannel, TerrainShape::kAABB, TerrainFlags::kNoAttack, 25800, 1900, 34500, 2400, MapTheme::POLAR_CHANNEL },
        { TerrainKind::kChannel, TerrainShape::kAABB, TerrainFlags::kNoAttack, 34300, 1800, 39500, 2300, MapTheme::POLAR_CHANNEL }
    })
});

inline MapZones const &map_zones(uint8_t map_id) {
    if (map_id >= MapID::kNumMaps) map_id = MapID::kGarden;
    return MAPS[map_id];
}

inline MapTerrain const &map_terrain(uint8_t map_id) {
    if (map_id >= MapID::kNumMaps) map_id = MapID::kGarden;
    return MAP_TERRAIN[map_id];
}

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
