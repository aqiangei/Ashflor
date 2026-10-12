#include <Shared/MapGen.hh>
#include <Helpers/Math.hh>

#include <cmath>
#include <cstdint>

namespace {

MapZones g_zones[MapID::kNumMaps];
MapTerrain g_terrain[MapID::kNumMaps];
uint8_t g_ready[MapID::kNumMaps] = {};

struct ThemePalette {
    uint32_t hill;
    uint32_t water;
    uint32_t channel;
};

ThemePalette const kThemes[MapID::kNumMaps] = {
    { MapTheme::GARDEN_HILL, MapTheme::GARDEN_WATER, MapTheme::GARDEN_CHANNEL },
    { MapTheme::DESERT_HILL, MapTheme::DESERT_WATER, MapTheme::DESERT_CHANNEL },
    { MapTheme::OCEAN_HILL, MapTheme::OCEAN_WATER, MapTheme::OCEAN_CHANNEL },
    { MapTheme::POLAR_HILL, MapTheme::POLAR_WATER, MapTheme::POLAR_CHANNEL },
};

uint32_t rng_u(uint32_t &s) {
    s = s * 1664525u + 1013904223u;
    return s;
}

float rng_f(uint32_t &s) {
    return (float)(rng_u(s) >> 8) * (1.f / 16777216.f);
}

float rng_range(uint32_t &s, float lo, float hi) {
    return lo + (hi - lo) * rng_f(s);
}

uint32_t map_seed(uint8_t map_id) {
    return 0xA5F108u ^ ((uint32_t)map_id * 0x9E3779B9u) ^ 0xC0FFEEu;
}

void push_feat(MapTerrain &t, uint8_t kind, uint8_t shape, uint8_t flags,
               float left, float top, float right, float bottom,
               uint32_t color, uint8_t seed, float param) {
    if (t.size() >= MAX_TERRAIN_PER_MAP) return;
    t.push(TerrainFeature{
        .kind = kind,
        .shape = shape,
        .flags = flags,
        .left = left,
        .top = top,
        .right = right,
        .bottom = bottom,
        .color = color,
        .seed = seed,
        .param = param
    });
}

// Ring wall segments with gate gaps — boxes along each side of a rect.
void ring_walls(MapTerrain &t, uint32_t &rng, float L, float T, float R, float B,
                float thickness, float gate_frac, ThemePalette const &pal, uint8_t base_seed) {
    float w = R - L, h = B - T;
    if (w < 800.f || h < 800.f) return;
    float gate_w = w * gate_frac;
    float gate_h = h * gate_frac;
    // Gate centers jittered per side
    float gx_top = L + w * rng_range(rng, 0.25f, 0.75f);
    float gx_bot = L + w * rng_range(rng, 0.25f, 0.75f);
    float gy_left = T + h * rng_range(rng, 0.25f, 0.75f);
    float gy_right = T + h * rng_range(rng, 0.25f, 0.75f);

    auto wall_h = [&](float y0, float y1, float x_gate, float gate_sz, uint8_t s) {
        float g0 = x_gate - gate_sz * 0.5f;
        float g1 = x_gate + gate_sz * 0.5f;
        if (g0 > L + 40.f)
            push_feat(t, TerrainKind::kHill, TerrainShape::kAABB, TerrainFlags::kImpassable,
                      L, y0, g0, y1, pal.hill, s, 0.12f);
        if (g1 < R - 40.f)
            push_feat(t, TerrainKind::kHill, TerrainShape::kAABB, TerrainFlags::kImpassable,
                      g1, y0, R, y1, pal.hill, (uint8_t)(s + 1), 0.12f);
    };
    auto wall_v = [&](float x0, float x1, float y_gate, float gate_sz, uint8_t s) {
        float g0 = y_gate - gate_sz * 0.5f;
        float g1 = y_gate + gate_sz * 0.5f;
        if (g0 > T + 40.f)
            push_feat(t, TerrainKind::kHill, TerrainShape::kAABB, TerrainFlags::kImpassable,
                      x0, T, x1, g0, pal.hill, s, 0.12f);
        if (g1 < B - 40.f)
            push_feat(t, TerrainKind::kHill, TerrainShape::kAABB, TerrainFlags::kImpassable,
                      x0, g1, x1, B, pal.hill, (uint8_t)(s + 1), 0.12f);
    };

    wall_h(T, T + thickness, gx_top, gate_w, base_seed);
    wall_h(B - thickness, B, gx_bot, gate_w, (uint8_t)(base_seed + 10));
    wall_v(L, L + thickness, gy_left, gate_h, (uint8_t)(base_seed + 20));
    wall_v(R - thickness, R, gy_right, gate_h, (uint8_t)(base_seed + 30));
}

void side_boxes(MapTerrain &t, uint32_t &rng, float L, float T, float R, float B,
                float hub_clear, ThemePalette const &pal, uint8_t base_seed, uint32_t count) {
    float cx = (float)ARENA_WIDTH * 0.5f;
    float cy = (float)ARENA_HEIGHT * 0.5f;
    for (uint32_t i = 0; i < count; ++i) {
        float bw = rng_range(rng, 420.f, 980.f);
        float bh = rng_range(rng, 360.f, 860.f);
        float x = rng_range(rng, L + 200.f, R - bw - 200.f);
        float y = rng_range(rng, T + 200.f, B - bh - 200.f);
        float mx = x + bw * 0.5f, my = y + bh * 0.5f;
        float dx = mx - cx, dy = my - cy;
        if (dx * dx + dy * dy < hub_clear * hub_clear) continue;
        if (rng_f(rng) < 0.45f) {
            push_feat(t, TerrainKind::kHill, TerrainShape::kAABB, TerrainFlags::kImpassable,
                      x, y, x + bw, y + bh, pal.hill, (uint8_t)(base_seed + i), 0.13f);
        } else {
            float rx = bw * 0.5f, ry = bh * 0.5f;
            push_feat(t, TerrainKind::kHill, TerrainShape::kEllipse, TerrainFlags::kImpassable,
                      mx, my, rx, ry, pal.hill, (uint8_t)(base_seed + i), 0.14f);
        }
    }
}

void generate_one(uint8_t map_id) {
    MapZones &zones = g_zones[map_id];
    MapTerrain &terrain = g_terrain[map_id];
    zones.clear();
    terrain.clear();

    uint32_t rng = map_seed(map_id);
    ThemePalette const &pal = kThemes[map_id];
    float W = (float)ARENA_WIDTH;
    float H = (float)ARENA_HEIGHT;
    float cx = W * 0.5f;
    float cy = H * 0.5f;

    // Asymmetric nested zones (non-square insets vary by seed + map)
    float hub_hw = rng_range(rng, 1700.f, 2200.f);
    float hub_hh = rng_range(rng, 1400.f, 1850.f);
    float ox = rng_range(rng, -280.f, 280.f);
    float oy = rng_range(rng, -220.f, 220.f);

    float m1l = rng_range(rng, 0.28f, 0.34f);
    float m1r = rng_range(rng, 0.28f, 0.34f);
    float m1t = rng_range(rng, 0.26f, 0.33f);
    float m1b = rng_range(rng, 0.26f, 0.33f);
    float m2l = rng_range(rng, 0.12f, 0.17f);
    float m2r = rng_range(rng, 0.12f, 0.17f);
    float m2t = rng_range(rng, 0.11f, 0.16f);
    float m2b = rng_range(rng, 0.11f, 0.16f);

    // Copy spawn templates from MAPS, rewrite geometry
    MapZones const &tmpl = MAPS[map_id];
    for (uint32_t i = 0; i < tmpl.size(); ++i) {
        ZoneDefinition z = tmpl[i];
        if (z.difficulty == 3) {
            z.left = 0; z.top = 0; z.right = W; z.bottom = H;
        } else if (z.difficulty == 2) {
            z.left = W * m2l; z.top = H * m2t;
            z.right = W * (1.f - m2r); z.bottom = H * (1.f - m2b);
        } else if (z.difficulty == 1) {
            z.left = W * m1l; z.top = H * m1t;
            z.right = W * (1.f - m1r); z.bottom = H * (1.f - m1b);
        } else {
            z.left = cx + ox - hub_hw;
            z.top = cy + oy - hub_hh;
            z.right = cx + ox + hub_hw;
            z.bottom = cy + oy + hub_hh;
        }
        zones.push(z);
    }

    // Find zone rects by difficulty for ring placement
    float z2L = W * m2l, z2T = H * m2t, z2R = W * (1.f - m2r), z2B = H * (1.f - m2b);
    float z1L = W * m1l, z1T = H * m1t, z1R = W * (1.f - m1r), z1B = H * (1.f - m1b);
    float hubL = cx + ox - hub_hw, hubT = cy + oy - hub_hh;
    float hubR = cx + ox + hub_hw, hubB = cy + oy + hub_hh;
    float hub_clear = std::max(hub_hw, hub_hh) + 400.f;

    // Perimeter ring walls between difficulty bands (gates = corridors into zones)
    float gate = rng_range(rng, 0.14f, 0.20f);
    ring_walls(terrain, rng, 180.f, 180.f, W - 180.f, H - 180.f,
               rng_range(rng, 280.f, 420.f), gate, pal, 11);
    ring_walls(terrain, rng, z2L, z2T, z2R, z2B,
               rng_range(rng, 240.f, 360.f), gate * 1.1f, pal, 41);
    ring_walls(terrain, rng, z1L, z1T, z1R, z1B,
               rng_range(rng, 200.f, 300.f), gate * 1.2f, pal, 71);

    // Side-branch boxes / ellipses (dead-end alcoves)
    side_boxes(terrain, rng, 400.f, 400.f, W - 400.f, H - 400.f,
               hub_clear, pal, 100, 10 + map_id * 2);
    side_boxes(terrain, rng, z2L + 100.f, z2T + 100.f, z2R - 100.f, z2B - 100.f,
               hub_clear * 0.85f, pal, 130, 6 + map_id);

    // Spirals — thick impassable arms creating spiral corridors into mid/outer bands
    uint32_t n_spirals = 1u + (map_id % 2u);
    for (uint32_t i = 0; i < n_spirals; ++i) {
        float ang = rng_range(rng, 0.f, 6.28f);
        float dist = rng_range(rng, 4200.f, 7800.f);
        float sx = cx + std::cos(ang) * dist;
        float sy = cy + std::sin(ang) * dist * (H / W);
        sx = fclamp(sx, 1800.f, W - 1800.f);
        sy = fclamp(sy, 1600.f, H - 1600.f);
        float outer = rng_range(rng, 1100.f, 1600.f);
        float turns = rng_range(rng, 2.0f, 2.8f);
        float arm = rng_range(rng, 130.f, 175.f);
        push_feat(terrain, TerrainKind::kWater, TerrainShape::kSpiral, TerrainFlags::kImpassable,
                  sx, sy, outer, turns, pal.water, (uint8_t)(50 + i * 7 + map_id), arm);
    }

    // Meandering impassable rivers between zones (with intentional openings via short segments)
    uint32_t n_rivers = 2u + (map_id % 3u);
    for (uint32_t i = 0; i < n_rivers; ++i) {
        float a0 = rng_range(rng, 0.f, 6.28f);
        float a1 = a0 + rng_range(rng, 1.2f, 2.8f) * (rng_f(rng) < 0.5f ? 1.f : -1.f);
        float r0 = rng_range(rng, 3500.f, 9000.f);
        float r1 = rng_range(rng, 3500.f, 9000.f);
        float x0 = fclamp(cx + std::cos(a0) * r0, 600.f, W - 600.f);
        float y0 = fclamp(cy + std::sin(a0) * r0 * (H / W), 500.f, H - 500.f);
        float x1 = fclamp(cx + std::cos(a1) * r1, 600.f, W - 600.f);
        float y1 = fclamp(cy + std::sin(a1) * r1 * (H / W), 500.f, H - 500.f);
        // Skip if both ends inside hub
        auto in_hub = [&](float x, float y) {
            return x >= hubL && x <= hubR && y >= hubT && y <= hubB;
        };
        if (in_hub(x0, y0) && in_hub(x1, y1)) continue;
        float hw = rng_range(rng, 140.f, 185.f);
        push_feat(terrain, TerrainKind::kWater, TerrainShape::kRiver, TerrainFlags::kImpassable,
                  x0, y0, x1, y1, pal.water, (uint8_t)(20 + i * 9 + map_id * 3), hw);
    }

    // Scattered water/hill circles in outer ring
    uint32_t n_blobs = 4u + map_id;
    for (uint32_t i = 0; i < n_blobs; ++i) {
        float ang = rng_range(rng, 0.f, 6.28f);
        float dist = rng_range(rng, 5500.f, 11000.f);
        float bx = fclamp(cx + std::cos(ang) * dist, 900.f, W - 900.f);
        float by = fclamp(cy + std::sin(ang) * dist * (H / W), 800.f, H - 800.f);
        float dx = bx - cx, dy = by - cy;
        if (dx * dx + dy * dy < hub_clear * hub_clear) continue;
        float rad = rng_range(rng, 480.f, 780.f);
        if (rng_f(rng) < 0.55f)
            push_feat(terrain, TerrainKind::kHill, TerrainShape::kCircle, TerrainFlags::kImpassable,
                      bx, by, rad, 0.f, pal.hill, (uint8_t)(160 + i), 0.14f);
        else
            push_feat(terrain, TerrainKind::kWater, TerrainShape::kCircle, TerrainFlags::kImpassable,
                      bx, by, rad * 0.85f, 0.f, pal.water, (uint8_t)(170 + i), 0.11f);
    }

    // No-attack channels: hub → mid → outer through cardinal gates
    float ch_hw = rng_range(rng, 190.f, 230.f);
    // Vertical through hub
    push_feat(terrain, TerrainKind::kChannel, TerrainShape::kRiver, TerrainFlags::kNoAttack,
              cx + ox * 0.3f, 400.f, cx + ox * 0.3f, H - 400.f, pal.channel,
              (uint8_t)(7 + map_id), ch_hw);
    // Horizontal through hub
    push_feat(terrain, TerrainKind::kChannel, TerrainShape::kRiver, TerrainFlags::kNoAttack,
              400.f, cy + oy * 0.3f, W - 400.f, cy + oy * 0.3f, pal.channel,
              (uint8_t)(19 + map_id), ch_hw);
    // Diagonal branch corridors (map-unique)
    float d0 = rng_range(rng, 0.3f, 0.7f);
    push_feat(terrain, TerrainKind::kChannel, TerrainShape::kRiver, TerrainFlags::kNoAttack,
              W * 0.12f, H * d0, W * 0.88f, H * (1.f - d0 * 0.5f), pal.channel,
              (uint8_t)(31 + map_id), ch_hw * 0.9f);
    if (map_id >= 2) {
        float d1 = rng_range(rng, 0.25f, 0.6f);
        push_feat(terrain, TerrainKind::kChannel, TerrainShape::kRiver, TerrainFlags::kNoAttack,
                  W * d1, H * 0.1f, W * (1.f - d1 * 0.4f), H * 0.9f, pal.channel,
                  (uint8_t)(43 + map_id), ch_hw * 0.85f);
    }
}

} // namespace

void MapGen::ensure(uint8_t map_id) {
    if (map_id >= MapID::kNumMaps) map_id = MapID::kGarden;
    if (g_ready[map_id]) return;
    generate_one(map_id);
    g_ready[map_id] = 1;
}

MapZones const &MapGen::zones(uint8_t map_id) {
    ensure(map_id);
    return g_zones[map_id];
}

MapTerrain const &MapGen::terrain(uint8_t map_id) {
    ensure(map_id);
    return g_terrain[map_id];
}

MapZones const &map_zones(uint8_t map_id) {
    return MapGen::zones(map_id);
}

MapTerrain const &map_terrain(uint8_t map_id) {
    return MapGen::terrain(map_id);
}
