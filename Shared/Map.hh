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

    // Soft pad around respawn: sparse hub fill + blocks high-diff spawns.
    // Was 5200×4000 (swallowed the whole Common hub → ~8 mobs total, looked empty).
    // Keep modest so hub *periphery* can pack Common/Unusual packs.
    inline bool in_spawn_plaza(float x, float y) {
        float dx = (x - MAP_SPAWN_X) / 1600.f;
        float dy = (y - MAP_SPAWN_Y) / 1200.f;
        return dx * dx + dy * dy < 1.f;
    }

    // Only no-aggro pad: tight ellipse on the hub respawn point (MAP_SPAWN ± jitter).
    // Wild mobs do not acquire / keep chase here; players may still attack.
    inline bool in_spawn_sanctuary(float x, float y) {
        float dx = (x - MAP_SPAWN_X) / 1000.f;
        float dy = (y - MAP_SPAWN_Y) / 800.f;
        return dx * dx + dy * dy < 1.f;
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
