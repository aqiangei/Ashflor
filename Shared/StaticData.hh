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

// Each entry is one independent map; arena is (0,0)..(ARENA_WIDTH,ARENA_HEIGHT)
inline std::array const MAPS = std::to_array<MapZones>({
    MapZones({
        {
            .left = 0,
            .top = 0,
            .right = (float)ARENA_WIDTH,
            .bottom = (float)ARENA_HEIGHT,
            .density = 1,
            .drop_multiplier = 0.3,
            .spawns = {
                { MobID::kRock, 500000 },
                { MobID::kLadybug, 100000 },
                { MobID::kBee, 100000 },
                { MobID::kBabyAnt, 25000 },
                { MobID::kCentipede, 10000 },
                { MobID::kBoulder, 10000 },
                { MobID::kMassiveLadybug, 200 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 0,
            .color = 0xff1ea761,
            .name = "Garden"
        }
    }),
    MapZones({
        {
            .left = 0,
            .top = 0,
            .right = (float)ARENA_WIDTH,
            .bottom = (float)ARENA_HEIGHT,
            .density = 1,
            .drop_multiplier = 0.15,
            .spawns = {
                { MobID::kCactus, 400000 },
                { MobID::kBeetle, 100000 },
                { MobID::kSandstorm, 50000 },
                { MobID::kBee, 50000 },
                { MobID::kScorpion, 50000 },
                { MobID::kLadybug, 50000 },
                { MobID::kDesertCentipede, 10000 },
                { MobID::kAntHole, 2000 },
                { MobID::kShinyLadybug, 1000 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 1,
            .color = 0xffdecf7c,
            .name = "Desert"
        }
    }),
    MapZones({
        {
            .left = 0,
            .top = 0,
            .right = (float)ARENA_WIDTH,
            .bottom = (float)ARENA_HEIGHT,
            .density = 1.1f,
            .drop_multiplier = 0.1,
            .spawns = {
                { MobID::kRock, 200000 },
                { MobID::kBoulder, 150000 },
                { MobID::kSandstorm, 120000 },
                { MobID::kHornet, 100000 },
                { MobID::kBeetle, 80000 },
                { MobID::kEvilCentipede, 15000 },
                { MobID::kMassiveBeetle, 2500 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 2,
            .color = 0xff2e86ab,
            .name = "Ocean"
        }
    }),
    MapZones({
        {
            .left = 0,
            .top = 0,
            .right = (float)ARENA_WIDTH,
            .bottom = (float)ARENA_HEIGHT,
            .density = 1.15f,
            .drop_multiplier = 0.025,
            .spawns = {
                { MobID::kBoulder, 150000 },
                { MobID::kSpider, 150000 },
                { MobID::kDarkLadybug, 150000 },
                { MobID::kHornet, 120000 },
                { MobID::kEvilCentipede, 20000 },
                { MobID::kMassiveBeetle, 3000 },
                { MobID::kAntHole, 2500 },
                { MobID::kSquare, 1 }
            },
            .difficulty = 3,
            .color = 0xff8ecae6,
            .name = "Polar"
        }
    })
});

inline MapZones const &map_zones(uint8_t map_id) {
    if (map_id >= MapID::kNumMaps) map_id = MapID::kGarden;
    return MAPS[map_id];
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
