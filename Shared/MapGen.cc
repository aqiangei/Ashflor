#include <Shared/MapGen.hh>
#include <Shared/Map.hh>
#include <Shared/RarityScale.hh>
#include <Helpers/Math.hh>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <initializer_list>
#include <iostream>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace {

MapZones g_zones[MapID::kNumMaps];
MapTerrain g_terrain[MapID::kNumMaps];
MapGen::BossAnchor g_boss[MapID::kNumMaps];
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
    { MapTheme::JUNGLE_HILL, MapTheme::JUNGLE_WATER, MapTheme::JUNGLE_CHANNEL },
    { MapTheme::ANT_HILL, MapTheme::ANT_WATER, MapTheme::ANT_CHANNEL },
    { MapTheme::VOLCANO_HILL, MapTheme::VOLCANO_WATER, MapTheme::VOLCANO_CHANNEL },
    { MapTheme::ROYAL_HILL, MapTheme::ROYAL_WATER, MapTheme::ROYAL_CHANNEL },
};

uint32_t rng_u(uint32_t &s) {
    s = s * 1664525u + 1013904223u;
    return s;
}
float rng_f(uint32_t &s) { return (float)(rng_u(s) >> 8) * (1.f / 16777216.f); }
float rng_range(uint32_t &s, float lo, float hi) { return lo + (hi - lo) * rng_f(s); }
uint32_t map_seed(uint8_t id) {
    return 0xA5F108u ^ ((uint32_t)id * 0x9E3779B9u) ^ 0xC0FFEEu;
}

void push_feat(MapTerrain &t, uint8_t kind, uint8_t shape, uint8_t flags,
               float l, float top, float r, float b, uint32_t color, uint8_t seed, float param) {
    if (t.size() >= MAX_TERRAIN_PER_MAP) return;
    t.push(TerrainFeature{
        .kind = kind, .shape = shape, .flags = flags,
        .left = l, .top = top, .right = r, .bottom = b,
        .color = color, .seed = seed, .param = param
    });
}

void hill_box(MapTerrain &t, ThemePalette const &p, uint8_t s,
              float l, float top, float r, float b) {
    if (r - l < 60.f || b - top < 60.f) return;
    push_feat(t, TerrainKind::kHill, TerrainShape::kAABB, TerrainFlags::kImpassable,
              l, top, r, b, p.hill, s, 0.11f);
}
void water_box(MapTerrain &t, ThemePalette const &p, uint8_t s,
               float l, float top, float r, float b) {
    if (r - l < 60.f || b - top < 60.f) return;
    push_feat(t, TerrainKind::kWater, TerrainShape::kAABB, TerrainFlags::kImpassable,
              l, top, r, b, p.water, s, 0.10f);
}
void hill_circle(MapTerrain &t, ThemePalette const &p, uint8_t s,
                 float cx, float cy, float rad) {
    if (rad < 80.f) return;
    push_feat(t, TerrainKind::kHill, TerrainShape::kCircle, TerrainFlags::kImpassable,
              cx, cy, rad, 0.f, p.hill, s, 0.12f);
}

void corridor_h(MapTerrain &t, ThemePalette const &p, uint8_t s,
                float x0, float x1, float y_mid, float gap, float wall_t, uint8_t water,
                uint8_t paint_lane = 1) {
    float half = gap * 0.5f;
    auto box = water ? water_box : hill_box;
    box(t, p, s, x0, y_mid - half - wall_t, x1, y_mid - half);
    box(t, p, (uint8_t)(s + 1), x0, y_mid + half, x1, y_mid + half + wall_t);
    // Paint the travel lane so walkable gaps aren't invisible against ground fill.
    // Callers may skip lane paint through spawn (title camera sits there).
    if (!water && paint_lane) {
        float lane = std::min(half * 0.55f, 280.f);
        push_feat(t, TerrainKind::kChannel, TerrainShape::kAABB, 0,
                  std::min(x0, x1), y_mid - lane, std::max(x0, x1), y_mid + lane,
                  p.channel, (uint8_t)(s + 2), 0.f);
    }
}
void corridor_v(MapTerrain &t, ThemePalette const &p, uint8_t s,
                float y0, float y1, float x_mid, float gap, float wall_t, uint8_t water,
                uint8_t paint_lane = 1) {
    float half = gap * 0.5f;
    auto box = water ? water_box : hill_box;
    box(t, p, s, x_mid - half - wall_t, y0, x_mid - half, y1);
    box(t, p, (uint8_t)(s + 1), x_mid + half, y0, x_mid + half + wall_t, y1);
    if (!water && paint_lane) {
        float lane = std::min(half * 0.55f, 280.f);
        push_feat(t, TerrainKind::kChannel, TerrainShape::kAABB, 0,
                  x_mid - lane, std::min(y0, y1), x_mid + lane, std::max(y0, y1),
                  p.channel, (uint8_t)(s + 2), 0.f);
    }
}

// Ring of thick wall segments with one gate — large open interior for multiplayer fights.
// Enclosed area is ~15× the legacy radius (radius × √15); walls thicken modestly.
void ring_valley(MapTerrain &t, ThemePalette const &p, uint32_t &rng,
                 float cx, float cy, float inner_r, float wall_t,
                 float gate_ang, float gate_arc, uint8_t seed0, uint8_t water) {
    constexpr float kAreaScaleR = 3.873f; // sqrt(15)
    inner_r *= kAreaScaleR;
    wall_t *= 2.0f;
    float max_r = std::min((float)ARENA_WIDTH, (float)ARENA_HEIGHT) * 0.30f;
    if (inner_r > max_r) inner_r = max_r;
    // More segments so the longer circumference still reads as a smooth ring.
    uint32_t segs = 18;
    for (uint32_t i = 0; i < segs; ++i) {
        float a0 = (float)(2.0 * M_PI) * ((float)i / (float)segs);
        float a1 = (float)(2.0 * M_PI) * ((float)(i + 1) / (float)segs);
        float amid = 0.5f * (a0 + a1);
        float d = amid - gate_ang;
        while (d > (float)M_PI) d -= (float)(2.0 * M_PI);
        while (d < -(float)M_PI) d += (float)(2.0 * M_PI);
        if (std::fabs(d) < gate_arc * 0.5f) continue;
        float mx = cx + std::cos(amid) * (inner_r + wall_t * 0.4f);
        float my = cy + std::sin(amid) * (inner_r + wall_t * 0.4f);
        float hw = wall_t * 0.5f + inner_r * 0.12f;
        float hh = wall_t * 0.75f;
        if (water) water_box(t, p, (uint8_t)(seed0 + i), mx - hw, my - hh, mx + hw, my + hh);
        else hill_box(t, p, (uint8_t)(seed0 + i), mx - hw, my - hh, mx + hw, my + hh);
    }
    (void)rng;
}

// H/V spiral corridor → circular valley (wide farm path + enclosed end).
void spiral_farm(MapTerrain &t, ThemePalette const &p, uint32_t &rng,
                 float sx, float sy, int dir, int turn,
                 float gap, float wall_t, float seg_len, uint32_t n_seg,
                 float valley_r, uint8_t seed0, uint8_t water,
                 float *out_x = nullptr, float *out_y = nullptr) {
    float x = sx, y = sy;
    int last = dir;
    for (uint32_t i = 0; i < n_seg; ++i) {
        float len = seg_len * rng_range(rng, 0.88f, 1.12f);
        last = dir;
        if (dir == 0) { corridor_h(t, p, (uint8_t)(seed0 + i * 3), x, x + len, y, gap, wall_t, water); x += len; }
        else if (dir == 2) { corridor_h(t, p, (uint8_t)(seed0 + i * 3), x - len, x, y, gap, wall_t, water); x -= len; }
        else if (dir == 1) { corridor_v(t, p, (uint8_t)(seed0 + i * 3), y, y + len, x, gap, wall_t, water); y += len; }
        else { corridor_v(t, p, (uint8_t)(seed0 + i * 3), y - len, y, x, gap, wall_t, water); y -= len; }
        dir = (dir + turn + 4) % 4;
    }
    float vx = x, vy = y;
    if (last == 0) vx += valley_r * 0.3f;
    else if (last == 2) vx -= valley_r * 0.3f;
    else if (last == 1) vy += valley_r * 0.3f;
    else vy -= valley_r * 0.3f;
    float W = (float)ARENA_WIDTH, H = (float)ARENA_HEIGHT;
    vx = fclamp(vx, valley_r + 500.f, W - valley_r - 500.f);
    vy = fclamp(vy, valley_r + 500.f, H - valley_r - 500.f);
    float gate = (last == 0) ? (float)M_PI : (last == 1) ? -(float)M_PI * 0.5f
                : (last == 2) ? 0.f : (float)M_PI * 0.5f;
    ring_valley(t, p, rng, vx, vy, valley_r, wall_t * 1.05f, gate,
                rng_range(rng, 0.5f, 0.8f), (uint8_t)(seed0 + 50), water);
    if (out_x) *out_x = vx;
    if (out_y) *out_y = vy;
}

// Maze cell: thick walls with random gates on each side of a rect.
void maze_cell(MapTerrain &t, ThemePalette const &p, uint32_t &rng, uint8_t s,
               float L, float T, float R, float B, float thick, float gate_min) {
    float w = R - L, h = B - T;
    if (w < 900.f || h < 900.f) return;
    // Hard floor: player-passable maze gates must stay ≥ kMinMazeGate (总纲 R7).
    gate_min = std::max(gate_min, Map::kMinMazeGate);
    float gw = std::max(gate_min, w * rng_range(rng, 0.18f, 0.32f));
    float gh = std::max(gate_min, h * rng_range(rng, 0.18f, 0.32f));
    auto punch_h = [&](float y0, float y1, float gx, uint8_t ss) {
        hill_box(t, p, ss, L, y0, gx - gw * 0.5f, y1);
        hill_box(t, p, (uint8_t)(ss + 1), gx + gw * 0.5f, y0, R, y1);
    };
    auto punch_v = [&](float x0, float x1, float gy, uint8_t ss) {
        hill_box(t, p, ss, x0, T, x1, gy - gh * 0.5f);
        hill_box(t, p, (uint8_t)(ss + 1), x0, gy + gh * 0.5f, x1, B);
    };
    float gx_n = L + w * rng_range(rng, 0.25f, 0.75f);
    float gx_s = L + w * rng_range(rng, 0.25f, 0.75f);
    float gy_w = T + h * rng_range(rng, 0.25f, 0.75f);
    float gy_e = T + h * rng_range(rng, 0.25f, 0.75f);
    punch_h(T, T + thick, gx_n, s);
    punch_h(B - thick, B, gx_s, (uint8_t)(s + 4));
    punch_v(L, L + thick, gy_w, (uint8_t)(s + 8));
    punch_v(R - thick, R, gy_e, (uint8_t)(s + 12));
    // Gate floors — short channel strips so openings read as paths, not bare ground.
    auto gate_h = [&](float y0, float y1, float gx, uint8_t ss) {
        float mid = 0.5f * (y0 + y1);
        float hh = std::max(36.f, (y1 - y0) * 0.35f);
        push_feat(t, TerrainKind::kChannel, TerrainShape::kAABB, 0,
                  gx - gw * 0.45f, mid - hh, gx + gw * 0.45f, mid + hh,
                  p.channel, ss, 0.f);
    };
    auto gate_v = [&](float x0, float x1, float gy, uint8_t ss) {
        float mid = 0.5f * (x0 + x1);
        float hw = std::max(36.f, (x1 - x0) * 0.35f);
        push_feat(t, TerrainKind::kChannel, TerrainShape::kAABB, 0,
                  mid - hw, gy - gh * 0.45f, mid + hw, gy + gh * 0.45f,
                  p.channel, ss, 0.f);
    };
    gate_h(T, T + thick, gx_n, (uint8_t)(s + 20));
    gate_h(B - thick, B, gx_s, (uint8_t)(s + 21));
    gate_v(L, L + thick, gy_w, (uint8_t)(s + 22));
    gate_v(R - thick, R, gy_e, (uint8_t)(s + 23));
}

void rock_field(MapTerrain &t, ThemePalette const &p, uint32_t &rng, uint8_t s0,
                float L, float T, float R, float B, uint32_t count, float rad_lo, float rad_hi) {
    for (uint32_t i = 0; i < count; ++i) {
        float rad = rng_range(rng, rad_lo, rad_hi);
        float x = rng_range(rng, L + rad, R - rad);
        float y = rng_range(rng, T + rad, B - rad);
        // Keep spawn plaza and future artery axes clear — rocks here create traps.
        if (Map::in_spawn_plaza(x, y)) continue;
        float ax = (float)ARENA_WIDTH * 0.5f, ay = (float)ARENA_HEIGHT * 0.5f;
        if (std::fabs(x - ax) < 700.f || std::fabs(y - ay) < 700.f) continue;
        hill_circle(t, p, (uint8_t)(s0 + i), x, y, rad);
    }
}

void push_zone(MapZones &z, ZoneDefinition tmpl, float l, float top, float r, float b,
               uint8_t diff, uint32_t color) {
    if (z.size() >= MAX_ZONES_PER_MAP) return;
    tmpl.left = l; tmpl.top = top; tmpl.right = r; tmpl.bottom = b;
    tmpl.difficulty = diff;
    tmpl.color = color;
    tmpl.density *= MOB_DENSITY_MULTIPLIER;
    z.push(tmpl);
}

ZoneDefinition eco_from(ZoneDefinition base, char const *name, uint8_t diff,
                        float density_mul, std::initializer_list<SpawnChance> spawns) {
    ZoneDefinition z = base;
    z.name = name;
    z.difficulty = diff;
    z.density = base.density * density_mul;
    z.drop_multiplier = zone_drop_tilt(diff);
    if (diff >= RarityID::kLegendary)
        z.drop_multiplier = fclamp(z.drop_multiplier * 1.15f, 0.f, 0.85f);
    z.spawns.clear();
    for (SpawnChance s : spawns) z.spawns.push(s);
    return z;
}

// Biome-specific eco tables (concept zone types → spawn mixes).
void append_biome_ecos(MapZones &zones, uint8_t map_id, ZoneDefinition const &hard,
                       ZoneDefinition const &mid, ZoneDefinition const &soft,
                       uint32_t &rng, float W, float H, float cx, float cy,
                       float hub_hw, float hub_hh) {
    struct Pocket {
        ZoneDefinition def;
        float min_sz;
        float max_sz;
        uint32_t count;
    };
    StaticArray<Pocket, 12> pockets;

    auto add = [&](ZoneDefinition d, float lo, float hi, uint32_t n) {
        if (pockets.size() >= 12) return;
        pockets.push(Pocket{ d, lo, hi, n });
    };

    switch (map_id) {
    case MapID::kGarden:
        // Fewer, larger pockets (小路大路 + 大 Zone).
        add(eco_from(soft, "Meadow", 1, 0.95f, {
            { MobID::kBee, 140000 }, { MobID::kLadybug, 120000 }, { MobID::kRock, 80000 },
            { MobID::kSquare, 1 }}), 2800.f, 4200.f, 3);
        add(eco_from(mid, "Hedge Maze", 2, 1.05f, {
            { MobID::kSpider, 110000 }, { MobID::kBee, 90000 }, { MobID::kBeetle, 80000 },
            { MobID::kCentipede, 14000 }, { MobID::kSquare, 1 }}), 2400.f, 3600.f, 3);
        add(eco_from(mid, "Creek Banks", 3, 1.0f, {
            { MobID::kHornet, 100000 }, { MobID::kBee, 90000 }, { MobID::kLadybug, 70000 },
            { MobID::kSquare, 1 }}), 2200.f, 3400.f, 2);
        add(eco_from(hard, "Thorn Field", 4, 1.3f, {
            { MobID::kDarkLadybug, 130000 }, { MobID::kHornet, 110000 }, { MobID::kSpider, 100000 },
            { MobID::kEvilCentipede, 26000 }, { MobID::kSquare, 1 }}), 2000.f, 3000.f, 2);
        add(eco_from(hard, "Flower Heart", 6, 1.45f, {
            { MobID::kMassiveBeetle, 16000 }, { MobID::kHornet, 120000 },
            { MobID::kDarkLadybug, 100000 }, { MobID::kBoulder, 10000 },
            { MobID::kSquare, 1 }}), 3600.f, 4800.f, 1);
        break;
    case MapID::kDesert:
        add(eco_from(soft, "Open Dunes", 1, 0.9f, {
            { MobID::kCactus, 130000 }, { MobID::kDuneBeetle, 130000 },
            { MobID::kSandMite, 155000 }, { MobID::kScorpion, 85000 },
            { MobID::kSquare, 1 }}), 2800.f, 4200.f, 3);
        add(eco_from(mid, "Cactus Maze", 2, 1.05f, {
            { MobID::kCactus, 130000 }, { MobID::kDuneBeetle, 130000 },
            { MobID::kDuneSpider, 105000 }, { MobID::kDuneHornet, 95000 },
            { MobID::kScorpion, 85000 }, { MobID::kSquare, 1 }}), 2400.f, 3600.f, 3);
        add(eco_from(mid, "Dry River", 3, 1.0f, {
            { MobID::kDuneBeetle, 130000 }, { MobID::kDuneSpider, 105000 },
            { MobID::kDuneHornet, 95000 }, { MobID::kSandstorm, 90000 },
            { MobID::kScorpion, 85000 }, { MobID::kSquare, 1 }}), 2200.f, 3400.f, 2);
        add(eco_from(hard, "Bone Yard", 4, 1.3f, {
            { MobID::kDuneSpider, 105000 }, { MobID::kSandstorm, 90000 },
            { MobID::kScorpion, 85000 }, { MobID::kDesertCentipede, 22000 },
            { MobID::kDuneTitan, 14000 }, { MobID::kSquare, 1 }}), 2000.f, 3000.f, 2);
        add(eco_from(hard, "Sandworm Nest", 6, 1.45f, {
            { MobID::kSandstorm, 90000 }, { MobID::kScorpion, 85000 },
            { MobID::kDesertCentipede, 22000 }, { MobID::kDuneTitan, 14000 },
            { MobID::kSquare, 1 }}), 3600.f, 4800.f, 1);
        break;
    case MapID::kOcean:
        add(eco_from(soft, "Shallow Sea", 1, 0.9f, {
            { MobID::kSeaFry, 150000 }, { MobID::kMoonJelly, 110000 },
            { MobID::kCoralStar, 120000 }, { MobID::kReefCrab, 100000 },
            { MobID::kSquare, 1 }}), 2800.f, 4200.f, 3);
        add(eco_from(mid, "Coral Maze", 2, 1.05f, {
            { MobID::kMoonJelly, 110000 }, { MobID::kCoralStar, 120000 },
            { MobID::kReefCrab, 100000 }, { MobID::kSpinePuffer, 105000 },
            { MobID::kSquare, 1 }}), 2400.f, 3600.f, 3);
        add(eco_from(mid, "Kelp Forest", 3, 1.0f, {
            { MobID::kReefCrab, 100000 }, { MobID::kSpinePuffer, 105000 },
            { MobID::kSeaLeech, 24000 }, { MobID::kMorayEel, 70000 },
            { MobID::kSquare, 1 }}), 2200.f, 3400.f, 2);
        add(eco_from(hard, "Shipwreck", 4, 1.3f, {
            { MobID::kSpinePuffer, 105000 }, { MobID::kSeaLeech, 24000 },
            { MobID::kMorayEel, 70000 }, { MobID::kInkCuttle, 20000 },
            { MobID::kAbyssShark, 14000 }, { MobID::kSquare, 1 }}), 2000.f, 3000.f, 2);
        add(eco_from(hard, "Abyss Core", 6, 1.45f, {
            { MobID::kSeaLeech, 24000 }, { MobID::kMorayEel, 70000 },
            { MobID::kInkCuttle, 20000 }, { MobID::kAbyssShark, 14000 },
            { MobID::kSquare, 1 }}), 3600.f, 4800.f, 1);
        break;
    case MapID::kPolar:
        add(eco_from(soft, "Snow Fields", 1, 0.9f, {
            { MobID::kSnowHare, 135000 }, { MobID::kIceSpider, 120000 },
            { MobID::kFrostHornet, 105000 }, { MobID::kSnowMite, 155000 },
            { MobID::kSquare, 1 }}), 2800.f, 4200.f, 3);
        add(eco_from(mid, "Pine Maze", 2, 1.05f, {
            { MobID::kSnowHare, 135000 }, { MobID::kIceSpider, 120000 },
            { MobID::kFrostHornet, 105000 }, { MobID::kRimeCentipede, 22000 },
            { MobID::kGlacierBeast, 90000 }, { MobID::kSquare, 1 }}), 2400.f, 3600.f, 3);
        add(eco_from(mid, "Ice River", 3, 1.0f, {
            { MobID::kIceSpider, 120000 }, { MobID::kRimeCentipede, 22000 },
            { MobID::kBlizzardWisp, 110000 }, { MobID::kGlacierBeast, 90000 },
            { MobID::kSquare, 1 }}), 2200.f, 3400.f, 2);
        add(eco_from(hard, "Frost Rift", 4, 1.3f, {
            { MobID::kRimeCentipede, 22000 }, { MobID::kBlizzardWisp, 110000 },
            { MobID::kGlacierBeast, 90000 }, { MobID::kFrostYeti, 18000 },
            { MobID::kIceGolem, 14000 }, { MobID::kSquare, 1 }}), 2000.f, 3000.f, 2);
        add(eco_from(hard, "Frost Core", 6, 1.45f, {
            { MobID::kRimeCentipede, 22000 }, { MobID::kGlacierBeast, 90000 },
            { MobID::kFrostYeti, 18000 }, { MobID::kIceGolem, 14000 },
            { MobID::kSquare, 1 }}), 3600.f, 4800.f, 1);
        break;
    case MapID::kJungle:
        // Fewer, larger pockets (wasp in every pocket)
        add(eco_from(soft, "Jungle Edge", 1, 0.95f, {
            { MobID::kLeafBeetle, 140000 }, { MobID::kJungleMite, 130000 }, { MobID::kVineBug, 90000 },
            { MobID::kJungleWasp, 100000 }, { MobID::kSquare, 1 }}), 2800.f, 4200.f, 3);
        add(eco_from(mid, "Dense Canopy", 2, 1.1f, {
            { MobID::kLeafBeetle, 100000 }, { MobID::kJungleSpider, 130000 }, { MobID::kVineBug, 90000 },
            { MobID::kEmeraldMantis, 70000 }, { MobID::kJungleWasp, 100000 }, { MobID::kSquare, 1 }}), 2400.f, 3600.f, 3);
        add(eco_from(mid, "Poison Grove", 2, 1.05f, {
            { MobID::kPoisonToad, 110000 }, { MobID::kSporeMushroom, 120000 },
            { MobID::kCarnivorousPlant, 80000 }, { MobID::kJungleMite, 90000 },
            { MobID::kJungleWasp, 100000 }, { MobID::kSquare, 1 }}), 2200.f, 3400.f, 2);
        add(eco_from(mid, "Jungle River", 3, 1.0f, {
            { MobID::kRiverCrab, 130000 }, { MobID::kPoisonToad, 100000 }, { MobID::kJungleSpider, 110000 },
            { MobID::kJungleWasp, 100000 }, { MobID::kSquare, 1 }}), 2200.f, 3400.f, 2);
        add(eco_from(hard, "Ancient Ruins", 4, 1.35f, {
            { MobID::kEmeraldMantis, 110000 }, { MobID::kJungleSpider, 100000 },
            { MobID::kStoneGuardian, 90000 }, { MobID::kCarnivorousPlant, 80000 },
            { MobID::kJungleWasp, 100000 }, { MobID::kSquare, 1 }}), 2000.f, 3000.f, 2);
        add(eco_from(hard, "Heart of Jungle", 6, 1.5f, {
            { MobID::kEmeraldMantis, 100000 }, { MobID::kStoneGuardian, 110000 },
            { MobID::kCarnivorousPlant, 100000 }, { MobID::kVineBug, 80000 },
            { MobID::kJungleWasp, 100000 }, { MobID::kSquare, 1 }}), 3600.f, 4800.f, 1);
        break;
    case MapID::kAntHell:
        add(eco_from(soft, "Forage Halls", 1, 1.0f, {
            { MobID::kBabyAnt, 160000 }, { MobID::kWorkerAnt, 130000 },
            { MobID::kScoutAnt, 100000 }, { MobID::kAntHole, 4000 },
            { MobID::kSquare, 1 }}), 2800.f, 4200.f, 4);
        add(eco_from(mid, "Dirt Maze", 2, 1.1f, {
            { MobID::kWorkerAnt, 120000 }, { MobID::kSoldierAnt, 100000 },
            { MobID::kFangAnt, 90000 }, { MobID::kScoutAnt, 70000 },
            { MobID::kBabyAnt, 60000 }, { MobID::kAntHole, 5000 },
            { MobID::kSquare, 1 }}), 2400.f, 3600.f, 4);
        add(eco_from(mid, "Fungal Farm", 3, 1.05f, {
            { MobID::kWorkerAnt, 110000 }, { MobID::kBabyAnt, 100000 },
            { MobID::kSporeAnt, 95000 }, { MobID::kBroodCarrier, 80000 },
            { MobID::kAntHole, 4000 }, { MobID::kSquare, 1 }}), 2200.f, 3400.f, 3);
        add(eco_from(hard, "Soldier Patrol", 4, 1.35f, {
            { MobID::kSoldierAnt, 120000 }, { MobID::kTunnelGuard, 100000 },
            { MobID::kArmoredAnt, 90000 }, { MobID::kWingedAnt, 70000 },
            { MobID::kAntHole, 14000 }, { MobID::kSquare, 1 }}), 2000.f, 3000.f, 3);
        add(eco_from(hard, "Queen Nest", 6, 1.55f, {
            { MobID::kRoyalSentinel, 120000 }, { MobID::kSoldierAnt, 100000 },
            { MobID::kTunnelGuard, 70000 }, { MobID::kAntHole, 18000 },
            { MobID::kSquare, 1 }}), 3600.f, 4800.f, 1);
        break;
    default: // Volcano
        add(eco_from(soft, "Ash Plain", 1, 0.9f, {
            { MobID::kAshMite, 155000 }, { MobID::kCinderMoth, 135000 },
            { MobID::kLavaBeetle, 125000 }, { MobID::kEmberWasp, 105000 },
            { MobID::kSquare, 1 }}), 2800.f, 4200.f, 3);
        add(eco_from(mid, "Obsidian Maze", 2, 1.05f, {
            { MobID::kCinderMoth, 135000 }, { MobID::kLavaBeetle, 125000 },
            { MobID::kEmberWasp, 105000 }, { MobID::kMagmaCrab, 105000 },
            { MobID::kFireSpider, 110000 }, { MobID::kSquare, 1 }}), 2400.f, 3600.f, 3);
        add(eco_from(mid, "Lava Banks", 3, 1.0f, {
            { MobID::kLavaBeetle, 125000 }, { MobID::kMagmaCrab, 105000 },
            { MobID::kFireSpider, 110000 }, { MobID::kMagmaCentipede, 22000 },
            { MobID::kSquare, 1 }}), 2200.f, 3400.f, 2);
        add(eco_from(hard, "Magma Nest", 7, 1.35f, {
            { MobID::kMagmaCrab, 105000 }, { MobID::kFireSpider, 110000 },
            { MobID::kMagmaCentipede, 22000 }, { MobID::kObsidianGolem, 18000 },
            { MobID::kLavaColossus, 14000 }, { MobID::kSquare, 1 }}), 2000.f, 3000.f, 2);
        add(eco_from(hard, "Molten Heart", 8, 1.5f, {
            { MobID::kFireSpider, 110000 }, { MobID::kMagmaCentipede, 22000 },
            { MobID::kObsidianGolem, 18000 }, { MobID::kLavaColossus, 14000 },
            { MobID::kSquare, 1 }}), 3600.f, 4800.f, 1);
        break;
    }

    // Landmark grid: place pockets around the map so they don't pile on the hub.
    // Color matches difficulty ring (unused for paint after ring-only ground).
    for (uint32_t pi = 0; pi < pockets.size(); ++pi) {
        Pocket const &pk = pockets[pi];
        for (uint32_t n = 0; n < pk.count; ++n) {
            // Large eco zones (少而大): inflate design sizes ~1.5× for path-maze hunting grounds.
            float ww = rng_range(rng, pk.min_sz * 1.45f, pk.max_sz * 1.55f);
            float hh = rng_range(rng, pk.min_sz * 1.05f, pk.max_sz * 1.35f);
            // Prefer corners / mid-rings via angle buckets
            float ang = (float)(2.0 * M_PI) * ((float)(pi * 3 + n) + rng_f(rng) * 0.35f) / 12.f;
            float dist = rng_range(rng, 3800.f, 10500.f);
            if (pk.def.difficulty >= RarityID::kLegendary)
                dist = rng_range(rng, 6500.f, 11500.f);
            else if (pk.def.difficulty <= RarityID::kUnusual)
                dist = rng_range(rng, 3200.f, 7500.f);
            // The spawn hub must stay free of eco pockets. Push the pocket outward
            // along its own bearing until its AABB clears the hub rectangle —
            // a plain "distance > X" test dropped most pockets near the centre.
            {
                float ca = std::cos(ang), sa = std::sin(ang);
                float need = 0.f;
                if (std::fabs(ca) > 1e-3f)
                    need = std::max(need, (hub_hw + ww * 0.5f) / std::fabs(ca));
                if (std::fabs(sa) > 1e-3f)
                    need = std::max(need, (hub_hh + hh * 0.5f) / std::fabs(sa));
                if (dist < need) dist = need;
            }
            float mx = cx + std::cos(ang) * dist;
            float my = cy + std::sin(ang) * dist;
            float x = fclamp(mx - ww * 0.5f, 350.f, W - ww - 350.f);
            float y = fclamp(my - hh * 0.5f, 350.f, H - hh - 350.f);
            uint32_t col = RARITY_COLORS[std::min<uint32_t>(pk.def.difficulty, MAX_DIFFICULTY)];
            push_zone(zones, pk.def, x, y, x + ww, y + hh, (uint8_t)pk.def.difficulty, col);
        }
    }
}

// Rarity rings (Common hub → Ultra outer) + biome eco-pockets (later indices win).
void build_zones(MapZones &zones, uint8_t map_id, uint32_t &rng,
                 float W, float H, float cx, float cy) {
    zones.clear();
    MapZones const &tmpl = MAPS[map_id];
    ZoneDefinition hard = tmpl[0], mid = tmpl[1], soft = tmpl[2], hub = tmpl[3];

    float hub_hw = rng_range(rng, 2800.f, 3400.f);
    float hub_hh = rng_range(rng, 2100.f, 2600.f);

    // Outer → inner insets (fraction of arena). Index 0 = Ultra (full map).
    // Nine bands align with RarityID / petal colours on the G-key minimap.
    static float const kInset[9] = {
        0.00f, 0.045f, 0.095f, 0.150f, 0.210f, 0.280f, 0.360f, 0.450f, 0.55f
    };
    auto pool_for = [&](uint32_t diff) -> ZoneDefinition {
        if (diff == 0) return hub;
        if (diff <= RarityID::kUnusual) return soft;
        if (diff <= RarityID::kEpic) return mid;
        return hard;
    };
    auto dens_for = [](uint32_t diff) -> float {
        static float const k[] = {
            0.18f, 2.4f, 2.55f, 2.7f, 2.85f, 3.0f, 3.15f, 3.25f, 3.35f
        };
        return k[diff > MAX_DIFFICULTY ? MAX_DIFFICULTY : diff];
    };

    for (uint32_t step = 0; step <= MAX_DIFFICULTY; ++step) {
        uint32_t diff = MAX_DIFFICULTY - step; // Ultra first, Common last (later wins)
        ZoneDefinition z = pool_for(diff);
        z.density = dens_for(diff);
        z.drop_multiplier = zone_drop_tilt(diff);
        float m = kInset[step];
        float l = W * m, t = H * m, r = W * (1.f - m), b = H * (1.f - m);
        if (diff == 0) {
            l = cx - hub_hw; t = cy - hub_hh;
            r = cx + hub_hw; b = cy + hub_hh;
        }
        push_zone(zones, z, l, t, r, b, (uint8_t)diff, RARITY_COLORS[diff]);
    }

    append_biome_ecos(zones, map_id, hard, mid, soft, rng, W, H, cx, cy, hub_hw, hub_hh);
}

void berm(MapTerrain &t, ThemePalette const &p, uint32_t &rng, float W, float H, float cx, float cy) {
    float berm_t = rng_range(rng, 340.f, 460.f);
    float open = rng_range(rng, 1500.f, 2000.f);
    hill_box(t, p, 1, 0, 0, cx - open * 0.5f, berm_t);
    hill_box(t, p, 2, cx + open * 0.5f, 0, W, berm_t);
    hill_box(t, p, 3, 0, H - berm_t, cx - open * 0.5f, H);
    hill_box(t, p, 4, cx + open * 0.5f, H - berm_t, W, H);
    hill_box(t, p, 5, 0, berm_t, berm_t, cy - open * 0.5f);
    hill_box(t, p, 6, 0, cy + open * 0.5f, berm_t, H - berm_t);
    hill_box(t, p, 7, W - berm_t, berm_t, W, cy - open * 0.5f);
    hill_box(t, p, 8, W - berm_t, cy + open * 0.5f, W, H - berm_t);
}

// Point on a travel channel — used while rebuilding terrain.
uint8_t point_on_channel(MapTerrain const &t, float x, float y) {
    for (TerrainFeature const &f : t) {
        if (f.kind != TerrainKind::kChannel) continue;
        if (Map::point_in_feature(f, x, y)) return 1;
    }
    return 0;
}

// Emit up to 4 AABB remnants around a cleared rect punched out of src.
template <uint32_t Cap>
void emit_aabb_minus_rect(StaticArray<TerrainFeature, Cap> &out, TerrainFeature const &src,
                          float cl, float ct, float cr, float cb) {
    float L = src.left, T = src.top, R = src.right, B = src.bottom;
    float pl = std::max(L, cl), pt = std::max(T, ct), pr = std::min(R, cr), pb = std::min(B, cb);
    if (pl >= pr || pt >= pb) {
        if (out.size() < Cap) out.push(src);
        return;
    }
    auto emit = [&](float l, float top, float r, float b) {
        if (r - l < 80.f || b - top < 80.f) return;
        if (out.size() >= Cap) return;
        TerrainFeature g = src;
        g.left = l; g.top = top; g.right = r; g.bottom = b;
        out.push(g);
    };
    if (pt > T + 40.f) emit(L, T, R, pt);
    if (B > pb + 40.f) emit(L, pb, R, B);
    if (pl > L + 40.f) emit(L, pt, pl, pb);
    if (R > pr + 40.f) emit(pr, pt, R, pb);
}

// Punch a vertical (axis==0) or horizontal (axis==1) band out of an AABB.
template <uint32_t Cap>
void emit_aabb_minus_band(StaticArray<TerrainFeature, Cap> &out, TerrainFeature const &src,
                          float c0, float c1, uint8_t axis) {
    float L = src.left, T = src.top, R = src.right, B = src.bottom;
    if (axis == 0) {
        float a = std::max(L, c0), b = std::min(R, c1);
        if (b <= a) {
            if (out.size() < Cap) out.push(src);
            return;
        }
        auto emit = [&](float l, float r) {
            if (r - l < 80.f) return;
            if (out.size() >= Cap) return;
            TerrainFeature g = src;
            g.left = l; g.right = r;
            out.push(g);
        };
        emit(L, a);
        emit(b, R);
    } else {
        float a = std::max(T, c0), b = std::min(B, c1);
        if (b <= a) {
            if (out.size() < Cap) out.push(src);
            return;
        }
        auto emit = [&](float top, float bot) {
            if (bot - top < 80.f) return;
            if (out.size() >= Cap) return;
            TerrainFeature g = src;
            g.top = top; g.bottom = bot;
            out.push(g);
        };
        emit(T, a);
        emit(b, B);
    }
}

// Sample channel spine through an AABB → cut band (axis + [c0,c1]).
uint8_t channel_cut_band(TerrainFeature const &ch, float L, float T, float R, float B,
                         float &c0, float &c1, uint8_t &axis) {
    // Corridor/gate paint uses AABB channels — carve their box, not a fake river chord.
    if (ch.shape == TerrainShape::kAABB) {
        float cl = ch.left, ct = ch.top, cr = ch.right, cb = ch.bottom;
        float iw = std::min(R, cr) - std::max(L, cl);
        float ih = std::min(B, cb) - std::max(T, ct);
        if (iw <= 40.f || ih <= 40.f) return 0;
        if ((cr - cl) >= (cb - ct)) {
            axis = 1; // horizontal lane → punch Y band
            c0 = std::max(T, ct);
            c1 = std::min(B, cb);
        } else {
            axis = 0;
            c0 = std::max(L, cl);
            c1 = std::min(R, cr);
        }
        return (c1 - c0) > 40.f ? 1 : 0;
    }
    float hw = Map::feature_half_width(ch) + 100.f; // player-margin carve
    float xmin = 1e9f, xmax = -1e9f, ymin = 1e9f, ymax = -1e9f;
    int n = 0;
    for (int i = 0; i <= 56; ++i) {
        float t = (float)i / 56.f;
        float cx, cy;
        Map::river_center_at(ch, t, cx, cy);
        if (cx + hw < L || cx - hw > R || cy + hw < T || cy - hw > B) continue;
        xmin = std::min(xmin, cx - hw);
        xmax = std::max(xmax, cx + hw);
        ymin = std::min(ymin, cy - hw);
        ymax = std::max(ymax, cy + hw);
        ++n;
    }
    if (!n) return 0;
    float xspan = xmax - xmin, yspan = ymax - ymin;
    if (xspan <= yspan) {
        axis = 0;
        c0 = std::max(L, xmin);
        c1 = std::min(R, xmax);
    } else {
        axis = 1;
        c0 = std::max(T, ymin);
        c1 = std::min(B, ymax);
    }
    return (c1 - c0) > 40.f ? 1 : 0;
}

// After arteries are placed: carve them (and the spawn plaza) out of impassable
// hills/water so geometry matches collision and cannot trap players in pockets.
void resolve_terrain_overlaps(MapTerrain &t) {
    MapTerrain channels;
    for (TerrainFeature const &f : t) {
        if (f.kind == TerrainKind::kChannel && channels.size() < MAX_TERRAIN_PER_MAP)
            channels.push(f);
    }

    float px = MAP_SPAWN_X, py = MAP_SPAWN_Y;
    // Plaza clear box ≈ spawn ellipse (5200×4000)
    float pl = px - 4800.f, pt = py - 3600.f, pr = px + 4800.f, pb = py + 3600.f;

    MapTerrain out;
    // Keep channels first
    for (TerrainFeature const &f : channels) {
        if (out.size() < MAX_TERRAIN_PER_MAP) out.push(f);
    }

    for (TerrainFeature const &f : t) {
        if (f.kind == TerrainKind::kChannel) continue;
        if (!(f.flags & TerrainFlags::kImpassable)) {
            if (out.size() < MAX_TERRAIN_PER_MAP) out.push(f);
            continue;
        }

        // Circles / ellipses: drop if center sits on artery or plaza
        if (f.shape == TerrainShape::kCircle || f.shape == TerrainShape::kEllipse) {
            float cx = f.left, cy = f.top;
            if (Map::in_spawn_plaza(cx, cy) || point_on_channel(channels, cx, cy))
                continue;
            if (out.size() < MAX_TERRAIN_PER_MAP) out.push(f);
            continue;
        }

        // Path waters: keep geometry; plaza bridges are added after the rebuild
        // so dual-X rivers cannot leave a hard cross-block at the hub.
        if (f.shape == TerrainShape::kRiver || f.shape == TerrainShape::kSpiral) {
            if (out.size() < MAX_TERRAIN_PER_MAP) out.push(f);
            continue;
        }

        // AABB hills/water: punch plaza, then each channel band
        if (f.shape != TerrainShape::kAABB) {
            if (out.size() < MAX_TERRAIN_PER_MAP) out.push(f);
            continue;
        }

        StaticArray<TerrainFeature, 24> rects;
        rects.push(f);

        // Plaza punch
        {
            StaticArray<TerrainFeature, 24> next;
            for (uint32_t i = 0; i < rects.size(); ++i) {
                TerrainFeature const &r = rects[i];
                float cx = 0.5f * (r.left + r.right), cy = 0.5f * (r.top + r.bottom);
                if (Map::in_spawn_plaza(cx, cy) &&
                    (r.right - r.left) < 3600.f && (r.bottom - r.top) < 3600.f)
                    continue; // small blocker wholly in plaza
                emit_aabb_minus_rect(next, r, pl, pt, pr, pb);
            }
            rects = next;
        }

        // Channel punches (iterate channels × current rects)
        for (TerrainFeature const &ch : channels) {
            StaticArray<TerrainFeature, 48> next;
            for (uint32_t i = 0; i < rects.size(); ++i) {
                TerrainFeature const &r = rects[i];
                float c0, c1;
                uint8_t axis = 0;
                if (!channel_cut_band(ch, r.left, r.top, r.right, r.bottom, c0, c1, axis)) {
                    if (next.size() < 48) next.push(r);
                    continue;
                }
                // Fully covered by cut → drop
                if (axis == 0 && c0 <= r.left + 40.f && c1 >= r.right - 40.f) continue;
                if (axis == 1 && c0 <= r.top + 40.f && c1 >= r.bottom - 40.f) continue;
                StaticArray<TerrainFeature, 8> tmp;
                emit_aabb_minus_band(tmp, r, c0, c1, axis);
                for (TerrainFeature const &g : tmp) {
                    if (next.size() < 48) next.push(g);
                }
            }
            // Copy next → rects (cap at 24 for next channel pass)
            rects.clear();
            for (uint32_t i = 0; i < next.size() && rects.size() < 24; ++i)
                rects.push(next[i]);
        }

        for (uint32_t i = 0; i < rects.size(); ++i) {
            TerrainFeature const &r = rects[i];
            float cx = 0.5f * (r.left + r.right), cy = 0.5f * (r.top + r.bottom);
            if (Map::in_spawn_plaza(cx, cy)) continue;
            if (point_on_channel(channels, cx, cy)) continue;
            if (out.size() < MAX_TERRAIN_PER_MAP) out.push(r);
        }
    }

    // Impassable rivers that skim the spawn plaza get a channel ford: collision
    // walks, and the road texture marks the gap (no invisible air-wall cross).
    uint8_t bridge_seed = 200;
    uint32_t n0 = out.size();
    for (uint32_t i = 0; i < n0 && out.size() < MAX_TERRAIN_PER_MAP; ++i) {
        TerrainFeature const &f = out[i];
        if (!(f.flags & TerrainFlags::kImpassable)) continue;
        if (f.shape != TerrainShape::kRiver) continue;
        float t_lo = 2.f, t_hi = -1.f;
        float hw = Map::feature_half_width(f) + 160.f;
        for (int s = 0; s <= 32; ++s) {
            float tt = (float)s / 32.f;
            float x, y;
            Map::river_center_at(f, tt, x, y);
            float dx = (x - MAP_SPAWN_X) / (5200.f + hw);
            float dy = (y - MAP_SPAWN_Y) / (4000.f + hw);
            if (dx * dx + dy * dy >= 1.f) continue;
            if (tt < t_lo) t_lo = tt;
            if (tt > t_hi) t_hi = tt;
        }
        if (t_hi < t_lo) continue;
        t_lo = std::max(0.f, t_lo - 0.05f);
        t_hi = std::min(1.f, t_hi + 0.05f);
        float x0, y0, x1, y1;
        Map::river_center_at(f, t_lo, x0, y0);
        Map::river_center_at(f, t_hi, x1, y1);
        float ford_hw = hw + 40.f;
        uint32_t ch_color = f.color;
        for (uint8_t m = 0; m < MapID::kNumMaps; ++m) {
            if (kThemes[m].water == f.color) { ch_color = kThemes[m].channel; break; }
        }
        push_feat(out, TerrainKind::kChannel, TerrainShape::kRiver, 0,
                  x0, y0, x1, y1, ch_color, bridge_seed++, ford_hw);
    }

    t = out;
}

// ---- Biome terrain ----

void gen_garden(MapTerrain &t, ThemePalette const &p, uint32_t &rng, float W, float H, float cx, float cy) {
    // 大路主廊道略加宽；迷宫内口仍由 kMinMazeGate 保底
    float gap = rng_range(rng, 1250.f, 1600.f);
    float wall = rng_range(rng, 420.f, 580.f);
    // Hedge maze blocks (tree-hedge walls) — 小路门宽
    maze_cell(t, p, rng, 30, W * 0.08f, H * 0.08f, W * 0.42f, H * 0.42f, wall, gap * 0.65f);
    maze_cell(t, p, rng, 50, W * 0.55f, H * 0.10f, W * 0.92f, H * 0.48f, wall * 0.9f, gap * 0.6f);
    maze_cell(t, p, rng, 70, W * 0.10f, H * 0.55f, W * 0.45f, H * 0.92f, wall, gap * 0.65f);
    // 东西向大路廊道串联迷宫与螺旋农场。
    // 不刷 channel 路面：廊道穿过出生点，首页标题镜头正好扫这条暖色横带。
    corridor_h(t, p, 40, W * 0.18f, W * 0.82f, cy - 200.f, gap * 0.95f, wall * 0.85f, 0, 0);
    // Spiral farm → flower-heart valley (Boss: Flower Guardian)
    float bx = 0.f, by = 0.f;
    spiral_farm(t, p, rng, cx + 2000.f, cy - 200.f, 0, +1, gap, wall,
                rng_range(rng, 2600.f, 3400.f), 3, rng_range(rng, 1300.f, 1700.f), 90, 0,
                &bx, &by);
    // No diagonal impassable creeks — dual X rivers looked like roads and trapped
    // players at the intersection (push-out from both banks fights itself).
    // Boulder prairie + mushroom rings
    rock_field(t, p, rng, 120, W * 0.55f, H * 0.55f, W * 0.90f, H * 0.90f, 8, 280.f, 520.f);
    ring_valley(t, p, rng, W * 0.22f, H * 0.78f, 900.f, 400.f, 0.8f, 0.6f, 140, 0);
    ring_valley(t, p, rng, W * 0.78f, H * 0.22f, 850.f, 380.f, 2.2f, 0.55f, 160, 0); // thorn pocket
    g_boss[MapID::kGarden] = { bx, by, (uint8_t)MobID::kFlowerGuardian };
}

void gen_desert(MapTerrain &t, ThemePalette const &p, uint32_t &rng, float W, float H, float cx, float cy) {
    float gap = rng_range(rng, 1200.f, 1550.f);
    float wall = rng_range(rng, 480.f, 650.f);
    // Dune belts with gates.
    // Two gates per belt: a random western gate (main progression) and one
    // aligned with the spiral corridor so the corridor can cross the belt
    // instead of being sealed off by it.
    float spiral_x = cx + 5040.f;
    for (uint32_t i = 0; i < 4; ++i) {
        float y = H * (0.18f + 0.18f * (float)i) + rng_range(rng, -120.f, 120.f);
        float gate = rng_range(rng, W * 0.16f, W * 0.46f);
        // West gate stays wide (≥1000); corridor seam ~0.5 block so belts still feel oppressive.
        float gw = std::max(1000.f, gap * 0.9f);
        float gw2 = rng_range(rng, 480.f, 560.f);
        float y0 = y - wall * 0.5f, y1 = y + wall * 0.5f;
        uint8_t s = (uint8_t)(30 + i * 2);
        // Segment 1: west edge → main gate
        hill_box(t, p, s, 500.f, y0, gate - gw * 0.5f, y1);
        // Segment 2: main gate → corridor-aligned gate
        hill_box(t, p, (uint8_t)(s + 1), gate + gw * 0.5f, y0, spiral_x - gw2 * 0.5f, y1);
        // Segment 3: corridor gate → east edge
        hill_box(t, p, (uint8_t)(s + 2), spiral_x + gw2 * 0.5f, y0, W - 500.f, y1);
    }
    // Rock canyon maze — kept inside one dune layer so it never seals a belt
    float maze_x0 = W * 0.55f, maze_y0 = H * 0.39f, maze_x1 = W * 0.92f, maze_y1 = H * 0.52f;
    maze_cell(t, p, rng, 50, maze_x0, maze_y0, maze_x1, maze_y1, wall, gap * 0.6f);
    // C4: scattered rock/bone piles in canyon + bone yard
    rock_field(t, p, rng, 180, maze_x0, maze_y0, maze_x1, maze_y1, 6, 280.f, 520.f);
    rock_field(t, p, rng, 190, W * 0.72f, H * 0.18f, W * 0.94f, H * 0.40f, 5, 280.f, 520.f);
    // Sandworm nest: east corridor that crosses the belts, ending in a sealed basin
    float bw_x = 0.f, bw_y = 0.f;
    {
        float nest_x = spiral_x;
        float nest_y = H * 0.24f;
        float leg = rng_range(rng, 2600.f, 3200.f);
        corridor_h(t, p, 70, cx + 2200.f, nest_x, nest_y, gap, wall * 0.95f, 0);
        corridor_v(t, p, 74, nest_y, nest_y + leg, nest_x, gap, wall * 0.95f, 0);
        bw_x = nest_x;
        bw_y = nest_y + leg;
        ring_valley(t, p, rng, bw_x, bw_y, rng_range(rng, 1200.f, 1600.f), 460.f, 3.0f, 0.5f, 140, 0);
    }
    // Dry river bed (travel road; attack allowed)
    push_feat(t, TerrainKind::kChannel, TerrainShape::kRiver, 0,
              W * 0.1f, H * 0.15f, W * 0.9f, H * 0.88f, p.channel, 100, rng_range(rng, 350.f, 450.f));
    // Oasis + bone yard basin
    ring_valley(t, p, rng, W * 0.20f, H * 0.75f, 950.f, 420.f, 1.2f, 0.55f, 120, 1);
    ring_valley(t, p, rng, W * 0.82f, H * 0.28f, 1100.f, 450.f, 3.5f, 0.5f, 140, 0);
    // Boss: sandworm monarch at the end of the spiral nest
    g_boss[MapID::kDesert] = { bw_x, bw_y, (uint8_t)MobID::kSandwormMonarch };
}

void gen_ocean(MapTerrain &t, ThemePalette const &p, uint32_t &rng, float W, float H, float cx, float cy) {
    // Reefs = hills; deep trenches = water barriers; open sea walkable
    float gap = rng_range(rng, 1100.f, 1400.f);
    float wall = rng_range(rng, 400.f, 560.f);
    maze_cell(t, p, rng, 30, W * 0.50f, H * 0.08f, W * 0.92f, H * 0.45f, wall, gap * 0.65f);
    spiral_farm(t, p, rng, cx + 1600.f, cy + 600.f, 0, -1, gap, wall,
                rng_range(rng, 2500.f, 3300.f), 3, rng_range(rng, 1250.f, 1650.f), 60, 0);
    spiral_farm(t, p, rng, cx - 2200.f, cy - 800.f, 3, +1, gap * 0.9f, wall,
                rng_range(rng, 2300.f, 3000.f), 2, rng_range(rng, 1100.f, 1450.f), 90, 0);
    // Deep trench (SW→NE) split into 3 segments with 2 shallow fords ≥1000
    {
        float x0 = W * 0.1f, y0 = H * 0.72f, x1 = W * 0.9f, y1 = H * 0.28f;
        float dx = x1 - x0, dy = y1 - y0;
        float len = std::sqrt(dx * dx + dy * dy);
        float hw = rng_range(rng, 220.f, 300.f);
        float gt = 1100.f / std::max(len, 1.f); // ford gap in t-space
        float f0 = 0.30f, f1 = 0.70f;
        auto trench = [&](float ta, float tb, uint8_t seed) {
            if (tb - ta < 0.04f) return;
            push_feat(t, TerrainKind::kWater, TerrainShape::kRiver, TerrainFlags::kImpassable,
                      x0 + dx * ta, y0 + dy * ta, x0 + dx * tb, y0 + dy * tb,
                      p.water, seed, hw);
        };
        trench(0.f, f0 - gt * 0.5f, 110);
        trench(f0 + gt * 0.5f, f1 - gt * 0.5f, 111);
        trench(f1 + gt * 0.5f, 1.f, 112);
        // Paint fords so shallow crossings read as roads, not bare seabed.
        auto ford = [&](float ft, uint8_t seed) {
            float a = ft - gt * 0.45f, b = ft + gt * 0.45f;
            push_feat(t, TerrainKind::kChannel, TerrainShape::kRiver, 0,
                      x0 + dx * a, y0 + dy * a, x0 + dx * b, y0 + dy * b,
                      p.channel, seed, hw + 100.f);
        };
        ford(f0, 113);
        ford(f1, 114);
    }
    rock_field(t, p, rng, 130, W * 0.12f, H * 0.55f, W * 0.40f, H * 0.88f, 7, 300.f, 560.f);
    ring_valley(t, p, rng, W * 0.18f, H * 0.20f, 880.f, 400.f, 0.4f, 0.55f, 150, 0);
    ring_valley(t, p, rng, W * 0.82f, H * 0.80f, 1050.f, 430.f, 2.8f, 0.5f, 170, 0); // abyss core
    // Boss: abyss maw in the deep core basin
    g_boss[MapID::kOcean] = { W * 0.82f, H * 0.80f, (uint8_t)MobID::kAbyssMaw };
}

void gen_snow(MapTerrain &t, ThemePalette const &p, uint32_t &rng, float W, float H, float cx, float cy) {
    float gap = rng_range(rng, 1150.f, 1450.f);
    float wall = rng_range(rng, 450.f, 600.f);
    maze_cell(t, p, rng, 30, W * 0.08f, H * 0.50f, W * 0.48f, H * 0.92f, wall, gap * 0.7f);
    spiral_farm(t, p, rng, cx - 400.f, cy + 2200.f, 1, +1, gap, wall,
                rng_range(rng, 2700.f, 3500.f), 3, rng_range(rng, 1300.f, 1700.f), 70, 0);
    spiral_farm(t, p, rng, cx + 2400.f, cy - 1600.f, 2, -1, gap * 0.9f, wall,
                rng_range(rng, 2400.f, 3100.f), 2, rng_range(rng, 1150.f, 1500.f), 100, 1);
    // Main ice river (2 shallows) + NW tributary
    {
        float x0 = W * 0.15f, y0 = H * 0.25f, x1 = W * 0.85f, y1 = H * 0.75f;
        float dx = x1 - x0, dy = y1 - y0;
        float len = std::sqrt(dx * dx + dy * dy);
        float hw = rng_range(rng, 200.f, 260.f);
        float gt = 1100.f / std::max(len, 1.f);
        float f0 = 0.28f, f1 = 0.65f;
        auto ice = [&](float ta, float tb, uint8_t seed) {
            if (tb - ta < 0.04f) return;
            push_feat(t, TerrainKind::kWater, TerrainShape::kRiver, TerrainFlags::kImpassable,
                      x0 + dx * ta, y0 + dy * ta, x0 + dx * tb, y0 + dy * tb,
                      p.water, seed, hw);
        };
        ice(0.f, f0 - gt * 0.5f, 120);
        ice(f0 + gt * 0.5f, f1 - gt * 0.5f, 121);
        ice(f1 + gt * 0.5f, 1.f, 122);
        auto ford = [&](float ft, uint8_t seed) {
            float a = ft - gt * 0.45f, b = ft + gt * 0.45f;
            push_feat(t, TerrainKind::kChannel, TerrainShape::kRiver, 0,
                      x0 + dx * a, y0 + dy * a, x0 + dx * b, y0 + dy * b,
                      p.channel, seed, hw + 100.f);
        };
        ford(f0, 124);
        ford(f1, 125);
        float mx = x0 + dx * 0.45f, my = y0 + dy * 0.45f;
        push_feat(t, TerrainKind::kWater, TerrainShape::kRiver, TerrainFlags::kImpassable,
                  mx, my, W * 0.22f, H * 0.58f, p.water, 123, hw * 0.85f);
    }
    rock_field(t, p, rng, 140, W * 0.55f, H * 0.55f, W * 0.90f, H * 0.90f, 6, 320.f, 580.f);
    ring_valley(t, p, rng, W * 0.80f, H * 0.20f, 1000.f, 420.f, 3.8f, 0.5f, 160, 1);
    // Boss: winter sovereign in the frost core basin
    g_boss[MapID::kPolar] = { W * 0.80f, H * 0.20f, (uint8_t)MobID::kWinterSovereign };
}

void gen_jungle(MapTerrain &t, ThemePalette const &p, uint32_t &rng, float W, float H, float cx, float cy) {
    // Dense canopy + vine corridors (thicker walls / slightly tighter gates than Garden)
    float gap = rng_range(rng, 1050.f, 1350.f);
    float wall = rng_range(rng, 480.f, 640.f);
    maze_cell(t, p, rng, 30, W * 0.06f, H * 0.06f, W * 0.46f, H * 0.46f, wall, gap * 0.55f);
    maze_cell(t, p, rng, 50, W * 0.54f, H * 0.52f, W * 0.94f, H * 0.94f, wall, gap * 0.55f);
    maze_cell(t, p, rng, 70, W * 0.54f, H * 0.06f, W * 0.94f, H * 0.44f, wall * 0.95f, gap * 0.6f);
    maze_cell(t, p, rng, 90, W * 0.06f, H * 0.54f, W * 0.46f, H * 0.94f, wall * 0.9f, gap * 0.6f);
    // Spiral into primordial tree-heart valley
    spiral_farm(t, p, rng, cx + 1600.f, cy + 200.f, 0, +1, gap, wall,
                rng_range(rng, 2400.f, 3100.f), 3, rng_range(rng, 1300.f, 1700.f), 90, 0);
    // Secondary vine corridor from SW clearing
    spiral_farm(t, p, rng, cx - 2800.f, cy + 1800.f, 1, -1, gap * 0.9f, wall * 0.85f,
                rng_range(rng, 2200.f, 2800.f), 2, rng_range(rng, 1000.f, 1350.f), 100, 0);
    // Swamp belts with hub fords — unbroken dual-X rivers froze players at the
    // spawn intersection (same trap Garden removed). Split each belt and leave
    // a painted channel bridge at the plaza.
    {
        auto swamp = [&](float x0, float y0, float x1, float y1,
                         float ford_t, uint8_t seed0, float hw) {
            float dx = x1 - x0, dy = y1 - y0;
            float len = std::sqrt(dx * dx + dy * dy);
            float gt = 1200.f / std::max(len, 1.f);
            auto seg = [&](float ta, float tb, uint8_t seed) {
                if (tb - ta < 0.04f) return;
                push_feat(t, TerrainKind::kWater, TerrainShape::kRiver, TerrainFlags::kImpassable,
                          x0 + dx * ta, y0 + dy * ta, x0 + dx * tb, y0 + dy * tb,
                          p.water, seed, hw);
            };
            seg(0.f, ford_t - gt * 0.5f, seed0);
            seg(ford_t + gt * 0.5f, 1.f, (uint8_t)(seed0 + 1));
            float fx0 = x0 + dx * (ford_t - gt * 0.45f);
            float fy0 = y0 + dy * (ford_t - gt * 0.45f);
            float fx1 = x0 + dx * (ford_t + gt * 0.45f);
            float fy1 = y0 + dy * (ford_t + gt * 0.45f);
            push_feat(t, TerrainKind::kChannel, TerrainShape::kRiver, 0,
                      fx0, fy0, fx1, fy1, p.channel, (uint8_t)(seed0 + 2), hw + 120.f);
        };
        swamp(W * 0.10f, H * 0.68f, W * 0.90f, H * 0.28f, 0.48f, 110,
              rng_range(rng, 230.f, 300.f));
        swamp(W * 0.18f, H * 0.15f, W * 0.78f, H * 0.88f, 0.52f, 112,
              rng_range(rng, 180.f, 240.f));
    }
    // Giant root clusters (circular canopy blockers)
    rock_field(t, p, rng, 120, W * 0.12f, H * 0.12f, W * 0.40f, H * 0.40f, 6, 360.f, 620.f);
    rock_field(t, p, rng, 140, W * 0.60f, H * 0.58f, W * 0.90f, H * 0.88f, 5, 320.f, 560.f);
    // Toxic pocket + insect nest + tree heart (boss)
    ring_valley(t, p, rng, W * 0.22f, H * 0.78f, 880.f, 400.f, 1.0f, 0.55f, 130, 0);
    ring_valley(t, p, rng, W * 0.72f, H * 0.70f, 820.f, 380.f, 2.2f, 0.5f, 145, 0);
    ring_valley(t, p, rng, W * 0.80f, H * 0.22f, 1150.f, 460.f, 2.6f, 0.48f, 160, 0);
    // Boss: the ancient treant waits in the primordial tree heart
    g_boss[MapID::kJungle] = { W * 0.80f, H * 0.22f, (uint8_t)MobID::kAncientTreant };
}

void gen_ant_hell(MapTerrain &t, ThemePalette const &p, uint32_t &rng, float W, float H, float cx, float cy) {
    // Dirt walls = hills; passable tunnels = gaps. Dense chamber network.
    float gap = rng_range(rng, 1000.f, 1300.f);
    float wall = rng_range(rng, 480.f, 640.f);
    maze_cell(t, p, rng, 20, W * 0.08f, H * 0.08f, W * 0.50f, H * 0.50f, wall, gap * 0.55f);
    maze_cell(t, p, rng, 40, W * 0.48f, H * 0.08f, W * 0.92f, H * 0.50f, wall, gap * 0.55f);
    maze_cell(t, p, rng, 60, W * 0.08f, H * 0.48f, W * 0.50f, H * 0.92f, wall, gap * 0.55f);
    maze_cell(t, p, rng, 80, W * 0.48f, H * 0.48f, W * 0.92f, H * 0.92f, wall, gap * 0.55f);
    // Cross tunnels (corridor paint marks the travel lanes)
    corridor_h(t, p, 100, W * 0.15f, W * 0.85f, H * 0.35f, gap * 0.85f, wall * 0.7f, 0);
    corridor_v(t, p, 110, H * 0.15f, H * 0.85f, W * 0.35f, gap * 0.85f, wall * 0.7f, 0);
    corridor_h(t, p, 120, W * 0.15f, W * 0.85f, H * 0.65f, gap * 0.85f, wall * 0.7f, 0);
    // Queen chamber — fixed block (21.0, 14.0) for stable Boss / docs
    {
        float qx = W * (21.f / 28.f);
        float qy = H * (14.f / 20.f);
        ring_valley(t, p, rng, qx, qy, 1200.f, 500.f, 2.4f, 0.45f, 140, 0);
        g_boss[MapID::kAntHell] = { qx, qy, (uint8_t)MobID::kQueenAnt };
    }
    // Water pool chambers
    ring_valley(t, p, rng, W * 0.22f, H * 0.22f, 750.f, 360.f, 0.6f, 0.5f, 160, 1);
}

void gen_volcano(MapTerrain &t, ThemePalette const &p, uint32_t &rng, float W, float H, float cx, float cy) {
    float gap = rng_range(rng, 1100.f, 1400.f);
    float wall = rng_range(rng, 450.f, 600.f);
    // Lava rivers with cooled rock bridges (gaps) so quadrants aren't island-only
    auto lava_with_bridge = [&](float x0, float y0, float x1, float y1,
                                float ford_t, uint8_t seed0, float hw) {
        float dx = x1 - x0, dy = y1 - y0;
        float len = std::sqrt(dx * dx + dy * dy);
        float gt = 1000.f / std::max(len, 1.f);
        auto seg = [&](float ta, float tb, uint8_t seed) {
            if (tb - ta < 0.04f) return;
            push_feat(t, TerrainKind::kWater, TerrainShape::kRiver, TerrainFlags::kImpassable,
                      x0 + dx * ta, y0 + dy * ta, x0 + dx * tb, y0 + dy * tb,
                      p.water, seed, hw);
        };
        seg(0.f, ford_t - gt * 0.5f, seed0);
        seg(ford_t + gt * 0.5f, 1.f, (uint8_t)(seed0 + 1));
        float a = ford_t - gt * 0.45f, b = ford_t + gt * 0.45f;
        push_feat(t, TerrainKind::kChannel, TerrainShape::kRiver, 0,
                  x0 + dx * a, y0 + dy * a, x0 + dx * b, y0 + dy * b,
                  p.channel, (uint8_t)(seed0 + 2), hw + 120.f);
    };
    lava_with_bridge(W * 0.1f, H * 0.25f, W * 0.9f, H * 0.78f, 0.45f, 30,
                     rng_range(rng, 240.f, 320.f));
    lava_with_bridge(W * 0.15f, H * 0.80f, W * 0.85f, H * 0.20f, 0.55f, 32,
                     rng_range(rng, 220.f, 300.f));
    // Obsidian maze
    maze_cell(t, p, rng, 40, W * 0.10f, H * 0.10f, W * 0.45f, H * 0.48f, wall, gap * 0.6f);
    maze_cell(t, p, rng, 60, W * 0.55f, H * 0.50f, W * 0.90f, H * 0.90f, wall, gap * 0.6f);
    spiral_farm(t, p, rng, cx + 2000.f, cy - 500.f, 0, +1, gap, wall,
                rng_range(rng, 2400.f, 3100.f), 2, rng_range(rng, 1200.f, 1550.f), 80, 0);
    rock_field(t, p, rng, 100, W * 0.55f, H * 0.10f, W * 0.90f, H * 0.45f, 7, 300.f, 560.f);
    // Magma lakes + molten heart
    ring_valley(t, p, rng, W * 0.22f, H * 0.75f, 900.f, 400.f, 1.1f, 0.55f, 130, 1);
    ring_valley(t, p, rng, W * 0.78f, H * 0.25f, 1150.f, 480.f, 3.2f, 0.48f, 150, 1);
    // Boss: molten overlord in the molten heart basin
    g_boss[MapID::kVolcano] = { W * 0.78f, H * 0.25f, (uint8_t)MobID::kMoltenOverlord };
}

// Exactly 6 large AABBs from MAPS template — no specialty eco scatter.
void build_zones_royal(MapZones &zones, uint8_t map_id, float W, float H, float cx, float cy) {
    zones.clear();
    MapZones const &tmpl = MAPS[map_id];
    // Keep spawn plaza at MAP_SPAWN as Outer Wall (index 0 wins when no overlay).
    float plaza_hw = 5400.f;
    float plaza_hh = 4200.f;
    for (uint32_t i = 0; i < tmpl.size() && i < 6; ++i) {
        ZoneDefinition z = tmpl[i];
        if (i == 0) {
            push_zone(zones, z, 0, 0, W, H, z.difficulty, z.color);
            continue;
        }
        // Shrink overlays away from center spawn so Outer Wall remains under MAP_SPAWN.
        float l = z.left, t = z.top, r = z.right, b = z.bottom;
        if (l < cx + plaza_hw && r > cx - plaza_hw && t < cy + plaza_hh && b > cy - plaza_hh) {
            // Prefer cutting the side closest to center so plaza stays clear.
            float cut_l = (cx - plaza_hw) - l;
            float cut_r = r - (cx + plaza_hw);
            float cut_t = (cy - plaza_hh) - t;
            float cut_b = b - (cy + plaza_hh);
            if (cut_l > 0 && cut_l <= cut_r && cut_l <= cut_t && cut_l <= cut_b)
                r = cx - plaza_hw;
            else if (cut_r > 0 && cut_r <= cut_t && cut_r <= cut_b)
                l = cx + plaza_hw;
            else if (cut_t > 0 && cut_t <= cut_b)
                b = cy - plaza_hh;
            else if (cut_b > 0)
                t = cy + plaza_hh;
        }
        if (r - l < 800.f || b - t < 800.f) {
            push_zone(zones, z, z.left, z.top, z.right, z.bottom, z.difficulty, z.color);
            continue;
        }
        push_zone(zones, z, l, t, r, b, z.difficulty, z.color);
    }
}

// Broken walls, street grid, hedge maze, barracks corridors, ruins rocks, palace rings.
void gen_royal(MapTerrain &t, ThemePalette const &p, uint32_t &rng, float W, float H, float cx, float cy) {
    float gap = rng_range(rng, 1200.f, 1450.f);
    float wall = rng_range(rng, 420.f, 560.f);

    // Outer broken ring wall with south/west/east gates (city wall motif)
    {
        float thick = rng_range(rng, 380.f, 480.f);
        float outer = 900.f;
        float gate = gap * 1.05f;
        // North berm segments
        hill_box(t, p, 20, outer, outer, cx - gate * 0.55f, outer + thick);
        hill_box(t, p, 21, cx + gate * 0.55f, outer, W - outer, outer + thick);
        // South — main entrance plaza gate (wide)
        hill_box(t, p, 22, outer, H - outer - thick, cx - gate * 0.9f, H - outer);
        hill_box(t, p, 23, cx + gate * 0.9f, H - outer - thick, W - outer, H - outer);
        // West / East walls with mid gates
        hill_box(t, p, 24, outer, outer + thick, outer + thick, cy - gate * 0.5f);
        hill_box(t, p, 25, outer, cy + gate * 0.5f, outer + thick, H - outer - thick);
        hill_box(t, p, 26, W - outer - thick, outer + thick, W - outer, cy - gate * 0.5f);
        hill_box(t, p, 27, W - outer - thick, cy + gate * 0.5f, W - outer, H - outer - thick);
    }

    // Common Quarter — orthogonal street grid (SW)
    {
        float L = W * 0.06f, T = H * 0.42f, R = W * 0.42f, B = H * 0.92f;
        float street = gap * 0.85f;
        float block = wall * 0.85f;
        for (uint32_t i = 0; i < 3; ++i) {
            float y = T + (B - T) * (0.22f + 0.28f * (float)i);
            float gx = rng_range(rng, L + street, R - street);
            hill_box(t, p, (uint8_t)(30 + i * 2), L, y - block * 0.5f, gx - street * 0.5f, y + block * 0.5f);
            hill_box(t, p, (uint8_t)(31 + i * 2), gx + street * 0.5f, y - block * 0.5f, R, y + block * 0.5f);
        }
        for (uint32_t i = 0; i < 2; ++i) {
            float x = L + (R - L) * (0.30f + 0.35f * (float)i);
            float gy = rng_range(rng, T + street, B - street);
            hill_box(t, p, (uint8_t)(40 + i * 2), x - block * 0.5f, T, x + block * 0.5f, gy - street * 0.5f);
            hill_box(t, p, (uint8_t)(41 + i * 2), x - block * 0.5f, gy + street * 0.5f, x + block * 0.5f, B);
        }
        // Market plaza pocket
        ring_valley(t, p, rng, W * 0.22f, H * 0.72f, 950.f, 380.f, 1.2f, 0.55f, 50, 0);
    }

    // Royal Garden — hedge maze + pools (east)
    maze_cell(t, p, rng, 60, W * 0.58f, H * 0.22f, W * 0.92f, H * 0.72f, wall * 0.9f, gap * 0.65f);
    water_box(t, p, 70, W * 0.68f, H * 0.38f, W * 0.78f, H * 0.48f);
    water_box(t, p, 71, W * 0.80f, H * 0.55f, W * 0.90f, H * 0.66f);
    ring_valley(t, p, rng, W * 0.78f, H * 0.30f, 820.f, 360.f, 2.4f, 0.5f, 75, 0);

    // Barracks — thick corridors + drill square (north)
    corridor_h(t, p, 80, W * 0.22f, W * 0.78f, H * 0.18f, gap * 0.9f, wall * 1.15f, 0);
    corridor_v(t, p, 85, H * 0.06f, H * 0.34f, W * 0.38f, gap * 0.85f, wall * 1.1f, 0);
    corridor_v(t, p, 90, H * 0.06f, H * 0.34f, W * 0.62f, gap * 0.85f, wall * 1.1f, 0);
    ring_valley(t, p, rng, W * 0.50f, H * 0.16f, 1000.f, 420.f, (float)M_PI * 0.5f, 0.55f, 95, 0);

    // Ruined Quarter — broken rock field (NW)
    rock_field(t, p, rng, 100, W * 0.06f, H * 0.08f, W * 0.36f, H * 0.42f, 8, 280.f, 520.f);
    maze_cell(t, p, rng, 110, W * 0.08f, H * 0.10f, W * 0.34f, H * 0.40f, wall * 0.75f, gap * 0.7f);
    ring_valley(t, p, rng, W * 0.18f, H * 0.28f, 780.f, 360.f, 0.6f, 0.5f, 120, 0);

    // Palace — multi-ring walls + boss basin (NE of center, clear of spawn)
    {
        float px = cx + 4200.f;
        float py = cy - 1800.f;
        ring_valley(t, p, rng, px, py, 2100.f, 480.f, 3.6f, 0.42f, 130, 0);
        ring_valley(t, p, rng, px, py, 1400.f, 400.f, 1.1f, 0.48f, 145, 0);
        // Boss throne basin — wide enough for multiplayer
        ring_valley(t, p, rng, px + 200.f, py - 100.f, 1150.f, 450.f, 2.8f, 0.5f, 160, 0);
        // Moat segment
        push_feat(t, TerrainKind::kWater, TerrainShape::kRiver, TerrainFlags::kImpassable,
                  px - 1800.f, py + 1600.f, px + 2200.f, py - 2000.f, p.water, 170,
                  rng_range(rng, 180.f, 240.f));
        // Boss: the fallen king holds the throne basin
        g_boss[MapID::kRoyalCapital] = { px + 200.f, py - 100.f, (uint8_t)MobID::kFallenKing };
    }

    // Clear spawn plaza of stray hills near MAP_SPAWN (berm still frames map)
    // (visual open square — no extra features pushed into center)
    (void)cx; (void)cy;
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

    if (map_id == MapID::kRoyalCapital) {
        build_zones_royal(zones, map_id, W, H, cx, cy);
        berm(terrain, pal, rng, W, H, cx, cy);
        gen_royal(terrain, pal, rng, W, H, cx, cy);
    } else {
        build_zones(zones, map_id, rng, W, H, cx, cy);
        berm(terrain, pal, rng, W, H, cx, cy);
        switch (map_id) {
            case MapID::kGarden:  gen_garden(terrain, pal, rng, W, H, cx, cy); break;
            case MapID::kDesert:  gen_desert(terrain, pal, rng, W, H, cx, cy); break;
            case MapID::kOcean:   gen_ocean(terrain, pal, rng, W, H, cx, cy); break;
            case MapID::kPolar:   gen_snow(terrain, pal, rng, W, H, cx, cy); break;
            case MapID::kJungle:  gen_jungle(terrain, pal, rng, W, H, cx, cy); break;
            case MapID::kAntHell: gen_ant_hell(terrain, pal, rng, W, H, cx, cy); break;
            default:              gen_volcano(terrain, pal, rng, W, H, cx, cy); break;
        }
    }
    // No global cross/diagonal arteries — they punched a beige cross through every
    // map and fought the biome layout. Theme-local channels (e.g. desert dry river) stay.
    resolve_terrain_overlaps(terrain);
}

} // namespace

// Connect substantial walkable pockets to the spawn component. Overlapping
// biome walls can seal a ring's intended gate; a short local channel opens the
// nearest join without changing the rest of the terrain layout.
static uint32_t open_sealed_pockets(uint8_t map_id) {
    constexpr float step = 180.f;
    constexpr float radius = 50.f;
    uint32_t gw = (uint32_t)((float)ARENA_WIDTH / step) + 1;
    uint32_t gh = (uint32_t)((float)ARENA_HEIGHT / step) + 1;
    uint32_t n = gw * gh;
    auto xy = [&](uint32_t i, float &x, float &y) {
        x = (float)(i % gw) * step;
        y = (float)(i / gw) * step;
    };
    auto neighbors = [&](uint32_t i, auto &&visit) {
        uint32_t x = i % gw, y = i / gw;
        if (x) visit(i - 1);
        if (x + 1 < gw) visit(i + 1);
        if (y) visit(i - gw);
        if (y + 1 < gh) visit(i + gw);
    };
    uint32_t opened = 0;
    for (; opened < 24 && g_terrain[map_id].size() < MAX_TERRAIN_PER_MAP; ++opened) {
        std::vector<uint8_t> walk(n, 0);
        for (uint32_t i = 0; i < n; ++i) {
            float x, y;
            xy(i, x, y);
            if (x < radius || y < radius || x > ARENA_WIDTH - radius || y > ARENA_HEIGHT - radius) continue;
            walk[i] = !Map::circle_hits_impassable(map_id, x, y, radius);
        }
        uint32_t spawn = (uint32_t)(MAP_SPAWN_Y / step) * gw + (uint32_t)(MAP_SPAWN_X / step);
        if (!walk[spawn]) {
            float best = 1e30f;
            for (uint32_t i = 0; i < n; ++i) {
                if (!walk[i]) continue;
                float x, y;
                xy(i, x, y);
                float d = (x - MAP_SPAWN_X) * (x - MAP_SPAWN_X)
                        + (y - MAP_SPAWN_Y) * (y - MAP_SPAWN_Y);
                if (d < best) { best = d; spawn = i; }
            }
            if (!walk[spawn]) break;
        }
        std::vector<int32_t> component(n, -1);
        std::vector<uint32_t> sizes;
        std::vector<uint32_t> queue;
        for (uint32_t i = 0; i < n; ++i) {
            if (!walk[i] || component[i] >= 0) continue;
            uint32_t id = (uint32_t)sizes.size();
            queue.clear();
            queue.push_back(i);
            component[i] = (int32_t)id;
            for (size_t qi = 0; qi < queue.size(); ++qi)
                neighbors(queue[qi], [&](uint32_t j) {
                    if (walk[j] && component[j] < 0) {
                        component[j] = (int32_t)id;
                        queue.push_back(j);
                    }
                });
            sizes.push_back((uint32_t)queue.size());
        }
        int32_t main = component[spawn];
        // Flood outward from every spawn-connected cell to locate the shortest
        // barrier between it and another sizeable walkable component.
        std::vector<int32_t> source(n, -1);
        queue.clear();
        for (uint32_t i = 0; i < n; ++i) {
            if (component[i] == main) {
                source[i] = (int32_t)i;
                queue.push_back(i);
            }
        }
        uint32_t from = n, to = n;
        for (size_t qi = 0; qi < queue.size() && to == n; ++qi) {
            uint32_t i = queue[qi];
            neighbors(i, [&](uint32_t j) {
                if (to != n || source[j] >= 0) return;
                int32_t c = component[j];
                if (c >= 0 && c != main && sizes[(uint32_t)c] >= 12) {
                    from = (uint32_t)source[i];
                    to = j;
                    return;
                }
                source[j] = source[i];
                queue.push_back(j);
            });
        }
        if (to == n) break;
        float x0, y0, x1, y1;
        xy(from, x0, y0);
        xy(to, x1, y1);
        float dx = x1 - x0, dy = y1 - y0;
        float length = std::sqrt(dx * dx + dy * dy);
        if (length < 1.f) break;
        float ux = dx / length, uy = dy / length;
        x0 = fclamp(x0 - ux * 300.f, 200.f, (float)ARENA_WIDTH - 200.f);
        y0 = fclamp(y0 - uy * 300.f, 200.f, (float)ARENA_HEIGHT - 200.f);
        x1 = fclamp(x1 + ux * 300.f, 200.f, (float)ARENA_WIDTH - 200.f);
        y1 = fclamp(y1 + uy * 300.f, 200.f, (float)ARENA_HEIGHT - 200.f);
        push_feat(g_terrain[map_id], TerrainKind::kChannel, TerrainShape::kRiver, 0,
                  x0, y0, x1, y1, kThemes[map_id].channel,
                  (uint8_t)(191 + opened * 3 + map_id), 430.f);
    }
    return opened;
}

static uint32_t open_blocked_zones(uint8_t map_id) {
    // Fine enough that thin maze walls cannot be skipped by grid adjacency.
    constexpr float step = 100.f;
    uint32_t gw = (uint32_t)((float)ARENA_WIDTH / step) + 1;
    uint32_t gh = (uint32_t)((float)ARENA_HEIGHT / step) + 1;
    uint32_t n = gw * gh;
    std::vector<uint8_t> seen(n, 0);
    std::vector<uint32_t> queue;
    uint32_t spawn = (uint32_t)(MAP_SPAWN_Y / step) * gw + (uint32_t)(MAP_SPAWN_X / step);
    if (Map::circle_hits_impassable(map_id, (float)(spawn % gw) * step,
                                   (float)(spawn / gw) * step, 50.f)) {
        float best = 1e30f;
        for (uint32_t i = 0; i < n; ++i) {
            float x = (float)(i % gw) * step, y = (float)(i / gw) * step;
            if (Map::circle_hits_impassable(map_id, x, y, 50.f)) continue;
            float d = (x - MAP_SPAWN_X) * (x - MAP_SPAWN_X)
                    + (y - MAP_SPAWN_Y) * (y - MAP_SPAWN_Y);
            if (d < best) { best = d; spawn = i; }
        }
    }
    queue.push_back(spawn);
    seen[spawn] = 1;
    for (size_t qi = 0; qi < queue.size(); ++qi) {
        uint32_t i = queue[qi], x = i % gw, y = i / gw;
        auto visit = [&](uint32_t j) {
            if (seen[j]) return;
            float wx = (float)(j % gw) * step, wy = (float)(j / gw) * step;
            if (wx < 50.f || wy < 50.f || wx > ARENA_WIDTH - 50.f || wy > ARENA_HEIGHT - 50.f) return;
            if (Map::circle_hits_impassable(map_id, wx, wy, 50.f)) return;
            seen[j] = 1;
            queue.push_back(j);
        };
        if (x) visit(i - 1);
        if (x + 1 < gw) visit(i + 1);
        if (y) visit(i - gw);
        if (y + 1 < gh) visit(i + gw);
    }
    uint32_t opened = 0;
    for (uint32_t z = 1; z < g_zones[map_id].size(); ++z) {
        ZoneDefinition const &zone = g_zones[map_id][z];
        float zx = 0.5f * (zone.left + zone.right);
        float zy = 0.5f * (zone.top + zone.bottom);
        // Match sample_connectivity: nearest walkable cell to the zone center
        // must be on the spawn BFS component (AABB overlap is not enough —
        // ring valleys leave outer cells open while the pocket stays sealed).
        bool reachable = false;
        {
            uint32_t ix = (uint32_t)(zx / step), iy = (uint32_t)(zy / step);
            if (ix >= gw) ix = gw - 1;
            if (iy >= gh) iy = gh - 1;
            int best_x = -1, best_y = -1;
            float best_d = 1e30f;
            for (int r = 0; r < 12; ++r) {
                for (int dy = -r; dy <= r; ++dy) {
                    for (int dx = -r; dx <= r; ++dx) {
                        int tx = (int)ix + dx, ty = (int)iy + dy;
                        if (tx < 0 || ty < 0 || (uint32_t)tx >= gw || (uint32_t)ty >= gh) continue;
                        float wx = (float)tx * step, wy = (float)ty * step;
                        if (Map::circle_hits_impassable(map_id, wx, wy, 50.f)) continue;
                        float d = (wx - zx) * (wx - zx) + (wy - zy) * (wy - zy);
                        if (d < best_d) { best_d = d; best_x = tx; best_y = ty; }
                    }
                }
                if (best_x >= 0) break;
            }
            if (best_x >= 0 && seen[(uint32_t)best_y * gw + (uint32_t)best_x])
                reachable = true;
        }
        if (reachable || g_terrain[map_id].size() >= MAX_TERRAIN_PER_MAP) continue;
        float best = 1e30f, sx = MAP_SPAWN_X, sy = MAP_SPAWN_Y;
        for (uint32_t i : queue) {
            float x = (float)(i % gw) * step, y = (float)(i / gw) * step;
            float d = (x - zx) * (x - zx) + (y - zy) * (y - zy);
            if (d < best) { best = d; sx = x; sy = y; }
        }
        // Stepped AABB pads (no river meander) so the bridge actually hits the
        // grid cells used by connectivity BFS through thick maze/ring walls.
        float dx = zx - sx, dy = zy - sy;
        float len = std::sqrt(dx * dx + dy * dy);
        if (len < 1.f) continue;
        float ux = dx / len, uy = dy / len;
        float pad = 420.f;
        uint8_t seed0 = (uint8_t)(113 + z * 7 + map_id);
        for (float t = 0.f; t <= len + 1.f && g_terrain[map_id].size() < MAX_TERRAIN_PER_MAP; t += 320.f) {
            float cx = sx + ux * std::min(t, len);
            float cy = sy + uy * std::min(t, len);
            push_feat(g_terrain[map_id], TerrainKind::kChannel, TerrainShape::kAABB, 0,
                      cx - pad, cy - pad, cx + pad, cy + pad,
                      kThemes[map_id].channel, seed0++, 0.f);
        }
        ++opened;
    }
    return opened;
}

// Mark walkable exits around the plaza rim with short channel strips so players
// can see where the hub opens into the biome (without a full-map beige cross).
static void mark_plaza_rim_exits(uint8_t map_id) {
    ThemePalette const &p = kThemes[map_id];
    constexpr int N = 16;
    constexpr float rad = 45.f;
    uint8_t seed = 220;
    for (int i = 0; i < N && g_terrain[map_id].size() < MAX_TERRAIN_PER_MAP; ++i) {
        float a = (float)i * (float)(2.0 * M_PI / (double)N);
        float ca = std::cos(a), sa = std::sin(a);
        float x0 = MAP_SPAWN_X + ca * 4800.f;
        float y0 = MAP_SPAWN_Y + sa * 3600.f;
        float x1 = MAP_SPAWN_X + ca * 6400.f;
        float y1 = MAP_SPAWN_Y + sa * 5000.f;
        x0 = fclamp(x0, 200.f, (float)ARENA_WIDTH - 200.f);
        y0 = fclamp(y0, 200.f, (float)ARENA_HEIGHT - 200.f);
        x1 = fclamp(x1, 200.f, (float)ARENA_WIDTH - 200.f);
        y1 = fclamp(y1, 200.f, (float)ARENA_HEIGHT - 200.f);
        // Require a mostly walkable ray that is not already a painted channel.
        uint32_t walk = 0, on_ch = 0, samples = 0;
        for (int s = 0; s <= 8; ++s) {
            float t = (float)s / 8.f;
            float x = x0 + (x1 - x0) * t;
            float y = y0 + (y1 - y0) * t;
            ++samples;
            if (!Map::circle_hits_impassable(map_id, x, y, rad)) ++walk;
            if (Map::in_safe_channel(map_id, x, y)) ++on_ch;
        }
        if (walk < samples - 1) continue; // blocked exit
        if (on_ch >= samples / 2) continue; // already marked
        push_feat(g_terrain[map_id], TerrainKind::kChannel, TerrainShape::kRiver, 0,
                  x0, y0, x1, y1, p.channel, seed++, 220.f);
    }
}

// Boss anchors must sit on walkable ground: a spiral end or a palace ring can
// land inside a wall. Nudge to the closest open spot around the anchor.
static void fix_boss_anchor(uint8_t map_id) {
    MapGen::BossAnchor &a = g_boss[map_id];
    if (!a.mob_id) return;
    if (!Map::circle_hits_impassable(map_id, a.x, a.y, 110.f)) return;
    for (uint32_t r = 80; r <= 1400; r += 80) {
        for (uint32_t k = 0; k < 16; ++k) {
            float ang = (float)k * (float)M_PI * 0.125f;
            float x = a.x + std::cos(ang) * (float)r;
            float y = a.y + std::sin(ang) * (float)r;
            if (Map::circle_hits_impassable(map_id, x, y, 110.f)) continue;
            a.x = x;
            a.y = y;
            return;
        }
    }
}

void MapGen::ensure(uint8_t map_id) {
    if (map_id >= MapID::kNumMaps) map_id = MapID::kGarden;
    if (g_ready[map_id]) return;
    generate_one(map_id);
    g_ready[map_id] = 1;
    uint32_t opened = 0;
    // Multi-pass: each bridge can unlock more pockets for the next BFS.
    for (int pass = 0; pass < 5; ++pass) {
        uint32_t n = open_sealed_pockets(map_id);
        n += open_blocked_zones(map_id);
        opened += n;
        if (!n) break;
    }
    mark_plaza_rim_exits(map_id);
    // Terrain is complete now, so the boss anchor can be validated against it.
    fix_boss_anchor(map_id);
    // Last resort at the same step sample_connectivity uses (220): force AABB
    // bridges for any zone center still off the spawn component.
    {
        constexpr float step = 220.f;
        constexpr float rad = 40.f;
        uint32_t gw = (uint32_t)std::ceil((float)ARENA_WIDTH / step) + 1;
        uint32_t gh = (uint32_t)std::ceil((float)ARENA_HEIGHT / step) + 1;
        uint32_t n = gw * gh;
        std::vector<uint8_t> walk(n, 0), seen(n, 0);
        for (uint32_t i = 0; i < n; ++i) {
            float x = (float)(i % gw) * step, y = (float)(i / gw) * step;
            if (x < rad || y < rad || x > ARENA_WIDTH - rad || y > ARENA_HEIGHT - rad) continue;
            walk[i] = !Map::circle_hits_impassable(map_id, x, y, rad);
        }
        uint32_t spawn = (uint32_t)(MAP_SPAWN_Y / step) * gw + (uint32_t)(MAP_SPAWN_X / step);
        if (spawn >= n || !walk[spawn]) {
            float bd = 1e30f;
            for (uint32_t i = 0; i < n; ++i) {
                if (!walk[i]) continue;
                float x = (float)(i % gw) * step, y = (float)(i / gw) * step;
                float d = (x - MAP_SPAWN_X) * (x - MAP_SPAWN_X)
                        + (y - MAP_SPAWN_Y) * (y - MAP_SPAWN_Y);
                if (d < bd) { bd = d; spawn = i; }
            }
        }
        std::vector<uint32_t> q;
        if (spawn < n && walk[spawn]) {
            q.push_back(spawn);
            seen[spawn] = 1;
            for (size_t qi = 0; qi < q.size(); ++qi) {
                uint32_t i = q[qi], x = i % gw, y = i / gw;
                auto visit = [&](uint32_t j) {
                    if (seen[j] || !walk[j]) return;
                    seen[j] = 1;
                    q.push_back(j);
                };
                if (x) visit(i - 1);
                if (x + 1 < gw) visit(i + 1);
                if (y) visit(i - gw);
                if (y + 1 < gh) visit(i + gw);
            }
        }
        for (uint32_t z = 1; z < g_zones[map_id].size()
             && g_terrain[map_id].size() + 8 < MAX_TERRAIN_PER_MAP; ++z) {
            ZoneDefinition const &zone = g_zones[map_id][z];
            float zx = 0.5f * (zone.left + zone.right);
            float zy = 0.5f * (zone.top + zone.bottom);
            uint32_t ix = (uint32_t)(zx / step), iy = (uint32_t)(zy / step);
            if (ix >= gw) ix = gw - 1;
            if (iy >= gh) iy = gh - 1;
            bool ok = false;
            for (int r = 0; r < 10 && !ok; ++r)
                for (int dy = -r; dy <= r && !ok; ++dy)
                    for (int dx = -r; dx <= r && !ok; ++dx) {
                        int tx = (int)ix + dx, ty = (int)iy + dy;
                        if (tx < 0 || ty < 0 || (uint32_t)tx >= gw || (uint32_t)ty >= gh) continue;
                        if (seen[(uint32_t)ty * gw + (uint32_t)tx]) ok = true;
                    }
            if (ok) continue;
            float best = 1e30f, sx = MAP_SPAWN_X, sy = MAP_SPAWN_Y;
            for (uint32_t i : q) {
                float x = (float)(i % gw) * step, y = (float)(i / gw) * step;
                float d = (x - zx) * (x - zx) + (y - zy) * (y - zy);
                if (d < best) { best = d; sx = x; sy = y; }
            }
            float dx = zx - sx, dy = zy - sy;
            float len = std::sqrt(dx * dx + dy * dy);
            if (len < 1.f) continue;
            float ux = dx / len, uy = dy / len;
            float pad = 480.f;
            uint8_t seed0 = (uint8_t)(170 + z * 3 + map_id);
            for (float t = 0.f; t <= len + 1.f && g_terrain[map_id].size() < MAX_TERRAIN_PER_MAP; t += 280.f) {
                float cx = sx + ux * std::min(t, len);
                float cy = sy + uy * std::min(t, len);
                push_feat(g_terrain[map_id], TerrainKind::kChannel, TerrainShape::kAABB, 0,
                          cx - pad, cy - pad, cx + pad, cy + pad,
                          kThemes[map_id].channel, seed0++, 0.f);
            }
            ++opened;
        }
    }
#ifdef SERVERSIDE
    uint32_t reached = Map::sample_connectivity(map_id, 220.f);
    MapZones const &z = g_zones[map_id];
    std::cout << "[map] " << (uint32_t)map_id << " connectivity: "
              << reached << "/" << z.size() << " zone centers reachable from spawn; terrain "
              << g_terrain[map_id].size() << "/" << MAX_TERRAIN_PER_MAP
              << ", opened " << opened << " local gates\n";
#endif
}
MapZones const &MapGen::zones(uint8_t map_id) { ensure(map_id); return g_zones[map_id]; }
MapTerrain const &MapGen::terrain(uint8_t map_id) { ensure(map_id); return g_terrain[map_id]; }
MapGen::BossAnchor const &MapGen::boss_anchor(uint8_t map_id) {
    if (map_id >= MapID::kNumMaps) map_id = MapID::kGarden;
    ensure(map_id);
    return g_boss[map_id];
}
MapZones const &map_zones(uint8_t map_id) { return MapGen::zones(map_id); }
MapTerrain const &map_terrain(uint8_t map_id) { return MapGen::terrain(map_id); }
