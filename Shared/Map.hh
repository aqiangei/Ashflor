#pragma once

#include <Shared/StaticData.hh>

#include <array>
#include <cstdint>
#include <vector>

#ifdef SERVERSIDE
#include <Helpers/Vector.hh>
#endif

SERVER_ONLY(class Simulation;)
SERVER_ONLY(class Entity;)

namespace Map {
    // 抽样参数：40% 的生成走等概率选区（保证小生态区不被大环带淹没）
    inline constexpr float kUniformZoneShare = 0.4f;
    // 玩家近身多少单位内不生成新怪
    inline constexpr float kMinSpawnPlayerDist = 800.f;
    // 贴障类落点的探墙距离上限（超过视为"该方向没有墙"）
    inline constexpr float kWallProbe = 900.f;
    // 伏击类（P1）允许在更远处找墙
    inline constexpr float kWallProbeNear = 1600.f;
    // 同款精英跨 Zone 唯一半径（总纲 R6）
    inline constexpr float kEliteUniqueRadius = 1500.f;
    // 迷宫格口硬下限（总纲 §1.2 / R7）
    inline constexpr float kMinMazeGate = 800.f;

    // florr L2 layouts from maps/*.tmj (1:1). Others = ashflor procedural.
    inline bool is_florr_layout(uint8_t map_id) {
        switch (map_id) {
            case MapID::kPrimeGarden:
            case MapID::kSewers:
            case MapID::kPrimeDesert:
            case MapID::kWildwood:
            case MapID::kHel:
            case MapID::kAntHell:
            case MapID::kPrimeOcean:
                return true;
            default:
                return false;
        }
    }

    // Per-map respawn hub (florr player_spawns origin; else arena center).
    inline void hub_xy(uint8_t map_id, float &x, float &y) {
        switch (map_id) {
#include <Shared/FlorrTmjHubs.inc>
        }
    }

    inline void florr_extent(uint8_t map_id, float &w, float &h) {
        switch (map_id) {
#include <Shared/FlorrTmjExtents.inc>
        }
    }

    // Soft pad around respawn: sparse hub fill + blocks high-diff spawns.
    inline bool in_spawn_plaza(uint8_t map_id, float x, float y) {
        float hx = MAP_SPAWN_X, hy = MAP_SPAWN_Y;
        hub_xy(map_id, hx, hy);
        // Tighter pad on florr hubs — do not clear authored walls around spawn.
        float sx = is_florr_layout(map_id) ? (ARENA_WIDTH * 0.028f) : (ARENA_WIDTH * 0.055f);
        float sy = is_florr_layout(map_id) ? (ARENA_HEIGHT * 0.022f) : (ARENA_HEIGHT * 0.042f);
        float dx = (x - hx) / sx;
        float dy = (y - hy) / sy;
        return dx * dx + dy * dy < 1.f;
    }
    // Legacy: arena-center plaza (L1 default / tests without map_id).
    inline bool in_spawn_plaza(float x, float y) {
        return in_spawn_plaza(MapID::kGarden, x, y);
    }

    // Clear ring around field portals — no auto-spawn / thin packs so gates stay usable.
    inline bool near_map_portal(uint8_t map_id, float x, float y) {
        for (MapPortalDef const &p : MAP_PORTALS) {
            if (p.from_map != map_id) continue;
            float clear = p.radius + 1200.f;
            float dx = x - p.x;
            float dy = y - p.y;
            if (dx * dx + dy * dy < clear * clear) return true;
        }
        return false;
    }

    // Only no-aggro pad: tight ellipse on the map hub.
    inline bool in_spawn_sanctuary(uint8_t map_id, float x, float y) {
        float hx = MAP_SPAWN_X, hy = MAP_SPAWN_Y;
        hub_xy(map_id, hx, hy);
        float sx = is_florr_layout(map_id) ? (ARENA_WIDTH * 0.018f) : (ARENA_WIDTH * 0.034f);
        float sy = is_florr_layout(map_id) ? (ARENA_HEIGHT * 0.015f) : (ARENA_HEIGHT * 0.028f);
        float dx = (x - hx) / sx;
        float dy = (y - hy) / sy;
        return dx * dx + dy * dy < 1.f;
    }
    inline bool in_spawn_sanctuary(float x, float y) {
        return in_spawn_sanctuary(MapID::kGarden, x, y);
    }

    extern uint32_t difficulty_at_level(uint32_t);
    extern uint32_t get_zone_from_pos(uint8_t map_id, float, float);
    extern uint32_t get_suitable_difficulty_zone(uint8_t map_id, uint32_t);

    // Terrain queries (Shared — used by server motion and client render helpers)
    extern uint8_t point_in_feature(TerrainFeature const &, float x, float y);
    extern uint8_t circle_hits_impassable(uint8_t map_id, float x, float y, float radius);
    /* Push circle out of impassable terrain; returns whether a correction was applied */
    extern uint8_t resolve_circle_terrain(uint8_t map_id, float &x, float &y, float radius);
    /* Continuous move: subdivide ox,oy → x,y so fast entities cannot tunnel hills */
    extern void ccd_move_circle(uint8_t map_id, float &x, float &y, float ox, float oy, float radius);
    /* After arena clamp: if still inside edge berms, slide toward map center until free.
       Collision box blocks crossing — never zero the player's movement intent. */
    extern void eject_from_impassable(uint8_t map_id, float &x, float &y, float radius);

    // Procedural outline / centerline (client fill + minimap). Returns point count.
    extern float terrain_hash(uint32_t seed, uint32_t i);
    extern void river_center_at(TerrainFeature const &, float t, float &x, float &y);
    extern void spiral_center_at(TerrainFeature const &, float t, float &x, float &y);
    extern float feature_half_width(TerrainFeature const &);
    extern uint32_t terrain_outline(TerrainFeature const &, float *xs, float *ys, uint32_t cap);
    // Shared: MapGen rim exits + spawn rules (no mobs on painted roads).
    extern uint8_t in_safe_channel(uint8_t map_id, float x, float y);

    #ifdef SERVERSIDE
    extern void remove_mob(Simulation *, uint32_t);
    extern void remove_mob(Simulation *, Entity const &);
    extern void spawn_random_mob(Simulation *, float, float);
    // 按 Zone 密度加权选区 → 按落点类别选点 → 抽怪生成
    extern void spawn_zone_mob(Simulation *);
    // Crowd lowered dens target: drain surplus hub/soft-zone mobs (no loot).
    extern void thin_soft_zone_excess(Simulation *);
    // Any flower combat FOV below ratio×local target (clone/rrolf underfill).
    extern uint8_t any_combat_fov_underfilled(Simulation *, float ratio);
    extern bool find_spawn_location(Simulation *, float, Vector &);
    // 在指定 Zone 内按落点类别采样；clearance 用 spawn_rarity 放大后半径。成功返回 1
    extern uint8_t sample_spawn_point(Simulation *, uint32_t zone_id, uint8_t mob_id, float &x, float &y, uint8_t spawn_rarity);
    // Zone 权重 = density × 独占面积；返回被抽中的 Zone 索引。
    extern uint32_t pick_spawn_zone(uint8_t map_id);
    // 落点相关查询
    extern float open_clearance(uint8_t map_id, float x, float y, float radius, float max_dist);
    extern float corridor_reach(uint8_t map_id, float x, float y, float dx, float dy, float radius, float max_dist);
    // Boss 锚点（每图至多一个）；无 Boss 时返回 0
    extern uint8_t boss_anchor(uint8_t map_id, float &x, float &y, uint8_t &mob_id);
    // 生成一只怪到指定点（套用 Zone 难度 + 给定 spawn_rarity 体型/战斗倍率并记账）
    extern void _spawn_mob_at(Simulation *, uint32_t zone_id, uint8_t mob_id, float x, float y, uint8_t spawn_rarity);
    extern uint8_t _spawn_zone_mob_once(Simulation *);
    #endif

    // Walkable BFS from spawn — returns how many of the sampled goals are reachable.
    extern uint32_t sample_connectivity(uint8_t map_id, float step = 200.f);
}
