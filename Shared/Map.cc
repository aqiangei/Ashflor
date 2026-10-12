#include <Shared/Map.hh>

#ifdef SERVERSIDE
#include <Server/Spawn.hh>
#include <Shared/Entity.hh>
#endif

#include <algorithm>
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

// Deterministic -1..1 noise (same spirit as florr_clone-web seededRandom)
float Map::terrain_hash(uint32_t seed, uint32_t i) {
    uint32_t h = seed * 374761393u + i * 668265263u;
    h = (h ^ (h >> 13)) * 1274126177u;
    h ^= h >> 16;
    return (float)(h & 0xffff) * (1.f / 32767.5f) - 1.f;
}

static float jagged_amp(TerrainFeature const &f) {
    if (f.param > 0.f && f.shape != TerrainShape::kRiver && f.shape != TerrainShape::kSpiral)
        return f.param;
    if (f.kind == TerrainKind::kHill) return 0.14f;
    if (f.kind == TerrainKind::kWater) return 0.10f;
    return 0.06f;
}

float Map::feature_half_width(TerrainFeature const &f) {
    if (f.shape == TerrainShape::kRiver || f.shape == TerrainShape::kSpiral)
        return f.param > 1.f ? f.param : 120.f;
    return 0.f;
}

void Map::river_center_at(TerrainFeature const &f, float t, float &x, float &y) {
    t = fclamp(t, 0.f, 1.f);
    float x0 = f.left, y0 = f.top, x1 = f.right, y1 = f.bottom;
    float dx = x1 - x0, dy = y1 - y0;
    float len = std::sqrt(dx * dx + dy * dy);
    if (len < 1.f) { x = x0; y = y0; return; }
    float ux = dx / len, uy = dy / len;
    float px = -uy, py = ux;
    float along = t * len;
    // Dual-frequency meander (seeded), like natural channels
    float s = (float)f.seed;
    float amp1 = 70.f + (float)(f.seed % 50) * 2.2f;
    float amp2 = 28.f + (float)(f.seed % 30);
    float freq1 = 0.0032f + (float)(f.seed % 7) * 0.00035f;
    float freq2 = 0.0095f + (float)(f.seed % 5) * 0.0008f;
    float meander = std::sin(along * freq1 + s * 0.31f) * amp1
                  + std::sin(along * freq2 + s * 0.77f) * amp2;
    x = x0 + ux * along + px * meander;
    y = y0 + uy * along + py * meander;
}

void Map::spiral_center_at(TerrainFeature const &f, float t, float &x, float &y) {
    t = fclamp(t, 0.f, 1.f);
    float turns = f.bottom > 0.2f ? f.bottom : 2.2f;
    float outer = f.right > 1.f ? f.right : 800.f;
    float theta = t * turns * (float)(2.0 * M_PI);
    float r = (outer / (turns * (float)(2.0 * M_PI))) * theta;
    float phase = (float)f.seed * 0.17f;
    x = f.left + std::cos(theta + phase) * r;
    y = f.top + std::sin(theta + phase) * r;
}

static float dist_to_path(TerrainFeature const &f, float x, float y, float &cx, float &cy) {
    uint32_t const samples = (f.shape == TerrainShape::kSpiral) ? 48u : 36u;
    float best = 1e12f;
    cx = f.left; cy = f.top;
    for (uint32_t i = 0; i <= samples; ++i) {
        float t = (float)i / (float)samples;
        float px, py;
        if (f.shape == TerrainShape::kSpiral)
            spiral_center_at(f, t, px, py);
        else
            river_center_at(f, t, px, py);
        float dx = x - px, dy = y - py;
        float d2 = dx * dx + dy * dy;
        if (d2 < best) { best = d2; cx = px; cy = py; }
    }
    return std::sqrt(best);
}

// Cheap conservative broad phase before sampling curved paths. The river
// meander's two sine amplitudes sum to less than 240 world units.
static bool feature_near_circle(TerrainFeature const &f, float x, float y, float radius) {
    float x0, y0, x1, y1;
    if (f.shape == TerrainShape::kRiver) {
        float pad = feature_half_width(f) + 240.f + radius;
        x0 = std::min(f.left, f.right) - pad;
        x1 = std::max(f.left, f.right) + pad;
        y0 = std::min(f.top, f.bottom) - pad;
        y1 = std::max(f.top, f.bottom) + pad;
    } else if (f.shape == TerrainShape::kSpiral) {
        float pad = (f.right > 1.f ? f.right : 800.f) + feature_half_width(f) + radius;
        x0 = f.left - pad; x1 = f.left + pad;
        y0 = f.top - pad; y1 = f.top + pad;
    } else if (f.shape == TerrainShape::kCircle || f.shape == TerrainShape::kEllipse) {
        float shape_r = f.shape == TerrainShape::kCircle
            ? (f.right > 0.f ? f.right : f.bottom) : f.right;
        float rx = shape_r + radius;
        float ry = (f.shape == TerrainShape::kCircle ? shape_r : f.bottom) + radius;
        x0 = f.left - rx; x1 = f.left + rx;
        y0 = f.top - ry; y1 = f.top + ry;
    } else {
        x0 = f.left - radius; x1 = f.right + radius;
        y0 = f.top - radius; y1 = f.bottom + radius;
    }
    return x >= x0 && x <= x1 && y >= y0 && y <= y1;
}

uint8_t Map::point_in_feature(TerrainFeature const &f, float x, float y) {
    if (!feature_near_circle(f, x, y, 0.f)) return 0;
    if (f.shape == TerrainShape::kRiver || f.shape == TerrainShape::kSpiral) {
        float cx, cy;
        return dist_to_path(f, x, y, cx, cy) <= feature_half_width(f) ? 1 : 0;
    }
    if (f.shape == TerrainShape::kCircle) {
        float r = f.right > 0.f ? f.right : f.bottom;
        if (r <= 0.f) return 0;
        float dx = x - f.left, dy = y - f.top;
        return (dx * dx + dy * dy) <= r * r ? 1 : 0;
    }
    if (f.shape == TerrainShape::kEllipse) {
        float rx = f.right, ry = f.bottom;
        if (rx <= 0.f || ry <= 0.f) return 0;
        float nx = (x - f.left) / rx;
        float ny = (y - f.top) / ry;
        return (nx * nx + ny * ny) <= 1.f ? 1 : 0;
    }
    // kAABB box
    return (x >= f.left && x <= f.right && y >= f.top && y <= f.bottom) ? 1 : 0;
}

static uint8_t circle_hits_feature(TerrainFeature const &f, float x, float y, float radius) {
    if (!feature_near_circle(f, x, y, radius)) return 0;
    if (f.shape == TerrainShape::kRiver || f.shape == TerrainShape::kSpiral) {
        float cx, cy;
        return dist_to_path(f, x, y, cx, cy) <= feature_half_width(f) + radius ? 1 : 0;
    }
    if (f.shape == TerrainShape::kCircle) {
        float r = (f.right > 0.f ? f.right : f.bottom) + radius;
        if (r <= 0.f) return 0;
        float dx = x - f.left, dy = y - f.top;
        return (dx * dx + dy * dy) <= r * r ? 1 : 0;
    }
    if (f.shape == TerrainShape::kEllipse) {
        float rx = f.right + radius, ry = f.bottom + radius;
        if (rx <= 0.f || ry <= 0.f) return 0;
        float nx = (x - f.left) / rx;
        float ny = (y - f.top) / ry;
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

static uint8_t push_out_path(TerrainFeature const &f, float &x, float &y, float radius) {
    float cx, cy;
    float d = dist_to_path(f, x, y, cx, cy);
    float need = feature_half_width(f) + radius;
    if (d >= need) return 0;
    float dx = x - cx, dy = y - cy;
    float len = std::sqrt(dx * dx + dy * dy);
    if (len < 1e-3f) {
        dx = 1.f; dy = 0.f; len = 1.f;
    }
    float scale = (need + 0.5f) / len;
    x = cx + dx * scale;
    y = cy + dy * scale;
    return 1;
}

static uint8_t push_out_feature(TerrainFeature const &f, float &x, float &y, float radius) {
    if (!feature_near_circle(f, x, y, radius)) return 0;
    if (f.shape == TerrainShape::kRiver || f.shape == TerrainShape::kSpiral)
        return push_out_path(f, x, y, radius);
    if (f.shape == TerrainShape::kCircle) {
        float r = (f.right > 0.f ? f.right : f.bottom) + radius;
        if (r <= 0.f) return 0;
        float dx = x - f.left, dy = y - f.top;
        float d2 = dx * dx + dy * dy;
        if (d2 > r * r || d2 <= 0.f) {
            if (d2 <= 0.f) { x = f.left + r; y = f.top; return 1; }
            return 0;
        }
        float d = std::sqrt(d2);
        float scale = (r + 0.5f) / d;
        x = f.left + dx * scale;
        y = f.top + dy * scale;
        return 1;
    }
    if (f.shape == TerrainShape::kEllipse) {
        float cx = f.left, cy = f.top;
        float rx = f.right + radius, ry = f.bottom + radius;
        if (rx <= 0.f || ry <= 0.f) return 0;
        float dx = x - cx, dy = y - cy;
        float nx = dx / rx, ny = dy / ry;
        float d2 = nx * nx + ny * ny;
        if (d2 > 1.f || d2 <= 0.f) {
            if (d2 <= 0.f) { x = cx + rx; y = cy; return 1; }
            return 0;
        }
        float d = std::sqrt(d2);
        float scale = 1.001f / d;
        x = cx + dx * scale;
        y = cy + dy * scale;
        return 1;
    }
    float nearest_x = fclamp(x, f.left, f.right);
    float nearest_y = fclamp(y, f.top, f.bottom);
    float dx = x - nearest_x;
    float dy = y - nearest_y;
    float dist2 = dx * dx + dy * dy;
    if (dist2 > radius * radius) return 0;
    if (dist2 < 1e-6f) {
        float dl = x - f.left, dr = f.right - x, dt = y - f.top, db = f.bottom - y;
        float m = dl; uint8_t side = 0;
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

// Closed outline for fill. Hills = sharp jagged (clone wall edges);
// water uses same samples (client smooths with qcurve).
uint32_t Map::terrain_outline(TerrainFeature const &f, float *xs, float *ys, uint32_t cap) {
    if (cap < 8) return 0;
    uint32_t seed = f.seed ? f.seed : (uint32_t)(f.left * 0.01f + f.top * 0.017f) + f.kind * 17u;

    if (f.shape == TerrainShape::kRiver || f.shape == TerrainShape::kSpiral) {
        uint32_t half = (cap / 2) < 24u ? (cap / 2) : 24u;
        if (half < 4) half = 4;
        float hw = feature_half_width(f);
        for (uint32_t i = 0; i < half; ++i) {
            float t = (float)i / (float)(half - 1);
            float cx, cy, nx, ny;
            float t0 = t > 0.001f ? t - 0.001f : 0.f;
            float t1 = t < 0.999f ? t + 0.001f : 1.f;
            float ax, ay, bx, by;
            if (f.shape == TerrainShape::kSpiral) {
                spiral_center_at(f, t, cx, cy);
                spiral_center_at(f, t0, ax, ay);
                spiral_center_at(f, t1, bx, by);
            } else {
                river_center_at(f, t, cx, cy);
                river_center_at(f, t0, ax, ay);
                river_center_at(f, t1, bx, by);
            }
            float dx = bx - ax, dy = by - ay;
            float len = std::sqrt(dx * dx + dy * dy);
            if (len < 1e-3f) { nx = 1.f; ny = 0.f; }
            else { nx = -dy / len; ny = dx / len; }
            float j = 1.f + terrain_hash(seed, i) * 0.12f;
            xs[i] = cx + nx * hw * j;
            ys[i] = cy + ny * hw * j;
        }
        for (uint32_t i = 0; i < half; ++i) {
            float t = 1.f - (float)i / (float)(half - 1);
            float cx, cy, nx, ny;
            float t0 = t > 0.001f ? t - 0.001f : 0.f;
            float t1 = t < 0.999f ? t + 0.001f : 1.f;
            float ax, ay, bx, by;
            if (f.shape == TerrainShape::kSpiral) {
                spiral_center_at(f, t, cx, cy);
                spiral_center_at(f, t0, ax, ay);
                spiral_center_at(f, t1, bx, by);
            } else {
                river_center_at(f, t, cx, cy);
                river_center_at(f, t0, ax, ay);
                river_center_at(f, t1, bx, by);
            }
            float dx = bx - ax, dy = by - ay;
            float len = std::sqrt(dx * dx + dy * dy);
            if (len < 1e-3f) { nx = 1.f; ny = 0.f; }
            else { nx = -dy / len; ny = dx / len; }
            float j = 1.f + terrain_hash(seed, i + 100) * 0.12f;
            xs[half + i] = cx - nx * hw * j;
            ys[half + i] = cy - ny * hw * j;
        }
        return half * 2;
    }

    if (f.shape == TerrainShape::kAABB) {
        // Box with protruding jagged edges (clone drawJaggedEdge spirit)
        float l = f.left, t = f.top, r = f.right, b = f.bottom;
        float w = r - l, h = b - t;
        float amp = jagged_amp(f) * std::min(w, h) * 0.5f;
        uint32_t per = 8;
        uint32_t n = 0;
        auto push = [&](float x, float y) {
            if (n < cap) { xs[n] = x; ys[n] = y; ++n; }
        };
        for (uint32_t i = 0; i < per && n < cap; ++i) {
            float u = (float)i / (float)per;
            float o = terrain_hash(seed, i) * amp;
            if (o < 0.f) o = 0.f;
            push(l + u * w, t - o);
        }
        for (uint32_t i = 0; i < per && n < cap; ++i) {
            float u = (float)i / (float)per;
            float o = terrain_hash(seed, i + 20) * amp;
            if (o < 0.f) o = 0.f;
            push(r + o, t + u * h);
        }
        for (uint32_t i = 0; i < per && n < cap; ++i) {
            float u = (float)i / (float)per;
            float o = terrain_hash(seed, i + 40) * amp;
            if (o < 0.f) o = 0.f;
            push(r - u * w, b + o);
        }
        for (uint32_t i = 0; i < per && n < cap; ++i) {
            float u = (float)i / (float)per;
            float o = terrain_hash(seed, i + 60) * amp;
            if (o < 0.f) o = 0.f;
            push(l - o, b - u * h);
        }
        return n;
    }

    // Ellipse / circle — radial jagged outline
    float cx = f.left, cy = f.top;
    float rx = f.right, ry = f.bottom;
    if (f.shape == TerrainShape::kCircle) {
        rx = f.right > 0.f ? f.right : f.bottom;
        ry = rx;
    }
    if (rx <= 0.f || ry <= 0.f) return 0;
    float amp = jagged_amp(f);
    uint32_t n = cap < 40u ? cap : 40u;
    if (n < 12) n = 12;
    if (n > cap) n = cap;
    for (uint32_t i = 0; i < n; ++i) {
        float a = (float)(2.0 * M_PI) * ((float)i / (float)n);
        float s = 1.f + terrain_hash(seed, i) * amp;
        // Occasional deeper bite (cluster spike feel)
        if (terrain_hash(seed, i + 200) > 0.65f)
            s += amp * 0.45f * terrain_hash(seed, i + 300);
        xs[i] = cx + std::cos(a) * rx * s;
        ys[i] = cy + std::sin(a) * ry * s;
    }
    return n;
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

static float zone_exclusive_area(uint8_t map_id, uint32_t zone_id) {
    MapZones const &zones = map_zones(map_id);
    ZoneDefinition const &zone = zones[zone_id];
    float area = (zone.right - zone.left) * (zone.bottom - zone.top);
    // Nested zones: later indices win inside their bounds. Subtract the largest
    // fully-contained later zone so density matches the exclusive ring/hub.
    for (uint32_t j = zone_id + 1; j < zones.size(); ++j) {
        ZoneDefinition const &inner = zones[j];
        if (inner.left >= zone.left && inner.right <= zone.right &&
            inner.top >= zone.top && inner.bottom <= zone.bottom) {
            area -= (inner.right - inner.left) * (inner.bottom - inner.top);
            break;
        }
    }
    return area > 0.f ? area : 0.f;
}

void Map::spawn_random_mob(Simulation *sim, float x, float y) {
    if (Map::circle_hits_impassable(sim->map_id, x, y, 40.f)) return;
    uint32_t zone_id = Map::get_zone_from_pos(sim->map_id, x, y);
    struct ZoneDefinition const &zone = map_zones(sim->map_id)[zone_id];
    if (zone.density * zone_exclusive_area(sim->map_id, zone_id) / (500 * 500) < sim->zone_mob_counts[zone_id]) return;
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
