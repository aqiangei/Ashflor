#include <Shared/Map.hh>
#include <Shared/RarityScale.hh>

#ifdef SERVERSIDE
#include <Server/Spawn.hh>
#include <Shared/Entity.hh>
#include <Shared/MapGen.hh>
#include <Shared/SpawnRules.hh>
#endif

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <unordered_map>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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

static uint32_t feature_seed(TerrainFeature const &f) {
    return f.seed ? f.seed : (uint32_t)(f.left * 0.01f + f.top * 0.017f) + f.kind * 17u;
}

// Same radial scale as terrain_outline (ellipse/circle vertices).
static float radial_jagged_s(uint32_t seed, uint32_t i, float amp) {
    float s = 1.f + Map::terrain_hash(seed, i) * amp;
    if (Map::terrain_hash(seed, i + 200) > 0.65f)
        s += amp * 0.45f * Map::terrain_hash(seed, i + 300);
    return s;
}

static constexpr uint32_t kRadialOutlineN = 40u;

// Interpolated jagged scale at world angle (matches outline sample count).
static float radial_scale_at_angle(uint32_t seed, float amp, float angle) {
    float two_pi = (float)(2.0 * M_PI);
    float t = angle / two_pi;
    if (t < 0.f) t += 1.f;
    if (t >= 1.f) t -= 1.f;
    float fi = t * (float)kRadialOutlineN;
    uint32_t i0 = (uint32_t)fi % kRadialOutlineN;
    uint32_t i1 = (i0 + 1u) % kRadialOutlineN;
    float frac = fi - std::floor(fi);
    float s0 = radial_jagged_s(seed, i0, amp);
    float s1 = radial_jagged_s(seed, i1, amp);
    return s0 + (s1 - s0) * frac;
}

static float max_radial_scale(float amp) {
    // Upper bound of radial_jagged_s (hash in -1..1, bite when hash>0.65)
    return 1.f + amp + amp * 0.45f;
}

float Map::feature_half_width(TerrainFeature const &f) {
    if (f.shape == TerrainShape::kRiver || f.shape == TerrainShape::kSpiral)
        return f.param > 1.f ? f.param : 120.f;
    return 0.f;
}

// River/spiral bank width multiplier at sample i (matches outline j = 1+hash*0.12).
static float path_width_scale(uint32_t seed, uint32_t i, uint8_t side) {
    uint32_t idx = side ? (i + 100u) : i;
    return 1.f + Map::terrain_hash(seed, idx) * 0.12f;
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

// Cheap conservative broad phase. Pads include jagged protrusions so we never
// miss a hit outside the smooth AABB/ellipse while art extends past it.
static void feature_bounds(TerrainFeature const &f, float radius,
                           float &x0, float &y0, float &x1, float &y1) {
    if (f.shape == TerrainShape::kRiver) {
        float pad = feature_half_width(f) * 1.12f + 240.f + radius;
        x0 = std::min(f.left, f.right) - pad;
        x1 = std::max(f.left, f.right) + pad;
        y0 = std::min(f.top, f.bottom) - pad;
        y1 = std::max(f.top, f.bottom) + pad;
    } else if (f.shape == TerrainShape::kSpiral) {
        float pad = (f.right > 1.f ? f.right : 800.f) + feature_half_width(f) * 1.12f + radius;
        x0 = f.left - pad; x1 = f.left + pad;
        y0 = f.top - pad; y1 = f.top + pad;
    } else if (f.shape == TerrainShape::kCircle || f.shape == TerrainShape::kEllipse) {
        float amp = jagged_amp(f);
        float ms = max_radial_scale(amp);
        float shape_r = f.shape == TerrainShape::kCircle
            ? (f.right > 0.f ? f.right : f.bottom) : f.right;
        float rx = shape_r * ms + radius;
        float ry = (f.shape == TerrainShape::kCircle ? shape_r : f.bottom) * ms + radius;
        x0 = f.left - rx; x1 = f.left + rx;
        y0 = f.top - ry; y1 = f.top + ry;
    } else {
        float w = f.right - f.left, h = f.bottom - f.top;
        float amp = jagged_amp(f) * std::min(w, h) * 0.5f;
        float pad = amp + radius;
        x0 = f.left - pad; x1 = f.right + pad;
        y0 = f.top - pad; y1 = f.bottom + pad;
    }
}

static bool feature_near_circle(TerrainFeature const &f, float x, float y, float radius) {
    float x0, y0, x1, y1;
    feature_bounds(f, radius, x0, y0, x1, y1);
    return x >= x0 && x <= x1 && y >= y0 && y <= y1;
}

static uint8_t point_in_polygon(float const *xs, float const *ys, uint32_t n, float x, float y) {
    if (n < 3) return 0;
    uint8_t inside = 0;
    for (uint32_t i = 0, j = n - 1; i < n; j = i++) {
        float yi = ys[i], yj = ys[j];
        float xi = xs[i], xj = xs[j];
        if (((yi > y) != (yj > y)) &&
            (x < (xj - xi) * (y - yi) / (yj - yi + 1e-12f) + xi))
            inside = (uint8_t)(1 - inside);
    }
    return inside;
}

static float dist2_point_seg(float px, float py, float ax, float ay, float bx, float by,
                            float &qx, float &qy) {
    float abx = bx - ax, aby = by - ay;
    float apx = px - ax, apy = py - ay;
    float ab2 = abx * abx + aby * aby;
    float t = ab2 > 1e-8f ? (apx * abx + apy * aby) / ab2 : 0.f;
    t = fclamp(t, 0.f, 1.f);
    qx = ax + abx * t;
    qy = ay + aby * t;
    float dx = px - qx, dy = py - qy;
    return dx * dx + dy * dy;
}

// Closest point on the river/spiral centerline polyline (matches drawn ribbon).
namespace {
struct PathSegment {
    float ax, ay, bx, by, tx, ty, hw[2];
};
struct CachedPath {
    TerrainFeature source{};
    uint32_t count = 0;
    std::array<PathSegment, 48> segments;
};
std::unordered_map<TerrainFeature const *, CachedPath> path_cache;

CachedPath const &cached_path(TerrainFeature const &f) {
    auto it = path_cache.find(&f);
    if (it != path_cache.end()) {
        TerrainFeature const &s = it->second.source;
        if (s.left == f.left && s.top == f.top && s.right == f.right && s.bottom == f.bottom
            && s.shape == f.shape && s.kind == f.kind && s.seed == f.seed && s.param == f.param)
            return it->second;
    } else {
        // Normally at most one entry per static map feature. Keep this bounded
        // even if a future caller passes temporary terrain objects.
        if (path_cache.size() >= MapID::kNumMaps * MAX_TERRAIN_PER_MAP)
            path_cache.erase(path_cache.begin());
        it = path_cache.emplace(&f, CachedPath{}).first;
    }
    CachedPath &path = it->second;
    path.source = f;
    path.count = f.shape == TerrainShape::kSpiral ? 48u : 36u;
    float ax, ay;
    if (f.shape == TerrainShape::kSpiral) Map::spiral_center_at(f, 0.f, ax, ay);
    else Map::river_center_at(f, 0.f, ax, ay);
    float base_hw = Map::feature_half_width(f);
    uint32_t seed = feature_seed(f);
    for (uint32_t i = 1; i <= path.count; ++i) {
        PathSegment &s = path.segments[i - 1];
        s.ax = ax; s.ay = ay;
        float t = (float)i / (float)path.count;
        if (f.shape == TerrainShape::kSpiral) Map::spiral_center_at(f, t, s.bx, s.by);
        else Map::river_center_at(f, t, s.bx, s.by);
        s.tx = s.bx - ax; s.ty = s.by - ay;
        float len = std::sqrt(s.tx * s.tx + s.ty * s.ty);
        if (len < 1e-3f) { s.tx = 1.f; s.ty = 0.f; }
        else { s.tx /= len; s.ty /= len; }
        s.hw[0] = base_hw * path_width_scale(seed, i - 1, 0);
        s.hw[1] = base_hw * path_width_scale(seed, i - 1, 1);
        ax = s.bx; ay = s.by;
    }
    return path;
}
} // namespace

// Old circle-around-nearest-sample blew past meander bends as invisible air walls.
static void closest_on_path(TerrainFeature const &f, float x, float y,
                            float &qx, float &qy, float &hw,
                            float &nx, float &ny, float &dist) {
    CachedPath const &path = cached_path(f);
    float base_hw = feature_half_width(f);
    float best_d2 = 1e12f;
    qx = f.left; qy = f.top;
    hw = base_hw;
    nx = 1.f; ny = 0.f;
    dist = 0.f;

    for (uint32_t i = 0; i < path.count; ++i) {
        PathSegment const &s = path.segments[i];
        float pqx = 0.f, pqy = 0.f;
        float d2 = dist2_point_seg(x, y, s.ax, s.ay, s.bx, s.by, pqx, pqy);
        if (d2 < best_d2) {
            best_d2 = d2;
            qx = pqx; qy = pqy;
            float tx = s.tx, ty = s.ty;
            // Bank scale matches terrain_outline (+n = side 0, -n = side 1)
            float nnx = -ty, nny = tx;
            float rx = x - pqx, ry = y - pqy;
            float cross = tx * ry - ty * rx;
            uint8_t side = cross >= 0.f ? 0 : 1;
            hw = s.hw[side];
            // Push normal: from centerline toward the query (stable for eject).
            float rlen = std::sqrt(rx * rx + ry * ry);
            if (rlen > 1e-3f) { nx = rx / rlen; ny = ry / rlen; }
            else if (side == 0) { nx = nnx; ny = nny; }
            else { nx = -nnx; ny = -nny; }
        }
    }
    dist = std::sqrt(best_d2);
}

static float aabb_jagged_amp(TerrainFeature const &f) {
    float w = f.right - f.left, h = f.bottom - f.top;
    return jagged_amp(f) * std::min(w, h) * 0.5f;
}

static uint8_t point_in_jagged_box(TerrainFeature const &f, float x, float y) {
    // Jagged edges only protrude outward — smooth interior is always inside
    if (x >= f.left && x <= f.right && y >= f.top && y <= f.bottom) return 1;
    float amp = aabb_jagged_amp(f);
    if (x < f.left - amp || x > f.right + amp || y < f.top - amp || y > f.bottom + amp)
        return 0;
    float xs[64], ys[64];
    uint32_t n = Map::terrain_outline(f, xs, ys, 64);
    return point_in_polygon(xs, ys, n, x, y);
}

static uint8_t circle_hits_jagged_box(TerrainFeature const &f, float x, float y, float radius) {
    // Fast path: circle vs smooth AABB (subset of jagged solid)
    float nearest_x = fclamp(x, f.left, f.right);
    float nearest_y = fclamp(y, f.top, f.bottom);
    float dx = x - nearest_x, dy = y - nearest_y;
    if (dx * dx + dy * dy <= radius * radius) return 1;
    float amp = aabb_jagged_amp(f);
    if (x < f.left - amp - radius || x > f.right + amp + radius
        || y < f.top - amp - radius || y > f.bottom + amp + radius)
        return 0;
    float xs[64], ys[64];
    uint32_t n = Map::terrain_outline(f, xs, ys, 64);
    if (n < 3) return 0;
    if (point_in_polygon(xs, ys, n, x, y)) return 1;
    float r2 = radius * radius;
    for (uint32_t i = 0; i < n; ++i) {
        uint32_t j = (i + 1) % n;
        float qx, qy;
        if (dist2_point_seg(x, y, xs[i], ys[i], xs[j], ys[j], qx, qy) <= r2)
            return 1;
    }
    return 0;
}

uint8_t Map::point_in_feature(TerrainFeature const &f, float x, float y) {
    if (!feature_near_circle(f, x, y, 0.f)) return 0;
    if (f.shape == TerrainShape::kRiver || f.shape == TerrainShape::kSpiral) {
        float qx, qy, hw, nx, ny, dist;
        closest_on_path(f, x, y, qx, qy, hw, nx, ny, dist);
        return dist <= hw ? 1 : 0;
    }
    if (f.shape == TerrainShape::kCircle || f.shape == TerrainShape::kEllipse) {
        float cx = f.left, cy = f.top;
        float rx = f.right, ry = f.bottom;
        if (f.shape == TerrainShape::kCircle) {
            rx = f.right > 0.f ? f.right : f.bottom;
            ry = rx;
        }
        if (rx <= 0.f || ry <= 0.f) return 0;
        float dx = x - cx, dy = y - cy;
        float ang = std::atan2(dy, dx);
        float s = radial_scale_at_angle(feature_seed(f), jagged_amp(f), ang);
        float nx = dx / (rx * s);
        float ny = dy / (ry * s);
        return (nx * nx + ny * ny) <= 1.f ? 1 : 0;
    }
    // kAABB — jagged outline PIP (matches drawn shore)
    return point_in_jagged_box(f, x, y);
}

static uint8_t circle_hits_feature(TerrainFeature const &f, float x, float y, float radius) {
    if (!feature_near_circle(f, x, y, radius)) return 0;
    if (f.shape == TerrainShape::kRiver || f.shape == TerrainShape::kSpiral) {
        float qx, qy, hw, nx, ny, dist;
        closest_on_path(f, x, y, qx, qy, hw, nx, ny, dist);
        return dist <= hw + radius ? 1 : 0;
    }
    if (f.shape == TerrainShape::kCircle || f.shape == TerrainShape::kEllipse) {
        float cx = f.left, cy = f.top;
        float rx = f.right, ry = f.bottom;
        if (f.shape == TerrainShape::kCircle) {
            rx = f.right > 0.f ? f.right : f.bottom;
            ry = rx;
        }
        if (rx <= 0.f || ry <= 0.f) return 0;
        float dx = x - cx, dy = y - cy;
        float ang = std::atan2(dy, dx);
        float s = radial_scale_at_angle(feature_seed(f), jagged_amp(f), ang);
        // Inflate by player radius along the same jagged normal
        float nx = dx / (rx * s + radius);
        float ny = dy / (ry * s + radius);
        return (nx * nx + ny * ny) <= 1.f ? 1 : 0;
    }
    return circle_hits_jagged_box(f, x, y, radius);
}

namespace {
// Immutable terrain gets its own broad phase, like florr_clone's enemyGrid.
// CCD asks this many times per moving entity; scanning the entire map made
// server tick cost grow with every mob and loot pile left in the world.
constexpr int kTerrainCell = 512;
constexpr int kTerrainCols = (ARENA_WIDTH + kTerrainCell - 1) / kTerrainCell;
constexpr int kTerrainRows = (ARENA_HEIGHT + kTerrainCell - 1) / kTerrainCell;
struct TerrainIndex {
    uint32_t feature_count = 0xffffffffu;
    std::array<std::vector<uint16_t>, kTerrainCols * kTerrainRows> cells;
    std::array<uint32_t, MAX_TERRAIN_PER_MAP> seen{};
    uint32_t generation = 0;
};
std::array<TerrainIndex, MapID::kNumMaps> terrain_indices;

int terrain_cell(float v, int count) {
    return (int)(fclamp(v, 0.f, (float)(count * kTerrainCell - 1)) / kTerrainCell);
}

TerrainIndex &terrain_index(uint8_t map_id, MapTerrain const &terrain) {
    TerrainIndex &index = terrain_indices[map_id];
    if (index.feature_count == terrain.size()) return index;
    // MapGen appends gates while validating connectivity; refresh the index
    // during those passes too, so newly carved corridors are immediately usable.
    for (auto &cell : index.cells) cell.clear();
    for (uint16_t i = 0; i < terrain.size(); ++i) {
        TerrainFeature const &f = terrain[i];
        if (f.kind != TerrainKind::kChannel && !(f.flags & TerrainFlags::kImpassable)) continue;
        float x0, y0, x1, y1;
        feature_bounds(f, 0.f, x0, y0, x1, y1);
        for (int cy = terrain_cell(y0, kTerrainRows); cy <= terrain_cell(y1, kTerrainRows); ++cy)
            for (int cx = terrain_cell(x0, kTerrainCols); cx <= terrain_cell(x1, kTerrainCols); ++cx)
                index.cells[cy * kTerrainCols + cx].push_back(i);
    }
    index.feature_count = terrain.size();
    return index;
}
} // namespace

uint8_t Map::circle_hits_impassable(uint8_t map_id, float x, float y, float radius) {
    MapTerrain const &terrain = map_terrain(map_id);
    TerrainIndex &index = terrain_index(map_id, terrain);
    if (++index.generation == 0) {
        index.seen.fill(0);
        index.generation = 1;
    }
    StaticArray<uint16_t, MAX_TERRAIN_PER_MAP> candidates;
    int x0 = terrain_cell(x - radius, kTerrainCols), x1 = terrain_cell(x + radius, kTerrainCols);
    int y0 = terrain_cell(y - radius, kTerrainRows), y1 = terrain_cell(y + radius, kTerrainRows);
    for (int cy = y0; cy <= y1; ++cy) {
        for (int cx = x0; cx <= x1; ++cx) {
            for (uint16_t i : index.cells[cy * kTerrainCols + cx]) {
                if (index.seen[i] == index.generation) continue;
                index.seen[i] = index.generation;
                candidates.push(i);
            }
        }
    }
    // Safe arteries carve walkable corridors through hills/water.
    // Use circle vs channel (not just center point) so edges don't trap players.
    for (uint16_t i : candidates) {
        TerrainFeature const &f = terrain[i];
        if (f.kind != TerrainKind::kChannel) continue;
        if (circle_hits_feature(f, x, y, radius)) return 0;
    }
    for (uint16_t i : candidates) {
        TerrainFeature const &f = terrain[i];
        if (!(f.flags & TerrainFlags::kImpassable)) continue;
        if (circle_hits_feature(f, x, y, radius)) return 1;
    }
    return 0;
}

static uint8_t push_out_path(TerrainFeature const &f, float &x, float &y, float radius) {
    float qx, qy, hw, nx, ny, dist;
    closest_on_path(f, x, y, qx, qy, hw, nx, ny, dist);
    float need = hw + radius;
    if (dist >= need) return 0;
    // Slide out along bank normal (ribbon collision), not a freeze clamp.
    float nlen = std::sqrt(nx * nx + ny * ny);
    if (nlen < 1e-3f) { nx = 1.f; ny = 0.f; nlen = 1.f; }
    else { nx /= nlen; ny /= nlen; }
    x = qx + nx * (need + 0.5f);
    y = qy + ny * (need + 0.5f);
    return 1;
}

static uint8_t in_arena_band(float x, float y, float radius) {
    return x >= radius && x <= (float)ARENA_WIDTH - radius
        && y >= radius && y <= (float)ARENA_HEIGHT - radius;
}

static uint8_t push_out_smooth_aabb(TerrainFeature const &f, float &x, float &y, float radius) {
    float nearest_x = fclamp(x, f.left, f.right);
    float nearest_y = fclamp(y, f.top, f.bottom);
    float dx = x - nearest_x;
    float dy = y - nearest_y;
    float dist2 = dx * dx + dy * dy;
    if (dist2 > radius * radius) return 0;
    if (dist2 < 1e-6f) {
        // Deep inside the box: try all four exits. Edge berms sit on the arena
        // border — pushing "outward" lands past the map, then Motion's fclamp
        // pins the flower back inside the berm (feels like movement freeze).
        float const exits[4][2] = {
            { f.left - radius - 0.5f, y },
            { f.right + radius + 0.5f, y },
            { x, f.top - radius - 0.5f },
            { x, f.bottom + radius + 0.5f },
        };
        float const depths[4] = {
            x - f.left, f.right - x, y - f.top, f.bottom - y
        };
        int best = -1;
        float best_d = 1e12f;
        for (int i = 0; i < 4; ++i) {
            if (!in_arena_band(exits[i][0], exits[i][1], radius)) continue;
            if (depths[i] < best_d) { best_d = depths[i]; best = i; }
        }
        if (best >= 0) {
            x = exits[best][0];
            y = exits[best][1];
            return 1;
        }
        // No in-arena face (degenerate edge strip) — shove toward map center.
        float cx = (float)ARENA_WIDTH * 0.5f;
        float cy = (float)ARENA_HEIGHT * 0.5f;
        float vx = cx - x, vy = cy - y;
        float vl = std::sqrt(vx * vx + vy * vy);
        if (vl < 1e-3f) { vx = 1.f; vy = 0.f; vl = 1.f; }
        x += vx / vl * (radius + 8.f);
        y += vy / vl * (radius + 8.f);
        return 1;
    }
    float dist = std::sqrt(dist2);
    float push = (radius - dist) / dist + 0.001f;
    float nx = x + dx * push;
    float ny = y + dy * push;
    // Natural push left the arena (edge berm) — pick nearest in-arena face instead.
    if (!in_arena_band(nx, ny, radius)) {
        float const exits[4][2] = {
            { f.left - radius - 0.5f, y },
            { f.right + radius + 0.5f, y },
            { x, f.top - radius - 0.5f },
            { x, f.bottom + radius + 0.5f },
        };
        float const depths[4] = {
            std::fabs(x - (f.left - radius)), std::fabs(x - (f.right + radius)),
            std::fabs(y - (f.top - radius)), std::fabs(y - (f.bottom + radius))
        };
        int best = -1;
        float best_d = 1e12f;
        for (int i = 0; i < 4; ++i) {
            if (!in_arena_band(exits[i][0], exits[i][1], radius)) continue;
            if (depths[i] < best_d) { best_d = depths[i]; best = i; }
        }
        if (best >= 0) { nx = exits[best][0]; ny = exits[best][1]; }
        else {
            float cx = (float)ARENA_WIDTH * 0.5f, cy = (float)ARENA_HEIGHT * 0.5f;
            float vx = cx - x, vy = cy - y;
            float vl = std::sqrt(vx * vx + vy * vy);
            if (vl > 1e-3f) { nx = x + vx / vl * (radius + 8.f); ny = y + vy / vl * (radius + 8.f); }
        }
    }
    x = nx;
    y = ny;
    return 1;
}

static uint8_t push_out_jagged_box(TerrainFeature const &f, float &x, float &y, float radius) {
    // Interior of smooth box: cheap axis push (jagged only adds outside protrusions)
    if (x >= f.left && x <= f.right && y >= f.top && y <= f.bottom)
        return push_out_smooth_aabb(f, x, y, radius);
    float amp = aabb_jagged_amp(f);
    if (x < f.left - amp - radius || x > f.right + amp + radius
        || y < f.top - amp - radius || y > f.bottom + amp + radius)
        return 0;
    float xs[64], ys[64];
    uint32_t n = Map::terrain_outline(f, xs, ys, 64);
    if (n < 3) return 0;
    uint8_t inside = point_in_polygon(xs, ys, n, x, y);
    float best_d2 = 1e12f;
    float bqx = x, bqy = y;
    for (uint32_t i = 0; i < n; ++i) {
        uint32_t j = (i + 1) % n;
        float qx, qy;
        float d2 = dist2_point_seg(x, y, xs[i], ys[i], xs[j], ys[j], qx, qy);
        if (d2 < best_d2) { best_d2 = d2; bqx = qx; bqy = qy; }
    }
    float dx = x - bqx, dy = y - bqy;
    float dist = std::sqrt(best_d2);
    if (!inside && dist > radius) return 0;
    if (dist < 1e-3f) {
        // Degenerate: push from polygon centroid
        float cx = 0.f, cy = 0.f;
        for (uint32_t i = 0; i < n; ++i) { cx += xs[i]; cy += ys[i]; }
        cx /= (float)n; cy /= (float)n;
        dx = x - cx; dy = y - cy;
        dist = std::sqrt(dx * dx + dy * dy);
        if (dist < 1e-3f) { dx = 1.f; dy = 0.f; dist = 1.f; }
    }
    float need = inside ? (dist + radius + 0.5f) : (radius + 0.5f);
    float scale = need / dist;
    x = bqx + dx * scale;
    y = bqy + dy * scale;
    // If we were inside, normal may point inward — flip if still inside
    if (point_in_polygon(xs, ys, n, x, y)) {
        x = bqx - dx * scale;
        y = bqy - dy * scale;
    }
    return 1;
}

static uint8_t push_out_feature(TerrainFeature const &f, float &x, float &y, float radius) {
    if (!feature_near_circle(f, x, y, radius)) return 0;
    if (f.shape == TerrainShape::kRiver || f.shape == TerrainShape::kSpiral)
        return push_out_path(f, x, y, radius);
    if (f.shape == TerrainShape::kCircle || f.shape == TerrainShape::kEllipse) {
        float cx = f.left, cy = f.top;
        float rx = f.right, ry = f.bottom;
        if (f.shape == TerrainShape::kCircle) {
            rx = f.right > 0.f ? f.right : f.bottom;
            ry = rx;
        }
        if (rx <= 0.f || ry <= 0.f) return 0;
        float dx = x - cx, dy = y - cy;
        float d2 = dx * dx + dy * dy;
        float ang = (d2 > 1e-8f) ? std::atan2(dy, dx) : 0.f;
        float s = radial_scale_at_angle(feature_seed(f), jagged_amp(f), ang);
        float erx = rx * s + radius;
        float ery = ry * s + radius;
        float nx = dx / erx, ny = dy / ery;
        float nd2 = nx * nx + ny * ny;
        if (nd2 > 1.f) return 0;
        if (nd2 <= 0.f) { x = cx + erx; y = cy; return 1; }
        float nd = std::sqrt(nd2);
        float scale = 1.001f / nd;
        x = cx + dx * scale;
        y = cy + dy * scale;
        return 1;
    }
    return push_out_jagged_box(f, x, y, radius);
}

// Closed outline for fill. Hills = sharp jagged (clone wall edges);
// water uses same samples (client smooths with qcurve).
uint32_t Map::terrain_outline(TerrainFeature const &f, float *xs, float *ys, uint32_t cap) {
    if (cap < 8) return 0;
    uint32_t seed = feature_seed(f);

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
    // Standing on a safe artery: do not shove the player out of overlapping hills.
    for (TerrainFeature const &ch : terrain) {
        if (ch.kind == TerrainKind::kChannel && circle_hits_feature(ch, x, y, radius))
            return 0;
    }
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

void Map::eject_from_impassable(uint8_t map_id, float &x, float &y, float radius) {
    if (!circle_hits_impassable(map_id, x, y, radius)) return;
    // First try normal multi-pass resolve (now prefers in-arena exits).
    resolve_circle_terrain(map_id, x, y, radius);
    x = fclamp(x, radius, (float)ARENA_WIDTH - radius);
    y = fclamp(y, radius, (float)ARENA_HEIGHT - radius);
    if (!circle_hits_impassable(map_id, x, y, radius)) return;

    // Still trapped in an edge berm / corner: walk toward map center.
    float cx = (float)ARENA_WIDTH * 0.5f;
    float cy = (float)ARENA_HEIGHT * 0.5f;
    float vx = cx - x, vy = cy - y;
    float vl = std::sqrt(vx * vx + vy * vy);
    if (vl < 1e-3f) return;
    vx /= vl; vy /= vl;
    float step = radius > 10.f ? radius * 0.5f : 10.f;
    for (float d = step; d <= 900.f; d += step) {
        float nx = fclamp(x + vx * d, radius, (float)ARENA_WIDTH - radius);
        float ny = fclamp(y + vy * d, radius, (float)ARENA_HEIGHT - radius);
        if (!circle_hits_impassable(map_id, nx, ny, radius)) {
            x = nx;
            y = ny;
            return;
        }
    }
}

// Binary-search the farthest free point on the segment [from → to].
static void ccd_approach(uint8_t map_id, float &x, float &y,
                         float from_x, float from_y, float to_x, float to_y, float radius) {
    float ax = from_x, ay = from_y, bx = to_x, by = to_y;
    for (int k = 0; k < 7; ++k) {
        float mx = (ax + bx) * 0.5f;
        float my = (ay + by) * 0.5f;
        if (Map::circle_hits_impassable(map_id, mx, my, radius)) {
            bx = mx; by = my;
        } else {
            ax = mx; ay = my;
        }
    }
    x = ax;
    y = ay;
}

void Map::ccd_move_circle(uint8_t map_id, float &x, float &y, float ox, float oy, float radius) {
    float cur_x = ox, cur_y = oy;
    // Already inside a wall (berm clamp / dual-river corner): eject first so the
    // rest of the move can slide instead of freezing at the penetration point.
    if (circle_hits_impassable(map_id, cur_x, cur_y, radius))
        eject_from_impassable(map_id, cur_x, cur_y, radius);

    float goal_x = x, goal_y = y;
    float dx0 = goal_x - cur_x, dy0 = goal_y - cur_y;
    float dist0 = std::sqrt(dx0 * dx0 + dy0 * dy0);
    if (!(dist0 > 0.f) || dist0 != dist0) {
        x = cur_x; y = cur_y;
        resolve_circle_terrain(map_id, x, y, radius);
        return;
    }

    float step = radius > 8.f ? radius * 0.5f : 8.f;
    auto slide_toward = [&](float &sx, float &sy) {
        if (!circle_hits_impassable(map_id, goal_x, sy, radius)) {
            sx = goal_x;
        } else if (std::fabs(goal_x - sx) > 0.5f) {
            ccd_approach(map_id, sx, sy, sx, sy, goal_x, sy, radius);
        }
        if (!circle_hits_impassable(map_id, sx, goal_y, radius)) {
            sy = goal_y;
        } else if (std::fabs(goal_y - sy) > 0.5f) {
            float tmp_x = sx;
            ccd_approach(map_id, tmp_x, sy, sx, sy, sx, goal_y, radius);
            sx = tmp_x;
        }
        float rem_x = goal_x - sx;
        float rem_y = goal_y - sy;
        float rem = std::sqrt(rem_x * rem_x + rem_y * rem_y);
        if (rem <= 0.75f) return;
        float best = 0.f;
        float bx = sx, by = sy;
        float slide = rem < step * 2.5f ? rem : step * 2.5f;
        for (int k = 0; k < 24; ++k) {
            float a = (float)k * (float)(2.0 * M_PI / 24.0);
            float cx = sx + std::cos(a) * slide;
            float cy = sy + std::sin(a) * slide;
            if (circle_hits_impassable(map_id, cx, cy, radius)) continue;
            float prog = (cx - sx) * rem_x + (cy - sy) * rem_y;
            if (prog > best) { best = prog; bx = cx; by = cy; }
        }
        if (best > 0.f) { sx = bx; sy = by; }
    };

    // Up to a few wall contacts per tick — keep sliding along, never hard-stop.
    for (int bounce = 0; bounce < 4; ++bounce) {
        float dx = goal_x - cur_x;
        float dy = goal_y - cur_y;
        float dist = std::sqrt(dx * dx + dy * dy);
        if (!(dist > 0.5f) || dist != dist) break;

        int steps = (int) std::ceil(dist / step);
        if (steps < 1) steps = 1;
        if (steps > 24) steps = 24;
        float px = cur_x, py = cur_y;
        uint8_t hit = 0;
        for (int i = 1; i <= steps; ++i) {
            float tx = cur_x + dx * ((float) i / (float) steps);
            float ty = cur_y + dy * ((float) i / (float) steps);
            if (!circle_hits_impassable(map_id, tx, ty, radius)) {
                px = tx; py = ty;
                continue;
            }
            float sx = px, sy = py;
            ccd_approach(map_id, sx, sy, px, py, tx, ty, radius);
            resolve_circle_terrain(map_id, sx, sy, radius);
            slide_toward(sx, sy);
            resolve_circle_terrain(map_id, sx, sy, radius);
            float moved = std::sqrt((sx - px) * (sx - px) + (sy - py) * (sy - py));
            if (moved < 0.2f && circle_hits_impassable(map_id, sx, sy, radius))
                eject_from_impassable(map_id, sx, sy, radius);
            // No progress toward goal → stop bouncing (corner / sealed pocket).
            float before = (goal_x - px) * (goal_x - px) + (goal_y - py) * (goal_y - py);
            float after = (goal_x - sx) * (goal_x - sx) + (goal_y - sy) * (goal_y - sy);
            cur_x = sx; cur_y = sy;
            hit = 1;
            if (after >= before - 0.5f) bounce = 4; // force exit
            break;
        }
        if (!hit) {
            cur_x = px;
            cur_y = py;
            break;
        }
    }

    x = cur_x;
    y = cur_y;
    resolve_circle_terrain(map_id, x, y, radius);
    if (circle_hits_impassable(map_id, x, y, radius))
        eject_from_impassable(map_id, x, y, radius);
}

uint32_t Map::sample_connectivity(uint8_t map_id, float step) {
    if (step < 80.f) step = 80.f;
    uint32_t gw = (uint32_t) std::ceil((float)ARENA_WIDTH / step) + 1;
    uint32_t gh = (uint32_t) std::ceil((float)ARENA_HEIGHT / step) + 1;
    if (gw * gh > 200000) return 0; // safety
    std::vector<uint8_t> walk(gw * gh, 0);
    auto idx = [&](uint32_t ix, uint32_t iy) { return iy * gw + ix; };
    auto world = [&](uint32_t ix, uint32_t iy, float &x, float &y) {
        x = (float)ix * step;
        y = (float)iy * step;
    };
    float rad = 40.f;
    for (uint32_t iy = 0; iy < gh; ++iy) {
        for (uint32_t ix = 0; ix < gw; ++ix) {
            float x, y;
            world(ix, iy, x, y);
            if (x < rad || y < rad || x > ARENA_WIDTH - rad || y > ARENA_HEIGHT - rad)
                continue;
            if (!circle_hits_impassable(map_id, x, y, rad))
                walk[idx(ix, iy)] = 1;
        }
    }
    // BFS from spawn
    uint32_t sx = (uint32_t)(MAP_SPAWN_X / step);
    uint32_t sy = (uint32_t)(MAP_SPAWN_Y / step);
    if (sx >= gw) sx = gw / 2;
    if (sy >= gh) sy = gh / 2;
    // Find nearest walkable cell to spawn
    for (int r = 0; r < 12; ++r) {
        bool found = false;
        for (int dy = -r; dy <= r && !found; ++dy) {
            for (int dx = -r; dx <= r && !found; ++dx) {
                int ix = (int)sx + dx, iy = (int)sy + dy;
                if (ix < 0 || iy < 0 || (uint32_t)ix >= gw || (uint32_t)iy >= gh) continue;
                if (walk[idx((uint32_t)ix, (uint32_t)iy)]) {
                    sx = (uint32_t)ix; sy = (uint32_t)iy; found = true;
                }
            }
        }
        if (found) break;
    }
    if (!walk[idx(sx, sy)]) return 0;
    std::vector<uint8_t> seen(gw * gh, 0);
    std::vector<uint32_t> q;
    q.push_back(idx(sx, sy));
    seen[idx(sx, sy)] = 1;
    for (size_t qi = 0; qi < q.size(); ++qi) {
        uint32_t cur = q[qi];
        uint32_t cx = cur % gw, cy = cur / gw;
        int nbs[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
        for (auto &nb : nbs) {
            int nx = (int)cx + nb[0], ny = (int)cy + nb[1];
            if (nx < 0 || ny < 0 || (uint32_t)nx >= gw || (uint32_t)ny >= gh) continue;
            uint32_t ni = idx((uint32_t)nx, (uint32_t)ny);
            if (seen[ni] || !walk[ni]) continue;
            seen[ni] = 1;
            q.push_back(ni);
        }
    }
    // Goals: zone centers
    MapZones const &zones = map_zones(map_id);
    uint32_t reachable = 0;
    for (uint32_t z = 0; z < zones.size(); ++z) {
        float zx = 0.5f * (zones[z].left + zones[z].right);
        float zy = 0.5f * (zones[z].top + zones[z].bottom);
        uint32_t ix = (uint32_t)(zx / step);
        uint32_t iy = (uint32_t)(zy / step);
        if (ix >= gw) ix = gw - 1;
        if (iy >= gh) iy = gh - 1;
        // Prefer the center, but a center may itself be a hill or water feature.
        // A zone is accessible when any walkable point inside its bounds is
        // connected to spawn.
        bool ok = false;
        for (int r = 0; r < 10 && !ok; ++r) {
            for (int dy = -r; dy <= r && !ok; ++dy) {
                for (int dx = -r; dx <= r && !ok; ++dx) {
                    int tx = (int)ix + dx, ty = (int)iy + dy;
                    if (tx < 0 || ty < 0 || (uint32_t)tx >= gw || (uint32_t)ty >= gh) continue;
                    uint32_t ti = idx((uint32_t)tx, (uint32_t)ty);
                    if (seen[ti]) ok = true;
                }
            }
        }
        if (!ok) {
            uint32_t lx = (uint32_t)std::max(0.f, zones[z].left / step);
            uint32_t rx = std::min(gw - 1, (uint32_t)(zones[z].right / step));
            uint32_t ty = (uint32_t)std::max(0.f, zones[z].top / step);
            uint32_t by = std::min(gh - 1, (uint32_t)(zones[z].bottom / step));
            for (uint32_t y = ty; y <= by && !ok; ++y)
                for (uint32_t x = lx; x <= rx && !ok; ++x)
                    if (seen[idx(x, y)]) ok = true;
        }
        if (ok) ++reachable;
#ifdef SERVERSIDE
        else std::cout << "[map] unreachable zone " << (uint32_t)map_id << ": "
                       << zones[z].name << " (" << z << ") ["
                       << zones[z].left << "," << zones[z].top << " .. "
                       << zones[z].right << "," << zones[z].bottom << "]\n";
#endif
    }
    return reachable;
}

// 是否位于道路通道内 —— 这类区域不刷怪（可攻击）。Shared with MapGen.
uint8_t Map::in_safe_channel(uint8_t map_id, float x, float y) {
    MapTerrain const &terrain = map_terrain(map_id);
    for (TerrainFeature const &f : terrain) {
        if (f.kind != TerrainKind::kChannel) continue;
        if (point_in_feature(f, x, y)) return 1;
    }
    return 0;
}

#ifdef SERVERSIDE
#include <Shared/Simulation.hh>
void Map::remove_mob(Simulation *sim, uint32_t zone) {
    DEBUG_ONLY(assert(zone < map_zones(sim->map_id).size());)
    if (sim->zone_mob_counts[zone] > 0)
        --sim->zone_mob_counts[zone];
}

// 广场（出生区）里的怪单独计数：整个 hub zone 计数无法区分广场内外的怪。
void Map::remove_mob(Simulation *sim, Entity const &ent) {
    uint32_t zone = ent.zone;
    if (zone >= map_zones(sim->map_id).size()) return;
    // Guard underflow — double remove (reclaim + entity_on_death) used to wrap
    // uint32_t and trigger spawn storms → entity-cap SEGV loops.
    if (sim->zone_mob_counts[zone] > 0)
        --sim->zone_mob_counts[zone];
    if (in_spawn_plaza(ent.get_x(), ent.get_y()) && sim->plaza_mob_count > 0)
        --sim->plaza_mob_count;
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

// Zone spawn rarity: primary/adjacent tilt, never below the mob's baked base.
// Cap Unique to Unique-difficulty rings only — Square's baked Unique used to
// force Unique in Common hubs ("低稀有度区全是唯一怪").
static uint8_t resolve_zone_spawn_rarity(uint8_t base_r, uint32_t zone_difficulty) {
    uint8_t spawn_r = pick_zone_spawn_rarity(zone_difficulty);
    if (base_r < RarityID::kNumRarities && spawn_r < base_r)
        spawn_r = base_r;
    uint8_t cap = zone_spawn_rarity_cap(zone_difficulty);
    if (spawn_r > cap) spawn_r = cap;
    return spawn_r;
}

// Zone dens soft floor only — crowd throttle lives in Simulation spawn budget.
// Former per-flower ×0.82 crush starved combat FOVs while abs_cap already gated.
static float soft_zone_density_mul(Simulation *, uint32_t difficulty) {
    // Soft hubs a bit denser; mid/high mild — screen feel from local_combat_target.
    return difficulty > RarityID::kUnusual ? 1.02f : 1.12f;
}

// Hub pad must stay open for respawn / dummies. Was 16 — kite + admin spawn
// still packed the ellipse because thinning only trusted plaza_mob_count
// (spawned-in-plaza) and ignored mobs walked / admin-dropped into the pad.
static constexpr uint32_t kPlazaMobCap = 4;
// Zone abs caps = safety valve only. Combat feel = local_combat_target.
// HK 8c/8G: roomy valve vs BJ 4c (was 72–136 / 5600, then 96–160 / 8000).
static constexpr uint32_t kSoftAbsCapCommon = 104;
static constexpr uint32_t kSoftAbsCapUnusual = 120;
static constexpr uint32_t kSoftAbsCapRare = 136;
static constexpr uint32_t kSoftAbsCapEpic = 152;
static constexpr uint32_t kSoftAbsCapLegendaryPlus = 168;
// Stop/thin earlier than ENTITY_CAP=8192. Crowd far-FOV cull kicks in well
// below cap — snow/forest were holding 1k+ ents with 1–5 players → collide 1006.
static constexpr uint32_t kEntitySpawnStop = 8050;
static constexpr uint32_t kEntityThinPanic = 9000;
static constexpr uint32_t kEntityThinCrowd = 7600;
static constexpr uint32_t kEntityThinFar = 900; // enable far-FOV cull sooner

static uint32_t soft_zone_abs_cap(uint32_t difficulty) {
    if (difficulty == 0) return kSoftAbsCapCommon;
    if (difficulty <= RarityID::kUnusual) return kSoftAbsCapUnusual;
    if (difficulty <= RarityID::kRare) return kSoftAbsCapRare;
    if (difficulty <= RarityID::kEpic) return kSoftAbsCapEpic;
    return kSoftAbsCapLegendaryPlus;
}

static float soft_zone_spawn_target(Simulation *sim, uint32_t zone_id,
                                   ZoneDefinition const &zone) {
    float dens = zone.density * soft_zone_density_mul(sim, zone.difficulty);
    float target = dens * zone_exclusive_area(sim->map_id, zone_id) / (500.f * 500.f);
    float abs_cap = (float)soft_zone_abs_cap(zone.difficulty);
    if (target > abs_cap) target = abs_cap;
    return target;
}

// Combat FOV ≈ base screen + margin (not the oversized spawn_aoi reject box).
static void combat_fov_half(float &hw, float &hh) {
    hw = 960.f / BASE_FOV + 280.f;
    hh = 540.f / BASE_FOV + 280.f;
}

// Screen wildlife target — ~16/viewport (florr_clone); soft mul under crowd.
static float local_combat_target(uint32_t difficulty, uint32_t flowers) {
    float t = 16.f;
    if (difficulty > RarityID::kUnusual) t *= 0.94f;
    if (flowers >= 4u) t *= 0.92f;
    if (flowers >= 8u) t *= 0.85f;
    if (flowers >= 12u) t *= 0.78f;
    return t;
}

static uint32_t count_alive_flowers(Simulation *sim) {
    uint32_t n = 0;
    sim->for_each<kFlower>([&](Simulation *, Entity &f) {
        if (f.has_component(kMob)) return;
        if (!f.has_component(kHealth)) return;
        ++n;
    });
    return n;
}

// Segments / hole children inflate the pool while zone_mob_counts stays +1.
static uint32_t spawn_entity_cost(MobID::T mob_id) {
    if (mob_id >= MobID::kNumMobs) return 1;
    uint32_t segs = MOB_DATA[mob_id].attributes.segments;
    if (segs > 1) return segs;
    if (mob_id == MobID::kAntHole) return 7; // hole + 6 ants (Spawn.cc)
    return 1;
}

static uint8_t is_countable_zone_wildlife(Entity &ent) {
    if (ent.pending_delete) return 0;
    if (!ent.has_component(kMob)) return 0;
    if (!BitMath::at(ent.flags, EntityFlags::kSpawnedFromZone)) return 0;
    if (ent.zone == 0xFF) return 0;
    if (ent.get_mob_id() == MobID::kTrainingDummy) return 0;
    return 1;
}

static uint32_t count_zone_wildlife_in_box(Simulation *sim, float cx, float cy,
                                          float hw, float hh) {
    uint32_t n = 0;
    sim->for_each<kMob>([&](Simulation *, Entity &ent) {
        if (!is_countable_zone_wildlife(ent)) return;
        if (std::fabs(ent.get_x() - cx) > hw || std::fabs(ent.get_y() - cy) > hh)
            return;
        ++n;
    });
    return n;
}

uint8_t Map::any_combat_fov_underfilled(Simulation *sim, float ratio) {
    if (ratio < 0.f) ratio = 0.f;
    float hw = 0.f, hh = 0.f;
    combat_fov_half(hw, hh);
    uint32_t flowers = count_alive_flowers(sim);
    uint8_t under = 0;
    sim->for_each<kFlower>([&](Simulation *s, Entity &f) {
        if (under || f.has_component(kMob)) return;
        if (!f.has_component(kHealth)) return;
        uint32_t z = Map::get_zone_from_pos(s->map_id, f.get_x(), f.get_y());
        MapZones const &zones = map_zones(s->map_id);
        uint32_t diff = 0;
        if (z < zones.size()) diff = zones[z].difficulty;
        float need = local_combat_target(diff, flowers) * ratio;
        if ((float)count_zone_wildlife_in_box(s, f.get_x(), f.get_y(), hw, hh) < need)
            under = 1;
    });
    return under;
}

void Map::thin_soft_zone_excess(Simulation *sim) {
    MapZones const &zones = map_zones(sim->map_id);
    uint32_t const ents = sim->entity_count();
    uint32_t const flowers = count_alive_flowers(sim);
    uint8_t const panic = ents >= kEntityThinPanic;
    // Far cull even with few players once biome packs up (yesterday midnight).
    uint8_t const far = ents >= kEntityThinFar || flowers >= 3u;
    uint8_t const crowd = panic || ents >= kEntityThinCrowd || flowers >= 10u || far;
    float combat_hw = 0.f, combat_hh = 0.f;
    combat_fov_half(combat_hw, combat_hh);
    // Snapshot flower positions once (crowd far-from-all-FOV deletes).
    StaticArray<float, 128> flower_xy;
    if (crowd) {
        sim->for_each<kFlower>([&](Simulation *, Entity &f) {
            if (f.has_component(kMob) || !f.has_component(kHealth)) return;
            if (flower_xy.size() + 2 > 128u) return;
            flower_xy.push(f.get_x());
            flower_xy.push(f.get_y());
        });
    }
    auto near_any_combat_fov = [&](float mx, float my) -> uint8_t {
        uint32_t n = flower_xy.size();
        for (uint32_t i = 0; i + 1 < n; i += 2) {
            if (std::fabs(flower_xy[i] - mx) <= combat_hw
                && std::fabs(flower_xy[i + 1] - my) <= combat_hh)
                return 1;
        }
        return 0;
    };
    for (uint32_t z = 0; z < zones.size(); ++z) {
        ZoneDefinition const &zone = zones[z];
        // Safety-valve drain — prefer culled; under crowd also cull packs that
        // sit outside every living flower's combat FOV (stacked AOIs).
        float target = soft_zone_spawn_target(sim, z, zone);
        float count = (float)sim->zone_mob_counts[z];
        // clone adjustEnemyCount: drain excess toward FOV/zone target under load.
        float slack = panic ? 0.5f : (crowd ? 1.0f : 2.5f);
        if (count <= target + slack) continue;
        float over = count - target;
        float rate = panic ? 0.40f : (crowd ? 0.32f : 0.18f);
        if (far && !panic) rate = std::max(rate, 0.30f);
        uint32_t budget = (uint32_t)(over * rate + 0.99f);
        if (budget < 1) budget = 1;
        uint32_t budget_cap = panic ? 36u : (crowd ? 24u : 10u);
        if (far && budget_cap < 20u) budget_cap = 20u;
        if (budget > budget_cap) budget = budget_cap;
        sim->for_each<kMob>([&](Simulation *s, Entity &ent) {
            if (budget == 0) return;
            if (ent.pending_delete) return;
            if (ent.zone != z) return;
            if (!BitMath::at(ent.flags, EntityFlags::kSpawnedFromZone)) return;
            if (BitMath::at(ent.flags, EntityFlags::kIsDespawning)) return;
            uint8_t culled = BitMath::at(ent.flags, EntityFlags::kIsCulled);
            if (!culled) {
                if (!crowd || flower_xy.size() < 2) return;
                if (near_any_combat_fov(ent.get_x(), ent.get_y())) return;
            }
            BitMath::set(ent.flags, EntityFlags::kNoDrops);
            s->request_delete(ent.id);
            --budget;
        });
    }
    // Live plaza census — kite packs + admin drops never bumped plaza_mob_count.
    {
        uint32_t live = 0;
        sim->for_each<kMob>([&](Simulation *, Entity &ent) {
            if (ent.pending_delete) return;
            if (!in_spawn_plaza(ent.get_x(), ent.get_y())) return;
            // Keep dummies / map boss / explore props.
            if (ent.get_mob_id() == MobID::kTrainingDummy) return;
            if (ent.zone == 0xFF) return;
            if (ent.damage <= 0.f && ent.score_reward == 0
                && BitMath::at(ent.flags, EntityFlags::kNoDrops))
                return;
            ++live;
        });
        sim->plaza_mob_count = live;
        if (live > kPlazaMobCap) {
            uint32_t budget = live - kPlazaMobCap;
            if (budget > 8) budget = 8;
            sim->for_each<kMob>([&](Simulation *s, Entity &ent) {
                if (budget == 0) return;
                if (ent.pending_delete) return;
                if (!in_spawn_plaza(ent.get_x(), ent.get_y())) return;
                if (ent.get_mob_id() == MobID::kTrainingDummy) return;
                if (ent.zone == 0xFF) return;
                if (ent.damage <= 0.f && ent.score_reward == 0
                    && BitMath::at(ent.flags, EntityFlags::kNoDrops))
                    return;
                BitMath::set(ent.flags, EntityFlags::kNoDrops);
                s->request_delete(ent.id);
                --budget;
            });
        }
    }
}

void Map::spawn_random_mob(Simulation *sim, float x, float y) {
    uint32_t zone_id = Map::get_zone_from_pos(sim->map_id, x, y);
    struct ZoneDefinition const &zone = map_zones(sim->map_id)[zone_id];
    // Respawn plaza: never auto-spawn — hub stays clear for players / dummies.
    if (in_spawn_plaza(x, y)) return;
    if ((float)sim->zone_mob_counts[zone_id] >= soft_zone_spawn_target(sim, zone_id, zone))
        return;
    float sum = 0;
    for (SpawnChance const &s : zone.spawns)
        sum += s.chance;
    sum *= frand();
    for (SpawnChance const &s : zone.spawns) {
        sum -= s.chance;
        if (sum <= 0) {
            uint8_t base_r = MOB_DATA[s.id].rarity;
            uint8_t spawn_r = resolve_zone_spawn_rarity(base_r, zone.difficulty);
            float radius = MOB_DATA[s.id].radius.get_single(1.f) * MOB_RADIUS_MULTIPLIER
                * mob_spawn_radius_mult(base_r, spawn_r);
            if (Map::circle_hits_impassable(sim->map_id, x, y, radius + 8.f)) return;
            _spawn_mob_at(sim, zone_id, s.id, x, y, spawn_r);
            return;
        }
    }
}

// 生成一只怪并套用 Zone 难度倍率 / 记账。spawn_rarity 须与采样 clearance 一致。
void Map::_spawn_mob_at(Simulation *sim, uint32_t zone_id, MobID::T mob_id, float x, float y, uint8_t spawn_rarity) {
    if (zone_id >= map_zones(sim->map_id).size() || mob_id >= MobID::kNumMobs) return;
    if (in_spawn_plaza(x, y)) return;
    struct ZoneDefinition const &zone = map_zones(sim->map_id)[zone_id];
    uint32_t diff = zone.difficulty;
    uint8_t base_r = MOB_DATA[mob_id].rarity;
    uint8_t spawn_r = spawn_rarity;
    if (spawn_r >= RarityID::kNumRarities) spawn_r = (uint8_t)diff;
    spawn_r = std::min(spawn_r, zone_spawn_rarity_cap(diff));
    // The zone resolver has already applied its cap. Baked rarity is a scaling
    // anchor, not a minimum: Square's Unique anchor must not override hub caps.
    // Nest trash Queens must stay below the map-anchor Unique boss.
    if (mob_id == MobID::kQueenAnt && spawn_r >= RarityID::kUnique)
        spawn_r = RarityID::kSuper;
    float const rs = mob_spawn_radius_mult(base_r, spawn_r);
    // Zone/plaza count once per spawn call — alloc_mob runs on_spawn for every
    // centipede segment, which would otherwise inflate density caps.
    sim->zone_mob_counts[zone_id]++;
    if (in_spawn_plaza(x, y)) sim->plaza_mob_count++;

    Entity &head = alloc_mob(sim, mob_id, x, y, NULL_ENTITY, [&](Entity &mob){
        mob.zone = zone_id;
        mob.home_x = x;
        mob.home_y = y;
        // Spawn grace: no outgoing contact for 3s (leech face-spawn etc.).
        // Damager also refuses hits while immunity_ticks > 0 on wild mobs.
        mob.immunity_ticks = (game_tick_t)(3 * TPS);
        // Do NOT set kSpawnedFromZone here: alloc_mob runs on_spawn before
        // adding kSegmented to body segments, so a has_component check would
        // still mark every segment and over-decrement zone_mob_counts on death.
        BitMath::set(mob.flags, EntityFlags::kHasCulling);
        mob.set_rarity(spawn_r);
        float th = mob_spawn_threat_mult(base_r, spawn_r);
        mob.max_health *= th;
        // Termites: Mysterious Relic-like HP on the zone spawn path too.
        if (MOB_DATA[mob_id].attributes.share_damage)
            mob.max_health *= 1.5f;
        mob.health = mob.max_health;
        mob.damage *= mob_spawn_damage_mult(base_r, spawn_r);
        mob.score_reward = (uint32_t) (
            MOB_DATA[mob_id].xp * zone_xp_mult(diff)
            * mob_spawn_xp_mult(base_r, spawn_r) + 0.5f
        );
        // Rarity body scale (Common→Unique ~7× vs baked base). Every segment
        // so Segment.cc spacing (r_self+r_parent) stays consistent.
        if (rs != 1.f) {
            float nr = mob.get_radius() * rs;
            mob.set_radius(nr);
            mob.mass = (1 + nr / BASE_FLOWER_RADIUS)
                * (MOB_DATA[mob_id].attributes.stationary ? 10000.f : 1.f);
        }
    });
    // Count/despawn bookkeeping only on the returned head (single-body or worm).
    BitMath::set(head.flags, EntityFlags::kSpawnedFromZone);
    // Safety net if terrain shifted between sample and alloc.
    {
        float hx = head.get_x(), hy = head.get_y();
        Map::eject_from_impassable(sim->map_id, hx, hy, head.get_radius() + 8.f);
        head.set_x(hx);
        head.set_y(hy);
        head.home_x = hx;
        head.home_y = hy;
    }
}

// 沿 (dx,dy) 方向的最长连续可通行距离。
float Map::corridor_reach(uint8_t map_id, float x, float y, float dx, float dy, float radius, float max_dist) {
    float const step = 40.f;
    float d = 0.f;
    while (d < max_dist) {
        float nd = d + step;
        if (circle_hits_impassable(map_id, x + dx * nd, y + dy * nd, radius)) break;
        d = nd;
    }
    if (d <= 0.f) return 0.f;
    // 二分细化最后一段，让"贴墙"落点更精确
    float lo = d - step, hi = d;
    for (uint32_t i = 0; i < 4; ++i) {
        float mid = (lo + hi) * 0.5f;
        if (circle_hits_impassable(map_id, x + dx * mid, y + dy * mid, radius)) hi = mid;
        else lo = mid;
    }
    return lo;
}

// 周围净空半径（8 方向取最小值）。
float Map::open_clearance(uint8_t map_id, float x, float y, float radius, float max_dist) {
    float best = max_dist;
    for (uint32_t k = 0; k < 8; ++k) {
        float ang = (float)k * (float)M_PI * 0.25f;
        float reach = corridor_reach(map_id, x, y, std::cos(ang), std::sin(ang), radius, max_dist);
        if (reach < best) best = reach;
    }
    return best;
}

uint8_t Map::boss_anchor(uint8_t map_id, float &x, float &y, uint8_t &mob_id) {
    MapGen::BossAnchor const &a = MapGen::boss_anchor(map_id);
    if (a.mob_id == 0) return 0;
    x = a.x; y = a.y; mob_id = a.mob_id;
    return 1;
}

// Zone 抽样：60% 按 密度×独占面积 加权，40% 等概率 —— 保证小生态区不被大环带淹没。
uint32_t Map::pick_spawn_zone(uint8_t map_id) {
    MapZones const &zones = map_zones(map_id);
    uint32_t n = zones.size();
    if (n == 0) return 0;
    float r = frand();
    if (r < kUniformZoneShare)
        return (uint32_t)(frand() * (float)n) % n;
    float total = 0.f;
    for (uint32_t i = 0; i < n; ++i)
        total += zones[i].density * zone_exclusive_area(map_id, i);
    if (total <= 0.f)
        return (uint32_t)(frand() * (float)n) % n;
    float pick = (r - kUniformZoneShare) / (1.f - kUniformZoneShare) * total;
    for (uint32_t i = 0; i < n; ++i) {
        pick -= zones[i].density * zone_exclusive_area(map_id, i);
        if (pick <= 0.f) return i;
    }
    return n - 1;
}

// Network-AOI half-extents (must match _spawn_zone_mob_once). Reject candidates
// before corridor/clearance probes — those were the ~90ms spawn_max spikes.
static void spawn_aoi_half(float &hw, float &hh) {
    constexpr float kMinClientZoom = 0.55f;
    hw = 960.f / (BASE_FOV * kMinClientZoom) + 420.f;
    hh = 540.f / (BASE_FOV * kMinClientZoom) + 420.f;
}

static uint8_t point_near_flower_aoi(Simulation *sim, float x, float y, float hw, float hh) {
    // No living flowers (boot / empty map): allow zone-wide seed spawn.
    // With players: only plant near someone's AOI so far biomes stay cheap.
    uint8_t near = 0;
    uint8_t any = 0;
    sim->for_each<kFlower>([&](Simulation *, Entity &f) {
        if (f.has_component(kMob)) return;
        any = 1;
        if (near) return;
        if (std::fabs(f.get_x() - x) <= hw && std::fabs(f.get_y() - y) <= hh)
            near = 1;
    });
    if (!any) return 1;
    return near;
}

// 在指定 Zone 内按落点类别采样合法落点（clearance 含稀有度体型）。
uint8_t Map::sample_spawn_point(Simulation *sim, uint32_t zone_id, uint8_t mob_id, float &ox, float &oy, uint8_t spawn_rarity) {
    MapZones const &zones = map_zones(sim->map_id);
    if (zone_id >= zones.size()) return 0;
    ZoneDefinition const &zone = zones[zone_id];
    SpawnRule const &rule = spawn_rules()[mob_id];
    uint8_t cls = spawn_class(mob_id);
    uint8_t base_r = MOB_DATA[mob_id].rarity;
    float radius = MOB_DATA[mob_id].radius.get_single(frand()) * MOB_RADIUS_MULTIPLIER
        * mob_spawn_radius_mult(base_r, spawn_rarity);
    float clearance = radius + 8.f;
    float const margin = 60.f;
    float zw = zone.right - zone.left - 2.f * margin;
    float zh = zone.bottom - zone.top - 2.f * margin;
    if (zw <= 0.f || zh <= 0.f) return 0;
    float aoi_hw = 0.f, aoi_hh = 0.f;
    spawn_aoi_half(aoi_hw, aoi_hh);

    for (uint32_t attempt = 0; attempt < 12; ++attempt) {
        float x = 0.f, y = 0.f;
        bool ok = true;
        switch (cls) {
        case SpawnClass::kAttach:
        case SpawnClass::kNearWall: {
            float bx = zone.left + margin + frand() * zw;
            float by = zone.top + margin + frand() * zh;
            // Cheap AOI reject before wall probes.
            if (!point_near_flower_aoi(sim, bx, by, aoi_hw, aoi_hh)) { ok = false; break; }
            if (circle_hits_impassable(sim->map_id, bx, by, clearance)) { ok = false; break; }
            // Find a real wall within reach — a direction that stays open for the
            // whole probe distance does not count as "attached to a wall".
            float probe = cls == SpawnClass::kNearWall ? kWallProbeNear : kWallProbe;
            bool found = false;
            for (uint32_t d = 0; d < 8 && !found; ++d) {
                float ang = frand() * 2.f * (float)M_PI;
                float dx = std::cos(ang), dy = std::sin(ang);
                float reach = corridor_reach(sim->map_id, bx, by, dx, dy, clearance, probe);
                if (reach >= probe) continue;
                float back = reach;
                if (cls == SpawnClass::kNearWall)
                    back = reach - (rule.wall_dist + frand() * 120.f);
                if (back < 0.f) back = 0.f;
                x = bx + dx * back;
                y = by + dy * back;
                found = true;
            }
            if (!found) { ok = false; break; }
            break;
        }
        case SpawnClass::kCorridor: {
            float bx = zone.left + margin + frand() * zw;
            float by = zone.top + margin + frand() * zh;
            if (!point_near_flower_aoi(sim, bx, by, aoi_hw, aoi_hh)) { ok = false; break; }
            if (circle_hits_impassable(sim->map_id, bx, by, clearance)) { ok = false; break; }
            float best = 0.f, bdx = 1.f, bdy = 0.f;
            // Cap probe — full 2000×4 was the main spawn hitch on failed attempts.
            float const kCorridorMax = 900.f;
            for (uint32_t k = 0; k < 4; ++k) {
                float ang = (float)k * (float)M_PI * 0.5f;
                float dx = std::cos(ang), dy = std::sin(ang);
                float reach = corridor_reach(sim->map_id, bx, by, dx, dy, clearance, kCorridorMax);
                if (reach > best) { best = reach; bdx = dx; bdy = dy; }
            }
            if (best < 400.f) { ok = false; break; }
            // Spread along the corridor instead of stacking on its midpoint
            float along = best * (0.25f + 0.5f * frand());
            x = bx + bdx * along;
            y = by + bdy * along;
            break;
        }
        case SpawnClass::kAnchor: {
            // 精英怪：优先落在 Zone 锚点附近的开阔地
            float ax = zone.anchor_x != 0.f ? zone.anchor_x : 0.5f * (zone.left + zone.right);
            float ay = zone.anchor_y != 0.f ? zone.anchor_y : 0.5f * (zone.top + zone.bottom);
            float spread = rule.anchor_range > 0.f ? rule.anchor_range : 400.f;
            if (attempt < 8) {
                x = ax + (frand() * 2.f - 1.f) * spread;
                y = ay + (frand() * 2.f - 1.f) * spread;
            } else {
                // Fallback: any open ground inside the zone
                x = zone.left + margin + frand() * zw;
                y = zone.top + margin + frand() * zh;
            }
            if (x < zone.left + margin || x > zone.right - margin
                || y < zone.top + margin || y > zone.bottom - margin) { ok = false; break; }
            break;
        }
        default: {
            x = zone.left + margin + frand() * zw;
            y = zone.top + margin + frand() * zh;
            break;
        }
        }
        if (!ok) continue;
        if (x < zone.left + margin || x > zone.right - margin) continue;
        if (y < zone.top + margin || y > zone.bottom - margin) continue;
        // Default/anchor picks: reject far-from-player before clearance probes.
        if (!point_near_flower_aoi(sim, x, y, aoi_hw, aoi_hh)) continue;
        if (circle_hits_impassable(sim->map_id, x, y, clearance)) continue;
        float need_clear = rule.min_clear > 0.f ? rule.min_clear
            : (cls == SpawnClass::kAnchor ? (attempt < 8 ? 700.f : 300.f) : 0.f);
        if (need_clear > 0.f
            && open_clearance(sim->map_id, x, y, clearance, need_clear) < need_clear) continue;
        if (in_safe_channel(sim->map_id, x, y)) continue;
        // 玩家近身不刷怪：距离含怪体型，Unique 大半径不能贴脸压死
        bool too_close = false;
        float const pad = kMinSpawnPlayerDist + clearance + BASE_FLOWER_RADIUS;
        float const pad2 = pad * pad;
        sim->for_each<kFlower>([&](Simulation *, Entity &ent){
            if (too_close) return;
            if (ent.has_component(kMob)) return;
            float dx = ent.get_x() - x, dy = ent.get_y() - y;
            if (dx * dx + dy * dy < pad2) too_close = true;
        });
        if (too_close) continue;
        ox = x; oy = y;
        return 1;
    }
    return 0;
}

// 主刷怪入口：选区 → 选点 → 抽怪 → 生成。
void Map::spawn_zone_mob(Simulation *sim) {
    // Three tries on 8c — two starved FOV refill when corridor probes missed.
    for (uint32_t attempt = 0; attempt < 3; ++attempt) {
        if (_spawn_zone_mob_once(sim)) return;
    }
}

uint8_t Map::_spawn_zone_mob_once(Simulation *sim) {
    MapZones const &zones = map_zones(sim->map_id);
    if (zones.size() == 0) return 0;

    // Spawn reject uses larger AOI; combat FOV drives density (clone ≈16/screen).
    float near_hw = 0.f, near_hh = 0.f;
    spawn_aoi_half(near_hw, near_hh);
    float combat_hw = 0.f, combat_hh = 0.f;
    combat_fov_half(combat_hw, combat_hh);

    // Prefer the flower whose combat FOV has the fewest zone wildlife (clone).
    Entity *focus = nullptr;
    uint32_t focus_count = UINT32_MAX;
    uint32_t focus_zone = zones.size();
    sim->for_each<kFlower>([&](Simulation *s, Entity &f) {
        if (f.has_component(kMob)) return;
        uint32_t z = Map::get_zone_from_pos(s->map_id, f.get_x(), f.get_y());
        if (z >= zones.size()) return;
        uint32_t n = count_zone_wildlife_in_box(s, f.get_x(), f.get_y(),
                                               combat_hw, combat_hh);
        if (n < focus_count) {
            focus_count = n;
            focus = &f;
            focus_zone = z;
        }
    });

    uint32_t zone_id = focus_zone;
    if (zone_id >= zones.size())
        zone_id = Map::pick_spawn_zone(sim->map_id);
    if (zone_id >= zones.size()) return 0;
    ZoneDefinition const &zone = zones[zone_id];

    // Pool pressure: never grow wildlife into the shared ENTITY_CAP cliff.
    // Also reserve room for multi-body spawns (centipede segments / ant holes).
    uint32_t const ents_now = sim->entity_count();
    if (ents_now >= kEntitySpawnStop)
        return 0;

    uint32_t flowers = count_alive_flowers(sim);
    // Primary gate: combat FOV already packed for the emptiest nearby flower.
    float local_need = local_combat_target(zone.difficulty, flowers);
    if (focus != nullptr && (float)focus_count >= local_need)
        return 0;
    // FOV underfilled: allow a small overshoot past dens×area soft so the screen
    // can refill while far/culled packs linger — NOT the full abs hard-cap
    // (that path filled 8k ents and ABRT'd under multiplayer).
    float soft = soft_zone_spawn_target(sim, zone_id, zone);
    float hard = (float)soft_zone_abs_cap(zone.difficulty);
    float fov_head = soft + 24.f;
    if (fov_head > hard) fov_head = hard;
    uint8_t need_local = focus != nullptr && (float)focus_count < local_need;
    if ((float)sim->zone_mob_counts[zone_id] >= (need_local ? fov_head : soft))
        return 0;

    float total = 0.f;
    for (SpawnChance const &s : zone.spawns) total += s.chance;
    if (total <= 0.f) return 0;
    float pick = frand() * total;
    MobID::T mob_id = MobID::kNumMobs;
    for (SpawnChance const &s : zone.spawns) {
        pick -= s.chance;
        if (pick <= 0) { mob_id = s.id; break; }
    }
    if (mob_id >= MobID::kNumMobs) return 0;

    uint8_t cls = spawn_class(mob_id);
    if (cls == SpawnClass::kBoss) return 0;   // Boss 由 ensure_boss 驱动

    // Centipede/hole: zone count +1 but many ents — refuse if cost would cross stop.
    if (ents_now + spawn_entity_cost(mob_id) > kEntitySpawnStop)
        return 0;

    uint8_t base_r = MOB_DATA[mob_id].rarity;
    uint8_t spawn_r = resolve_zone_spawn_rarity(base_r, zone.difficulty);

    float x = 0.f, y = 0.f;
    // sample_spawn_point already rejects outside flower AOI before heavy probes.
    if (!Map::sample_spawn_point(sim, zone_id, mob_id, x, y, spawn_r)) return 0;
    if (!point_near_flower_aoi(sim, x, y, near_hw, near_hh)) return 0;
    // Prefer landing in the underfilled flower's combat FOV when we have one.
    if (focus != nullptr) {
        float dx = std::fabs(x - focus->get_x());
        float dy = std::fabs(y - focus->get_y());
        if (dx > combat_hw || dy > combat_hh) {
            // Soft bias: prefer emptiest FOV without starving when samples miss.
            if (frand() < 0.55f) return 0;
        }
    }
    if (cls == SpawnClass::kAnchor) {
        // 同款精英 1500 单位半径内至多 1 只（跨 Zone，总纲 R6）。
        // Must use for_each: spawn runs before spatial_hash.insert this tick.
        float const r2 = kEliteUniqueRadius * kEliteUniqueRadius;
        bool near = false;
        sim->for_each<kMob>([&](Simulation *, Entity &m) {
            if (near || m.pending_delete) return;
            if (m.get_mob_id() != mob_id) return;
            float dx = m.get_x() - x, dy = m.get_y() - y;
            if (dx * dx + dy * dy < r2) near = true;
        });
        if (near) return 0;
    }
    // Never auto-spawn inside the respawn ellipse (kite/admin piles belong elsewhere).
    if (in_spawn_plaza(x, y)) return 0;
    _spawn_mob_at(sim, zone_id, mob_id, x, y, spawn_r);
    return 1;
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
