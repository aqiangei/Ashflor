#include <Client/MapChunkCache.hh>
#include <Client/ThemeMaterial.hh>
#include <Client/TerrainPattern.hh>

#include <Client/Render/Renderer.hh>
#include <Client/Input.hh>

#include <Shared/Map.hh>
#include <Shared/RarityScale.hh>
#include <Shared/StaticData.hh>
#include <Shared/StaticDefinitions.hh>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <memory>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace {
constexpr int kChunkSize = 512;
constexpr int kChunkPad = 2;

struct Chunk {
    int x = 0;
    int y = 0;
    uint64_t used = 0;
    // Fog depends on player difficulty; stale canvases stay blit-able until rebake.
    uint32_t baked_difficulty = 0xffffffffu;
    std::unique_ptr<Renderer> canvas;
};

std::vector<Chunk> chunks;
uint8_t cached_map = 255;
uint32_t cached_difficulty = 0xffffffffu;
uint64_t frame = 0;
// Hysteresis so FOV/zoom near the chunk-count limit does not thrash between
// full-viewport paint_static and chunk blit (visible as a periodic flash).
uint8_t full_paint_mode = 0;

bool feature_intersects(TerrainFeature const &f, float left, float top, float right, float bottom) {
    float x0, y0, x1, y1;
    if (f.shape == TerrainShape::kAABB) {
        float pad = std::max(f.right - f.left, f.bottom - f.top) * 0.2f + 100.f;
        x0 = f.left - pad; y0 = f.top - pad;
        x1 = f.right + pad; y1 = f.bottom + pad;
    } else if (f.shape == TerrainShape::kCircle || f.shape == TerrainShape::kEllipse) {
        float rx = f.right * 1.3f + 100.f;
        float ry = (f.shape == TerrainShape::kCircle ? f.right : f.bottom) * 1.3f + 100.f;
        x0 = f.left - rx; x1 = f.left + rx;
        y0 = f.top - ry; y1 = f.top + ry;
    } else if (f.shape == TerrainShape::kSpiral) {
        float radius = f.right + Map::feature_half_width(f) + 120.f;
        x0 = f.left - radius; x1 = f.left + radius;
        y0 = f.top - radius; y1 = f.top + radius;
    } else {
        float pad = Map::feature_half_width(f) + 350.f;
        x0 = std::min(f.left, f.right) - pad;
        x1 = std::max(f.left, f.right) + pad;
        y0 = std::min(f.top, f.bottom) - pad;
        y1 = std::max(f.top, f.bottom) + pad;
    }
    return x0 < right && x1 > left && y0 < bottom && y1 > top;
}

void feature_center(TerrainFeature const &f, float &cx, float &cy) {
    if (f.shape == TerrainShape::kAABB) {
        cx = 0.5f * (f.left + f.right);
        cy = 0.5f * (f.top + f.bottom);
    } else {
        cx = f.left;
        cy = f.top;
    }
}

// G04-style channel / dirt-road detail (clipped by caller). Wide arteries need
// dense soft blobs + grit so they don't read as flat solid fills.
void paint_channel_detail(Renderer &r, TerrainFeature const &f,
                          TerrainArt::ThemeMaterial const &tm,
                          float leftX, float topY, float rightX, float bottomY) {
    float hw = Map::feature_half_width(f);
    if (hw < 8.f) hw = 48.f;
    float step = std::clamp(hw * 0.22f, 28.f, 56.f);
    uint32_t seed = f.seed ? f.seed : 1u;

    auto sample_spine = [&](float t, float &cx, float &cy) {
        if (f.shape == TerrainShape::kSpiral)
            Map::spiral_center_at(f, t, cx, cy);
        else if (f.shape == TerrainShape::kRiver)
            Map::river_center_at(f, t, cx, cy);
        else if (f.shape == TerrainShape::kAABB) {
            cx = f.left + (f.right - f.left) * t;
            cy = f.top + (f.bottom - f.top) * 0.5f;
        } else {
            cx = f.left;
            cy = f.top;
        }
    };

    // Opaque mixes — alpha comes only from globalAlpha (channel_speck is translucent).
    uint32_t soil_dark = (tm.channel_speck & 0x00ffffffu) | 0xff000000u;
    // Mix toward theme ground_light (not pure white) so Polar channels don't
    // flash #fff grit against near-white snow when chunks rebake.
    uint32_t soil_hi = (tm.ground_light & 0x00ffffffu) | 0xff000000u;
    uint32_t soil_light = Renderer::MIX(tm.channel, soil_hi, 0.22f) | 0xff000000u;
    uint32_t soil_mid = Renderer::MIX(tm.channel, soil_dark, 0.35f) | 0xff000000u;

    // Soft shade / light soil patches along the road bed (G04)
    uint32_t n_blob = (uint32_t)std::clamp(hw * 0.08f, 18.f, 48.f);
    for (uint32_t i = 0; i < n_blob; ++i) {
        float t = (float)i / (float)(n_blob - 1u);
        float cx, cy;
        sample_spine(t, cx, cy);
        float jx = Map::terrain_hash(seed, i + 101) * hw * 0.72f;
        float jy = Map::terrain_hash(seed, i + 131) * hw * 0.55f;
        float px = cx + jx, py = cy + jy;
        if (px < leftX - 60.f || px > rightX + 60.f || py < topY - 60.f || py > bottomY + 60.f)
            continue;
        if (!Map::point_in_feature(f, px, py)) continue;
        float br = 22.f + std::fabs(Map::terrain_hash(seed, i + 151)) * 42.f;
        r.set_fill(((i & 1u) == 0u) ? soil_dark : soil_light);
        r.set_global_alpha(((i & 1u) == 0u) ? 0.28f : 0.2f);
        r.begin_path();
        r.ellipse(px, py, br, br * 0.68f);
        r.fill();
        r.set_global_alpha(1.f);
    }

    // Round grit / soil grains (art spec: 4–11px)
    uint32_t n_grit = (uint32_t)std::clamp(hw * 0.22f, 48.f, 140.f);
    for (uint32_t i = 0; i < n_grit; ++i) {
        float t = Map::terrain_hash(seed, i + 201) * 0.5f + 0.5f;
        float cx, cy;
        sample_spine(t, cx, cy);
        float jx = Map::terrain_hash(seed, i + 221) * hw * 0.85f;
        float jy = Map::terrain_hash(seed, i + 241) * hw * 0.7f;
        float px = cx + jx, py = cy + jy;
        if (px < leftX - 20.f || px > rightX + 20.f || py < topY - 20.f || py > bottomY + 20.f)
            continue;
        if (!Map::point_in_feature(f, px, py)) continue;
        float gr = 2.4f + std::fabs(Map::terrain_hash(seed, i + 261)) * 6.5f;
        r.set_fill(((i % 3u) == 0u) ? soil_dark : soil_mid);
        r.begin_path();
        r.ellipse(px, py, gr, gr * 0.75f);
        r.fill();
    }

    // Short scrapes
    uint32_t n_scratch = (uint32_t)std::clamp(hw * 0.06f, 12.f, 32.f);
    r.round_line_cap();
    for (uint32_t i = 0; i < n_scratch; ++i) {
        float t = (float)i / (float)(n_scratch + 1u);
        float cx, cy;
        sample_spine(t, cx, cy);
        float jx = Map::terrain_hash(seed, i + 301) * hw * 0.5f;
        float jy = Map::terrain_hash(seed, i + 321) * hw * 0.4f;
        float px = cx + jx, py = cy + jy;
        if (!Map::point_in_feature(f, px, py)) continue;
        float ang = Map::terrain_hash(seed, i + 341) * (float)M_PI;
        float len = 14.f + std::fabs(Map::terrain_hash(seed, i + 361)) * 22.f;
        float dx = std::cos(ang) * len * 0.5f;
        float dy = std::sin(ang) * len * 0.5f;
        r.set_stroke(soil_light);
        r.set_line_width(2.0f + std::fabs(Map::terrain_hash(seed, i + 381)) * 2.2f);
        r.set_global_alpha(0.45f);
        r.begin_path();
        r.move_to(px - dx, py - dy);
        r.line_to(px + dx, py + dy);
        r.stroke();
        r.set_global_alpha(1.f);
    }

    // Theme accent (fallen leaf / spark / fleck)
    uint32_t n_accent = (uint32_t)std::clamp(hw / step, 4.f, 14.f);
    for (uint32_t i = 0; i < n_accent; ++i) {
        float t = 0.08f + 0.84f * (float)i / (float)(n_accent - 1u);
        float cx, cy;
        sample_spine(t, cx, cy);
        float jx = Map::terrain_hash(seed, i + 401) * hw * 0.6f;
        float jy = Map::terrain_hash(seed, i + 421) * hw * 0.5f;
        float px = cx + jx, py = cy + jy;
        if (!Map::point_in_feature(f, px, py)) continue;
        if (tm.style == TerrainArt::Style::kHedge || tm.style == TerrainArt::Style::kJungle) {
            r.set_stroke(0xff6B8F3C);
            r.set_line_width(2.2f);
            r.begin_path();
            r.move_to(px - 5.f, py);
            r.qcurve_to(px, py - 7.f, px + 6.f, py - 2.f);
            r.stroke();
        } else if (tm.style == TerrainArt::Style::kMasonry) {
            r.set_fill(soil_light);
            r.begin_path();
            r.arc(px, py, 2.6f);
            r.fill();
        } else {
            r.set_fill(soil_light);
            r.begin_path();
            r.ellipse(px, py, 3.5f, 2.2f);
            r.fill();
        }
    }
}

void paint_static(Renderer &renderer, uint8_t mid, uint32_t difficulty,
                  float leftX, float topY, float rightX, float bottomY) {
    TerrainArt::ThemeMaterial const &tm = TerrainArt::theme(mid);

    // --- Pass 1: ground fills (zone rings tinted toward theme ground) ---
    // Base theme fill first so the dark void never peeks through unbaked tiles
    // (title preview paints tm.ground full-screen; in-game used to show charcoal).
    float const gl = std::max(leftX, 0.f);
    float const gt = std::max(topY, 0.f);
    float const gr = std::min(rightX, (float)ARENA_WIDTH);
    float const gb = std::min(bottomY, (float)ARENA_HEIGHT);
    if (gr > gl && gb > gt) {
        renderer.set_fill(Renderer::MIX(tm.ground, tm.ground_light, 0.2f));
        renderer.fill_rect(gl, gt, gr - gl, gb - gt);
    }
    // Specialty eco-pockets drive spawns but must not paint flat rectangles.
    MapZones const &zones = map_zones(mid);
    // First MAX_DIFFICULTY+1 entries are concentric rarity rings (outer→hub).
    uint32_t ring_n = zones.size() < (MAX_DIFFICULTY + 1) ? zones.size() : (MAX_DIFFICULTY + 1);
    for (uint32_t i = 0; i < ring_n; ++i) {
        ZoneDefinition const &def = zones[i];
        if (def.right <= leftX || def.left >= rightX || def.bottom <= topY || def.top >= bottomY)
            continue;
        // Tint ground toward rarity colour (subtle); hub stays light.
        float tint = 0.04f + 0.03f * (float)def.difficulty / (float)MAX_DIFFICULTY;
        if (def.difficulty == 0) tint = 0.f;
        uint32_t col = tm.ground;
        if (def.difficulty == 0)
            col = Renderer::MIX(tm.ground, tm.ground_light, 0.35f);
        else if (def.difficulty >= RarityID::kLegendary)
            col = Renderer::MIX(tm.ground, tm.wall_dark, 0.12f + tint);
        else if (def.difficulty >= RarityID::kRare)
            col = Renderer::MIX(tm.ground, tm.wall_mid, 0.08f + tint * 0.5f);
        // Soft "above your band" cue — was 0x40000000 black overlays that read as
        // ash/charcoal once several nested AABBs or Unique gunmetal mixed in.
        if (difficulty > def.difficulty && def.difficulty > 0)
            col = Renderer::MIX(col, tm.ground_shade | 0xff000000u, 0.14f);
        renderer.set_fill(col);
        renderer.fill_rect(def.left, def.top, def.right - def.left, def.bottom - def.top);
    }

    TerrainPattern::draw(renderer, mid, leftX, topY, rightX, bottomY);

    MapTerrain const &terrain = map_terrain(mid);
    float oxs[TERRAIN_OUTLINE_MAX];
    float oys[TERRAIN_OUTLINE_MAX];
    auto path_outline = [&](TerrainFeature const &f, uint8_t smooth) {
        uint32_t n = Map::terrain_outline(f, oxs, oys, TERRAIN_OUTLINE_MAX);
        if (n < 3) return 0u;
        renderer.begin_path();
        renderer.move_to(oxs[0], oys[0]);
        if (smooth && n >= 4) {
            for (uint32_t i = 1; i + 1 < n; ++i) {
                float mx = (oxs[i] + oxs[i + 1]) * 0.5f;
                float my = (oys[i] + oys[i + 1]) * 0.5f;
                renderer.qcurve_to(oxs[i], oys[i], mx, my);
            }
            renderer.line_to(oxs[n - 1], oys[n - 1]);
        } else {
            for (uint32_t i = 1; i < n; ++i)
                renderer.line_to(oxs[i], oys[i]);
        }
        renderer.close_path();
        return n;
    };

    auto feature_center_on_channel = [&](TerrainFeature const &f) -> uint8_t {
        float cx, cy;
        feature_center(f, cx, cy);
        for (TerrainFeature const &ch : terrain) {
            if (ch.kind != TerrainKind::kChannel) continue;
            if (Map::point_in_feature(ch, cx, cy)) return 1;
        }
        return 0;
    };

    // --- Pass 2a: terrain fills (hills / water) — florr_clone fill-before-edge ---
    for (TerrainFeature const &f : terrain) {
        if (f.kind == TerrainKind::kChannel || !feature_intersects(f, leftX, topY, rightX, bottomY))
            continue;
        // Skip blockers whose center sits on an artery (overlap residue)
        if (feature_center_on_channel(f)) continue;
        uint8_t smooth = (f.kind == TerrainKind::kWater || f.shape == TerrainShape::kRiver) ? 1 : 0;
        if (f.kind == TerrainKind::kHill) {
            renderer.set_fill(tm.wall_deep);
            if (!path_outline(f, 0)) continue;
            renderer.fill();
            RenderContext wall_clip(&renderer);
            renderer.clip();
            TerrainPattern::draw_wall(renderer, f, mid);
        } else {
            renderer.set_fill(tm.water);
            if (!path_outline(f, smooth)) continue;
            renderer.fill();
            // Short open currents keep the surface directional without rings.
            renderer.set_stroke(tm.water_hi);
            renderer.set_line_width(2.2f);
            renderer.round_line_cap();
            if (f.shape == TerrainShape::kRiver || f.shape == TerrainShape::kSpiral) {
                for (uint32_t i = 1; i < 11; ++i) {
                    float t = (float)i / 12.f;
                    float cx, cy;
                    if (f.shape == TerrainShape::kSpiral)
                        Map::spiral_center_at(f, t, cx, cy);
                    else
                        Map::river_center_at(f, t, cx, cy);
                    renderer.begin_path();
                    renderer.move_to(cx - 38.f, cy + 4.f);
                    renderer.bcurve_to(cx - 13.f, cy - 9.f,
                                       cx + 16.f, cy + 10.f, cx + 42.f, cy - 2.f);
                    renderer.stroke();
                }
            } else {
                float cx, cy; feature_center(f, cx, cy);
                for (uint32_t i = 0; i < 3; ++i) {
                    float x = cx - 64.f + i * 32.f;
                    float y = cy - 24.f + i * 26.f;
                    renderer.begin_path();
                    renderer.move_to(x, y);
                    renderer.bcurve_to(x + 26.f, y - 12.f,
                                       x + 50.f, y + 8.f, x + 77.f, y - 5.f);
                    renderer.stroke();
                }
            }
        }
    }

    // --- Pass 2b: hard outlines (before channels so roads stay clean on top) ---
    renderer.round_line_join();
    renderer.round_line_cap();
    for (TerrainFeature const &f : terrain) {
        if (f.kind == TerrainKind::kChannel || !feature_intersects(f, leftX, topY, rightX, bottomY))
            continue;
        if (feature_center_on_channel(f)) continue;
        uint8_t smooth = (f.kind == TerrainKind::kWater || f.shape == TerrainShape::kRiver) ? 1 : 0;
        if (f.kind != TerrainKind::kHill) continue;
        renderer.set_stroke(tm.wall_outline);
        renderer.set_line_width(tm.outline_w);
        if (!path_outline(f, smooth)) continue;
        renderer.stroke();
        if (f.kind == TerrainKind::kHill) {
            renderer.set_stroke(tm.wall_hi);
            renderer.set_line_width(2.4f);
            path_outline(f, 0);
            renderer.stroke();
        }
    }

    // --- Pass 2c: channels LAST — dirt roads sit above wall fills/outlines ---
    for (TerrainFeature const &f : terrain) {
        if (f.kind != TerrainKind::kChannel || !feature_intersects(f, leftX, topY, rightX, bottomY))
            continue;
        if (!path_outline(f, 1)) continue;
        renderer.set_fill(tm.channel);
        renderer.fill();
        // Clip detail to the road silhouette so grit doesn't spill onto grass
        RenderContext clip_guard(&renderer);
        if (path_outline(f, 1)) {
            renderer.clip();
            paint_channel_detail(renderer, f, tm, leftX, topY, rightX, bottomY);
        }
    }
}

void render_chunk(Chunk &chunk, uint8_t map_id, uint32_t difficulty) {
    Renderer &r = *chunk.canvas;
    r.set_dimensions(kChunkSize + 2 * kChunkPad, kChunkSize + 2 * kChunkPad);
    r.reset();
    r.clear();
    float left = (float)(chunk.x * kChunkSize);
    float top = (float)(chunk.y * kChunkSize);
    r.translate(-left + kChunkPad, -top + kChunkPad);
    paint_static(r, map_id, difficulty, left - kChunkPad, top - kChunkPad,
                 left + kChunkSize + kChunkPad, top + kChunkSize + kChunkPad);
    chunk.baked_difficulty = difficulty;
}
} // namespace

void MapChunkCache::clear() {
    chunks.clear();
    cached_map = 255;
    cached_difficulty = 0xffffffffu;
    full_paint_mode = 0;
}

void MapChunkCache::draw(Renderer &target, uint8_t map_id, uint32_t difficulty,
                         float left, float top, float right, float bottom, bool mobile) {
    // Map change: terrain differs — drop canvases.
    // Difficulty change: fog is baked into zone fills; NEVER clear() here
    // (emptied every chunk → whole-viewport flash when level crosses
    // LEVELS_PER_EXTRA_SLOT). Keep old canvases blit-able; rebake lazily.
    if (map_id != cached_map) {
        clear();
        cached_map = map_id;
        cached_difficulty = difficulty;
    } else if (difficulty != cached_difficulty) {
        cached_difficulty = difficulty;
    }
    ++frame;

    // Only chunks whose origin lies inside the arena (edge chunks may pad past).
    int const max_cx = std::max(0, ((int)ARENA_WIDTH - 1) / kChunkSize);
    int const max_cy = std::max(0, ((int)ARENA_HEIGHT - 1) / kChunkSize);
    int x0 = std::max(0, (int)std::floor(left / kChunkSize));
    int y0 = std::max(0, (int)std::floor(top / kChunkSize));
    int x1 = std::min(max_cx, (int)std::floor(right / kChunkSize));
    int y1 = std::min(max_cy, (int)std::floor(bottom / kChunkSize));
    if (x1 < x0 || y1 < y0) return;

    // Clamp live paint to arena so the void outside the map stays the solid
    // backdrop color (avoids edge-chunk pad flashing ground past the border).
    float const paint_l = std::max(left, 0.f);
    float const paint_t = std::max(top, 0.f);
    float const paint_r = std::min(right, (float)ARENA_WIDTH);
    float const paint_b = std::min(bottom, (float)ARENA_HEIGHT);
    if (paint_r <= paint_l || paint_b <= paint_t) return;

    // Larger desktop cache stays in blit mode longer (full paint_static every
    // frame under zoom-out was a steady hitch).
    size_t limit = mobile ? 48u : 96u;
    size_t leave_limit = mobile ? 32u : 72u;
    size_t needed = (size_t)(x1 - x0 + 1) * (size_t)(y1 - y0 + 1);
    if (needed > limit) full_paint_mode = 1;
    else if (needed <= leave_limit) full_paint_mode = 0;
    if (full_paint_mode) {
        // Still bake a few chunks so leaving full-paint is cheaper next frames.
        size_t const warm = mobile ? 2u : 4u;
        size_t warmed = 0;
        for (int y = y0; y <= y1 && warmed < warm; ++y) {
            for (int x = x0; x <= x1 && warmed < warm; ++x) {
                auto it = std::find_if(chunks.begin(), chunks.end(), [=](Chunk const &c) {
                    return c.x == x && c.y == y;
                });
                if (it != chunks.end()) {
                    it->used = frame;
                    if (it->baked_difficulty != difficulty) {
                        render_chunk(*it, map_id, difficulty);
                        ++warmed;
                    }
                    continue;
                }
                if (chunks.size() == limit) {
                    it = std::min_element(chunks.begin(), chunks.end(),
                        [](Chunk const &a, Chunk const &b) { return a.used < b.used; });
                    it->x = x; it->y = y;
                    it->baked_difficulty = 0xffffffffu;
                } else {
                    Chunk fresh;
                    fresh.x = x; fresh.y = y;
                    fresh.canvas = std::make_unique<Renderer>();
                    chunks.push_back(std::move(fresh));
                    it = chunks.end() - 1;
                }
                render_chunk(*it, map_id, difficulty);
                it->used = frame;
                ++warmed;
            }
        }
        paint_static(target, map_id, difficulty, paint_l, paint_t, paint_r, paint_b);
        return;
    }

    // Cold cache (spawn / map change): more first-frame canvas allocs stall the
    // title→game circle wipe; keep budget modest but above steady-state.
    size_t missing_pre = 0;
    for (int y = y0; y <= y1; ++y) {
        for (int x = x0; x <= x1; ++x) {
            auto it = std::find_if(chunks.begin(), chunks.end(), [=](Chunk const &c) {
                return c.x == x && c.y == y;
            });
            if (it == chunks.end()) ++missing_pre;
        }
    }
    size_t bake_budget = mobile ? 3u : 6u;
    if (missing_pre * 2u >= needed)
        bake_budget = mobile ? 5u : 10u;
    size_t baked = 0;
    size_t missing = 0;

    // Prefer refreshing existing canvases (stale fog) over allocating holes so
    // a difficulty-band change never leaves an empty frame.
    for (int y = y0; y <= y1; ++y) {
        for (int x = x0; x <= x1; ++x) {
            auto it = std::find_if(chunks.begin(), chunks.end(), [=](Chunk const &c) {
                return c.x == x && c.y == y;
            });
            if (it != chunks.end()) {
                it->used = frame;
                if (it->baked_difficulty != difficulty && baked < bake_budget) {
                    render_chunk(*it, map_id, difficulty);
                    ++baked;
                }
                continue;
            }
            ++missing;
            if (baked >= bake_budget) continue;
            if (chunks.size() == limit) {
                it = std::min_element(chunks.begin(), chunks.end(), [](Chunk const &a, Chunk const &b) {
                    return a.used < b.used;
                });
                it->x = x;
                it->y = y;
                it->baked_difficulty = 0xffffffffu;
            } else {
                Chunk fresh;
                fresh.x = x;
                fresh.y = y;
                fresh.used = 0;
                fresh.baked_difficulty = 0xffffffffu;
                fresh.canvas = std::make_unique<Renderer>();
                chunks.push_back(std::move(fresh));
                it = chunks.end() - 1;
            }
            render_chunk(*it, map_id, difficulty);
            it->used = frame;
            ++baked;
            --missing;
        }
    }

    // Many holes: one viewport paint_static (zone/terrain loops once) instead of
    // N per-tile paints — that N× cost made the spawn circle hitch every frame
    // until the cache filled. Keep baking above so later frames blit.
    if (missing > 1u) {
        paint_static(target, map_id, difficulty, paint_l, paint_t, paint_r, paint_b);
        return;
    }

    // Steady: blit existing canvases (stale fog OK); hole-fill at most one miss.
    for (int y = y0; y <= y1; ++y) {
        for (int x = x0; x <= x1; ++x) {
            auto it = std::find_if(chunks.begin(), chunks.end(), [=](Chunk const &c) {
                return c.x == x && c.y == y;
            });
            float const cx0 = (float)(x * kChunkSize);
            float const cy0 = (float)(y * kChunkSize);
            float const cx1 = cx0 + (float)kChunkSize;
            float const cy1 = cy0 + (float)kChunkSize;
            if (it != chunks.end() && it->canvas) {
                target.draw_image_at(*it->canvas,
                                     cx0 - (float)kChunkPad,
                                     cy0 - (float)kChunkPad);
            } else {
                float hl = std::max(cx0, paint_l);
                float ht = std::max(cy0, paint_t);
                float hr = std::min(cx1, paint_r);
                float hb = std::min(cy1, paint_b);
                if (hr > hl && hb > ht)
                    paint_static(target, map_id, difficulty, hl, ht, hr, hb);
            }
        }
    }
}
