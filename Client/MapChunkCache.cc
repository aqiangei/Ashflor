#include <Client/MapChunkCache.hh>
#include <Client/ThemeMaterial.hh>

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

void feature_center(TerrainFeature const &f, float &cx, float &cy) {
    if (f.shape == TerrainShape::kAABB) {
        cx = 0.5f * (f.left + f.right);
        cy = 0.5f * (f.top + f.bottom);
    } else {
        cx = f.left;
        cy = f.top;
    }
}

// Approximate interior sample for cluster decoration (AABB / circle / ellipse).
uint8_t sample_in_feature(TerrainFeature const &f, float &ox, float &oy, uint32_t seed, uint32_t i) {
    float u = Map::terrain_hash(seed, i);
    float v = Map::terrain_hash(seed, i + 17);
    if (f.shape == TerrainShape::kAABB) {
        float mx = 0.5f * (f.left + f.right), my = 0.5f * (f.top + f.bottom);
        float hw = (f.right - f.left) * 0.38f, hh = (f.bottom - f.top) * 0.38f;
        ox = mx + u * hw;
        oy = my + v * hh;
        return 1;
    }
    if (f.shape == TerrainShape::kCircle) {
        float r = f.right > 0.f ? f.right : 400.f;
        float ang = u * (float)(2.0 * M_PI);
        float rad = (0.15f + 0.55f * std::fabs(v)) * r;
        ox = f.left + std::cos(ang) * rad;
        oy = f.top + std::sin(ang) * rad;
        return 1;
    }
    if (f.shape == TerrainShape::kEllipse) {
        float ang = u * (float)(2.0 * M_PI);
        float rx = f.right * (0.15f + 0.55f * std::fabs(v));
        float ry = f.bottom * (0.15f + 0.55f * std::fabs(Map::terrain_hash(seed, i + 31)));
        ox = f.left + std::cos(ang) * rx;
        oy = f.top + std::sin(ang) * ry;
        return 1;
    }
    return 0;
}

void paint_ground_detail(Renderer &r, uint8_t mid, TerrainArt::ThemeMaterial const &tm,
                         float leftX, float topY, float rightX, float bottomY) {
    float step = tm.ground_step;
    int x0 = (int)std::floor(leftX / step) - 1;
    int y0 = (int)std::floor(topY / step) - 1;
    int x1 = (int)std::ceil(rightX / step) + 1;
    int y1 = (int)std::ceil(bottomY / step) + 1;
    uint32_t seed0 = 0xC0FFEEu ^ ((uint32_t)mid * 0x9E3779B9u);

    r.round_line_cap();
    r.round_line_join();

    for (int gy = y0; gy <= y1; ++gy) {
        for (int gx = x0; gx <= x1; ++gx) {
            float bx = (float)gx * step;
            float by = (float)gy * step;
            uint32_t cell = seed0 + (uint32_t)gx * 73856093u + (uint32_t)gy * 19349663u;
            float jx = Map::terrain_hash(cell, 1) * step * 0.35f;
            float jy = Map::terrain_hash(cell, 2) * step * 0.35f;
            float px = bx + jx, py = by + jy;
            if (px < leftX - 40.f || px > rightX + 40.f || py < topY - 40.f || py > bottomY + 40.f)
                continue;
            if (Map::circle_hits_impassable(mid, px, py, 28.f)) continue;

            // Low-freq shade blob
            if ((cell & 3u) != 0u) {
                float br = 28.f + std::fabs(Map::terrain_hash(cell, 3)) * 40.f;
                r.set_fill(tm.ground_shade);
                r.begin_path();
                r.ellipse(px, py, br, br * 0.72f);
                r.fill();
            }

            // Style-specific micro detail (per biome art spec)
            switch (tm.style) {
            case TerrainArt::Style::kHedge:
            case TerrainArt::Style::kJungle: {
                uint8_t dense = (tm.style == TerrainArt::Style::kJungle) ? 1 : 0;
                if ((cell & 7u) < (dense ? 6u : 5u)) {
                    r.set_stroke(dense ? tm.wall_dark : tm.wall_mid);
                    r.set_line_width(dense ? 2.8f : 2.4f);
                    r.begin_path();
                    for (uint32_t k = 0; k < 3; ++k) {
                        float ox = Map::terrain_hash(cell, 10 + k) * 10.f;
                        float h = 9.f + std::fabs(Map::terrain_hash(cell, 20 + k)) * 8.f;
                        r.move_to(px + ox, py + 4.f);
                        r.qcurve_to(px + ox + 3.f, py - h * 0.4f, px + ox - 2.f, py - h);
                    }
                    r.stroke();
                }
                if (!dense && (cell % 11u) == 0u) {
                    r.set_fill(tm.ground_light);
                    r.begin_path();
                    for (uint32_t k = 0; k < 4; ++k) {
                        float ang = (float)k * (float)M_PI * 0.5f;
                        r.ellipse(px + std::cos(ang) * 5.f, py + std::sin(ang) * 5.f, 3.2f, 3.2f);
                    }
                    r.fill();
                    r.set_fill(0xffF0C040);
                    r.begin_path();
                    r.arc(px, py, 2.2f);
                    r.fill();
                } else if ((cell % 5u) == 0u) {
                    r.set_fill(tm.ground_light);
                    r.begin_path();
                    r.arc(px, py, dense ? 2.4f : 1.6f);
                    r.fill();
                }
                break;
            }
            case TerrainArt::Style::kRock: {
                // Dune ripples + sand grains
                if ((cell & 1u) == 0u) {
                    r.set_stroke(tm.wall_dark);
                    r.set_line_width(1.8f);
                    r.set_global_alpha(0.35f);
                    r.begin_path();
                    float w = 22.f + std::fabs(Map::terrain_hash(cell, 4)) * 18.f;
                    r.move_to(px - w, py);
                    r.qcurve_to(px, py + 6.f * Map::terrain_hash(cell, 5), px + w, py);
                    r.stroke();
                    r.set_global_alpha(1.f);
                }
                if ((cell & 3u) == 0u) {
                    r.set_fill(tm.ground_light);
                    r.begin_path();
                    r.ellipse(px, py, 4.f + std::fabs(Map::terrain_hash(cell, 6)) * 6.f,
                              3.f + std::fabs(Map::terrain_hash(cell, 7)) * 4.f);
                    r.fill();
                }
                break;
            }
            case TerrainArt::Style::kReef: {
                // Soft caustics + coral speckles
                if ((cell & 3u) != 0u) {
                    r.set_stroke(tm.water_hi);
                    r.set_line_width(1.4f);
                    r.begin_path();
                    r.ellipse(px, py, 18.f, 8.f);
                    r.stroke();
                }
                if ((cell % 9u) == 0u) {
                    r.set_fill(tm.channel);
                    r.begin_path();
                    r.arc(px, py, 3.5f);
                    r.fill();
                }
                break;
            }
            case TerrainArt::Style::kIce: {
                // Soft snow drifts + sparkle
                if ((cell & 3u) == 0u) {
                    r.set_fill(tm.ground_light);
                    r.begin_path();
                    r.ellipse(px, py, 16.f + std::fabs(Map::terrain_hash(cell, 4)) * 14.f,
                              10.f + std::fabs(Map::terrain_hash(cell, 5)) * 8.f);
                    r.fill();
                }
                if ((cell % 6u) == 0u) {
                    r.set_fill(0xffffffff);
                    r.begin_path();
                    r.ellipse(px, py, 2.2f, 7.f);
                    r.fill();
                }
                break;
            }
            case TerrainArt::Style::kDirt: {
                // Packed nest floor: soft soil blobs + grit
                if ((cell & 3u) == 0u) {
                    r.set_fill(tm.ground_shade);
                    r.begin_path();
                    r.ellipse(px, py,
                              14.f + std::fabs(Map::terrain_hash(cell, 2)) * 18.f,
                              9.f + std::fabs(Map::terrain_hash(cell, 3)) * 10.f);
                    r.fill();
                }
                if ((cell & 7u) == 0u) {
                    r.set_fill(tm.wall_dark);
                    r.begin_path();
                    r.ellipse(px + 4.f, py - 2.f, 5.f, 3.5f);
                    r.fill();
                }
                // Dry root curls
                if ((cell % 11u) == 0u) {
                    r.set_stroke(tm.channel_speck);
                    r.set_line_width(2.2f);
                    r.begin_path();
                    float rr = 10.f + std::fabs(Map::terrain_hash(cell, 4)) * 8.f;
                    r.move_to(px - rr * 0.4f, py);
                    r.qcurve_to(px, py - rr * 0.6f, px + rr * 0.5f, py + rr * 0.2f);
                    r.stroke();
                }
                // Occasional egg / seed fleck
                if ((cell % 17u) == 0u) {
                    r.set_fill(0xffF2DDB5);
                    r.begin_path();
                    r.ellipse(px, py, 3.2f, 2.4f);
                    r.fill();
                }
                break;
            }
            case TerrainArt::Style::kLava: {
                // Ash grit + ember sparks
                if ((cell & 3u) == 0u) {
                    r.set_fill(tm.wall_dark);
                    r.begin_path();
                    r.ellipse(px, py, 6.f, 4.f);
                    r.fill();
                }
                if ((cell % 10u) == 0u) {
                    r.set_fill(tm.ground_light);
                    r.begin_path();
                    r.arc(px, py, 2.8f);
                    r.fill();
                }
                break;
            }
            case TerrainArt::Style::kMasonry: {
                // Warm stone tile seams (readable plaza)
                float tile = 48.f;
                float tx = std::floor(px / tile) * tile;
                float ty = std::floor(py / tile) * tile;
                if ((cell & 7u) == 0u) {
                    r.set_stroke(tm.ground_shade | 0xff000000);
                    r.set_line_width(2.2f);
                    r.set_global_alpha(0.45f);
                    r.begin_path();
                    r.rect(tx, ty, tile, tile);
                    r.stroke();
                    r.set_global_alpha(1.f);
                }
                if ((cell % 13u) == 0u) {
                    r.set_fill(tm.channel_speck); // gold fleck
                    r.begin_path();
                    r.arc(px, py, 2.4f);
                    r.fill();
                }
                break;
            }
            default:
                break;
            }
        }
    }
}

void paint_wall_clusters(Renderer &r, TerrainFeature const &f, TerrainArt::ThemeMaterial const &tm) {
    uint32_t seed = f.seed ? f.seed : 1u;
    uint32_t n = 10;
    if (f.shape == TerrainShape::kAABB) {
        float area = std::max(1.f, (f.right - f.left) * (f.bottom - f.top));
        n = (uint32_t)std::clamp(area / 180000.f, 8.f, 28.f);
    } else if (f.shape == TerrainShape::kCircle || f.shape == TerrainShape::kEllipse) {
        n = 12;
    } else {
        return; // rivers / spirals: outline only
    }

    r.round_line_cap();
    for (uint32_t i = 0; i < n; ++i) {
        float sx, sy;
        if (!sample_in_feature(f, sx, sy, seed, i)) continue;
        float rr = tm.cluster_r_lo + std::fabs(Map::terrain_hash(seed, i + 50)) * (tm.cluster_r_hi - tm.cluster_r_lo);

        if (tm.style == TerrainArt::Style::kMasonry) {
            // Squared masonry blocks + mortar
            float bw = rr * 1.6f, bh = rr * 1.1f;
            r.set_fill(tm.wall_mid);
            if ((i & 1u) == 0u) r.set_fill(Renderer::MIX(tm.wall_mid, tm.wall_hi, 0.2f));
            r.begin_path();
            r.round_rect(sx - bw * 0.5f, sy - bh * 0.5f, bw, bh, 3.f);
            r.fill();
            r.set_stroke(tm.wall_dark);
            r.set_line_width(2.4f);
            r.begin_path();
            r.round_rect(sx - bw * 0.5f, sy - bh * 0.5f, bw, bh, 3.f);
            r.stroke();
            if ((i % 4u) == 0u) {
                r.set_fill(tm.channel_speck); // royal gold trim
                r.begin_path();
                r.ellipse(sx, sy - bh * 0.25f, bw * 0.2f, 2.5f);
                r.fill();
            }
            continue;
        }

        if (tm.style == TerrainArt::Style::kRock || tm.style == TerrainArt::Style::kLava) {
            // Slightly angular rock blobs
            r.set_fill(tm.wall_mid);
            if ((i & 1u) == 0u) r.set_fill(Renderer::MIX(tm.wall_mid, tm.wall_hi, 0.2f));
            r.begin_path();
            r.ellipse(sx, sy, rr * 1.15f, rr * 0.75f);
            r.fill();
            r.set_stroke(tm.wall_dark);
            r.set_line_width(2.8f);
            r.begin_path();
            r.move_to(sx - rr, sy);
            r.line_to(sx - rr * 0.2f, sy + rr * 0.55f);
            r.line_to(sx + rr * 0.9f, sy + rr * 0.2f);
            r.stroke();
            if (tm.style == TerrainArt::Style::kLava && (i % 3u) == 0u) {
                r.set_fill(tm.wall_hi);
                r.begin_path();
                r.arc(sx + rr * 0.2f, sy - rr * 0.15f, 3.5f);
                r.fill();
            }
            continue;
        }

        if (tm.style == TerrainArt::Style::kIce) {
            // Faceted ice: deep base + bright cap
            r.set_fill(tm.wall_deep);
            r.begin_path();
            r.ellipse(sx, sy, rr, rr * 0.9f);
            r.fill();
            r.set_fill(tm.wall_mid);
            r.begin_path();
            r.ellipse(sx, sy - rr * 0.15f, rr * 0.7f, rr * 0.55f);
            r.fill();
            r.set_fill(tm.wall_hi);
            r.begin_path();
            r.ellipse(sx - rr * 0.15f, sy - rr * 0.35f, rr * 0.35f, rr * 0.22f);
            r.fill();
            continue;
        }

        // Hedge / jungle / dirt / reef — soft clusters
        r.set_fill(tm.wall_mid);
        if ((i & 1u) == 0u) r.set_fill(Renderer::MIX(tm.wall_mid, tm.wall_hi, 0.25f));
        r.begin_path();
        r.ellipse(sx, sy, rr, rr * 0.88f);
        r.fill();
        r.set_stroke(tm.wall_dark);
        r.set_line_width(3.2f);
        r.begin_path();
        r.partial_arc(sx, sy + rr * 0.15f, rr * 0.7f, 0.3f, (float)M_PI - 0.3f, 0);
        r.stroke();
        r.set_stroke(tm.wall_hi);
        r.set_line_width(3.8f);
        r.begin_path();
        r.partial_arc(sx, sy - rr * 0.2f, rr * 0.55f, (float)M_PI + 0.4f, (float)(2.0 * M_PI) - 0.4f, 0);
        r.stroke();

        if ((tm.style == TerrainArt::Style::kHedge || tm.style == TerrainArt::Style::kJungle)
            && (i % 3u) == 0u) {
            r.set_stroke(tm.wall_hi);
            r.set_line_width(2.2f);
            r.begin_path();
            float ang = Map::terrain_hash(seed, i + 80) * (float)M_PI;
            r.move_to(sx, sy);
            r.qcurve_to(sx + std::cos(ang) * rr, sy + std::sin(ang) * rr * 0.2f,
                        sx + std::cos(ang) * rr * 1.2f, sy + std::sin(ang) * rr * 0.9f);
            r.stroke();
        }
    }
}

void paint_static(Renderer &renderer, uint8_t mid, uint32_t difficulty,
                  float leftX, float topY, float rightX, float bottomY) {
    TerrainArt::ThemeMaterial const &tm = TerrainArt::theme(mid);

    // --- Pass 1: ground fills (zone rings tinted toward theme ground) ---
    // Specialty eco-pockets drive spawns but must not paint flat rectangles.
    MapZones const &zones = map_zones(mid);
    uint32_t ring_n = zones.size() < 4 ? zones.size() : 4;
    for (uint32_t i = 0; i < ring_n; ++i) {
        ZoneDefinition const &def = zones[i];
        if (def.right <= leftX || def.left >= rightX || def.bottom <= topY || def.top >= bottomY)
            continue;
        // Outer rings slightly darker for depth cue
        uint32_t col = tm.ground;
        if (def.difficulty >= 3) col = Renderer::MIX(tm.ground, tm.wall_dark, 0.18f);
        else if (def.difficulty == 2) col = Renderer::MIX(tm.ground, tm.wall_mid, 0.10f);
        else if (def.difficulty == 0) col = Renderer::MIX(tm.ground, tm.ground_light, 0.35f);
        renderer.set_fill(col);
        renderer.fill_rect(def.left, def.top, def.right - def.left, def.bottom - def.top);
        if (difficulty > def.difficulty) {
            renderer.set_fill(0x40000000);
            renderer.fill_rect(def.left, def.top, def.right - def.left, def.bottom - def.top);
        }
    }

    // Soft weave accent (secondary; contrast now comes from wall outlines)
    constexpr float weave = 72.f;
    renderer.set_stroke(MapTheme::WEAVE_DARK);
    renderer.set_line_width(1.0f);
    renderer.begin_path();
    int diagonal_first = (int)std::floor((leftX - bottomY) / weave) - 1;
    int diagonal_last = (int)std::ceil((rightX - topY) / weave) + 1;
    for (int i = diagonal_first; i <= diagonal_last; i += 2) {
        float k = i * weave;
        renderer.move_to(k + topY, topY);
        renderer.line_to(k + bottomY, bottomY);
    }
    renderer.stroke();

    paint_ground_detail(renderer, mid, tm, leftX, topY, rightX, bottomY);

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
            paint_wall_clusters(renderer, f, tm);
        } else {
            renderer.set_fill(tm.water);
            if (!path_outline(f, smooth)) continue;
            renderer.fill();
            // Ripples
            renderer.set_stroke(tm.water_hi);
            renderer.set_line_width(1.6f);
            if (f.shape == TerrainShape::kRiver || f.shape == TerrainShape::kSpiral) {
                for (uint32_t i = 1; i < 8; ++i) {
                    float t = (float)i / 9.f;
                    float cx, cy;
                    if (f.shape == TerrainShape::kSpiral)
                        Map::spiral_center_at(f, t, cx, cy);
                    else
                        Map::river_center_at(f, t, cx, cy);
                    renderer.begin_path();
                    renderer.ellipse(cx, cy, 30.f, 13.f);
                    renderer.stroke();
                }
            } else {
                float cx, cy; feature_center(f, cx, cy);
                for (uint32_t i = 0; i < 3; ++i) {
                    renderer.begin_path();
                    renderer.ellipse(cx, cy, 44.f + i * 30.f, 24.f + i * 16.f);
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
        renderer.set_stroke(f.kind == TerrainKind::kHill ? tm.wall_outline : Renderer::MIX(tm.water, 0xff000000, 0.45f));
        renderer.set_line_width(f.kind == TerrainKind::kHill ? tm.outline_w : tm.outline_w * 0.65f);
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
        renderer.set_fill(tm.channel);
        path_outline(f, 1);
        renderer.fill();
        renderer.set_fill(tm.channel_speck);
        for (uint32_t i = 0; i < 22; ++i) {
            float t = (float)i / 21.f;
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
            float jx = Map::terrain_hash(f.seed, i) * 42.f;
            float jy = Map::terrain_hash(f.seed, i + 9) * 32.f;
            renderer.begin_path();
            renderer.ellipse(cx + jx, cy + jy, 7.f, 4.f);
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

    size_t const bake_budget = mobile ? 1u : 2u;
    size_t baked = 0;
    uint8_t missing = 0;

    for (int y = y0; y <= y1; ++y) {
        for (int x = x0; x <= x1; ++x) {
            auto it = std::find_if(chunks.begin(), chunks.end(), [=](Chunk const &c) {
                return c.x == x && c.y == y;
            });
            if (it != chunks.end()) {
                it->used = frame;
                continue;
            }
            missing = 1;
            if (baked >= bake_budget) continue;
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
            it->used = frame;
            ++baked;
        }
    }

    if (missing) {
        paint_static(target, map_id, difficulty, left, top, right, bottom);
        return;
    }

    for (int y = y0; y <= y1; ++y) {
        for (int x = x0; x <= x1; ++x) {
            auto it = std::find_if(chunks.begin(), chunks.end(), [=](Chunk const &c) {
                return c.x == x && c.y == y;
            });
            if (it == chunks.end()) continue;
            target.draw_image_at(*it->canvas,
                                 (float)(x * kChunkSize - kChunkPad),
                                 (float)(y * kChunkSize - kChunkPad));
        }
    }
}
