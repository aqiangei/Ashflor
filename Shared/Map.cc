#include <Shared/Map.hh>
#include <Shared/RarityScale.hh>

#ifdef SERVERSIDE
#include <Server/Spawn.hh>
#include <Shared/Entity.hh>
#endif

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

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
static bool feature_near_circle(TerrainFeature const &f, float x, float y, float radius) {
    float x0, y0, x1, y1;
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

// Nearest sample index along path (same sample count as dist_to_path).
static uint32_t nearest_path_sample(TerrainFeature const &f, float x, float y, float &cx, float &cy) {
    uint32_t const samples = (f.shape == TerrainShape::kSpiral) ? 48u : 36u;
    float best = 1e12f;
    uint32_t best_i = 0;
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
        if (d2 < best) { best = d2; cx = px; cy = py; best_i = i; }
    }
    return best_i < samples ? best_i : samples - 1;
}

static float path_half_width_at(TerrainFeature const &f, float x, float y, float &cx, float &cy) {
    uint32_t seed = feature_seed(f);
    uint32_t i = nearest_path_sample(f, x, y, cx, cy);
    // Side: which bank is closer — use sign of cross vs path tangent approx via
    // averaging both banks' max so collision hugs the wider drawn shore.
    float s0 = path_width_scale(seed, i, 0);
    float s1 = path_width_scale(seed, i, 1);
    float s = s0 > s1 ? s0 : s1;
    return feature_half_width(f) * s;
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
        float cx, cy;
        float hw = path_half_width_at(f, x, y, cx, cy);
        float dx = x - cx, dy = y - cy;
        return (dx * dx + dy * dy) <= hw * hw ? 1 : 0;
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
        float cx, cy;
        float hw = path_half_width_at(f, x, y, cx, cy);
        float dx = x - cx, dy = y - cy;
        float need = hw + radius;
        return (dx * dx + dy * dy) <= need * need ? 1 : 0;
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
    float hw = path_half_width_at(f, x, y, cx, cy);
    float dx = x - cx, dy = y - cy;
    float d = std::sqrt(dx * dx + dy * dy);
    float need = hw + radius;
    if (d >= need) return 0;
    if (d < 1e-3f) {
        dx = 1.f; dy = 0.f; d = 1.f;
    }
    float scale = (need + 0.5f) / d;
    x = cx + dx * scale;
    y = cy + dy * scale;
    return 1;
}

static uint8_t push_out_smooth_aabb(TerrainFeature const &f, float &x, float &y, float radius) {
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

void Map::ccd_move_circle(uint8_t map_id, float &x, float &y, float ox, float oy, float radius) {
    float dx = x - ox;
    float dy = y - oy;
    float dist = std::sqrt(dx * dx + dy * dy);
    if (!(dist > 0.f) || dist != dist) { // NaN / zero
        resolve_circle_terrain(map_id, x, y, radius);
        return;
    }
    // Step by ~half radius so thin walls cannot be skipped in one tick
    float step = radius > 8.f ? radius * 0.5f : 8.f;
    int steps = (int) std::ceil(dist / step);
    if (steps < 1) steps = 1;
    if (steps > 24) steps = 24;
    float px = ox, py = oy;
    for (int i = 1; i <= steps; ++i) {
        float tx = ox + dx * ((float) i / (float) steps);
        float ty = oy + dy * ((float) i / (float) steps);
        if (circle_hits_impassable(map_id, tx, ty, radius)) {
            float ax = px, ay = py, bx = tx, by = ty;
            for (int k = 0; k < 7; ++k) {
                float mx = (ax + bx) * 0.5f;
                float my = (ay + by) * 0.5f;
                if (circle_hits_impassable(map_id, mx, my, radius)) {
                    bx = mx; by = my;
                } else {
                    ax = mx; ay = my;
                }
            }
            x = ax;
            y = ay;
            resolve_circle_terrain(map_id, x, y, radius);
            return;
        }
        px = tx;
        py = ty;
    }
    x = px;
    y = py;
    resolve_circle_terrain(map_id, x, y, radius);
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
        // nearest walkable around zone center
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
        if (ok) ++reachable;
    }
    return reachable;
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
                uint32_t diff = zone.difficulty;
                float th = zone_threat_mult(diff);
                mob.max_health *= th;
                mob.health = mob.max_health;
                mob.damage *= zone_damage_mult(diff);
                mob.score_reward = (uint32_t) (MOB_DATA[mob.get_mob_id()].xp * zone_xp_mult(diff) + 0.5f);
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
