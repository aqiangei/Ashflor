#include <Shared/Map.hh>

#ifdef SERVERSIDE
#include <Server/Spawn.hh>
#include <Shared/Entity.hh>
#endif

#include <cmath>
#include <iostream>

using namespace Map;

uint32_t Map::difficulty_at_level(uint32_t level) {
    if (level / LEVELS_PER_EXTRA_SLOT > MAX_DIFFICULTY) return MAX_DIFFICULTY;
    return level / LEVELS_PER_EXTRA_SLOT;
}

uint32_t Map::get_zone_from_pos(uint8_t map_id, float x, float y) {
    MapZones const &zones = map_zones(map_id);
    uint32_t ret = 0;
    for (uint32_t i = 1; i < zones.size(); ++i) {
        struct ZoneDefinition const &zone = zones[i];
        if (fclamp(x, zone.left, zone.right) == x && fclamp(y, zone.top, zone.bottom) == y)
            ret = i;
    }
    return ret;
}

uint32_t Map::get_suitable_difficulty_zone(uint8_t map_id, uint32_t power) {
    MapZones const &zones = map_zones(map_id);
    std::vector<uint32_t> possible_zones;
    for (uint32_t i = 0; i < zones.size(); ++i)
        if (zones[i].difficulty == power) possible_zones.push_back(i);
    if (possible_zones.size() == 0) return 0;
    return possible_zones[frand() * possible_zones.size()];
}

uint8_t Map::point_in_feature(TerrainFeature const &f, float x, float y) {
    if (f.shape == TerrainShape::kEllipse) {
        float cx = f.left;
        float cy = f.top;
        float rx = f.right;
        float ry = f.bottom;
        if (rx <= 0.f || ry <= 0.f) return 0;
        float nx = (x - cx) / rx;
        float ny = (y - cy) / ry;
        return (nx * nx + ny * ny) <= 1.f ? 1 : 0;
    }
    // AABB: left/top/right/bottom
    return (x >= f.left && x <= f.right && y >= f.top && y <= f.bottom) ? 1 : 0;
}

static uint8_t circle_hits_feature(TerrainFeature const &f, float x, float y, float radius) {
    if (f.shape == TerrainShape::kEllipse) {
        float cx = f.left;
        float cy = f.top;
        float rx = f.right + radius;
        float ry = f.bottom + radius;
        if (rx <= 0.f || ry <= 0.f) return 0;
        float nx = (x - cx) / rx;
        float ny = (y - cy) / ry;
        return (nx * nx + ny * ny) <= 1.f ? 1 : 0;
    }
    float nearest_x = fclamp(x, f.left, f.right);
    float nearest_y = fclamp(y, f.top, f.bottom);
    float dx = x - nearest_x;
    float dy = y - nearest_y;
    return (dx * dx + dy * dy) <= radius * radius ? 1 : 0;
}

uint8_t Map::circle_hits_impassable(uint8_t map_id, float x, float y, float radius) {
    MapTerrain const &terrain = map_terrain(map_id);
    for (TerrainFeature const &f : terrain) {
        if (!(f.flags & TerrainFlags::kImpassable)) continue;
        if (circle_hits_feature(f, x, y, radius)) return 1;
    }
    return 0;
}

uint8_t Map::in_no_attack_zone(uint8_t map_id, float x, float y) {
    MapTerrain const &terrain = map_terrain(map_id);
    for (TerrainFeature const &f : terrain) {
        if (!(f.flags & TerrainFlags::kNoAttack)) continue;
        if (point_in_feature(f, x, y)) return 1;
    }
    return 0;
}

static uint8_t push_out_feature(TerrainFeature const &f, float &x, float &y, float radius) {
    if (f.shape == TerrainShape::kEllipse) {
        float cx = f.left;
        float cy = f.top;
        float rx = f.right + radius;
        float ry = f.bottom + radius;
        if (rx <= 0.f || ry <= 0.f) return 0;
        float dx = x - cx;
        float dy = y - cy;
        float nx = dx / rx;
        float ny = dy / ry;
        float d2 = nx * nx + ny * ny;
        if (d2 > 1.f || d2 <= 0.f) {
            if (d2 <= 0.f) {
                x = cx + rx;
                y = cy;
                return 1;
            }
            return 0;
        }
        float d = std::sqrt(d2);
        float scale = 1.001f / d;
        x = cx + dx * scale;
        y = cy + dy * scale;
        // Remap from inflated ellipse back toward surface of (rx,ry)
        // Already used inflated rx/ry so position is outside original + radius.
        return 1;
    }
    float nearest_x = fclamp(x, f.left, f.right);
    float nearest_y = fclamp(y, f.top, f.bottom);
    float dx = x - nearest_x;
    float dy = y - nearest_y;
    float dist2 = dx * dx + dy * dy;
    if (dist2 > radius * radius) return 0;
    if (dist2 < 1e-6f) {
        // Center inside AABB: push out shortest axis
        float dl = x - f.left;
        float dr = f.right - x;
        float dt = y - f.top;
        float db = f.bottom - y;
        float m = dl;
        uint8_t side = 0;
        if (dr < m) { m = dr; side = 1; }
        if (dt < m) { m = dt; side = 2; }
        if (db < m) { m = db; side = 3; }
        if (side == 0) x = f.left - radius;
        else if (side == 1) x = f.right + radius;
        else if (side == 2) y = f.top - radius;
        else y = f.bottom + radius;
        return 1;
    }
    float dist = std::sqrt(dist2);
    float push = (radius - dist) / dist + 0.001f;
    x += dx * push;
    y += dy * push;
    return 1;
}

uint8_t Map::resolve_circle_terrain(uint8_t map_id, float &x, float &y, float radius) {
    MapTerrain const &terrain = map_terrain(map_id);
    uint8_t moved = 0;
    for (uint32_t pass = 0; pass < 3; ++pass) {
        uint8_t any = 0;
        for (TerrainFeature const &f : terrain) {
            if (!(f.flags & TerrainFlags::kImpassable)) continue;
            if (!circle_hits_feature(f, x, y, radius)) continue;
            if (push_out_feature(f, x, y, radius)) {
                any = 1;
                moved = 1;
            }
        }
        if (!any) break;
    }
    return moved;
}

#ifdef SERVERSIDE
#include <Shared/Simulation.hh>
void Map::remove_mob(Simulation *sim, uint32_t zone) {
    DEBUG_ONLY(assert(zone < map_zones(sim->map_id).size());)
    --sim->zone_mob_counts[zone];
}

void Map::spawn_random_mob(Simulation *sim, float x, float y) {
    if (Map::circle_hits_impassable(sim->map_id, x, y, 40.f)) return;
    uint32_t zone_id = Map::get_zone_from_pos(sim->map_id, x, y);
    struct ZoneDefinition const &zone = map_zones(sim->map_id)[zone_id];
    if (zone.density * (zone.right - zone.left) * (zone.bottom - zone.top) / (500 * 500) < sim->zone_mob_counts[zone_id]) return;
    float sum = 0;
    for (SpawnChance const &s : zone.spawns)
        sum += s.chance;
    sum *= frand();
    for (SpawnChance const &s : zone.spawns) {
        sum -= s.chance;
        if (sum <= 0) {
            Entity &ent = alloc_mob(sim, s.id, x, y, NULL_ENTITY, [&](Entity &mob){
                mob.zone = zone_id;
                mob.immunity_ticks = TPS;
                BitMath::set(mob.flags, EntityFlags::kSpawnedFromZone);
                BitMath::set(mob.flags, EntityFlags::kHasCulling);
                sim->zone_mob_counts[zone_id]++;
                mob.score_reward = MOB_DATA[mob.get_mob_id()].xp;
            });
            return;
        }
    }
}

bool Map::find_spawn_location(Simulation *sim, float d, Vector &vref) {
    for (uint32_t i = 0; i < 16; ++i) {
        vref.set(frand() * ARENA_WIDTH, frand() * ARENA_HEIGHT);
        if (Map::circle_hits_impassable(sim->map_id, vref.x, vref.y, 60.f)) continue;
        bool valid = true;
        sim->for_each<kFlower>([&](Simulation *, Entity &ent) {
            if (ent.has_component(kMob)) return;
            if (!valid) return;
            if (Vector(ent.get_x() - vref.x, ent.get_y() - vref.y).magnitude() < d)
                valid = false;
        });
        if (valid) return true;
    }
    return false;
}
#endif
