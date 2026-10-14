#include <Shared/MapGen.hh>
#include <Shared/Map.hh>
#include <Helpers/Math.hh>

#include <cmath>
#include <cstdint>
#include <initializer_list>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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
                float x0, float x1, float y_mid, float gap, float wall_t, uint8_t water) {
    float half = gap * 0.5f;
    auto box = water ? water_box : hill_box;
    box(t, p, s, x0, y_mid - half - wall_t, x1, y_mid - half);
    box(t, p, (uint8_t)(s + 1), x0, y_mid + half, x1, y_mid + half + wall_t);
}
void corridor_v(MapTerrain &t, ThemePalette const &p, uint8_t s,
                float y0, float y1, float x_mid, float gap, float wall_t, uint8_t water) {
    float half = gap * 0.5f;
    auto box = water ? water_box : hill_box;
    box(t, p, s, x_mid - half - wall_t, y0, x_mid - half, y1);
    box(t, p, (uint8_t)(s + 1), x_mid + half, y0, x_mid + half + wall_t, y1);
}

// Ring of thick wall segments with one gate — large open interior for multiplayer fights.
void ring_valley(MapTerrain &t, ThemePalette const &p, uint32_t &rng,
                 float cx, float cy, float inner_r, float wall_t,
                 float gate_ang, float gate_arc, uint8_t seed0, uint8_t water) {
    uint32_t segs = 12;
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
        float hw = wall_t * 0.5f + inner_r * 0.16f;
        float hh = wall_t * 0.8f;
        if (water) water_box(t, p, (uint8_t)(seed0 + i), mx - hw, my - hh, mx + hw, my + hh);
        else hill_box(t, p, (uint8_t)(seed0 + i), mx - hw, my - hh, mx + hw, my + hh);
    }
    (void)rng;
}

// H/V spiral corridor → circular valley (wide farm path + enclosed end).
void spiral_farm(MapTerrain &t, ThemePalette const &p, uint32_t &rng,
                 float sx, float sy, int dir, int turn,
                 float gap, float wall_t, float seg_len, uint32_t n_seg,
                 float valley_r, uint8_t seed0, uint8_t water) {
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
}

// Maze cell: thick walls with random gates on each side of a rect.
void maze_cell(MapTerrain &t, ThemePalette const &p, uint32_t &rng, uint8_t s,
               float L, float T, float R, float B, float thick, float gate_min) {
    float w = R - L, h = B - T;
    if (w < 900.f || h < 900.f) return;
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
    punch_h(T, T + thick, L + w * rng_range(rng, 0.25f, 0.75f), s);
    punch_h(B - thick, B, L + w * rng_range(rng, 0.25f, 0.75f), (uint8_t)(s + 4));
    punch_v(L, L + thick, T + h * rng_range(rng, 0.25f, 0.75f), (uint8_t)(s + 8));
    punch_v(R - thick, R, T + h * rng_range(rng, 0.25f, 0.75f), (uint8_t)(s + 12));
}

void rock_field(MapTerrain &t, ThemePalette const &p, uint32_t &rng, uint8_t s0,
                float L, float T, float R, float B, uint32_t count, float rad_lo, float rad_hi) {
    for (uint32_t i = 0; i < count; ++i) {
        float rad = rng_range(rng, rad_lo, rad_hi);
        float x = rng_range(rng, L + rad, R - rad);
        float y = rng_range(rng, T + rad, B - rad);
        hill_circle(t, p, (uint8_t)(s0 + i), x, y, rad);
    }
}

void push_zone(MapZones &z, ZoneDefinition tmpl, float l, float top, float r, float b,
               uint8_t diff, uint32_t color) {
    if (z.size() >= MAX_ZONES_PER_MAP) return;
    tmpl.left = l; tmpl.top = top; tmpl.right = r; tmpl.bottom = b;
    tmpl.difficulty = diff;
    tmpl.color = color;
    z.push(tmpl);
}

ZoneDefinition eco_from(ZoneDefinition base, char const *name, uint8_t diff,
                        float density_mul, std::initializer_list<SpawnChance> spawns) {
    ZoneDefinition z = base;
    z.name = name;
    z.difficulty = diff;
    z.density = base.density * density_mul;
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
        add(eco_from(soft, "Meadow", 1, 0.95f, {
            { MobID::kBee, 140000 }, { MobID::kLadybug, 120000 }, { MobID::kRock, 80000 },
            { MobID::kSquare, 1 }}), 2000.f, 3400.f, 3);
        add(eco_from(mid, "Hedge Maze", 2, 1.05f, {
            { MobID::kSpider, 110000 }, { MobID::kBee, 90000 }, { MobID::kBeetle, 80000 },
            { MobID::kCentipede, 14000 }, { MobID::kSquare, 1 }}), 1600.f, 2600.f, 3);
        add(eco_from(mid, "Creek Banks", 2, 1.0f, {
            { MobID::kHornet, 100000 }, { MobID::kBee, 90000 }, { MobID::kLadybug, 70000 },
            { MobID::kSquare, 1 }}), 1400.f, 2400.f, 2);
        add(eco_from(hard, "Thorn Field", 3, 1.1f, {
            { MobID::kDarkLadybug, 130000 }, { MobID::kHornet, 110000 }, { MobID::kSpider, 100000 },
            { MobID::kEvilCentipede, 16000 }, { MobID::kSquare, 1 }}), 1100.f, 1800.f, 2);
        add(eco_from(hard, "Flower Heart", 3, 1.2f, {
            { MobID::kMassiveBeetle, 5000 }, { MobID::kHornet, 120000 }, { MobID::kDarkLadybug, 100000 },
            { MobID::kAntHole, 3000 }, { MobID::kSquare, 1 }}), 2600.f, 3400.f, 1);
        break;
    case MapID::kDesert:
        add(eco_from(soft, "Open Dunes", 1, 0.9f, {
            { MobID::kSandstorm, 120000 }, { MobID::kRock, 140000 }, { MobID::kLadybug, 80000 },
            { MobID::kSquare, 1 }}), 2200.f, 3600.f, 3);
        add(eco_from(mid, "Cactus Maze", 2, 1.05f, {
            { MobID::kBeetle, 120000 }, { MobID::kSandstorm, 100000 }, { MobID::kSpider, 70000 },
            { MobID::kSquare, 1 }}), 1500.f, 2500.f, 3);
        add(eco_from(mid, "Dry River", 2, 1.0f, {
            { MobID::kBoulder, 140000 }, { MobID::kRock, 120000 }, { MobID::kBee, 60000 },
            { MobID::kSquare, 1 }}), 1400.f, 2300.f, 2);
        add(eco_from(hard, "Bone Yard", 3, 1.1f, {
            { MobID::kHornet, 120000 }, { MobID::kBeetle, 100000 }, { MobID::kEvilCentipede, 20000 },
            { MobID::kSquare, 1 }}), 1100.f, 1800.f, 2);
        add(eco_from(hard, "Sandworm Nest", 3, 1.2f, {
            { MobID::kMassiveBeetle, 4500 }, { MobID::kSandstorm, 140000 }, { MobID::kHornet, 100000 },
            { MobID::kSquare, 1 }}), 2600.f, 3400.f, 1);
        break;
    case MapID::kOcean:
        add(eco_from(soft, "Shallow Sea", 1, 0.9f, {
            { MobID::kRock, 150000 }, { MobID::kBee, 90000 }, { MobID::kLadybug, 70000 },
            { MobID::kSquare, 1 }}), 2200.f, 3600.f, 3);
        add(eco_from(mid, "Coral Maze", 2, 1.05f, {
            { MobID::kHornet, 110000 }, { MobID::kBeetle, 90000 }, { MobID::kBoulder, 100000 },
            { MobID::kSquare, 1 }}), 1500.f, 2500.f, 3);
        add(eco_from(mid, "Kelp Forest", 2, 1.0f, {
            { MobID::kBee, 100000 }, { MobID::kSpider, 80000 }, { MobID::kSandstorm, 70000 },
            { MobID::kSquare, 1 }}), 1400.f, 2300.f, 2);
        add(eco_from(hard, "Shipwreck", 3, 1.1f, {
            { MobID::kHornet, 130000 }, { MobID::kEvilCentipede, 22000 }, { MobID::kBeetle, 90000 },
            { MobID::kSquare, 1 }}), 1100.f, 1800.f, 2);
        add(eco_from(hard, "Abyss Core", 3, 1.2f, {
            { MobID::kMassiveBeetle, 4800 }, { MobID::kHornet, 130000 }, { MobID::kEvilCentipede, 28000 },
            { MobID::kSquare, 1 }}), 2600.f, 3400.f, 1);
        break;
    case MapID::kPolar:
        add(eco_from(soft, "Snow Fields", 1, 0.9f, {
            { MobID::kRock, 150000 }, { MobID::kLadybug, 100000 }, { MobID::kBee, 70000 },
            { MobID::kSquare, 1 }}), 2200.f, 3600.f, 3);
        add(eco_from(mid, "Pine Maze", 2, 1.05f, {
            { MobID::kSpider, 110000 }, { MobID::kBoulder, 120000 }, { MobID::kBee, 70000 },
            { MobID::kSquare, 1 }}), 1500.f, 2500.f, 3);
        add(eco_from(mid, "Ice River", 2, 1.0f, {
            { MobID::kBoulder, 140000 }, { MobID::kHornet, 90000 }, { MobID::kBee, 60000 },
            { MobID::kSquare, 1 }}), 1400.f, 2300.f, 2);
        add(eco_from(hard, "Frost Rift", 3, 1.1f, {
            { MobID::kDarkLadybug, 130000 }, { MobID::kHornet, 110000 }, { MobID::kSpider, 100000 },
            { MobID::kSquare, 1 }}), 1100.f, 1800.f, 2);
        add(eco_from(hard, "Frost Core", 3, 1.2f, {
            { MobID::kMassiveBeetle, 4500 }, { MobID::kDarkLadybug, 120000 }, { MobID::kEvilCentipede, 24000 },
            { MobID::kAntHole, 2500 }, { MobID::kSquare, 1 }}), 2600.f, 3400.f, 1);
        break;
    case MapID::kJungle:
        add(eco_from(soft, "Clearing", 1, 0.95f, {
            { MobID::kBee, 130000 }, { MobID::kLadybug, 110000 }, { MobID::kSpider, 60000 },
            { MobID::kSquare, 1 }}), 2000.f, 3400.f, 3);
        add(eco_from(mid, "Vine Corridor", 2, 1.1f, {
            { MobID::kSpider, 140000 }, { MobID::kHornet, 100000 }, { MobID::kCentipede, 20000 },
            { MobID::kSquare, 1 }}), 1500.f, 2500.f, 3);
        add(eco_from(mid, "Swamp", 2, 1.0f, {
            { MobID::kBeetle, 100000 }, { MobID::kSpider, 90000 }, { MobID::kBee, 70000 },
            { MobID::kSquare, 1 }}), 1400.f, 2300.f, 2);
        add(eco_from(hard, "Toxic Grove", 3, 1.15f, {
            { MobID::kSpider, 140000 }, { MobID::kHornet, 120000 }, { MobID::kEvilCentipede, 24000 },
            { MobID::kDarkLadybug, 80000 }, { MobID::kSquare, 1 }}), 1100.f, 1800.f, 2);
        add(eco_from(hard, "Tree Heart", 3, 1.25f, {
            { MobID::kQueenAnt, 2200 }, { MobID::kMassiveBeetle, 4000 }, { MobID::kSpider, 120000 },
            { MobID::kAntHole, 3500 }, { MobID::kSquare, 1 }}), 2600.f, 3400.f, 1);
        break;
    case MapID::kAntHell:
        add(eco_from(soft, "Forage Halls", 1, 1.0f, {
            { MobID::kBabyAnt, 180000 }, { MobID::kWorkerAnt, 140000 }, { MobID::kRock, 80000 },
            { MobID::kSquare, 1 }}), 1800.f, 3000.f, 3);
        add(eco_from(mid, "Dirt Maze", 2, 1.1f, {
            { MobID::kWorkerAnt, 150000 }, { MobID::kSoldierAnt, 120000 }, { MobID::kBabyAnt, 90000 },
            { MobID::kSquare, 1 }}), 1400.f, 2400.f, 3);
        add(eco_from(mid, "Fungal Farm", 2, 1.05f, {
            { MobID::kWorkerAnt, 140000 }, { MobID::kBabyAnt, 120000 }, { MobID::kAntHole, 4000 },
            { MobID::kSquare, 1 }}), 1300.f, 2200.f, 2);
        add(eco_from(hard, "Soldier Patrol", 3, 1.15f, {
            { MobID::kSoldierAnt, 180000 }, { MobID::kWorkerAnt, 100000 }, { MobID::kAntHole, 6000 },
            { MobID::kSquare, 1 }}), 1100.f, 1800.f, 2);
        add(eco_from(hard, "Queen Nest", 3, 1.3f, {
            { MobID::kQueenAnt, 3500 }, { MobID::kSoldierAnt, 160000 }, { MobID::kAntHole, 8000 },
            { MobID::kMassiveBeetle, 2500 }, { MobID::kSquare, 1 }}), 2600.f, 3400.f, 1);
        break;
    default: // Volcano
        add(eco_from(soft, "Ash Plain", 1, 0.9f, {
            { MobID::kRock, 160000 }, { MobID::kLadybug, 90000 }, { MobID::kBeetle, 70000 },
            { MobID::kSquare, 1 }}), 2200.f, 3600.f, 3);
        add(eco_from(mid, "Obsidian Maze", 2, 1.05f, {
            { MobID::kSoldierAnt, 110000 }, { MobID::kBeetle, 100000 }, { MobID::kBoulder, 100000 },
            { MobID::kSquare, 1 }}), 1500.f, 2500.f, 3);
        add(eco_from(mid, "Lava Banks", 2, 1.0f, {
            { MobID::kBeetle, 110000 }, { MobID::kHornet, 90000 }, { MobID::kSpider, 70000 },
            { MobID::kSquare, 1 }}), 1400.f, 2300.f, 2);
        add(eco_from(hard, "Magma Nest", 3, 1.15f, {
            { MobID::kSoldierAnt, 130000 }, { MobID::kHornet, 110000 }, { MobID::kBeetle, 90000 },
            { MobID::kSquare, 1 }}), 1100.f, 1800.f, 2);
        add(eco_from(hard, "Molten Heart", 3, 1.25f, {
            { MobID::kMassiveBeetle, 5000 }, { MobID::kSoldierAnt, 140000 }, { MobID::kHornet, 110000 },
            { MobID::kSquare, 1 }}), 2600.f, 3400.f, 1);
        break;
    }

    // Landmark grid: place pockets around the map so they don't pile on the hub.
    // Color matches difficulty ring (unused for paint after ring-only ground).
    for (uint32_t pi = 0; pi < pockets.size(); ++pi) {
        Pocket const &pk = pockets[pi];
        for (uint32_t n = 0; n < pk.count; ++n) {
            float ww = rng_range(rng, pk.min_sz, pk.max_sz);
            float hh = rng_range(rng, pk.min_sz * 0.72f, pk.max_sz * 0.95f);
            // Prefer corners / mid-rings via angle buckets
            float ang = (float)(2.0 * M_PI) * ((float)(pi * 3 + n) + rng_f(rng) * 0.35f) / 12.f;
            float dist = rng_range(rng, 3800.f, 10500.f);
            if (pk.def.difficulty >= 3) dist = rng_range(rng, 6500.f, 11500.f);
            else if (pk.def.difficulty <= 1) dist = rng_range(rng, 3200.f, 7500.f);
            float mx = cx + std::cos(ang) * dist;
            float my = cy + std::sin(ang) * dist;
            float x = fclamp(mx - ww * 0.5f, 350.f, W - ww - 350.f);
            float y = fclamp(my - hh * 0.5f, 350.f, H - hh - 350.f);
            if (std::fabs((x + ww * 0.5f) - cx) < hub_hw * 0.85f
                && std::fabs((y + hh * 0.5f) - cy) < hub_hh * 0.85f
                && pk.def.difficulty >= 2)
                continue;
            // Color = ring color for that difficulty (paint ignores pockets anyway)
            uint32_t col = (pk.def.difficulty >= 3) ? hard.color
                         : (pk.def.difficulty == 2) ? mid.color : soft.color;
            push_zone(zones, pk.def, x, y, x + ww, y + hh, (uint8_t)pk.def.difficulty, col);
        }
    }
}

// Difficulty rings + biome eco-pockets (later indices win for spawns).
void build_zones(MapZones &zones, uint8_t map_id, uint32_t &rng,
                 float W, float H, float cx, float cy) {
    zones.clear();
    MapZones const &tmpl = MAPS[map_id];
    ZoneDefinition hard = tmpl[0], mid = tmpl[1], soft = tmpl[2], hub = tmpl[3];

    float hub_hw = rng_range(rng, 2200.f, 2800.f);
    float hub_hh = rng_range(rng, 1800.f, 2300.f);
    float m1 = rng_range(rng, 0.26f, 0.32f);
    float m2 = rng_range(rng, 0.10f, 0.15f);

    push_zone(zones, hard, 0, 0, W, H, 3, hard.color);
    push_zone(zones, mid, W * m2, H * m2, W * (1.f - m2), H * (1.f - m2), 2, mid.color);
    push_zone(zones, soft, W * m1, H * m1, W * (1.f - m1), H * (1.f - m1), 1, soft.color);
    push_zone(zones, hub, cx - hub_hw, cy - hub_hh, cx + hub_hw, cy + hub_hh, 0, hub.color);

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

void safe_arteries(MapTerrain &t, ThemePalette const &p, uint32_t &rng,
                   float W, float H, float cx, float cy, uint8_t map_id) {
    float ch = rng_range(rng, 480.f, 620.f); // wide enough for many players
    push_feat(t, TerrainKind::kChannel, TerrainShape::kRiver, TerrainFlags::kNoAttack,
              cx, 400.f, cx, H - 400.f, p.channel, (uint8_t)(3 + map_id), ch);
    push_feat(t, TerrainKind::kChannel, TerrainShape::kRiver, TerrainFlags::kNoAttack,
              400.f, cy, W - 400.f, cy, p.channel, (uint8_t)(13 + map_id), ch);
    float d = rng_range(rng, 0.25f, 0.75f);
    push_feat(t, TerrainKind::kChannel, TerrainShape::kRiver, TerrainFlags::kNoAttack,
              W * 0.08f, H * d, W * 0.92f, H * (1.f - d * 0.4f), p.channel,
              (uint8_t)(23 + map_id), ch * 0.8f);
}

// ---- Biome terrain ----

void gen_garden(MapTerrain &t, ThemePalette const &p, uint32_t &rng, float W, float H, float cx, float cy) {
    float gap = rng_range(rng, 1150.f, 1450.f);
    float wall = rng_range(rng, 420.f, 580.f);
    // Hedge maze blocks (tree-hedge walls)
    maze_cell(t, p, rng, 30, W * 0.08f, H * 0.08f, W * 0.42f, H * 0.42f, wall, gap * 0.7f);
    maze_cell(t, p, rng, 50, W * 0.55f, H * 0.10f, W * 0.92f, H * 0.48f, wall * 0.9f, gap * 0.65f);
    maze_cell(t, p, rng, 70, W * 0.10f, H * 0.55f, W * 0.45f, H * 0.92f, wall, gap * 0.7f);
    // Spiral farm → flower-heart valley
    spiral_farm(t, p, rng, cx + 2000.f, cy - 200.f, 0, +1, gap, wall,
                rng_range(rng, 2600.f, 3400.f), 3, rng_range(rng, 1300.f, 1700.f), 90, 0);
    // Creek cutting map
    push_feat(t, TerrainKind::kWater, TerrainShape::kRiver, TerrainFlags::kImpassable,
              W * 0.12f, H * 0.20f, W * 0.88f, H * 0.82f, p.water, 110, rng_range(rng, 200.f, 280.f));
    push_feat(t, TerrainKind::kWater, TerrainShape::kRiver, TerrainFlags::kImpassable,
              W * 0.20f, H * 0.85f, W * 0.75f, H * 0.18f, p.water, 112, rng_range(rng, 180.f, 240.f));
    // Boulder prairie + mushroom rings
    rock_field(t, p, rng, 120, W * 0.55f, H * 0.55f, W * 0.90f, H * 0.90f, 8, 280.f, 520.f);
    ring_valley(t, p, rng, W * 0.22f, H * 0.78f, 900.f, 400.f, 0.8f, 0.6f, 140, 0);
    ring_valley(t, p, rng, W * 0.78f, H * 0.22f, 850.f, 380.f, 2.2f, 0.55f, 160, 0); // thorn pocket
}

void gen_desert(MapTerrain &t, ThemePalette const &p, uint32_t &rng, float W, float H, float cx, float cy) {
    float gap = rng_range(rng, 1200.f, 1550.f);
    float wall = rng_range(rng, 480.f, 650.f);
    // Dune belts with gates
    for (uint32_t i = 0; i < 4; ++i) {
        float y = H * (0.18f + 0.18f * (float)i) + rng_range(rng, -120.f, 120.f);
        float gate = rng_range(rng, W * 0.25f, W * 0.75f);
        float gw = gap * 0.9f;
        hill_box(t, p, (uint8_t)(30 + i * 2), 500.f, y - wall * 0.5f, gate - gw * 0.5f, y + wall * 0.5f);
        hill_box(t, p, (uint8_t)(31 + i * 2), gate + gw * 0.5f, y - wall * 0.5f, W - 500.f, y + wall * 0.5f);
    }
    // Rock canyon maze
    maze_cell(t, p, rng, 50, W * 0.55f, H * 0.12f, W * 0.92f, H * 0.55f, wall, gap * 0.6f);
    spiral_farm(t, p, rng, cx + 3200.f, cy - 2500.f, 1, +1, gap, wall * 0.95f,
                rng_range(rng, 2400.f, 3100.f), 2, rng_range(rng, 1200.f, 1600.f), 80, 0);
    // Dry river bed
    push_feat(t, TerrainKind::kChannel, TerrainShape::kRiver, TerrainFlags::kNoAttack,
              W * 0.1f, H * 0.15f, W * 0.9f, H * 0.88f, p.channel, 100, rng_range(rng, 350.f, 450.f));
    // Oasis + sandworm basin
    ring_valley(t, p, rng, W * 0.20f, H * 0.75f, 950.f, 420.f, 1.2f, 0.55f, 120, 1);
    ring_valley(t, p, rng, W * 0.82f, H * 0.28f, 1100.f, 450.f, 3.5f, 0.5f, 140, 0);
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
    // Deep trenches
    push_feat(t, TerrainKind::kWater, TerrainShape::kRiver, TerrainFlags::kImpassable,
              W * 0.1f, H * 0.72f, W * 0.9f, H * 0.28f, p.water, 110, rng_range(rng, 220.f, 300.f));
    rock_field(t, p, rng, 130, W * 0.12f, H * 0.55f, W * 0.40f, H * 0.88f, 7, 300.f, 560.f);
    ring_valley(t, p, rng, W * 0.18f, H * 0.20f, 880.f, 400.f, 0.4f, 0.55f, 150, 0);
    ring_valley(t, p, rng, W * 0.82f, H * 0.80f, 1050.f, 430.f, 2.8f, 0.5f, 170, 0); // abyss core
}

void gen_snow(MapTerrain &t, ThemePalette const &p, uint32_t &rng, float W, float H, float cx, float cy) {
    float gap = rng_range(rng, 1150.f, 1450.f);
    float wall = rng_range(rng, 450.f, 600.f);
    maze_cell(t, p, rng, 30, W * 0.08f, H * 0.50f, W * 0.48f, H * 0.92f, wall, gap * 0.7f);
    spiral_farm(t, p, rng, cx - 400.f, cy + 2200.f, 1, +1, gap, wall,
                rng_range(rng, 2700.f, 3500.f), 3, rng_range(rng, 1300.f, 1700.f), 70, 0);
    spiral_farm(t, p, rng, cx + 2400.f, cy - 1600.f, 2, -1, gap * 0.9f, wall,
                rng_range(rng, 2400.f, 3100.f), 2, rng_range(rng, 1150.f, 1500.f), 100, 1);
    push_feat(t, TerrainKind::kWater, TerrainShape::kRiver, TerrainFlags::kImpassable,
              W * 0.15f, H * 0.25f, W * 0.85f, H * 0.75f, p.water, 120, rng_range(rng, 200.f, 260.f));
    rock_field(t, p, rng, 140, W * 0.55f, H * 0.55f, W * 0.90f, H * 0.90f, 6, 320.f, 580.f);
    ring_valley(t, p, rng, W * 0.80f, H * 0.20f, 1000.f, 420.f, 3.8f, 0.5f, 160, 1);
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
    // Swamp / river belts cutting the canopy
    push_feat(t, TerrainKind::kWater, TerrainShape::kRiver, TerrainFlags::kImpassable,
              W * 0.10f, H * 0.68f, W * 0.90f, H * 0.28f, p.water, 110, rng_range(rng, 230.f, 300.f));
    push_feat(t, TerrainKind::kWater, TerrainShape::kRiver, TerrainFlags::kImpassable,
              W * 0.18f, H * 0.15f, W * 0.78f, H * 0.88f, p.water, 112, rng_range(rng, 180.f, 240.f));
    // Giant root clusters (circular canopy blockers)
    rock_field(t, p, rng, 120, W * 0.12f, H * 0.12f, W * 0.40f, H * 0.40f, 6, 360.f, 620.f);
    rock_field(t, p, rng, 140, W * 0.60f, H * 0.58f, W * 0.90f, H * 0.88f, 5, 320.f, 560.f);
    // Toxic pocket + insect nest + tree heart (boss)
    ring_valley(t, p, rng, W * 0.22f, H * 0.78f, 880.f, 400.f, 1.0f, 0.55f, 130, 0);
    ring_valley(t, p, rng, W * 0.72f, H * 0.70f, 820.f, 380.f, 2.2f, 0.5f, 145, 0);
    ring_valley(t, p, rng, W * 0.80f, H * 0.22f, 1150.f, 460.f, 2.6f, 0.48f, 160, 0);
}

void gen_ant_hell(MapTerrain &t, ThemePalette const &p, uint32_t &rng, float W, float H, float cx, float cy) {
    // Dirt walls = hills; passable tunnels = gaps. Dense chamber network.
    float gap = rng_range(rng, 1000.f, 1300.f);
    float wall = rng_range(rng, 480.f, 640.f);
    maze_cell(t, p, rng, 20, W * 0.08f, H * 0.08f, W * 0.50f, H * 0.50f, wall, gap * 0.55f);
    maze_cell(t, p, rng, 40, W * 0.48f, H * 0.08f, W * 0.92f, H * 0.50f, wall, gap * 0.55f);
    maze_cell(t, p, rng, 60, W * 0.08f, H * 0.48f, W * 0.50f, H * 0.92f, wall, gap * 0.55f);
    maze_cell(t, p, rng, 80, W * 0.48f, H * 0.48f, W * 0.92f, H * 0.92f, wall, gap * 0.55f);
    // Cross tunnels (safe channels already add arteries; add more chamber links)
    corridor_h(t, p, 100, W * 0.15f, W * 0.85f, H * 0.35f, gap * 0.85f, wall * 0.7f, 0);
    corridor_v(t, p, 110, H * 0.15f, H * 0.85f, W * 0.35f, gap * 0.85f, wall * 0.7f, 0);
    corridor_h(t, p, 120, W * 0.15f, W * 0.85f, H * 0.65f, gap * 0.85f, wall * 0.7f, 0);
    // Queen chamber
    {
        float qx = fclamp(cx + rng_range(rng, 3500.f, 5500.f) * (rng_f(rng) < 0.5f ? 1.f : -1.f),
                          1800.f, W - 1800.f);
        float qy = fclamp(cy + rng_range(rng, 2000.f, 4000.f) * (rng_f(rng) < 0.5f ? 1.f : -1.f),
                          1800.f, H - 1800.f);
        ring_valley(t, p, rng, qx, qy, 1200.f, 500.f, rng_range(rng, 0.f, 6.28f), 0.45f, 140, 0);
    }
    // Water pool chambers
    ring_valley(t, p, rng, W * 0.22f, H * 0.22f, 750.f, 360.f, 0.6f, 0.5f, 160, 1);
}

void gen_volcano(MapTerrain &t, ThemePalette const &p, uint32_t &rng, float W, float H, float cx, float cy) {
    float gap = rng_range(rng, 1100.f, 1400.f);
    float wall = rng_range(rng, 450.f, 600.f);
    // Lava rivers (water kind = lava color)
    push_feat(t, TerrainKind::kWater, TerrainShape::kRiver, TerrainFlags::kImpassable,
              W * 0.1f, H * 0.25f, W * 0.9f, H * 0.78f, p.water, 30, rng_range(rng, 240.f, 320.f));
    push_feat(t, TerrainKind::kWater, TerrainShape::kRiver, TerrainFlags::kImpassable,
              W * 0.15f, H * 0.80f, W * 0.85f, H * 0.20f, p.water, 32, rng_range(rng, 220.f, 300.f));
    // Obsidian maze
    maze_cell(t, p, rng, 40, W * 0.10f, H * 0.10f, W * 0.45f, H * 0.48f, wall, gap * 0.6f);
    maze_cell(t, p, rng, 60, W * 0.55f, H * 0.50f, W * 0.90f, H * 0.90f, wall, gap * 0.6f);
    spiral_farm(t, p, rng, cx + 2000.f, cy - 500.f, 0, +1, gap, wall,
                rng_range(rng, 2400.f, 3100.f), 2, rng_range(rng, 1200.f, 1550.f), 80, 0);
    rock_field(t, p, rng, 100, W * 0.55f, H * 0.10f, W * 0.90f, H * 0.45f, 7, 300.f, 560.f);
    // Magma lakes + molten heart
    ring_valley(t, p, rng, W * 0.22f, H * 0.75f, 900.f, 400.f, 1.1f, 0.55f, 130, 1);
    ring_valley(t, p, rng, W * 0.78f, H * 0.25f, 1150.f, 480.f, 3.2f, 0.48f, 150, 1);
}

// Exactly 6 large AABBs from MAPS template — no specialty eco scatter.
void build_zones_royal(MapZones &zones, uint8_t map_id, float W, float H, float cx, float cy) {
    zones.clear();
    MapZones const &tmpl = MAPS[map_id];
    // Keep spawn plaza at MAP_SPAWN as Outer Wall (index 0 wins when no overlay).
    float plaza_hw = 2600.f;
    float plaza_hh = 2100.f;
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
    safe_arteries(terrain, pal, rng, W, H, cx, cy, map_id);
}

} // namespace

void MapGen::ensure(uint8_t map_id) {
    if (map_id >= MapID::kNumMaps) map_id = MapID::kGarden;
    if (g_ready[map_id]) return;
    generate_one(map_id);
    g_ready[map_id] = 1;
#ifdef SERVERSIDE
    if (map_id == MapID::kRoyalCapital) {
        uint32_t reached = Map::sample_connectivity(map_id, 220.f);
        MapZones const &z = g_zones[map_id];
        std::cout << "[map] Royal Capital connectivity: "
                  << reached << "/" << z.size() << " zone centers reachable from spawn\n";
    }
#endif
}
MapZones const &MapGen::zones(uint8_t map_id) { ensure(map_id); return g_zones[map_id]; }
MapTerrain const &MapGen::terrain(uint8_t map_id) { ensure(map_id); return g_terrain[map_id]; }
MapZones const &map_zones(uint8_t map_id) { return MapGen::zones(map_id); }
MapTerrain const &map_terrain(uint8_t map_id) { return MapGen::terrain(map_id); }
