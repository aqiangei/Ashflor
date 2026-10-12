#include <Client/MapChunkCache.hh>

#include <Client/Render/Renderer.hh>
#include <Client/Input.hh>

#include <Shared/Map.hh>
#include <Shared/StaticData.hh>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <memory>
#include <vector>

namespace {
constexpr int kChunkSize = 512;
constexpr int kChunkPad = 2;

struct Chunk {
    int x = 0;
    int y = 0;
    uint64_t used = 0;
    std::unique_ptr<Renderer> canvas;
};

std::vector<Chunk> chunks;
uint8_t cached_map = 255;
uint32_t cached_difficulty = std::numeric_limits<uint32_t>::max();
uint64_t frame = 0;

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

void paint_static(Renderer &renderer, uint8_t mid, uint32_t difficulty,
                  float leftX, float topY, float rightX, float bottomY) {
    MapZones const &zones = map_zones(mid);
    for (ZoneDefinition const &def : zones) {
        if (def.right <= leftX || def.left >= rightX || def.bottom <= topY || def.top >= bottomY)
            continue;
        renderer.set_fill(def.color);
        renderer.fill_rect(def.left, def.top, def.right - def.left, def.bottom - def.top);
        if (difficulty > def.difficulty) {
            renderer.set_fill(0x40000000);
            renderer.fill_rect(def.left, def.top, def.right - def.left, def.bottom - def.top);
        }
    }

    // World-anchored weave, so adjacent cached chunks join without moving with the camera.
    constexpr float weave = 64.f;
    renderer.set_stroke(MapTheme::WEAVE_DARK);
    renderer.set_line_width(1.2f);
    renderer.begin_path();
    int diagonal_first = (int)std::floor((leftX - bottomY) / weave) - 1;
    int diagonal_last = (int)std::ceil((rightX - topY) / weave) + 1;
    for (int i = diagonal_first; i <= diagonal_last; ++i) {
        float k = i * weave;
        renderer.move_to(k + topY, topY);
        renderer.line_to(k + bottomY, bottomY);
    }
    int angled_first = (int)std::floor((topY - 0.35f * rightX) / weave) - 1;
    int angled_last = (int)std::ceil((bottomY - 0.35f * leftX) / weave) + 1;
    for (int i = angled_first; i <= angled_last; ++i) {
        float k = i * weave;
        renderer.move_to(leftX, k + 0.35f * leftX);
        renderer.line_to(rightX, k + 0.35f * rightX);
    }
    renderer.stroke();
    renderer.set_stroke(MapTheme::WEAVE_LIGHT);
    renderer.set_line_width(0.8f);
    renderer.begin_path();
    for (int i = diagonal_first; i <= diagonal_last; ++i) {
        float k = i * weave + weave * 0.5f;
        renderer.move_to(k + topY, topY);
        renderer.line_to(k + bottomY, bottomY);
    }
    renderer.stroke();

    // Terrain drawing is shared with the uncached fallback; all details are retained.
        // Terrain fills — jagged outlines (florr_clone-web wall-edges style).
        // Hills: sharp polyline. Water/river: quadratic-smoothed banks.
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
        auto draw_feature = [&](TerrainFeature const &f) {
            uint8_t smooth = (f.kind == TerrainKind::kWater || f.shape == TerrainShape::kRiver) ? 1 : 0;
            renderer.set_fill(f.color);
            if (!path_outline(f, smooth)) return;
            renderer.fill();
            renderer.set_stroke(MapTheme::WEAVE_LIGHT);
            renderer.set_line_width(f.kind == TerrainKind::kHill ? 2.4f : 1.8f);
            path_outline(f, smooth);
            renderer.stroke();
            // Kind-specific interior detail
            if (f.kind == TerrainKind::kHill) {
                renderer.set_fill(MapTheme::WEAVE_DARK);
                uint32_t seed = f.seed ? f.seed : 1u;
                for (uint32_t i = 0; i < 10; ++i) {
                    float u = Map::terrain_hash(seed, i) * 0.45f;
                    float v = Map::terrain_hash(seed, i + 40) * 0.45f;
                    float sx, sy;
                    if (f.shape == TerrainShape::kAABB) {
                        sx = (f.left + f.right) * 0.5f + u * (f.right - f.left) * 0.35f;
                        sy = (f.top + f.bottom) * 0.5f + v * (f.bottom - f.top) * 0.35f;
                    } else if (f.shape == TerrainShape::kCircle) {
                        float r = f.right > 0.f ? f.right : 400.f;
                        sx = f.left + u * r * 0.55f;
                        sy = f.top + v * r * 0.55f;
                    } else if (f.shape == TerrainShape::kEllipse) {
                        sx = f.left + u * f.right * 0.55f;
                        sy = f.top + v * f.bottom * 0.55f;
                    } else continue;
                    renderer.begin_path();
                    renderer.ellipse(sx, sy, 14.f + i * 1.2f, 9.f + i);
                    renderer.fill();
                }
            } else if (f.kind == TerrainKind::kWater) {
                // Ripple arcs along river / around lakes
                renderer.set_stroke(MapTheme::WEAVE_LIGHT);
                renderer.set_line_width(1.2f);
                if (f.shape == TerrainShape::kRiver || f.shape == TerrainShape::kSpiral) {
                    for (uint32_t i = 1; i < 8; ++i) {
                        float t = (float)i / 9.f;
                        float cx, cy;
                        if (f.shape == TerrainShape::kSpiral)
                            Map::spiral_center_at(f, t, cx, cy);
                        else
                            Map::river_center_at(f, t, cx, cy);
                        renderer.begin_path();
                        renderer.ellipse(cx, cy, 28.f, 12.f);
                        renderer.stroke();
                    }
                } else {
                    float cx = (f.shape == TerrainShape::kAABB)
                        ? (f.left + f.right) * 0.5f : f.left;
                    float cy = (f.shape == TerrainShape::kAABB)
                        ? (f.top + f.bottom) * 0.5f : f.top;
                    for (uint32_t i = 0; i < 3; ++i) {
                        renderer.begin_path();
                        renderer.ellipse(cx, cy, 40.f + i * 28.f, 22.f + i * 16.f);
                        renderer.stroke();
                    }
                }
            }
        };
        for (TerrainFeature const &f : terrain) {
            if (f.kind == TerrainKind::kChannel || !feature_intersects(f, leftX, topY, rightX, bottomY)) continue;
            draw_feature(f);
        }
        for (TerrainFeature const &f : terrain) {
            if (f.kind != TerrainKind::kChannel || !feature_intersects(f, leftX, topY, rightX, bottomY)) continue;
            renderer.set_fill(f.color);
            path_outline(f, 1);
            renderer.fill();
            // Channel speckles along the path (leaf / sand / coral)
            renderer.set_fill(MapTheme::WEAVE_LIGHT);
            for (uint32_t i = 0; i < 18; ++i) {
                float t = (float)i / 17.f;
                float cx, cy;
                if (f.shape == TerrainShape::kRiver || f.shape == TerrainShape::kSpiral) {
                    if (f.shape == TerrainShape::kSpiral)
                        Map::spiral_center_at(f, t, cx, cy);
                    else
                        Map::river_center_at(f, t, cx, cy);
                } else if (f.shape == TerrainShape::kAABB) {
                    cx = f.left + (f.right - f.left) * t;
                    cy = f.top + (f.bottom - f.top) * 0.5f;
                } else {
                    cx = f.left; cy = f.top;
                }
                float jx = Map::terrain_hash(f.seed, i) * 40.f;
                float jy = Map::terrain_hash(f.seed, i + 9) * 30.f;
                renderer.begin_path();
                renderer.ellipse(cx + jx, cy + jy, 8.f, 4.5f);
                renderer.fill();
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
}
} // namespace

void MapChunkCache::clear() {
    chunks.clear();
    cached_map = 255;
    cached_difficulty = std::numeric_limits<uint32_t>::max();
}

void MapChunkCache::draw(Renderer &target, uint8_t map_id, uint32_t difficulty,
                         float left, float top, float right, float bottom, bool mobile) {
    if (map_id != cached_map || difficulty != cached_difficulty) {
        clear();
        cached_map = map_id;
        cached_difficulty = difficulty;
    }
    ++frame;

    int x0 = std::max(0, (int)std::floor(left / kChunkSize));
    int y0 = std::max(0, (int)std::floor(top / kChunkSize));
    int x1 = std::min((int)std::floor((float)ARENA_WIDTH / kChunkSize),
                      (int)std::floor(right / kChunkSize));
    int y1 = std::min((int)std::floor((float)ARENA_HEIGHT / kChunkSize),
                      (int)std::floor(bottom / kChunkSize));
    if (x1 < x0 || y1 < y0) return;

    size_t limit = mobile ? 32u : 48u;
    size_t needed = (size_t)(x1 - x0 + 1) * (size_t)(y1 - y0 + 1);
    if (needed > limit) {
        paint_static(target, map_id, difficulty, left, top, right, bottom);
        return;
    }
    for (int y = y0; y <= y1; ++y) {
        for (int x = x0; x <= x1; ++x) {
            auto it = std::find_if(chunks.begin(), chunks.end(), [=](Chunk const &c) {
                return c.x == x && c.y == y;
            });
            if (it == chunks.end()) {
                if (chunks.size() == limit) {
                    it = std::min_element(chunks.begin(), chunks.end(), [](Chunk const &a, Chunk const &b) {
                        return a.used < b.used;
                    });
                    it->x = x;
                    it->y = y;
                } else {
                    chunks.push_back({x, y, 0, std::make_unique<Renderer>()});
                    it = chunks.end() - 1;
                }
                render_chunk(*it, map_id, difficulty);
            }
            it->used = frame;
            target.draw_image_at(*it->canvas,
                                 (float)(x * kChunkSize - kChunkPad),
                                 (float)(y * kChunkSize - kChunkPad));
        }
    }
}
