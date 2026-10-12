#pragma once

#include <Shared/StaticData.hh>

#include <array>
#include <cstdint>
#include <vector>

#ifdef SERVERSIDE
#include <Helpers/Vector.hh>
#endif

SERVER_ONLY(class Simulation;)

namespace Map {
    extern uint32_t difficulty_at_level(uint32_t);
    extern uint32_t get_zone_from_pos(uint8_t map_id, float, float);
    extern uint32_t get_suitable_difficulty_zone(uint8_t map_id, uint32_t);

    // Terrain queries (Shared — used by server motion and client render helpers)
    extern uint8_t point_in_feature(TerrainFeature const &, float x, float y);
    extern uint8_t circle_hits_impassable(uint8_t map_id, float x, float y, float radius);
    extern uint8_t in_no_attack_zone(uint8_t map_id, float x, float y);
    /* Push circle out of impassable terrain; returns whether a correction was applied */
    extern uint8_t resolve_circle_terrain(uint8_t map_id, float &x, float &y, float radius);

    #ifdef SERVERSIDE
    extern void remove_mob(Simulation *, uint32_t);
    extern void spawn_random_mob(Simulation *, float, float);
    extern bool find_spawn_location(Simulation *, float, Vector &);
    #endif
}
