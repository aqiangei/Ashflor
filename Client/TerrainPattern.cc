#include <Client/TerrainPattern.hh>

#include <Client/Render/Renderer.hh>
#include <Client/ThemeMaterial.hh>

#include <Shared/StaticData.hh>
#include <Shared/StaticDefinitions.hh>
#include <Shared/Map.hh>

#include <algorithm>
#include <cmath>
#include <vector>

namespace {

struct Point { float x, y; };

uint32_t hash(int x, int y, uint32_t salt) {
    uint32_t v = (uint32_t)x * 0x9e3779b9u ^ (uint32_t)y * 0x85ebca6bu ^ salt;
    v ^= v >> 16; v *= 0x7feb352du; v ^= v >> 15; v *= 0x846ca68bu;
    return v ^ (v >> 16);
}
float unit(int x, int y, uint32_t salt) {
    return (float)(hash(x, y, salt) & 0xffffu) / 65535.f;
}
float signed_unit(int x, int y, uint32_t salt) { return unit(x, y, salt) * 2.f - 1.f; }

// Soft silhouette with long sides and curved ends, rather than stacked circles.
void ribbon(Renderer &r, float x, float y, float w, float h, uint32_t fill) {
    r.set_fill(fill);
    r.begin_path();
    r.move_to(x - w * .52f, y);
    r.bcurve_to(x - w * .21f, y - h * .92f,
                x + w * .26f, y - h * .72f, x + w * .42f, y - h * .24f);
    r.qcurve_to(x + w * .62f, y - h * .38f, x + w * .55f, y + h * .15f);
    r.bcurve_to(x + w * .14f, y + h * .70f,
                x - w * .44f, y + h * .38f, x - w * .52f, y);
    r.close_path();
    r.fill();
}

void sweep(Renderer &r, float x, float y, float w, float bend,
           uint32_t color, float width) {
    r.set_stroke(color);
    r.set_line_width(width);
    r.round_line_cap();
    r.begin_path();
    r.move_to(x - w * .5f, y + bend * .25f);
    r.bcurve_to(x - w * .17f, y - bend,
                x + w * .2f, y + bend * .65f, x + w * .5f, y - bend * .15f);
    r.stroke();
}

void leaf(Renderer &r, float x, float y, float length, float width,
          uint32_t fill, uint32_t vein) {
    r.set_fill(fill);
    r.begin_path();
    r.move_to(x, y + length * .5f);
    r.bcurve_to(x - width, y + length * .13f,
                x - width * .65f, y - length * .38f, x, y - length * .5f);
    r.bcurve_to(x + width * .62f, y - length * .45f,
                x + width, y + length * .18f, x, y + length * .5f);
    r.fill();
    r.set_stroke(vein);
    r.set_line_width(2.f);
    r.begin_path();
    r.move_to(x, y + length * .42f);
    r.qcurve_to(x + width * .08f, y, x, y - length * .42f);
    r.move_to(x, y - length * .05f);
    r.line_to(x - width * .55f, y - length * .23f);
    r.move_to(x, y + length * .08f);
    r.line_to(x + width * .55f, y - length * .12f);
    r.stroke();
}

Point ocean_site(int gx, int gy) {
    constexpr float step = 650.f;
    return { (gx + .5f + signed_unit(gx, gy, 0x18a1u) * .33f) * step,
             (gy + .5f + signed_unit(gx, gy, 0x18a2u) * .33f) * step };
}

// Voronoi cells share exact world-space boundaries. Insetting and rounding each
// cell exposes a connected, varying-width caustic channel like the reference.
void ocean(Renderer &r, float left, float top, float right, float bottom) {
    constexpr float step = 650.f;
    r.set_fill(0xffCDEEF0);
    r.fill_rect(left, top, right - left, bottom - top);
    int x0 = (int)std::floor(left / step) - 2;
    int x1 = (int)std::ceil(right / step) + 2;
    int y0 = (int)std::floor(top / step) - 2;
    int y1 = (int)std::ceil(bottom / step) + 2;
    for (int gy = y0; gy <= y1; ++gy) for (int gx = x0; gx <= x1; ++gx) {
        Point site = ocean_site(gx, gy);
        std::vector<Point> poly = {
            {site.x - step * 2, site.y - step * 2},
            {site.x + step * 2, site.y - step * 2},
            {site.x + step * 2, site.y + step * 2},
            {site.x - step * 2, site.y + step * 2}
        };
        for (int dy = -2; dy <= 2 && !poly.empty(); ++dy)
            for (int dx = -2; dx <= 2 && !poly.empty(); ++dx) {
                if (dx == 0 && dy == 0) continue;
                Point other = ocean_site(gx + dx, gy + dy);
                float nx = other.x - site.x, ny = other.y - site.y;
                float threshold = (other.x * other.x + other.y * other.y
                                   - site.x * site.x - site.y * site.y) * .5f;
                std::vector<Point> clipped;
                for (size_t i = 0; i < poly.size(); ++i) {
                    Point a = poly[i], b = poly[(i + 1) % poly.size()];
                    float da = a.x * nx + a.y * ny - threshold;
                    float db = b.x * nx + b.y * ny - threshold;
                    if (da <= 0) clipped.push_back(a);
                    if ((da < 0 && db > 0) || (da > 0 && db < 0)) {
                        float t = da / (da - db);
                        clipped.push_back({a.x + (b.x - a.x) * t,
                                           a.y + (b.y - a.y) * t});
                    }
                }
                poly.swap(clipped);
            }
        if (poly.size() < 3) continue;
        float cx = 0, cy = 0;
        for (Point p : poly) { cx += p.x; cy += p.y; }
        cx /= poly.size(); cy /= poly.size();
        for (Point &p : poly) {
            p.x = cx + (p.x - cx) * .955f;
            p.y = cy + (p.y - cy) * .955f;
        }
        r.set_fill(0xffA9DEE2);
        r.begin_path();
        Point first = poly[0], last = poly.back();
        r.move_to(first.x*.77f+last.x*.23f,
                  first.y*.77f+last.y*.23f);
        for (size_t i = 0; i < poly.size(); ++i) {
            Point a = poly[i], b = poly[(i + 1) % poly.size()];
            r.qcurve_to(a.x, a.y,
                        a.x*.77f+b.x*.23f,
                        a.y*.77f+b.y*.23f);
            r.line_to(a.x*.23f+b.x*.77f,
                      a.y*.23f+b.y*.77f);
        }
        r.close_path();
        r.fill();
    }
}

void royal(Renderer &r, float left, float top, float right, float bottom) {
    constexpr float bw = 106.f, bh = 60.f;
    r.set_fill(0xffBDA783);
    r.fill_rect(left, top, right-left, bottom-top);
    int y0 = (int)std::floor(top / bh) - 1;
    int y1 = (int)std::ceil(bottom / bh) + 1;
    for (int gy = y0; gy <= y1; ++gy) {
        float offset = (gy & 1) ? bw * .5f : 0.f;
        int x0 = (int)std::floor((left - offset) / bw) - 1;
        int x1 = (int)std::ceil((right - offset) / bw) + 1;
        for (int gx = x0; gx <= x1; ++gx) {
            float x = gx * bw + offset, y = gy * bh;
            uint32_t colors[] = {0xffEFE0C2, 0xffDCC9A6, 0xffD2BD9B};
            r.set_fill(colors[hash(gx, gy, 0x19b1u) % 3]);
            r.begin_path();
            r.round_rect(x + 2.5f, y + 2.5f, bw - 5.f, bh - 5.f, 6.f);
            r.fill();
            if ((hash(gx, gy, 0x19b2u) & 7u) == 0u)
                sweep(r, x + 50.f, y + 27.f, 27.f, 3.f, 0xffBDA783, 1.8f);
        }
    }
}

// Long, connected terrain colour bands are generated in world coordinates.
// They continue across cache boundaries and give the maps the flowing graphic
// rhythm shown in the supplied terrain examples.
void flowing_bands(Renderer &r, uint8_t mid,
                   float left, float top, float right, float bottom) {
    float pitch = mid == MapID::kDesert ? 790.f : 850.f;
    float slope = mid == MapID::kPolar ? .08f : .13f;
    uint32_t color = 0;
    float opacity = .25f;
    switch (mid) {
    case MapID::kGarden: color = 0xff8FC777; break;
    case MapID::kDesert: color = 0xffE6BE79; opacity = .34f; break;
    case MapID::kPolar: color = 0xffD4E5ED; opacity = .52f; break;
    case MapID::kJungle: color = 0xff659A5D; opacity = .31f; break;
    case MapID::kAntNest:
    case MapID::kAntHell: color = 0xffA77E58; opacity = .24f; break;
    case MapID::kVolcano: color = 0xff373645; opacity = .28f; break;
    default: return;
    }
    float x0 = left - 140.f, x1 = right + 140.f;
    int k0 = (int)std::floor((top + x0*slope - 330.f) / pitch) - 2;
    int k1 = (int)std::ceil((bottom + x1*slope + 330.f) / pitch) + 2;
    auto center = [&](float x, int k) {
        return k*pitch - x*slope
            + std::sin(x*.00175f + k*1.71f)*155.f
            + std::sin(x*.0039f + k*.61f)*34.f;
    };
    r.set_fill(color);
    r.set_global_alpha(opacity);
    for (int k=k0; k<=k1; ++k) {
        float band = 245.f + 105.f*unit(k,mid,0x23abu);
        r.begin_path();
        auto half_width = [&](float x) {
            return band * (.48f + .09f*std::sin(x*.0028f+k*.79f));
        };
        r.move_to(x0, center(x0,k)-half_width(x0));
        for (float x=x0+48.f; x<=x1+48.f; x+=48.f)
            r.line_to(x,center(x,k)-half_width(x));
        for (float x=x1; x>=x0-48.f; x-=48.f)
            r.line_to(x,center(x,k)+half_width(x));
        r.close_path();
        r.fill();
    }
    r.set_global_alpha(1.f);
}

void organic(Renderer &r, uint8_t mid, float left, float top, float right, float bottom) {
    flowing_bands(r, mid, left, top, right, bottom);
    constexpr float step = 335.f;
    int x0 = (int)std::floor(left / step) - 1;
    int x1 = (int)std::ceil(right / step) + 1;
    int y0 = (int)std::floor(top / step) - 1;
    int y1 = (int)std::ceil(bottom / step) + 1;
    r.round_line_cap();
    for (int gy = y0; gy <= y1; ++gy) for (int gx = x0; gx <= x1; ++gx) {
        float x = (gx + .5f) * step + signed_unit(gx, gy, 201) * 67.f;
        float y = (gy + .5f) * step + signed_unit(gx, gy, 202) * 67.f;
        float v = unit(gx, gy, 203);
        if (mid == MapID::kGarden) {
            for (int i=0; i<3; ++i) {
                float px=x-60.f+i*47.f, py=y+35.f+signed_unit(gx+i,gy,204)*20.f;
                r.set_stroke(0xff69AD64); r.set_line_width(2.6f);
                r.begin_path(); r.move_to(px,py+8); r.qcurve_to(px-7,py-8,px-7,py);
                r.move_to(px,py+8); r.qcurve_to(px+5,py-9,px+5,py);
                r.move_to(px,py+8); r.qcurve_to(px+11,py-4,px+10,py+3); r.stroke();
            }
            if (v < .28f) {
                r.set_fill(0xffF6E9B5);
                for (int i=0;i<4;++i) { r.begin_path(); r.arc(x+std::cos(i*1.5708f)*6.f,y+std::sin(i*1.5708f)*6.f,3.3f); r.fill(); }
                r.set_fill(0xffEAC761); r.begin_path(); r.arc(x,y,2.5f); r.fill();
            } else if (v < .42f) {
                // Soft pink petal tuft
                r.set_fill(0xffF5A7BD);
                r.begin_path(); r.ellipse(x + 18.f, y - 12.f, 5.f, 3.5f); r.fill();
                r.begin_path(); r.ellipse(x + 24.f, y - 8.f, 4.f, 3.f); r.fill();
            } else if (v < .55f) {
                // Pebble cluster
                r.set_fill(0xffA8C49A);
                r.begin_path(); r.ellipse(x - 22.f, y + 18.f, 4.5f, 3.f); r.fill();
                r.begin_path(); r.ellipse(x - 16.f, y + 22.f, 3.2f, 2.4f); r.fill();
            }
            if (unit(gx, gy, 0x30a1u) < .35f) {
                // Fallen leaf litter (deterministic, no collision)
                leaf(r, x + 40.f, y - 28.f, 14.f, 5.5f, 0xff8FC777, 0xffB2CE85);
            }
        } else if (mid == MapID::kDesert) {
            sweep(r,x,y,155.f,18.f,0xffF9E7BA,5.f);
            sweep(r,x+12.f,y+22.f,116.f,13.f,0xffD5AD72,2.5f);
            if(v>.55f) sweep(r,x-30.f,y+42.f,65.f,6.f,0xffD5AD72,2.f);
        } else if (mid == MapID::kPolar) {
            // Soft blue frost only — pure white streaks strobe on pale snow.
            sweep(r,x,y+20.f,150.f,13.f,0xffC1DBE7,3.f);
            sweep(r,x-18.f,y+33.f,86.f,7.f,0xffD4E5ED,2.f);
        } else if (mid == MapID::kJungle) {
            leaf(r,x-30.f,y+13.f,29.f,11.f,0xff659A5D,0xffB2CE85);
            leaf(r,x-8.f,y+5.f,22.f,9.f,0xff5D9D64,0xffB2CE85);
            if(v>.6f) leaf(r,x+47.f,y-18.f,20.f,8.f,0xffA88759,0xffD5C37C);
        } else if ((mid == MapID::kAntHell || mid == MapID::kAntNest)) {
            sweep(r,x,y,142.f,13.f,0xffE0BF92,3.3f);
            sweep(r,x+9.f,y+25.f,105.f,9.f,0xff8A5E43,3.f);
            r.set_stroke(0xffD9C09B); r.set_line_width(2.f);
            r.begin_path(); r.move_to(x-49,y+48); r.qcurve_to(x+18,y+45,x-12,y+32);
            r.move_to(x-9,y+34); r.line_to(x+1,y+45); r.stroke();
        } else if (mid == MapID::kVolcano) {
            r.set_global_alpha(.29f);
            r.set_fill((hash(gx,gy,205)&1u)?0xff747082:0xff373645);
            r.begin_path(); r.move_to(x-86,y-35); r.line_to(x-42,y-58);
            r.qcurve_to(x+55,y-48,x+32,y-52); r.line_to(x+89,y-5);
            r.line_to(x+52,y+44); r.qcurve_to(x-62,y+39,x-15,y+55);
            r.close_path(); r.fill(); r.set_global_alpha(1.f);
            r.set_stroke(0xff373645); r.set_line_width(3.f);
            r.begin_path(); r.move_to(x-36,y-9); r.line_to(x+8,y+3);
            r.line_to(x+25,y-11); r.stroke();
            if(v>.84f) {
                r.set_stroke(0xffA74739); r.set_line_width(4.f);
                r.begin_path(); r.move_to(x+10,y+3); r.line_to(x+24,y-10); r.stroke();
            }
        }
    }
}

// Sparse cached landmarks (design Phase 1 / empty-map POI). Drawn inside chunk
// cache — no per-frame random, no network, no collision. Step ~1400 keeps cost low.
void sparse_landmarks(Renderer &r, uint8_t mid,
                      float left, float top, float right, float bottom) {
    constexpr float step = 1400.f;
    int x0 = (int) std::floor(left / step) - 1;
    int x1 = (int) std::ceil(right / step) + 1;
    int y0 = (int) std::floor(top / step) - 1;
    int y1 = (int) std::ceil(bottom / step) + 1;
    // Skip hub plaza so spawn stays clear
    float hx = MAP_SPAWN_X, hy = MAP_SPAWN_Y;
    for (int gy = y0; gy <= y1; ++gy) for (int gx = x0; gx <= x1; ++gx) {
        // ~1/5 cells get a landmark (deterministic)
        if ((hash(gx, gy, 0x51deu ^ mid) % 5u) != 0u) continue;
        float x = (gx + .5f) * step + signed_unit(gx, gy, 0x51dfu) * 180.f;
        float y = (gy + .5f) * step + signed_unit(gx, gy, 0x51e0u) * 180.f;
        float dhx = x - hx, dhy = y - hy;
        if (dhx * dhx + dhy * dhy < 1600.f * 1600.f) continue;
        if (x < left - 80.f || x > right + 80.f || y < top - 80.f || y > bottom + 80.f)
            continue;
        float s = .85f + unit(gx, gy, 0x51e1u) * .35f;
        switch (mid) {
        case MapID::kGarden:
            // Flower ring cairn
            r.set_fill(0xff6DAD58);
            for (int i = 0; i < 5; ++i) {
                float a = i * 1.2566f;
                r.begin_path();
                r.ellipse(x + std::cos(a) * 18.f * s, y + std::sin(a) * 14.f * s,
                          10.f * s, 7.f * s);
                r.fill();
            }
            r.set_fill(0xffF5A7BD);
            r.begin_path(); r.arc(x, y, 6.f * s); r.fill();
            break;
        case MapID::kDesert:
            r.set_fill(0xffC89B56);
            r.begin_path();
            r.move_to(x - 22 * s, y + 10 * s);
            r.line_to(x - 6 * s, y - 28 * s);
            r.line_to(x + 18 * s, y - 8 * s);
            r.line_to(x + 10 * s, y + 16 * s);
            r.close_path(); r.fill();
            r.set_fill(0xff6DAD68);
            r.begin_path(); r.ellipse(x + 28 * s, y + 6 * s, 7 * s, 16 * s); r.fill();
            break;
        case MapID::kOcean:
            r.set_fill(0xff4CC9F0);
            r.begin_path(); r.arc(x, y, 14.f * s); r.fill();
            r.set_stroke(0xffFFE0EA); r.set_line_width(2.5f);
            r.begin_path(); r.arc(x, y, 20.f * s); r.stroke();
            break;
        case MapID::kPolar:
            r.set_fill(0xffA5DDEA);
            r.begin_path();
            r.move_to(x, y - 26 * s); r.line_to(x + 16 * s, y + 10 * s);
            r.line_to(x - 16 * s, y + 10 * s); r.close_path(); r.fill();
            break;
        case MapID::kJungle:
            leaf(r, x, y, 36.f * s, 14.f * s, 0xff40916C, 0xff95D5B2);
            leaf(r, x + 14 * s, y - 8 * s, 28.f * s, 11.f * s, 0xff2D6A4F, 0xff95D5B2);
            break;
        case MapID::kAntNest:
    case MapID::kAntHell:
            r.set_fill(0xff704A35);
            r.begin_path(); r.ellipse(x, y, 22 * s, 14 * s); r.fill();
            r.set_fill(0xffC9A477);
            r.begin_path(); r.arc(x - 4 * s, y - 2 * s, 6 * s); r.fill();
            break;
        case MapID::kVolcano:
            r.set_fill(0xff1A1A2E);
            r.begin_path();
            r.move_to(x - 20 * s, y + 12 * s); r.line_to(x, y - 24 * s);
            r.line_to(x + 22 * s, y + 12 * s); r.close_path(); r.fill();
            r.set_fill(0xffE76F51);
            r.begin_path(); r.arc(x, y + 2 * s, 5 * s); r.fill();
            break;
        case MapID::kRoyalCapital:
            r.set_fill(0xff8B8D98);
            r.begin_path(); r.round_rect(x - 16 * s, y - 10 * s, 32 * s, 20 * s, 3.f); r.fill();
            r.set_fill(0xffC94D4C);
            r.begin_path(); r.arc(x, y - 18 * s, 5 * s); r.fill();
            break;
        default: break;
        }
    }
}

// L0 fine litter — deterministic, no collision / network. Hub plaza kept clear.
void map_fine_litter(Renderer &r, uint8_t mid,
                     float left, float top, float right, float bottom) {
    constexpr float fine = 210.f;
    int fx0 = (int) std::floor(left / fine) - 1;
    int fx1 = (int) std::ceil(right / fine) + 1;
    int fy0 = (int) std::floor(top / fine) - 1;
    int fy1 = (int) std::ceil(bottom / fine) + 1;
    float hx = MAP_SPAWN_X, hy = MAP_SPAWN_Y;
    uint32_t salt = 0x40b2u ^ ((uint32_t) mid * 0x9e3779b9u);
    for (int gy = fy0; gy <= fy1; ++gy) for (int gx = fx0; gx <= fx1; ++gx) {
        if ((hash(gx, gy, salt) % 7u) != 0u) continue;
        float x = (gx + .5f) * fine + signed_unit(gx, gy, salt + 1) * 40.f;
        float y = (gy + .5f) * fine + signed_unit(gx, gy, salt + 2) * 40.f;
        float dhx = x - hx, dhy = y - hy;
        if (dhx * dhx + dhy * dhy < 900.f * 900.f) continue;
        uint32_t kind = hash(gx, gy, salt + 3) % 3u;
        switch (mid) {
        case MapID::kGarden:
            if (kind == 0) {
                r.set_fill(0xffC9D9A8);
                r.begin_path(); r.ellipse(x, y, 3.5f, 2.4f); r.fill();
            } else if (kind == 1) {
                r.set_stroke(0xff6DAD58); r.set_line_width(1.8f);
                r.begin_path();
                r.move_to(x, y + 4.f);
                r.qcurve_to(x - 4.f, y - 2.f, x - 3.f, y + 1.f);
                r.move_to(x, y + 4.f);
                r.qcurve_to(x + 3.f, y - 3.f, x + 2.5f, y + 1.f);
                r.stroke();
            } else {
                r.set_fill(0xffE8B4C8);
                r.begin_path(); r.arc(x, y, 2.2f); r.fill();
            }
            break;
        case MapID::kDesert:
            if (kind == 0) {
                r.set_fill(0xffD5AD72);
                r.begin_path(); r.ellipse(x, y, 4.f, 2.2f); r.fill();
            } else {
                sweep(r, x, y, 36.f, 4.f, 0xffF9E7BA, 1.6f);
            }
            break;
        case MapID::kOcean:
            r.set_fill(kind ? 0xff7EC8E3 : 0xffFFE0EA);
            r.begin_path(); r.arc(x, y, 2.4f + kind); r.fill();
            break;
        case MapID::kPolar:
            r.set_fill(0xffD4E5ED);
            r.begin_path(); r.ellipse(x, y, 3.f, 2.f); r.fill();
            break;
        case MapID::kJungle:
            if (kind == 0)
                leaf(r, x, y, 10.f, 4.f, 0xff40916C, 0xff95D5B2);
            else {
                r.set_fill(0xff2D6A4F);
                r.begin_path(); r.arc(x, y, 2.f); r.fill();
            }
            break;
        case MapID::kAntNest:
    case MapID::kAntHell:
            r.set_fill(kind ? 0xff8A5E43 : 0xffD9C09B);
            r.begin_path(); r.ellipse(x, y, 3.2f, 2.f); r.fill();
            break;
        case MapID::kVolcano:
            r.set_fill(kind ? 0xffE76F51 : 0xff373645);
            r.begin_path(); r.arc(x, y, 2.5f); r.fill();
            break;
        case MapID::kRoyalCapital:
            r.set_fill(kind ? 0xff8B8D98 : 0xffC94D4C);
            r.begin_path(); r.round_rect(x - 2.5f, y - 2.f, 5.f, 4.f, 1.f); r.fill();
            break;
        default: break;
        }
    }
}

} // namespace

void TerrainPattern::draw(Renderer &r, uint8_t map_id,
                          float left, float top, float right, float bottom) {
    if (map_id == MapID::kOcean) ocean(r,left,top,right,bottom);
    else if (map_id == MapID::kRoyalCapital) royal(r,left,top,right,bottom);
    else organic(r,map_id,left,top,right,bottom);
    sparse_landmarks(r, map_id, left, top, right, bottom);
    map_fine_litter(r, map_id, left, top, right, bottom);
    r.set_global_alpha(1.f);
}

void TerrainPattern::draw_wall(Renderer &r, TerrainFeature const &f, uint8_t map_id) {
    if (f.shape != TerrainShape::kAABB && f.shape != TerrainShape::kCircle
        && f.shape != TerrainShape::kEllipse) return;
    TerrainArt::ThemeMaterial const &tm = TerrainArt::theme(map_id);
    float cx = f.shape == TerrainShape::kAABB ? (f.left + f.right)*.5f : f.left;
    float cy = f.shape == TerrainShape::kAABB ? (f.top + f.bottom)*.5f : f.top;
    float rx = f.shape == TerrainShape::kAABB ? (f.right-f.left)*.5f : f.right;
    float ry = f.shape == TerrainShape::kAABB ? (f.bottom-f.top)*.5f
             : f.shape == TerrainShape::kCircle ? f.right : f.bottom;
    float area = std::max(1.f, rx*ry*4.f);
    uint32_t n = (uint32_t)std::clamp(area/115000.f, 7.f, 32.f);
    uint32_t seed = f.seed ? f.seed : 1u;
    for (uint32_t i=0; i<n; ++i) {
        float x=cx + (unit((int)i,(int)seed,0x75a1u)*2.f-1.f)*rx*.86f;
        float y=cy + (unit((int)i,(int)seed,0x75a2u)*2.f-1.f)*ry*.86f;
        if (!Map::point_in_feature(f,x,y)) continue;
        float s=.75f+unit((int)i,(int)seed,0x75a3u)*.6f;
        switch(map_id) {
        case MapID::kGarden:
            leaf(r,x,y,43.f*s,17.f*s,0xff4E914C,0xff85C96D);
            if ((i&3u)==0u) leaf(r,x+16.f,y-8.f,28.f*s,10.f*s,
                                   0xff5A9E53,0xff85C96D);
            break;
        case MapID::kJungle:
            leaf(r,x,y,81.f*s,27.f*s,0xff315F42,0xff5D9D64);
            if ((i&1u)==0u) leaf(r,x+18.f,y+10.f,58.f*s,20.f*s,
                                   0xff214B36,0xff75C9A6);
            break;
        case MapID::kDesert:
            ribbon(r,x,y,118.f*s,46.f*s,0xffB77B4C);
            sweep(r,x,y-12.f,100.f*s,9.f,0xffE8AD69,3.f);
            sweep(r,x+5.f,y+8.f,76.f*s,6.f,0xff98643E,3.f);
            break;
        case MapID::kOcean:
            ribbon(r,x,y,66.f*s,38.f*s,0xff327F98);
            sweep(r,x,y-5.f,48.f*s,8.f,0xff83C8C8,3.f);
            if ((i%5u)==0u) {
                r.set_stroke((i&1u)?0xffF2A37E:0xffB884C8);
                r.set_line_width(6.f); r.round_line_cap(); r.begin_path();
                r.move_to(x,y+7); r.qcurve_to(x-11,y-20,x-9,y-7);
                r.move_to(x,y); r.qcurve_to(x+13,y-17,x+9,y-5); r.stroke();
            }
            break;
        case MapID::kPolar:
            r.set_fill(0xff4B829F); r.begin_path();
            r.move_to(x-31*s,y+22*s); r.line_to(x-8*s,y-29*s);
            r.line_to(x+17*s,y-19*s); r.line_to(x+37*s,y+14*s);
            r.qcurve_to(x-31*s,y+22*s,x+2*s,y+35*s); r.fill();
            sweep(r,x,y-19*s,42*s,3.f,0xffA5DDEA,3.f);
            break;
        case MapID::kAntNest:
    case MapID::kAntHell:
            ribbon(r,x,y,110.f*s,44.f*s,0xff704A35);
            sweep(r,x,y-10.f,91.f*s,7.f,0xffB4865E,3.f);
            sweep(r,x+7.f,y+11.f,83.f*s,6.f,0xff8A5E43,5.f);
            if ((i&3u)==0u) {
                r.set_stroke(0xffD9C09B); r.set_line_width(2.5f);
                r.begin_path(); r.move_to(x-22,y-16); r.qcurve_to(x+20,y+17,x,y+8);
                r.move_to(x+1,y+7); r.line_to(x+11,y+22); r.stroke();
            }
            break;
        case MapID::kVolcano:
            r.set_fill(0xff373645); r.begin_path();
            r.move_to(x-43*s,y+12*s); r.line_to(x-24*s,y-19*s);
            r.line_to(x+19*s,y-24*s); r.line_to(x+45*s,y+8*s);
            r.line_to(x+6*s,y+26*s); r.close_path(); r.fill();
            sweep(r,x,y-9.f,49.f*s,4.f,0xff747082,2.5f);
            if ((i&3u)==0u) sweep(r,x+7.f,y+15.f,27.f*s,4.f,0xffA74739,3.f);
            break;
        case MapID::kRoyalCapital:
            r.set_fill(0xff8B8D98); r.begin_path();
            r.round_rect(x-38*s,y-19*s,76*s,38*s,5.f); r.fill();
            r.set_stroke(0xff646878); r.set_line_width(3.f); r.begin_path();
            r.round_rect(x-38*s,y-19*s,76*s,38*s,5.f); r.stroke();
            if ((i&3u)==0u) sweep(r,x,y-7.f,32.f*s,3.f,0xffB5B4BB,2.f);
            break;
        }
    }
}
