#include <Client/Assets/ThemeArt.hh>

#include <Helpers/Math.hh>

#include <cmath>
#include <string_view>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace ThemeArt {
namespace {

inline float breath(float anim, uint32_t seed) {
    return sinf(anim * 0.65f + (float) (seed % 97) * 0.07f);
}
inline float pulse(float anim, uint32_t seed) {
    return 0.5f + 0.5f * sinf(anim * 0.8f + (float) (seed % 53) * 0.11f);
}

inline void shadow(Renderer &ctx, float y = 39.f, float rx = 29.f, uint32_t c = 0x22000000) {
    ctx.set_global_alpha(((c >> 24) & 255) / 255.f);
    ctx.set_fill(0xff000000 | (c & 0xffffff));
    ctx.begin_path();
    ctx.ellipse(0, y, rx, 8);
    ctx.fill();
    ctx.set_global_alpha(1.f);
}

inline void eyes(Renderer &ctx, float x, float y, float r, uint32_t c = 0xff1a1a1a) {
    ctx.set_fill(c);
    ctx.begin_path();
    ctx.arc(-x, y, r);
    ctx.arc(x, y, r);
    ctx.fill();
}

inline uint8_t name_has(std::string_view n, char const *needle) {
    if (n.empty() || needle == nullptr) return 0;
    return n.find(needle) != std::string_view::npos;
}

// ---- Garden NPCs ----
// A01 Leaf Korok: keep current mashup silhouette (per design note).
void garden_main(Renderer &ctx, float anim, uint32_t seed) {
    (void) anim; (void) seed;
    ctx.set_fill(0xff4A7A3A);
    ctx.begin_path();
    ctx.ellipse(0, 8, 18, 14);
    ctx.fill();
    ctx.set_fill(0xffF0D878);
    ctx.begin_path();
    ctx.arc(0, -6, 14);
    ctx.fill();
    ctx.set_fill(0xff6B6B6B);
    ctx.begin_path();
    ctx.ellipse(0, -14, 16, 8);
    ctx.fill();
    eyes(ctx, 5, -8, 2.2f);
    ctx.set_fill(0xffC9A227);
    ctx.begin_path();
    ctx.arc(14, 10, 7);
    ctx.fill();
    ctx.set_stroke(0xff5A3D22);
    ctx.set_line_width(2);
    ctx.begin_path();
    ctx.move_to(-10, -18);
    ctx.qcurve_to(-22, -28, -8, -32);
    ctx.qcurve_to(0, -24, -4, -16);
    ctx.close_path();
    ctx.set_fill(0xff6FAF4A);
    ctx.fill();
    ctx.stroke();
}

void garden_wayside(Renderer &ctx, float anim, uint32_t seed) {
    float b = breath(anim, seed);
    shadow(ctx, 36, 22, 0x28000000);
    ctx.set_fill(0xffA6D78A);
    ctx.begin_path();
    ctx.move_to(-6, -4);
    ctx.qcurve_to(-36, -22 + b * 2, -32, -38);
    ctx.qcurve_to(-10, -28, -4, -12);
    ctx.close_path();
    ctx.fill();
    ctx.set_fill(0xff77BC65);
    ctx.begin_path();
    ctx.move_to(4, -2);
    ctx.qcurve_to(22, -30 + b * 2, 18, -40);
    ctx.qcurve_to(6, -26, 2, -10);
    ctx.close_path();
    ctx.fill();
    ctx.set_fill(0xffC9A16D);
    ctx.set_stroke(0xff765D42);
    ctx.set_line_width(2.5f);
    ctx.begin_path();
    ctx.ellipse(0, 8, 21, 25);
    ctx.fill();
    ctx.stroke();
    eyes(ctx, 7, 5, 2.8f);
    // Sprout
    ctx.set_stroke(0xff77BC65);
    ctx.set_line_width(3);
    ctx.begin_path();
    ctx.move_to(2, -17);
    ctx.qcurve_to(6, -34, 11, -48);
    ctx.stroke();
    ctx.set_fill(0xffA6D78A);
    ctx.begin_path();
    ctx.ellipse(13, -47, 9, 5);
    ctx.fill();
    // Guide leaf
    ctx.set_fill(0xff6FAF4A);
    ctx.begin_path();
    ctx.ellipse(24, 12, 8, 4);
    ctx.fill();
}

// ---- Desert ----
void desert_main(Renderer &ctx, float anim, uint32_t seed) {
    float b = breath(anim, seed) * 0.03f;
    shadow(ctx, 40, 30, 0x2a000000);
    // Pack
    ctx.set_fill(0xffA97449);
    ctx.begin_path();
    ctx.ellipse(29, 7, 20, 28);
    ctx.fill();
    ctx.set_stroke(0xff6B4A30);
    ctx.set_line_width(2.5f);
    ctx.begin_path();
    ctx.move_to(17, -12); ctx.line_to(31, -18); ctx.line_to(41, -10);
    ctx.stroke();
    // Cape
    ctx.set_fill(0xffC89256);
    ctx.begin_path();
    ctx.move_to(-24, -1); ctx.line_to(24, -1); ctx.line_to(30, 39); ctx.line_to(-30, 39);
    ctx.close_path();
    ctx.fill();
    // Head + turban
    ctx.set_fill(0xff79583D);
    ctx.begin_path();
    ctx.ellipse(0, -10, 28, 27);
    ctx.fill();
    ctx.set_fill(0xffD8A45D);
    ctx.begin_path();
    ctx.ellipse(0, -18, 30, 18);
    ctx.fill();
    ctx.set_fill(0xffEED49A);
    ctx.begin_path();
    ctx.ellipse(-3 + b * 20, -35, 10, 7);
    ctx.fill();
    eyes(ctx, 9, -9, 3.f);
    // Venom barb ornament
    ctx.set_stroke(0xffC9A227);
    ctx.set_line_width(3);
    ctx.begin_path();
    ctx.move_to(23, 15);
    ctx.qcurve_to(39, 11, 35, -2);
    ctx.stroke();
    ctx.set_fill(0xffC9A227);
    ctx.begin_path();
    ctx.move_to(35, -2); ctx.line_to(40, -8); ctx.line_to(42, 0);
    ctx.close_path();
    ctx.fill();
}

void desert_wayside(Renderer &ctx, float anim, uint32_t seed) {
    float b = breath(anim, seed);
    shadow(ctx, 38, 22, 0x28000000);
    ctx.set_fill(0xffC89256);
    ctx.begin_path();
    ctx.move_to(-14, -16); ctx.line_to(15, -16); ctx.line_to(23, 37); ctx.line_to(-20, 37);
    ctx.close_path();
    ctx.fill();
    ctx.set_fill(0xff79583D);
    ctx.begin_path();
    ctx.ellipse(0, -13, 20, 22);
    ctx.fill();
    ctx.set_fill(0xffD8A45D);
    ctx.begin_path();
    ctx.ellipse(0, -18, 22, 10);
    ctx.fill();
    // Face band
    ctx.set_fill(0xffA97449);
    ctx.begin_path();
    ctx.ellipse(0, -12, 18, 6);
    ctx.fill();
    eyes(ctx, 7, -10, 2.5f);
    ctx.set_fill(0xffA97449);
    ctx.begin_path();
    ctx.ellipse(-22, 15 + b, 11, 14);
    ctx.fill();
}

// ---- Ocean ----
void ocean_main(Renderer &ctx, float anim, uint32_t seed) {
    float b = breath(anim, seed) * 2.f; // P3: keep float ≤ ±2
    shadow(ctx, 39, 29, 0x30000000);
    ctx.translate(0, b);
    // Shell backpack
    ctx.set_fill(0xffF5D58B);
    ctx.set_stroke(0xffB89550);
    ctx.set_line_width(2);
    ctx.begin_path();
    ctx.ellipse(19, 18, 17, 21);
    ctx.fill();
    ctx.stroke();
    for (int i = 0; i < 4; ++i) {
        float a = -0.6f + i * 0.35f;
        ctx.begin_path();
        ctx.move_to(19, 18);
        ctx.line_to(19 + cosf(a) * 14, 18 + sinf(a) * 16);
        ctx.stroke();
    }
    // Body
    ctx.set_fill(0xff59B7CC);
    ctx.begin_path();
    ctx.ellipse(0, 16, 27, 26);
    ctx.fill();
    // Helmet
    ctx.set_fill(0xff7CCDDC);
    ctx.set_stroke(0xff3E7185);
    ctx.set_line_width(4);
    ctx.begin_path();
    ctx.arc(0, -12, 32);
    ctx.fill();
    ctx.stroke();
    ctx.set_fill(0xffC7F0ED);
    ctx.begin_path();
    ctx.ellipse(0, -10, 23, 21);
    ctx.fill();
    ctx.set_fill(0xff3E7185);
    ctx.begin_path();
    ctx.arc(-31, -10, 8);
    ctx.arc(31, -10, 8);
    ctx.fill();
    eyes(ctx, 9, -10, 3.2f);
    // Snorkel
    ctx.set_fill(0xff3E7185);
    ctx.begin_path();
    ctx.round_rect(-5, -51, 10, 14, 4);
    ctx.fill();
    ctx.set_fill(0xa0ffffff);
    ctx.begin_path();
    ctx.arc(0, -56, 5);
    ctx.fill();
    ctx.translate(0, -b);
}

void ocean_wayside(Renderer &ctx, float anim, uint32_t seed) {
    float b = breath(anim, seed);
    shadow(ctx, 38, 26, 0x28000000);
    ctx.set_fill(0xff59B7CC);
    ctx.begin_path();
    ctx.ellipse(0, 7, 24, 28);
    ctx.fill();
    // Shell hat (fan arcs)
    ctx.set_fill(0xffF5D58B);
    ctx.begin_path();
    ctx.ellipse(0, -22, 29, 16);
    ctx.fill();
    ctx.set_stroke(0xffB89550);
    ctx.set_line_width(2);
    for (int i = -2; i <= 2; ++i) {
        ctx.begin_path();
        ctx.move_to(0, -18);
        ctx.qcurve_to((float) i * 10, -40, (float) i * 18, -28);
        ctx.stroke();
    }
    // Kelp shawl
    ctx.set_stroke(0xff4FA06A);
    ctx.set_line_width(3.5f);
    for (int i = 0; i < 3; ++i) {
        float x0 = -18.f - i * 3;
        ctx.begin_path();
        ctx.move_to(x0, -2);
        ctx.qcurve_to(x0 - 10, 12 + b * 3, x0 - 6, 28 + i * 3);
        ctx.stroke();
    }
    ctx.set_fill(0xffF5D58B);
    ctx.begin_path();
    ctx.ellipse(24, 14, 9, 7);
    ctx.fill();
    eyes(ctx, 8, 2, 2.8f);
    ctx.set_stroke(0xff1a1a1a);
    ctx.set_line_width(2);
    ctx.begin_path();
    ctx.move_to(-5, 10); ctx.line_to(5, 10);
    ctx.stroke();
}

// ---- Polar ----
void polar_main(Renderer &ctx, float anim, uint32_t seed) {
    float b = breath(anim, seed) * 2.f;
    shadow(ctx, 40, 28, 0x1a000000);
    ctx.set_global_alpha(0.62f);
    ctx.set_fill(0xffBDE8F4);
    ctx.begin_path();
    ctx.ellipse(0, 10 + b, 25, 34);
    ctx.fill();
    ctx.set_global_alpha(1.f);
    // Shoulder ice
    ctx.set_fill(0xffE6F6FA);
    ctx.set_stroke(0xff557D99);
    ctx.set_line_width(2.5f);
    ctx.begin_path();
    ctx.move_to(-16, -19); ctx.line_to(-40, -31); ctx.line_to(-49, -9); ctx.line_to(-27, 7);
    ctx.close_path();
    ctx.fill(); ctx.stroke();
    ctx.begin_path();
    ctx.move_to(16, -16); ctx.line_to(42, -28); ctx.line_to(48, -4); ctx.line_to(28, 10);
    ctx.close_path();
    ctx.fill(); ctx.stroke();
    // Breastplate
    ctx.set_fill(0xff91CFE8);
    ctx.begin_path();
    ctx.move_to(-23, -9); ctx.line_to(23, -9); ctx.line_to(18, 29); ctx.line_to(0, 37); ctx.line_to(-18, 29);
    ctx.close_path();
    ctx.fill();
    ctx.stroke();
    // Helm arc
    ctx.set_stroke(0xffE6F6FA);
    ctx.set_line_width(6);
    ctx.begin_path();
    ctx.arc(0, -15, 25);
    ctx.stroke();
    ctx.set_fill(0xff7E93D7);
    ctx.begin_path();
    ctx.ellipse(-8, -14, 3, 5);
    ctx.ellipse(8, -14, 3, 5);
    ctx.fill();
    // Broken runes
    ctx.set_stroke(0xffE6F6FA);
    ctx.set_line_width(2);
    ctx.begin_path();
    ctx.move_to(-8, 8); ctx.line_to(4, 14);
    ctx.move_to(-2, 18); ctx.line_to(10, 12);
    ctx.move_to(-6, 22); ctx.line_to(6, 26);
    ctx.stroke();
}

void polar_wayside(Renderer &ctx, float anim, uint32_t seed) {
    float b = breath(anim, seed) * 2.f;
    ctx.set_global_alpha(0.5f);
    ctx.set_fill(0xffBDE8F4);
    ctx.begin_path();
    ctx.ellipse(0, 8 + b, 19, 31);
    ctx.fill();
    ctx.set_global_alpha(1.f);
    ctx.set_fill(0xffE6F6FA);
    ctx.set_stroke(0xff557D99);
    ctx.set_line_width(2);
    ctx.begin_path();
    ctx.move_to(-12, -14); ctx.line_to(-32, -22); ctx.line_to(-28, 2);
    ctx.close_path();
    ctx.fill(); ctx.stroke();
    // Floating shards
    ctx.set_global_alpha(0.35f);
    ctx.begin_path();
    ctx.move_to(18, -8); ctx.line_to(30, -18); ctx.line_to(26, 0);
    ctx.close_path();
    ctx.fill();
    ctx.set_global_alpha(1.f);
    ctx.set_stroke(0xffE6F6FA);
    ctx.set_line_width(4);
    ctx.begin_path();
    ctx.arc(0, -18, 16);
    ctx.stroke();
    eyes(ctx, 6, -16, 2.f, 0xff7E93D7);
}

// ---- Jungle ----
void jungle_main(Renderer &ctx, float anim, uint32_t seed) {
    float b = breath(anim, seed);
    shadow(ctx, 41, 32, 0x28000000);
    ctx.set_stroke(0xff6B4E2E);
    ctx.set_line_width(10);
    ctx.round_line_cap();
    ctx.begin_path();
    ctx.move_to(-19, 6); ctx.qcurve_to(-38, 0 + b, -42, -13);
    ctx.move_to(19, 6); ctx.qcurve_to(38, 0 - b, 42, -13);
    ctx.stroke();
    ctx.set_fill(0xff99734D);
    ctx.set_stroke(0xff6B4E2E);
    ctx.set_line_width(3);
    ctx.begin_path();
    ctx.ellipse(0, 12, 27, 33);
    ctx.fill(); ctx.stroke();
    // Canopy triple
    ctx.set_fill(0xff4A8A3A);
    ctx.begin_path();
    ctx.ellipse(-19, -29, 21, 16);
    ctx.ellipse(17, -31, 22, 18);
    ctx.ellipse(0, -40, 22, 17);
    ctx.fill();
    // Vines
    ctx.set_stroke(0xff3A6A2A);
    ctx.set_line_width(3);
    ctx.begin_path();
    ctx.move_to(-10, -28); ctx.qcurve_to(-16, -10, -12, -5);
    ctx.move_to(10, -30); ctx.qcurve_to(18, -12, 14, -5);
    ctx.stroke();
    eyes(ctx, 8, 4, 3.f, 0xff1a3a12);
    ctx.set_stroke(0xffC9A16D);
    ctx.set_line_width(2);
    ctx.begin_path();
    ctx.arc(0, 14, 10);
    ctx.arc(0, 14, 15);
    ctx.stroke();
}

void jungle_wayside(Renderer &ctx, float anim, uint32_t seed) {
    float b = breath(anim, seed);
    shadow(ctx, 38, 20, 0x24000000);
    ctx.set_fill(0xffB8956A);
    ctx.begin_path();
    ctx.ellipse(0, 13, 17, 29);
    ctx.fill();
    ctx.set_fill(0xff6FAF4A);
    ctx.begin_path();
    ctx.ellipse(-14, -22, 14, 10);
    ctx.ellipse(12, -26, 12, 11);
    ctx.fill();
    // Staff
    ctx.set_stroke(0xff6B4E2E);
    ctx.set_line_width(4);
    ctx.begin_path();
    ctx.move_to(20, 4); ctx.line_to(29, 36);
    ctx.stroke();
    ctx.set_fill(0xff6FAF4A);
    ctx.begin_path();
    ctx.ellipse(30, 34 + b, 7, 4);
    ctx.fill();
    eyes(ctx, 6, 2, 2.5f, 0xff1a3a12);
}

// ---- Ant Hell ----
void ant_main(Renderer &ctx, float anim, uint32_t seed) {
    float b = breath(anim, seed);
    shadow(ctx, 39, 30, 0x28000000);
    ctx.set_fill(0xff8E6244);
    ctx.begin_path();
    ctx.ellipse(0, 23, 24, 21);
    ctx.fill();
    ctx.set_fill(0xff9A6A48);
    ctx.begin_path();
    ctx.ellipse(0, 4, 18, 19);
    ctx.fill();
    ctx.set_fill(0xffAD7852);
    ctx.set_stroke(0xff5A3D22);
    ctx.set_line_width(2.5f);
    ctx.begin_path();
    ctx.ellipse(0, -24, 25, 23);
    ctx.fill(); ctx.stroke();
    // Antennae
    ctx.set_stroke(0xff5A3D22);
    ctx.set_line_width(5);
    ctx.round_line_cap();
    ctx.begin_path();
    ctx.move_to(-11, -40); ctx.qcurve_to(-28, -52 + b * 4, -34, -44);
    ctx.move_to(11, -40); ctx.qcurve_to(28, -52 - b * 4, 34, -44);
    ctx.stroke();
    eyes(ctx, 10, -22, 3.f);
    ctx.set_fill(0xff8E6244);
    ctx.begin_path();
    ctx.ellipse(0, 4, 11, 9);
    ctx.fill();
    // Flag
    ctx.set_stroke(0xff5A3D22);
    ctx.set_line_width(3);
    ctx.begin_path();
    ctx.move_to(22, -2); ctx.line_to(34, -38);
    ctx.stroke();
    ctx.set_fill(0xffC45A3A);
    ctx.begin_path();
    ctx.move_to(34, -38); ctx.line_to(48, -32); ctx.line_to(34, -24);
    ctx.close_path();
    ctx.fill();
}

void ant_wayside(Renderer &ctx, float anim, uint32_t seed) {
    float b = breath(anim, seed);
    shadow(ctx, 38, 22, 0x24000000);
    ctx.set_fill(0xff6E4A32);
    ctx.begin_path();
    ctx.ellipse(0, 27, 19, 17);
    ctx.fill();
    ctx.set_fill(0xff8E6244);
    ctx.begin_path();
    ctx.ellipse(0, 4, 13, 22);
    ctx.fill();
    ctx.set_fill(0xffAD7852);
    ctx.begin_path();
    ctx.ellipse(0, -25, 19, 20);
    ctx.fill();
    ctx.set_stroke(0xff5A3D22);
    ctx.set_line_width(4);
    ctx.begin_path();
    ctx.move_to(-8, -40); ctx.qcurve_to(-22, -36 + b * 6, -30, -20);
    ctx.move_to(8, -40); ctx.qcurve_to(22, -36 - b * 6, 30, -20);
    ctx.stroke();
    // Short legs
    ctx.set_line_width(3);
    for (int s = -1; s <= 1; s += 2) {
        ctx.begin_path();
        ctx.move_to((float) s * 10, 8); ctx.line_to((float) s * 22, 18);
        ctx.move_to((float) s * 8, 18); ctx.line_to((float) s * 20, 28);
        ctx.stroke();
    }
    eyes(ctx, 7, -24, 2.8f, 0xff1a1a1a);
    ctx.set_fill(0xffC45A3A);
    ctx.begin_path();
    ctx.ellipse(0, -2, 6, 5);
    ctx.fill();
}

// ---- Volcano ----
void volcano_main(Renderer &ctx, float anim, uint32_t seed) {
    float p = pulse(anim, seed);
    shadow(ctx, 42, 30, 0x30000000);
    // Chains
    ctx.set_stroke(0xff846B60);
    ctx.set_line_width(3);
    for (int s = -1; s <= 1; s += 2) {
        for (int i = 0; i < 3; ++i) {
            ctx.begin_path();
            ctx.ellipse((float) s * (28 + i), -10.f + i * 12.f, 5, 7);
            ctx.stroke();
        }
    }
    // Body poly
    ctx.set_fill(0xff45424A);
    ctx.set_stroke(0xff2A282E);
    ctx.set_line_width(3);
    ctx.begin_path();
    ctx.move_to(-25, -29); ctx.line_to(16, -36); ctx.line_to(31, -10);
    ctx.line_to(26, 29); ctx.line_to(5, 39); ctx.line_to(-27, 26); ctx.line_to(-32, -7);
    ctx.close_path();
    ctx.fill(); ctx.stroke();
    // Rag shoulders
    ctx.set_fill(0xff6A656E);
    ctx.begin_path();
    ctx.ellipse(-18, -18, 12, 8);
    ctx.ellipse(14, -20, 11, 7);
    ctx.fill();
    // Magma crack
    ctx.set_stroke(0xff8B2A18);
    ctx.set_line_width(8);
    ctx.begin_path();
    ctx.move_to(-8, -22); ctx.line_to(3, -8); ctx.line_to(-5, 5); ctx.line_to(12, 21);
    ctx.stroke();
    ctx.set_stroke(0xffE66C40);
    ctx.set_global_alpha(0.7f + p * 0.3f);
    ctx.set_line_width(5);
    ctx.begin_path();
    ctx.move_to(-8, -22); ctx.line_to(3, -8); ctx.line_to(-5, 5); ctx.line_to(12, 21);
    ctx.stroke();
    ctx.set_global_alpha(1.f);
    ctx.set_fill(0xffF6BA59);
    ctx.begin_path();
    ctx.ellipse(-9, -17, 4, 2.5f);
    ctx.ellipse(9, -17, 4, 2.5f);
    ctx.fill();
    // Wrist rings
    ctx.set_stroke(0xff846B60);
    ctx.set_line_width(3);
    ctx.begin_path();
    ctx.ellipse(-28, 18, 7, 5);
    ctx.ellipse(28, 16, 7, 5);
    ctx.stroke();
}

void volcano_wayside(Renderer &ctx, float anim, uint32_t seed) {
    float b = breath(anim, seed);
    shadow(ctx, 40, 22, 0x28000000);
    ctx.set_fill(0xff5A585E);
    ctx.set_stroke(0xff2A282E);
    ctx.set_line_width(2.5f);
    ctx.begin_path();
    ctx.move_to(-18, -30); ctx.line_to(12, -34); ctx.line_to(22, -4);
    ctx.line_to(16, 30); ctx.line_to(-8, 34); ctx.line_to(-22, 8);
    ctx.close_path();
    ctx.fill(); ctx.stroke();
    // Rag hood
    ctx.set_fill(0xff6A656E);
    ctx.begin_path();
    ctx.move_to(-16, -22); ctx.line_to(0, -38); ctx.line_to(14, -20); ctx.line_to(10, -8); ctx.line_to(-12, -6);
    ctx.close_path();
    ctx.fill();
    // Dim crack
    ctx.set_stroke(0xff8B3A2A);
    ctx.set_line_width(2.5f);
    ctx.begin_path();
    ctx.move_to(-2, -8); ctx.line_to(4, 12);
    ctx.stroke();
    // One chain
    ctx.set_stroke(0xff846B60);
    ctx.set_line_width(2.5f);
    for (int i = 0; i < 3; ++i) {
        ctx.begin_path();
        ctx.ellipse(24.f + i, 4.f + i * 10.f + b, 4, 6);
        ctx.stroke();
    }
    eyes(ctx, 6, -14, 2.2f, 0xffC87850);
}

// ---- Royal ----
void royal_main(Renderer &ctx, float anim, uint32_t seed) {
    (void) anim; (void) seed;
    shadow(ctx, 41, 30, 0x28000000);
    ctx.set_fill(0xff6B4E2E);
    ctx.begin_path();
    ctx.round_rect(15, 1, 22, 31, 7);
    ctx.fill();
    ctx.set_fill(0xff5A5A5E);
    ctx.set_stroke(0xff3A3A3E);
    ctx.set_line_width(3);
    ctx.begin_path();
    ctx.round_rect(-25, -6, 50, 43, 11);
    ctx.fill(); ctx.stroke();
    ctx.set_fill(0xff8A6A50);
    ctx.begin_path();
    ctx.ellipse(0, -24, 29, 25);
    ctx.fill();
    ctx.set_fill(0xff6A5040);
    ctx.begin_path();
    ctx.round_rect(-30, -28, 60, 12, 4);
    ctx.fill();
    // Side armor
    ctx.set_fill(0xff7A6050);
    ctx.begin_path();
    ctx.move_to(-32, -18); ctx.line_to(-40, -4); ctx.line_to(-30, 10); ctx.line_to(-24, -6);
    ctx.close_path();
    ctx.fill();
    ctx.begin_path();
    ctx.move_to(32, -16); ctx.line_to(42, 0); ctx.line_to(30, 12); ctx.line_to(24, -4);
    ctx.close_path();
    ctx.fill();
    // Visor slit
    ctx.set_fill(0xff2A2A2E);
    ctx.begin_path();
    ctx.round_rect(-18, -22, 36, 8, 2);
    ctx.fill();
    eyes(ctx, 10, -19, 2.2f, 0xffC9A227);
    // Gear badge
    ctx.set_fill(0xffC9A227);
    ctx.begin_path();
    ctx.arc(0, 14, 13);
    ctx.fill();
    ctx.set_fill(0xff5A5A5E);
    ctx.begin_path();
    ctx.arc(0, 14, 5);
    ctx.fill();
    for (int i = 0; i < 6; ++i) {
        float a = (float) i * (float) M_PI / 3.f;
        ctx.set_fill(0xffC9A227);
        ctx.begin_path();
        ctx.ellipse(cosf(a) * 13, 14 + sinf(a) * 13, 3.5f, 2.f);
        ctx.fill();
    }
    // Rust spots
    ctx.set_fill(0xff8A5040);
    ctx.begin_path();
    ctx.arc(-12, 6, 3);
    ctx.arc(14, 22, 2.5f);
    ctx.arc(-8, 28, 2);
    ctx.fill();
}

void royal_wayside(Renderer &ctx, float anim, uint32_t seed) {
    (void) anim; (void) seed;
    shadow(ctx, 38, 24, 0x24000000);
    ctx.set_fill(0xff5A5A5E);
    ctx.begin_path();
    ctx.round_rect(-20, -4, 40, 39, 12);
    ctx.fill();
    ctx.set_fill(0xff8A6A50);
    ctx.begin_path();
    ctx.ellipse(0, -21, 21, 20);
    ctx.fill();
    // Missing side armor → dark inner
    ctx.set_fill(0xff2A2A2E);
    ctx.begin_path();
    ctx.ellipse(14, -18, 8, 10);
    ctx.fill();
    ctx.set_fill(0xff2A2A2E);
    ctx.begin_path();
    ctx.round_rect(-14, -24, 28, 7, 2);
    ctx.fill();
    eyes(ctx, 7, -20, 2.f, 0xffC9A227);
    ctx.set_fill(0xff6B4E2E);
    ctx.begin_path();
    ctx.round_rect(12, 8, 14, 18, 4);
    ctx.fill();
    // Small coin pouch
    ctx.set_fill(0xffC9A227);
    ctx.begin_path();
    ctx.arc(-16, 22, 6);
    ctx.fill();
}

// ---- Escorts ----
void escort_bee(Renderer &ctx, float anim, uint32_t seed) {
    float b = breath(anim, seed) * 2.f;
    float w = sinf(anim * 9.f + seed) * 0.25f;
    ctx.translate(0, b);
    ctx.set_fill(0xffF0D050);
    ctx.begin_path();
    ctx.ellipse(0, 3, 23, 18);
    ctx.fill();
    ctx.set_stroke(0xff5A3D22);
    ctx.set_line_width(3);
    ctx.begin_path();
    ctx.arc(0, 0, 14);
    ctx.arc(0, 6, 12);
    ctx.stroke();
    ctx.set_fill(0xffF0D050);
    ctx.begin_path();
    ctx.arc(-16, -4, 13);
    ctx.fill();
    eyes(ctx, 5, -4, 3.5f);
    ctx.set_global_alpha(0.8f);
    ctx.set_fill(0xffFFF8EC);
    ctx.begin_path();
    ctx.ellipse(-7, -19, 12, 8 + w * 3);
    ctx.ellipse(10, -20, 12, 8 - w * 3);
    ctx.fill();
    ctx.set_global_alpha(1.f);
    ctx.set_fill(0xff5A3D22);
    ctx.begin_path();
    ctx.move_to(20, 4); ctx.line_to(28, 8); ctx.line_to(20, 10);
    ctx.close_path();
    ctx.fill();
    ctx.translate(0, -b);
}

void escort_traveler(Renderer &ctx, float anim, uint32_t seed) {
    float b = breath(anim, seed);
    ctx.set_fill(0xffD8A45D);
    ctx.begin_path();
    ctx.ellipse(0, 13, 19, 24);
    ctx.fill();
    ctx.set_fill(0xff79583D);
    ctx.begin_path();
    ctx.arc(0, -19, 17);
    ctx.fill();
    ctx.set_fill(0xffEED49A);
    ctx.begin_path();
    ctx.ellipse(0, -16, 12, 8);
    ctx.fill();
    eyes(ctx, 5, -16, 2.8f);
    ctx.set_fill(0xffA97449);
    ctx.begin_path();
    ctx.ellipse(21, 14 + b, 10, 15);
    ctx.fill();
    ctx.set_stroke(0xffC89256);
    ctx.set_line_width(3);
    ctx.begin_path();
    ctx.move_to(-8, -28); ctx.qcurve_to(-18, -20, -16, -8);
    ctx.move_to(8, -28); ctx.qcurve_to(18, -22, 16, -8);
    ctx.stroke();
}

void escort_jelly(Renderer &ctx, float anim, uint32_t seed) {
    float s = 1.f + 0.04f * breath(anim, seed);
    ctx.scale(s, 1.f / s);
    ctx.set_global_alpha(0.75f);
    ctx.set_fill(0xff7CCDDC);
    ctx.begin_path();
    ctx.ellipse(0, -5, 25, 23);
    ctx.fill();
    ctx.set_global_alpha(1.f);
    ctx.set_stroke(0xff3E7185);
    ctx.set_line_width(4);
    ctx.begin_path();
    ctx.arc(0, -8, 22);
    ctx.stroke();
    eyes(ctx, 9, -7, 3.f, 0xff1a3a5a);
    ctx.set_global_alpha(0.65f);
    ctx.set_fill(0xffF5D58B);
    ctx.begin_path();
    ctx.arc(0, 5, 7);
    ctx.fill();
    ctx.set_global_alpha(1.f);
    ctx.set_stroke(0xff59B7CC);
    ctx.set_line_width(3);
    for (int i = 0; i < 4; ++i) {
        float x = -12.f + i * 8.f;
        float ph = sinf(anim * 2.f + i + seed) * 4.f;
        ctx.begin_path();
        ctx.move_to(x, 14);
        ctx.qcurve_to(x + ph, 26, x, 38);
        ctx.stroke();
    }
    ctx.scale(1.f / s, s);
}

void escort_penguin(Renderer &ctx, float anim, uint32_t seed) {
    float tilt = breath(anim, seed) * 0.05f;
    ctx.rotate(tilt);
    ctx.set_fill(0xff2A3A4A);
    ctx.begin_path();
    ctx.ellipse(0, 8, 26, 36);
    ctx.fill();
    ctx.set_fill(0xffF4F6F8);
    ctx.begin_path();
    ctx.ellipse(0, 17, 18, 26);
    ctx.fill();
    ctx.set_fill(0xff2A3A4A);
    ctx.begin_path();
    ctx.arc(0, -20, 24);
    ctx.fill();
    ctx.set_fill(0xffffffff);
    ctx.begin_path();
    ctx.ellipse(-9, -22, 6, 8);
    ctx.ellipse(9, -22, 6, 8);
    ctx.fill();
    eyes(ctx, 9, -22, 2.5f);
    ctx.set_fill(0xffE8A040);
    ctx.begin_path();
    ctx.move_to(-7, -10); ctx.line_to(7, -10); ctx.line_to(0, -1);
    ctx.close_path();
    ctx.fill();
    // Flippers
    ctx.set_fill(0xff2A3A4A);
    ctx.begin_path();
    ctx.ellipse(-34, 8, 10, 16);
    ctx.ellipse(34, 8, 10, 16);
    ctx.fill();
    ctx.set_fill(0xffE8A040);
    ctx.begin_path();
    ctx.ellipse(-13, 40, 12, 6);
    ctx.ellipse(13, 40, 12, 6);
    ctx.fill();
    ctx.rotate(-tilt);
}

void escort_critter(Renderer &ctx, float anim, uint32_t seed) {
    float b = breath(anim, seed);
    ctx.set_fill(0xff5A9A4A);
    ctx.begin_path();
    ctx.ellipse(0, 9, 25, 23);
    ctx.fill();
    ctx.set_fill(0xff6FAF4A);
    ctx.begin_path();
    ctx.ellipse(0, -14, 21, 18);
    ctx.fill();
    for (int i = 0; i < 5; ++i) {
        float a = -1.2f + i * 0.6f;
        ctx.set_fill(0xff4A8A3A);
        ctx.begin_path();
        ctx.ellipse(sinf(a) * 14, -28 + cosf(a) * 4, 4, 7);
        ctx.fill();
    }
    eyes(ctx, 8, -12, 3.5f);
    ctx.set_fill(0xff6FAF4A);
    ctx.begin_path();
    ctx.ellipse(-18, -22, 7, 5);
    ctx.ellipse(18, -22, 7, 5);
    ctx.fill();
    ctx.set_stroke(0xff4A8A3A);
    ctx.set_line_width(3);
    ctx.begin_path();
    ctx.move_to(16, 12); ctx.qcurve_to(28, 18 + b * 3, 29, 24);
    ctx.stroke();
    ctx.set_fill(0xffC9A227);
    ctx.begin_path();
    ctx.arc(29, 24, 6);
    ctx.fill();
}

void escort_worker(Renderer &ctx, float anim, uint32_t seed) {
    float b = breath(anim, seed);
    ctx.set_fill(0xffAD7852);
    ctx.begin_path();
    ctx.arc(0, -24, 20);
    ctx.fill();
    ctx.set_fill(0xff8E6244);
    ctx.begin_path();
    ctx.ellipse(0, 0, 13, 15);
    ctx.fill();
    ctx.set_fill(0xff8E6244);
    ctx.begin_path();
    ctx.ellipse(0, 23, 21, 18);
    ctx.fill();
    ctx.set_stroke(0xff5A3D22);
    ctx.set_line_width(3.5f);
    ctx.begin_path();
    ctx.move_to(-8, -38); ctx.qcurve_to(-18, -44 + b * 4, -22, -36);
    ctx.move_to(8, -38); ctx.qcurve_to(18, -44 - b * 4, 22, -36);
    ctx.stroke();
    eyes(ctx, 6.5f, -24, 3.f);
    for (int s = -1; s <= 1; s += 2)
        for (int i = 0; i < 3; ++i) {
            ctx.begin_path();
            ctx.move_to((float) s * 8, (float) i * 8);
            ctx.line_to((float) s * 18, 4.f + i * 8);
            ctx.stroke();
        }
    ctx.set_fill(0xff4A8A3A);
    ctx.begin_path();
    ctx.ellipse(0, -2, 8, 6);
    ctx.fill();
}

void escort_ember(Renderer &ctx, float anim, uint32_t seed) {
    float b = breath(anim, seed) * 2.f;
    float p = pulse(anim, seed);
    ctx.translate(0, b);
    ctx.set_global_alpha(0.15f);
    ctx.set_fill(0xffE66C40);
    ctx.begin_path();
    ctx.arc(0, 5, 29);
    ctx.fill();
    ctx.set_global_alpha(1.f);
    // Soft flame tongues
    for (int i = 0; i < 3; ++i) {
        float ph = sinf(anim * 2.5f + i * 2.1f + seed) * 4.f;
        float x = -12.f + i * 12.f;
        ctx.set_fill(i == 1 ? 0xffF6BA59 : 0xffE66C40);
        ctx.begin_path();
        ctx.move_to(x - 8, 10);
        ctx.qcurve_to(x + ph, -18 - p * 6, x + 8, 10);
        ctx.close_path();
        ctx.fill();
    }
    ctx.set_fill(0xffF6BA59);
    ctx.begin_path();
    ctx.arc(0, 8, 20);
    ctx.fill();
    eyes(ctx, 7, 7, 3.f, 0xff5A3D22);
    ctx.set_stroke(0xffE66C40);
    ctx.set_line_width(2);
    ctx.begin_path();
    ctx.arc(0, 14, 10);
    ctx.stroke();
    ctx.translate(0, -b);
}

void escort_knight(Renderer &ctx, float anim, uint32_t seed) {
    (void) anim; (void) seed;
    ctx.set_fill(0xff8A6A50);
    ctx.begin_path();
    ctx.ellipse(0, -22, 21, 19);
    ctx.fill();
    ctx.set_fill(0xff3A3A3E);
    ctx.begin_path();
    ctx.round_rect(-16, -25, 32, 10, 4);
    ctx.fill();
    eyes(ctx, 6, -21, 2.f, 0xffC9A227);
    ctx.set_fill(0xff5A5A5E);
    ctx.begin_path();
    ctx.round_rect(-20, -4, 40, 36, 10);
    ctx.fill();
    // Shield crest
    ctx.set_fill(0xffC9A227);
    ctx.begin_path();
    ctx.move_to(0, 2); ctx.line_to(8, 10); ctx.line_to(0, 22); ctx.line_to(-8, 10);
    ctx.close_path();
    ctx.fill();
    ctx.set_fill(0xff6A6A70);
    ctx.begin_path();
    ctx.round_rect(-28, 2, 10, 17, 3);
    ctx.round_rect(18, 2, 10, 17, 3);
    ctx.fill();
    ctx.set_fill(0xff4A4A50);
    ctx.begin_path();
    ctx.round_rect(-14, 28, 10, 10, 2);
    ctx.round_rect(4, 28, 10, 10, 2);
    ctx.fill();
}

void draw_escort(Renderer &ctx, uint8_t map_id, float anim, uint32_t seed) {
    switch (map_id) {
    case MapID::kGarden: escort_bee(ctx, anim, seed); break;
    case MapID::kDesert: escort_traveler(ctx, anim, seed); break;
    case MapID::kOcean: escort_jelly(ctx, anim, seed); break;
    case MapID::kPolar: escort_penguin(ctx, anim, seed); break;
    case MapID::kJungle: escort_critter(ctx, anim, seed); break;
    case MapID::kAntHell: escort_worker(ctx, anim, seed); break;
    case MapID::kVolcano: escort_ember(ctx, anim, seed); break;
    case MapID::kRoyalCapital: escort_knight(ctx, anim, seed); break;
    default: escort_bee(ctx, anim, seed); break;
    }
}

void draw_main(Renderer &ctx, uint8_t map_id, float anim, uint32_t seed) {
    switch (map_id) {
    case MapID::kGarden: garden_main(ctx, anim, seed); break;
    case MapID::kDesert: desert_main(ctx, anim, seed); break;
    case MapID::kOcean: ocean_main(ctx, anim, seed); break;
    case MapID::kPolar: polar_main(ctx, anim, seed); break;
    case MapID::kJungle: jungle_main(ctx, anim, seed); break;
    case MapID::kAntHell: ant_main(ctx, anim, seed); break;
    case MapID::kVolcano: volcano_main(ctx, anim, seed); break;
    case MapID::kRoyalCapital: royal_main(ctx, anim, seed); break;
    default: garden_main(ctx, anim, seed); break;
    }
}

void draw_wayside(Renderer &ctx, uint8_t map_id, float anim, uint32_t seed) {
    switch (map_id) {
    case MapID::kGarden: garden_wayside(ctx, anim, seed); break;
    case MapID::kDesert: desert_wayside(ctx, anim, seed); break;
    case MapID::kOcean: ocean_wayside(ctx, anim, seed); break;
    case MapID::kPolar: polar_wayside(ctx, anim, seed); break;
    case MapID::kJungle: jungle_wayside(ctx, anim, seed); break;
    case MapID::kAntHell: ant_wayside(ctx, anim, seed); break;
    case MapID::kVolcano: volcano_wayside(ctx, anim, seed); break;
    case MapID::kRoyalCapital: royal_wayside(ctx, anim, seed); break;
    default: garden_wayside(ctx, anim, seed); break;
    }
}

// ---- Themed props ----
void pad_garden(Renderer &ctx, float anim, uint32_t seed) {
    float p = pulse(anim, seed);
    ctx.set_global_alpha(0.45f + 0.2f * p);
    ctx.set_fill(0xff7CFF8A);
    for (int i = 0; i < 6; ++i) {
        float a = (float) i * (float) M_PI / 3.f;
        ctx.begin_path();
        ctx.ellipse(cosf(a) * 28, sinf(a) * 28, 18, 10);
        ctx.fill();
    }
    ctx.set_global_alpha(0.55f);
    ctx.set_fill(0xffA6D78A);
    ctx.begin_path();
    ctx.arc(0, 0, 26);
    ctx.fill();
    ctx.set_global_alpha(1.f);
    ctx.set_fill(0xff4A8A3A);
    ctx.begin_path();
    ctx.ellipse(-6, 0, 10, 5);
    ctx.ellipse(6, 0, 10, 5);
    ctx.fill();
    ctx.set_fill(0xffC9A227);
    ctx.begin_path();
    ctx.arc(0, 0, 4);
    ctx.fill();
}

void pad_desert(Renderer &ctx, float anim, uint32_t seed) {
    float p = pulse(anim, seed);
    ctx.set_global_alpha(0.5f + 0.2f * p);
    ctx.set_stroke(0xffD8A45D);
    ctx.set_line_width(4);
    for (int i = 0; i < 4; ++i) {
        float a0 = (float) i * 1.4f;
        ctx.begin_path();
        ctx.arc(0, 0, 40);
        ctx.stroke();
        (void) a0;
    }
    ctx.set_stroke(0xffEED49A);
    ctx.set_line_width(3);
    ctx.begin_path();
    ctx.move_to(-18, 8); ctx.qcurve_to(0, -13, 18, 8);
    ctx.move_to(-14, 14); ctx.qcurve_to(0, -3, 14, 14);
    ctx.move_to(-10, 20); ctx.qcurve_to(0, 8, 10, 20);
    ctx.stroke();
    ctx.set_fill(0xffC9A227);
    ctx.begin_path();
    ctx.move_to(-22, 0); ctx.line_to(-18, -6); ctx.line_to(-14, 0); ctx.line_to(-18, 6);
    ctx.close_path();
    ctx.move_to(14, 0); ctx.line_to(18, -6); ctx.line_to(22, 0); ctx.line_to(18, 6);
    ctx.close_path();
    ctx.fill();
    ctx.set_global_alpha(1.f);
}

void pad_ocean(Renderer &ctx, float anim, uint32_t seed) {
    float p = pulse(anim, seed);
    ctx.set_global_alpha(0.5f + 0.25f * p);
    ctx.set_stroke(0xff7CCDDC);
    ctx.set_line_width(4);
    ctx.begin_path();
    ctx.arc(0, 0, 40);
    ctx.arc(0, 0, 28);
    ctx.stroke();
    ctx.set_fill(0xffF5D58B);
    ctx.begin_path();
    ctx.ellipse(0, 0, 14, 11);
    ctx.fill();
    ctx.set_stroke(0xffB89550);
    ctx.set_line_width(2);
    ctx.begin_path();
    ctx.move_to(0, 0); ctx.line_to(10, -8);
    ctx.move_to(0, 0); ctx.line_to(-8, -6);
    ctx.move_to(0, 0); ctx.line_to(4, 10);
    ctx.stroke();
    ctx.set_global_alpha(1.f);
}

void pad_polar(Renderer &ctx, float anim, uint32_t seed) {
    float p = pulse(anim, seed);
    ctx.set_global_alpha(0.55f + 0.2f * p);
    ctx.set_stroke(0xffE6F6FA);
    ctx.set_line_width(3.5f);
    // Hex-ish rune
    ctx.begin_path();
    for (int i = 0; i < 6; ++i) {
        float a = (float) i * (float) M_PI / 3.f - (float) M_PI / 6.f;
        float x = cosf(a) * 36, y = sinf(a) * 36;
        if (i == 0) ctx.move_to(x, y); else ctx.line_to(x, y);
    }
    ctx.close_path();
    ctx.stroke();
    ctx.set_fill(0xff91CFE8);
    ctx.begin_path();
    ctx.move_to(0, -14); ctx.line_to(10, 0); ctx.line_to(0, 14); ctx.line_to(-10, 0);
    ctx.close_path();
    ctx.fill();
    ctx.set_global_alpha(1.f);
}

void pad_jungle(Renderer &ctx, float anim, uint32_t seed) {
    float p = pulse(anim, seed);
    ctx.set_global_alpha(0.5f + 0.2f * p);
    ctx.set_stroke(0xff4A8A3A);
    ctx.set_line_width(5);
    ctx.begin_path();
    ctx.arc(0, 0, 38);
    ctx.stroke();
    ctx.set_fill(0xff6FAF4A);
    for (int i = 0; i < 3; ++i) {
        float a = (float) i * 2.1f;
        ctx.begin_path();
        ctx.ellipse(cosf(a) * 12, sinf(a) * 12, 14, 6);
        ctx.fill();
    }
    ctx.set_fill(0xffC9A227);
    ctx.begin_path();
    ctx.arc(0, 0, 5);
    ctx.fill();
    ctx.set_global_alpha(1.f);
}

void pad_ant(Renderer &ctx, float anim, uint32_t seed) {
    float p = pulse(anim, seed);
    ctx.set_global_alpha(0.5f + 0.2f * p);
    ctx.set_fill(0xff8E6244);
    ctx.begin_path();
    ctx.arc(0, 0, 36);
    ctx.fill();
    ctx.set_fill(0xffAD7852);
    ctx.begin_path();
    ctx.arc(0, 0, 22);
    ctx.fill();
    // Hex nest mark
    ctx.set_stroke(0xff5A3D22);
    ctx.set_line_width(3);
    ctx.begin_path();
    for (int i = 0; i < 6; ++i) {
        float a = (float) i * (float) M_PI / 3.f;
        float x = cosf(a) * 12, y = sinf(a) * 12;
        if (i == 0) ctx.move_to(x, y); else ctx.line_to(x, y);
    }
    ctx.close_path();
    ctx.stroke();
    ctx.set_global_alpha(1.f);
}

void pad_volcano(Renderer &ctx, float anim, uint32_t seed) {
    float p = pulse(anim, seed);
    ctx.set_global_alpha(0.55f + 0.25f * p);
    ctx.set_stroke(0xffE66C40);
    ctx.set_line_width(5);
    ctx.begin_path();
    ctx.arc(0, 0, 40);
    ctx.stroke();
    ctx.set_fill(0xff45424A);
    ctx.begin_path();
    ctx.arc(0, 0, 22);
    ctx.fill();
    ctx.set_fill(0xffF6BA59);
    ctx.begin_path();
    ctx.move_to(0, -12); ctx.line_to(8, 4); ctx.line_to(-8, 4);
    ctx.close_path();
    ctx.fill();
    ctx.set_global_alpha(1.f);
}

void pad_royal(Renderer &ctx, float anim, uint32_t seed) {
    float p = pulse(anim, seed);
    ctx.set_global_alpha(0.5f + 0.2f * p);
    ctx.set_stroke(0xffC9A227);
    ctx.set_line_width(4);
    ctx.begin_path();
    ctx.round_rect(-36, -36, 72, 72, 8);
    ctx.stroke();
    ctx.set_fill(0xff5A5A5E);
    ctx.begin_path();
    ctx.arc(0, 0, 20);
    ctx.fill();
    ctx.set_fill(0xffC9A227);
    ctx.begin_path();
    ctx.move_to(0, -12); ctx.line_to(10, -2); ctx.line_to(6, 12); ctx.line_to(-6, 12); ctx.line_to(-10, -2);
    ctx.close_path();
    ctx.fill();
    ctx.set_global_alpha(1.f);
}

void draw_pad(Renderer &ctx, uint8_t map_id, float anim, uint32_t seed) {
    switch (map_id) {
    case MapID::kGarden: pad_garden(ctx, anim, seed); break;
    case MapID::kDesert: pad_desert(ctx, anim, seed); break;
    case MapID::kOcean: pad_ocean(ctx, anim, seed); break;
    case MapID::kPolar: pad_polar(ctx, anim, seed); break;
    case MapID::kJungle: pad_jungle(ctx, anim, seed); break;
    case MapID::kAntHell: pad_ant(ctx, anim, seed); break;
    case MapID::kVolcano: pad_volcano(ctx, anim, seed); break;
    case MapID::kRoyalCapital: pad_royal(ctx, anim, seed); break;
    default: pad_garden(ctx, anim, seed); break;
    }
}

void crate_base(Renderer &ctx, uint32_t fill, uint32_t stroke) {
    ctx.set_fill(fill);
    ctx.set_stroke(stroke);
    ctx.set_line_width(3.5f);
    ctx.round_line_join();
    ctx.begin_path();
    ctx.round_rect(-29, -29, 58, 58, 10);
    ctx.fill();
    ctx.stroke();
}

void draw_crate(Renderer &ctx, uint8_t map_id, float anim, uint32_t seed) {
    (void) anim; (void) seed;
    switch (map_id) {
    case MapID::kGarden:
        crate_base(ctx, 0xffC4A36A, 0xff6B4E2E);
        ctx.set_stroke(0xff8B6840);
        ctx.set_line_width(2);
        ctx.begin_path();
        ctx.move_to(-20, -10); ctx.qcurve_to(0, -16, 20, -8);
        ctx.move_to(-18, 4); ctx.qcurve_to(0, 0, 18, 6);
        ctx.move_to(-16, 16); ctx.qcurve_to(0, 12, 16, 18);
        ctx.stroke();
        ctx.set_fill(0xff6FAF4A);
        for (int i = 0; i < 4; ++i) {
            float x = (i & 1) ? 22.f : -22.f, y = (i & 2) ? 22.f : -22.f;
            ctx.begin_path();
            ctx.ellipse(x, y, 6, 3);
            ctx.fill();
        }
        ctx.set_fill(0xffE8D5A3);
        ctx.begin_path();
        ctx.arc(0, 0, 12);
        ctx.fill();
        break;
    case MapID::kDesert:
        crate_base(ctx, 0xffD2B48C, 0xff8B6914);
        ctx.set_stroke(0xffA08060);
        ctx.set_line_width(2);
        ctx.begin_path();
        ctx.move_to(-20, -15); ctx.line_to(20, -12);
        ctx.move_to(-18, 5); ctx.line_to(18, 8);
        ctx.stroke();
        ctx.set_fill(0xffC9A227);
        ctx.begin_path();
        ctx.move_to(0, -8); ctx.line_to(8, 0); ctx.line_to(0, 8); ctx.line_to(-8, 0);
        ctx.close_path();
        ctx.fill();
        break;
    case MapID::kOcean:
        crate_base(ctx, 0xff8B6B4A, 0xff4B2C21);
        ctx.set_fill(0xff59B7CC);
        ctx.begin_path();
        ctx.ellipse(0, -4, 16, 6);
        ctx.fill();
        ctx.set_stroke(0xffF5D58B);
        ctx.set_line_width(2);
        ctx.begin_path();
        ctx.arc(0, 8, 10);
        ctx.stroke();
        break;
    case MapID::kPolar:
        crate_base(ctx, 0xffD0E4EC, 0xff557D99);
        ctx.set_stroke(0xff91CFE8);
        ctx.set_line_width(3);
        ctx.begin_path();
        ctx.move_to(-22, -22); ctx.line_to(0, -8); ctx.line_to(22, -22);
        ctx.move_to(-22, 22); ctx.line_to(0, 8); ctx.line_to(22, 22);
        ctx.stroke();
        ctx.set_fill(0xffE6F6FA);
        ctx.begin_path();
        ctx.arc(0, 0, 8);
        ctx.fill();
        break;
    case MapID::kJungle:
        crate_base(ctx, 0xff6B8F4E, 0xff3A5A2A);
        ctx.set_stroke(0xff4A8A3A);
        ctx.set_line_width(4);
        ctx.begin_path();
        ctx.arc(0, 0, 18);
        ctx.stroke();
        ctx.set_fill(0xffC9A227);
        ctx.begin_path();
        ctx.arc(0, 0, 7);
        ctx.fill();
        break;
    case MapID::kAntHell:
        crate_base(ctx, 0xff8E6244, 0xff5A3D22);
        ctx.set_fill(0xffAD7852);
        ctx.begin_path();
        for (int i = 0; i < 6; ++i) {
            float a = (float) i * (float) M_PI / 3.f;
            float x = cosf(a) * 14, y = sinf(a) * 14;
            if (i == 0) ctx.move_to(x, y); else ctx.line_to(x, y);
        }
        ctx.close_path();
        ctx.fill();
        break;
    case MapID::kVolcano:
        crate_base(ctx, 0xff3A383E, 0xff1A181C);
        ctx.set_stroke(0xffE66C40);
        ctx.set_line_width(3);
        ctx.begin_path();
        ctx.move_to(-16, -16); ctx.line_to(4, 0); ctx.line_to(-8, 16); ctx.line_to(16, 20);
        ctx.stroke();
        ctx.set_fill(0xffF6BA59);
        ctx.begin_path();
        ctx.arc(0, 0, 6);
        ctx.fill();
        break;
    case MapID::kRoyalCapital:
        crate_base(ctx, 0xff5A5A5E, 0xffC9A227);
        ctx.set_stroke(0xffC9A227);
        ctx.set_line_width(2.5f);
        ctx.begin_path();
        ctx.move_to(-29, 0); ctx.line_to(29, 0);
        ctx.move_to(0, -29); ctx.line_to(0, 29);
        ctx.stroke();
        ctx.set_fill(0xffC9A227);
        ctx.begin_path();
        ctx.arc(0, 0, 9);
        ctx.fill();
        break;
    default:
        crate_base(ctx, 0xffC4A36A, 0xff6B4E2E);
        break;
    }
}

void chest_draw(Renderer &ctx, uint8_t map_id, float alpha, float anim, uint32_t seed) {
    float p = pulse(anim, seed);
    ctx.set_global_alpha(alpha);
    uint32_t body = 0xffA86B3C, lid = 0xffF0CB70, trim = 0xff4B2C21, lock = 0xffE5B95C;
    switch (map_id) {
    case MapID::kGarden: body = 0xffA86B3C; lid = 0xffF0CB70; lock = 0xffC9A227; break;
    case MapID::kDesert: body = 0xffC4A060; lid = 0xffE8D090; trim = 0xff8B6914; lock = 0xffC9A227; break;
    case MapID::kOcean: body = 0xff3E7185; lid = 0xff7CCDDC; trim = 0xff2A4A5A; lock = 0xffF5D58B; break;
    case MapID::kPolar: body = 0xff91CFE8; lid = 0xffE6F6FA; trim = 0xff557D99; lock = 0xffBDE8F4; break;
    case MapID::kJungle: body = 0xff6B8F4E; lid = 0xffA6D78A; trim = 0xff3A5A2A; lock = 0xffC9A227; break;
    case MapID::kAntHell: body = 0xff8E6244; lid = 0xffAD7852; trim = 0xff5A3D22; lock = 0xffC45A3A; break;
    case MapID::kVolcano: body = 0xff45424A; lid = 0xffE66C40; trim = 0xff2A282E; lock = 0xffF6BA59; break;
    case MapID::kRoyalCapital: body = 0xff5A5A5E; lid = 0xffC9A227; trim = 0xff3A3A3E; lock = 0xffE8D5A3; break;
    default: break;
    }
    ctx.set_fill(body);
    ctx.set_stroke(trim);
    ctx.set_line_width(3);
    ctx.round_line_join();
    ctx.begin_path();
    ctx.round_rect(-28, -16, 56, 40, 6);
    ctx.fill();
    ctx.stroke();
    ctx.set_fill(lid);
    ctx.begin_path();
    ctx.round_rect(-30, -22, 60, 14, 5);
    ctx.fill();
    // Theme lock
    ctx.set_fill(lock);
    ctx.set_stroke(trim);
    ctx.set_line_width(2);
    if (map_id == MapID::kGarden) {
        for (int i = 0; i < 5; ++i) {
            float a = (float) i * 1.256f;
            ctx.begin_path();
            ctx.arc(cosf(a) * 5, 2 + sinf(a) * 5, 3);
            ctx.fill();
        }
    } else if (map_id == MapID::kRoyalCapital) {
        ctx.begin_path();
        ctx.round_rect(-8, -2, 16, 14, 2);
        ctx.fill(); ctx.stroke();
    } else if (map_id == MapID::kVolcano) {
        ctx.begin_path();
        ctx.move_to(0, -6); ctx.line_to(8, 8); ctx.line_to(-8, 8);
        ctx.close_path();
        ctx.fill();
    } else {
        ctx.begin_path();
        ctx.round_rect(-6, -2, 12, 12, 2);
        ctx.fill(); ctx.stroke();
    }
    if (alpha < 0.9f) {
        // Mirage ghost outline
        ctx.set_global_alpha(alpha * 0.5f);
        ctx.set_stroke(lid);
        ctx.set_line_width(2);
        ctx.begin_path();
        ctx.round_rect(-32 + p * 2, -24, 60, 44, 6);
        ctx.stroke();
    }
    ctx.set_global_alpha(1.f);
}

float height_scale(std::string_view name) {
    // Small / mid / tall from WorldEvent height_pad_name.
    if (name_has(name, "矮") || name_has(name, "淡") || name_has(name, "小")
        || name_has(name, "暗") || name_has(name, "温") || name_has(name, "旧"))
        return 0.72f;
    if (name_has(name, "高") || name_has(name, "艳") || name_has(name, "大")
        || name_has(name, "耀") || name_has(name, "灼") || name_has(name, "烈")
        || name_has(name, "新"))
        return 1.28f;
    return 1.f;
}

void flee_garden(Renderer &ctx, float anim, uint32_t seed) {
    float b = breath(anim, seed) * 2.f;
    ctx.set_fill(0xff4A8A3A);
    ctx.begin_path();
    ctx.ellipse(-12, 4 + b, 14, 18);
    ctx.ellipse(8, 2 - b, 16, 20);
    ctx.ellipse(0, -6, 18, 16);
    ctx.ellipse(-18, 10, 12, 14);
    ctx.ellipse(16, 12, 11, 13);
    ctx.fill();
    ctx.set_stroke(0xff3A5A2A);
    ctx.set_line_width(3);
    ctx.begin_path();
    ctx.move_to(-6, 22); ctx.line_to(-8, 32);
    ctx.move_to(6, 22); ctx.line_to(8, 32);
    ctx.stroke();
    eyes(ctx, 6, -2, 2.5f);
}

void flee_desert(Renderer &ctx, float anim, uint32_t seed) {
    float b = breath(anim, seed);
    ctx.set_fill(0xffD8A45D);
    ctx.begin_path();
    ctx.ellipse(0, 12, 28, 14);
    ctx.ellipse(-10, 4, 18, 12);
    ctx.ellipse(8, 0 + b, 16, 11);
    ctx.fill();
    ctx.set_fill(0xffEED49A);
    ctx.begin_path();
    ctx.ellipse(0, -8, 10, 8);
    ctx.fill();
}

void flee_ocean(Renderer &ctx, float anim, uint32_t seed) {
    float b = breath(anim, seed) * 2.f; // P3: damp kelp sway
    ctx.set_fill(0xff4A6A50);
    ctx.begin_path();
    ctx.ellipse(0, 28, 10, 6);
    ctx.fill();
    ctx.set_stroke(0xff3A8A5A);
    ctx.set_line_width(7);
    ctx.round_line_cap();
    for (int i = 0; i < 3; ++i) {
        float x = -12.f + i * 12.f;
        ctx.begin_path();
        ctx.move_to(x, 24);
        ctx.qcurve_to(x + b, 0, x - b * 0.5f, -28);
        ctx.stroke();
    }
}

void flee_polar(Renderer &ctx, float anim, uint32_t seed) {
    float b = breath(anim, seed);
    ctx.set_fill(0xffF4F8FC);
    ctx.set_stroke(0xffBDE8F4);
    ctx.set_line_width(2);
    ctx.begin_path();
    ctx.ellipse(0, 10, 26, 16);
    ctx.ellipse(-8, 0, 18, 12);
    ctx.ellipse(10, -2 + b, 16, 11);
    ctx.fill();
    ctx.stroke();
    ctx.set_fill(0xffE6F6FA);
    ctx.begin_path();
    ctx.ellipse(0, -12, 10, 7);
    ctx.fill();
}

void flee_jungle(Renderer &ctx, float anim, uint32_t seed) {
    float b = breath(anim, seed);
    ctx.set_fill(0xff3A6A2A);
    ctx.begin_path();
    ctx.ellipse(0, 8, 22, 20);
    ctx.ellipse(-14, 0 + b, 14, 18);
    ctx.ellipse(14, 2 - b, 13, 16);
    ctx.fill();
    ctx.set_fill(0xffC45A6A);
    ctx.begin_path();
    ctx.ellipse(0, -10, 12, 8);
    ctx.fill();
    eyes(ctx, 4, 4, 2.f);
}

void flee_ant(Renderer &ctx, float anim, uint32_t seed) {
    float b = breath(anim, seed);
    ctx.set_fill(0xff8E6244);
    ctx.begin_path();
    ctx.ellipse(0, 16, 28, 18);
    ctx.ellipse(0, 4 + b, 20, 14);
    ctx.fill();
    ctx.set_fill(0xffAD7852);
    ctx.begin_path();
    ctx.ellipse(0, -6, 12, 10);
    ctx.fill();
    // Egg hint
    ctx.set_stroke(0xff5A3D22);
    ctx.set_line_width(2);
    ctx.begin_path();
    ctx.ellipse(8, 8, 6, 8);
    ctx.stroke();
}

void flee_volcano(Renderer &ctx, float anim, uint32_t seed) {
    float p = pulse(anim, seed);
    ctx.set_fill(0xff5A585E);
    ctx.begin_path();
    ctx.ellipse(0, 12, 26, 14);
    ctx.ellipse(-8, 2, 16, 12);
    ctx.ellipse(10, 0, 14, 10);
    ctx.fill();
    ctx.set_stroke(0xffE66C40);
    ctx.set_global_alpha(0.5f + 0.3f * p);
    ctx.set_line_width(3);
    ctx.begin_path();
    ctx.move_to(-6, -4); ctx.line_to(2, 8); ctx.line_to(-4, 16);
    ctx.stroke();
    ctx.set_global_alpha(1.f);
}

void flee_royal(Renderer &ctx, float anim, uint32_t seed) {
    float b = breath(anim, seed) * 0.08f;
    ctx.rotate(b);
    ctx.set_fill(0xff8A3040);
    ctx.begin_path();
    ctx.move_to(-4, -36); ctx.line_to(28, -28); ctx.line_to(28, 8); ctx.line_to(-4, 0);
    ctx.close_path();
    ctx.fill();
    ctx.set_stroke(0xffC9A227);
    ctx.set_line_width(4);
    ctx.begin_path();
    ctx.move_to(-4, -36); ctx.line_to(-4, 36);
    ctx.stroke();
    ctx.set_fill(0xffC9A227);
    ctx.begin_path();
    ctx.arc(-4, -36, 5);
    ctx.fill();
    ctx.rotate(-b);
}

void draw_flee(Renderer &ctx, uint8_t map_id, float anim, uint32_t seed) {
    switch (map_id) {
    case MapID::kGarden: flee_garden(ctx, anim, seed); break;
    case MapID::kDesert: flee_desert(ctx, anim, seed); break;
    case MapID::kOcean: flee_ocean(ctx, anim, seed); break;
    case MapID::kPolar: flee_polar(ctx, anim, seed); break;
    case MapID::kJungle: flee_jungle(ctx, anim, seed); break;
    case MapID::kAntHell: flee_ant(ctx, anim, seed); break;
    case MapID::kVolcano: flee_volcano(ctx, anim, seed); break;
    case MapID::kRoyalCapital: flee_royal(ctx, anim, seed); break;
    default: flee_garden(ctx, anim, seed); break;
    }
}

void height_mushroom(Renderer &ctx, float s) {
    ctx.scale(s, s);
    ctx.set_fill(0xffF0E8D0);
    ctx.begin_path();
    ctx.round_rect(-5, -3, 10, 17, 4);
    ctx.fill();
    ctx.set_fill(0xffE07080);
    ctx.begin_path();
    ctx.ellipse(0, -8, 17, 10);
    ctx.fill();
    ctx.set_fill(0xffF8D0D8);
    ctx.begin_path();
    ctx.arc(-5, -10, 2.5f);
    ctx.arc(5, -8, 2.2f);
    ctx.fill();
    ctx.scale(1.f / s, 1.f / s);
}

void height_cactus(Renderer &ctx, float s) {
    ctx.scale(s, s);
    ctx.set_fill(0xff5A9A4A);
    ctx.begin_path();
    ctx.round_rect(-7, -22, 14, 44, 7);
    ctx.fill();
    ctx.set_stroke(0xff5A9A4A);
    ctx.set_line_width(8);
    ctx.round_line_cap();
    ctx.begin_path();
    ctx.move_to(-7, -4); ctx.line_to(-18, -4); ctx.line_to(-18, -16);
    ctx.move_to(7, 4); ctx.line_to(18, 4); ctx.line_to(18, -8);
    ctx.stroke();
    ctx.set_fill(0xffE07080);
    for (int i = 0; i < 5; ++i) {
        float a = (float) i * 1.256f;
        ctx.begin_path();
        ctx.arc(cosf(a) * 5, -26 + sinf(a) * 5, 3);
        ctx.fill();
    }
    ctx.set_fill(0xffF6BA59);
    ctx.begin_path();
    ctx.arc(0, -26, 3);
    ctx.fill();
    ctx.scale(1.f / s, 1.f / s);
}

void height_shell(Renderer &ctx, float s) {
    ctx.scale(s, s);
    ctx.set_fill(0xffF5D58B);
    ctx.set_stroke(0xffB89550);
    ctx.set_line_width(2);
    ctx.begin_path();
    ctx.move_to(0, 16);
    ctx.qcurve_to(-22, 0, -18, -14);
    ctx.qcurve_to(0, -22, 18, -14);
    ctx.qcurve_to(22, 0, 0, 16);
    ctx.close_path();
    ctx.fill();
    ctx.stroke();
    ctx.set_stroke(0xffB89550);
    ctx.set_line_width(1.5f);
    for (int i = -2; i <= 2; ++i) {
        ctx.begin_path();
        ctx.move_to(0, 12);
        ctx.qcurve_to((float) i * 6, 0, (float) i * 10, -10);
        ctx.stroke();
    }
    ctx.set_fill(0xffFFF8EC);
    ctx.begin_path();
    ctx.arc(0, 12, 5);
    ctx.fill();
    ctx.scale(1.f / s, 1.f / s);
}

void height_ice(Renderer &ctx, float s) {
    ctx.scale(s, s);
    ctx.set_fill(0xffBDE8F4);
    ctx.set_stroke(0xff557D99);
    ctx.set_line_width(2.5f);
    ctx.begin_path();
    ctx.move_to(0, -25); ctx.line_to(13, -12); ctx.line_to(13, 15);
    ctx.line_to(0, 27); ctx.line_to(-13, 15); ctx.line_to(-13, -12);
    ctx.close_path();
    ctx.fill();
    ctx.stroke();
    ctx.set_stroke(0xffE6F6FA);
    ctx.set_line_width(2);
    ctx.begin_path();
    ctx.move_to(0, -25); ctx.line_to(0, 27);
    ctx.move_to(-6, -8); ctx.line_to(-6, 10);
    ctx.move_to(6, -4); ctx.line_to(6, 12);
    ctx.stroke();
    ctx.scale(1.f / s, 1.f / s);
}

void height_resin(Renderer &ctx, float s) {
    ctx.scale(s, s);
    ctx.set_fill(0xffE8D5A3);
    ctx.begin_path();
    ctx.round_rect(-5, -3, 10, 17, 4);
    ctx.fill();
    ctx.set_fill(0xff6B8F4E);
    ctx.begin_path();
    ctx.ellipse(0, -8, 17, 10);
    ctx.fill();
    // Amber resin drops (distinct from garden mushrooms).
    ctx.set_fill(0xffC9A227);
    ctx.begin_path();
    ctx.ellipse(-4, -6, 3, 4);
    ctx.ellipse(5, -10, 2.5f, 3.5f);
    ctx.fill();
    ctx.scale(1.f / s, 1.f / s);
}

void height_fungus(Renderer &ctx, float s) {
    ctx.scale(s, s);
    ctx.set_fill(0xffC9B890);
    ctx.begin_path();
    ctx.ellipse(0, 8, 14, 10);
    ctx.fill();
    ctx.set_fill(0xff8E9A60);
    ctx.begin_path();
    ctx.ellipse(0, -4, 16, 12);
    ctx.fill();
    ctx.set_fill(0xffF0E8D0);
    ctx.begin_path();
    ctx.arc(-4, -6, 2);
    ctx.arc(5, -2, 2.5f);
    ctx.fill();
    ctx.scale(1.f / s, 1.f / s);
}

void height_sulfur(Renderer &ctx, float s) {
    ctx.scale(s, s);
    ctx.set_fill(0xffE8C840);
    ctx.set_stroke(0xffA08020);
    ctx.set_line_width(2);
    ctx.begin_path();
    ctx.move_to(0, -22); ctx.line_to(14, -4); ctx.line_to(8, 20);
    ctx.line_to(-8, 20); ctx.line_to(-14, -4);
    ctx.close_path();
    ctx.fill();
    ctx.stroke();
    ctx.set_fill(0xffF6BA59);
    ctx.begin_path();
    ctx.arc(0, 0, 5);
    ctx.fill();
    ctx.scale(1.f / s, 1.f / s);
}

void height_crest(Renderer &ctx, float s) {
    ctx.scale(s, s);
    ctx.set_fill(0xff5A5A5E);
    ctx.set_stroke(0xffC9A227);
    ctx.set_line_width(3);
    ctx.begin_path();
    ctx.move_to(0, -24); ctx.line_to(18, -8); ctx.line_to(14, 20);
    ctx.line_to(0, 28); ctx.line_to(-14, 20); ctx.line_to(-18, -8);
    ctx.close_path();
    ctx.fill();
    ctx.stroke();
    ctx.set_fill(0xffC9A227);
    ctx.begin_path();
    ctx.arc(0, 0, 7);
    ctx.fill();
    ctx.scale(1.f / s, 1.f / s);
}

void draw_height(Renderer &ctx, uint8_t map_id, float anim, uint32_t seed,
                 std::string_view name) {
    (void) anim; (void) seed;
    float s = height_scale(name);
    switch (map_id) {
    case MapID::kDesert: height_cactus(ctx, s); break;
    case MapID::kOcean: height_shell(ctx, s); break;
    case MapID::kPolar: height_ice(ctx, s); break;
    case MapID::kJungle: height_resin(ctx, s); break;
    case MapID::kAntHell: height_fungus(ctx, s); break;
    case MapID::kVolcano: height_sulfur(ctx, s); break;
    case MapID::kRoyalCapital: height_crest(ctx, s); break;
    default: height_mushroom(ctx, s); break;
    }
}

void draw_lore(Renderer &ctx, uint8_t map_id, float anim, uint32_t seed,
               std::string_view name) {
    float p = pulse(anim, seed);
    float b = breath(anim, seed);
    ctx.set_global_alpha(0.78f + 0.18f * p);

    if (name_has(name, "脚印") || name_has(name, "足迹") || name_has(name, "Print")
        || name_has(name, "蟹迹") || name_has(name, "猴迹")) {
        // Footprints / prints along a slight curve.
        uint32_t c = map_id == MapID::kPolar ? 0xa0BDE8F4
                   : map_id == MapID::kJungle ? 0xa06FAF4A : 0xa0ffffff;
        ctx.set_fill(c);
        int n = name_has(name, "猴") ? 3 : 5;
        for (int i = 0; i < n; ++i) {
            float t = (float) i / (float) (n - 1);
            float x = -22.f + t * 44.f;
            float y = sinf(t * 3.f + seed) * 8.f + ((i & 1) ? 5.f : -5.f);
            ctx.begin_path();
            ctx.ellipse(x, y, 6, 3.5f);
            ctx.fill();
            if (name_has(name, "猴")) {
                ctx.begin_path();
                ctx.arc(x - 3, y - 4, 1.8f);
                ctx.arc(x, y - 5, 1.8f);
                ctx.arc(x + 3, y - 4, 1.8f);
                ctx.fill();
            }
        }
    } else if (name_has(name, "引路猴")) {
        // Mini guide monkey (unique lore prop).
        ctx.set_fill(0xffC9A16D);
        ctx.begin_path();
        ctx.arc(0, -18, 19);
        ctx.fill();
        ctx.set_fill(0xffAD7852);
        ctx.begin_path();
        ctx.arc(-19, -17, 8);
        ctx.arc(19, -17, 8);
        ctx.fill();
        ctx.set_fill(0xffB8956A);
        ctx.begin_path();
        ctx.ellipse(0, 14 + b, 17, 23);
        ctx.fill();
        eyes(ctx, 7, -18, 3.f);
        ctx.set_stroke(0xff6B4E2E);
        ctx.set_line_width(4);
        ctx.begin_path();
        ctx.move_to(14, 25); ctx.qcurve_to(34, 11 + b * 2, 28, 0);
        ctx.stroke();
    } else if (name_has(name, "石碑") || name_has(name, "Stele") || name_has(name, "碑")
               || name_has(name, "君王余烬") || name_has(name, "皇家石碑")) {
        uint32_t body = map_id == MapID::kVolcano ? 0xff45424A
                      : map_id == MapID::kDesert ? 0xffC4A060 : 0xff8A8A90;
        ctx.set_fill(body);
        ctx.set_stroke(0xff4A4A50);
        ctx.set_line_width(2.5f);
        ctx.begin_path();
        ctx.round_rect(-18, -30, 36, 54, 4);
        ctx.fill();
        ctx.stroke();
        // Broken top
        ctx.set_fill(0xff3A3A40);
        ctx.begin_path();
        ctx.move_to(-18, -30); ctx.line_to(-4, -38); ctx.line_to(10, -30);
        ctx.close_path();
        ctx.fill();
        ctx.set_fill(map_id == MapID::kVolcano ? 0xffF6BA59 : 0xffC9A227);
        if (map_id == MapID::kRoyalCapital || name_has(name, "皇家")) {
            ctx.begin_path();
            ctx.move_to(0, -16); ctx.line_to(8, -8); ctx.line_to(4, 2);
            ctx.line_to(-4, 2); ctx.line_to(-8, -8);
            ctx.close_path();
            ctx.fill();
        } else if (map_id == MapID::kVolcano) {
            ctx.set_stroke(0xffF6BA59);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.move_to(0, -18); ctx.line_to(-6, 4); ctx.line_to(0, 12); ctx.line_to(8, 0);
            ctx.stroke();
        } else {
            ctx.begin_path();
            ctx.arc(0, -6, 6);
            ctx.fill();
        }
    } else if (name_has(name, "旗帜") == 0 && (name_has(name, "风痕") || name_has(name, "沙尘")
               || name_has(name, "光纹") || name_has(name, "潮纹") || name_has(name, "前兆")
               || name_has(name, "飘落") || name_has(name, "声波") || name_has(name, "回声"))) {
        // Wind / wave / ripple trails
        uint32_t c = map_id == MapID::kOcean ? 0xff7CCDDC
                   : map_id == MapID::kPolar ? 0xffE6F6FA
                   : map_id == MapID::kVolcano ? 0xffA0A0A8
                   : map_id == MapID::kDesert ? 0xffD8A45D : 0xffC9A227;
        ctx.set_stroke(c);
        ctx.set_line_width(2.5f);
        for (int i = 0; i < 3; ++i) {
            float y = -12.f + i * 12.f;
            float ox = b * 4.f + i * 3.f;
            ctx.begin_path();
            ctx.move_to(-28 + ox, y);
            ctx.qcurve_to(0 + ox, y - 8, 28 + ox, y);
            ctx.stroke();
        }
        ctx.set_fill(c);
        for (int i = 0; i < 4; ++i) {
            ctx.begin_path();
            ctx.arc(-10.f + i * 8.f + b * 2, (float) ((i & 1) * 10 - 5), 2.5f);
            ctx.fill();
        }
    } else if (name_has(name, "裂纹") || name_has(name, "裂缝") || name_has(name, "预警")
               || name_has(name, "落石")) {
        ctx.set_stroke(map_id == MapID::kVolcano ? 0xffE66C40 : 0xff91CFE8);
        ctx.set_line_width(3);
        ctx.begin_path();
        ctx.move_to(0, 0);
        ctx.line_to(-18, -16); ctx.move_to(0, 0); ctx.line_to(20, -10);
        ctx.move_to(0, 0); ctx.line_to(-8, 22); ctx.move_to(0, 0); ctx.line_to(14, 18);
        ctx.move_to(0, 0); ctx.line_to(-22, 8);
        ctx.stroke();
        if (map_id == MapID::kVolcano || name_has(name, "预警")) {
            ctx.set_fill(0xffE66C40);
            ctx.begin_path();
            ctx.move_to(0, -10); ctx.line_to(8, 6); ctx.line_to(-8, 6);
            ctx.close_path();
            ctx.fill();
        }
        if (name_has(name, "落石")) {
            ctx.set_fill(0xff6A656E);
            ctx.begin_path();
            ctx.ellipse(-12, 16, 8, 6);
            ctx.ellipse(6, 20, 10, 7);
            ctx.ellipse(16, 10, 6, 5);
            ctx.fill();
        }
    } else if (name_has(name, "气泡") || name_has(name, "Bubble")) {
        ctx.set_stroke(0xff7CCDDC);
        ctx.set_line_width(2);
        float rs[3] = { 8.f, 14.f, 20.f };
        for (int i = 0; i < 3; ++i) {
            float r = rs[i] * (0.9f + 0.1f * p);
            ctx.begin_path();
            ctx.arc(0, 8 - i * 4, r);
            ctx.stroke();
        }
    } else if (name_has(name, "萤火") || name_has(name, "Firefly")) {
        ctx.set_fill(0xa0A6D78A);
        for (int i = 0; i < 7; ++i) {
            float a = (float) i * 0.9f + anim * 0.5f;
            float x = cosf(a) * (12.f + (i % 3) * 6.f);
            float y = sinf(a * 1.3f) * 16.f;
            ctx.begin_path();
            ctx.arc(x, y, 2.5f + (i & 1));
            ctx.fill();
        }
    } else if (name_has(name, "帐篷") || name_has(name, "休息") || name_has(name, "Camp")) {
        ctx.set_fill(0xffC89256);
        ctx.begin_path();
        ctx.move_to(0, -28); ctx.line_to(26, 18); ctx.line_to(-26, 18);
        ctx.close_path();
        ctx.fill();
        ctx.set_stroke(0xff6B4E2E);
        ctx.set_line_width(3);
        ctx.begin_path();
        ctx.move_to(0, -28); ctx.line_to(0, 22);
        ctx.stroke();
        ctx.set_fill(0xffA97449);
        ctx.begin_path();
        ctx.ellipse(18, 20, 8, 6);
        ctx.fill();
    } else if (name_has(name, "回响") || name_has(name, "王冠") || name_has(name, "Echo")) {
        ctx.set_global_alpha(0.45f + 0.2f * p);
        ctx.set_stroke(0xffC9A227);
        ctx.set_line_width(3);
        ctx.begin_path();
        ctx.move_to(-20, 0); ctx.line_to(-12, -18); ctx.line_to(0, -8);
        ctx.line_to(12, -18); ctx.line_to(20, 0); ctx.line_to(12, 4); ctx.line_to(-12, 4);
        ctx.close_path();
        ctx.stroke();
        ctx.begin_path();
        ctx.arc(0, 16, 18);
        ctx.arc(0, 16, 26);
        ctx.stroke();
    } else if (name_has(name, "游行") || name_has(name, "运输") || name_has(name, "迁徙")
               || name_has(name, "Parade") || name_has(name, "Caravan")) {
        ctx.set_stroke(0xffC9A227);
        ctx.set_line_width(2);
        ctx.begin_path();
        ctx.move_to(-30, 8); ctx.qcurve_to(0, -4, 30, 8);
        ctx.stroke();
        ctx.set_fill(map_id == MapID::kAntHell ? 0xff8E6244 : 0xffC9A227);
        for (int i = 0; i < 5; ++i) {
            float x = -24.f + i * 12.f;
            ctx.begin_path();
            if (map_id == MapID::kAntHell) {
                ctx.ellipse(x, 0, 4, 3);
                ctx.ellipse(x + 5, 0, 3, 2.5f);
                ctx.ellipse(x + 9, 0, 3.5f, 3);
            } else {
                // Tiny banner marks
                ctx.move_to(x, -6); ctx.line_to(x + 8, -2); ctx.line_to(x, 2);
                ctx.close_path();
            }
            ctx.fill();
        }
    } else if (name_has(name, "古树") || name_has(name, "树轮") || name_has(name, "Elder")) {
        ctx.set_stroke(0xff99734D);
        ctx.set_line_width(3);
        ctx.begin_path();
        ctx.arc(0, 0, 28);
        ctx.arc(0, 0, 18);
        ctx.arc(0, 0, 10);
        ctx.stroke();
        ctx.set_fill(0xff4A8A3A);
        ctx.begin_path();
        ctx.arc(-4, -2, 3);
        ctx.arc(4, -2, 3);
        ctx.fill();
    } else if (name_has(name, "巢芯") || name_has(name, "Hive") || name_has(name, "甲壳")) {
        ctx.set_fill(0xff8E6244);
        ctx.begin_path();
        for (int i = 0; i < 6; ++i) {
            float a = (float) i * (float) M_PI / 3.f;
            float x = cosf(a) * 20, y = sinf(a) * 20;
            if (i == 0) ctx.move_to(x, y); else ctx.line_to(x, y);
        }
        ctx.close_path();
        ctx.fill();
        ctx.set_fill(0xff6FAF4A);
        ctx.begin_path();
        ctx.arc(0, 0, 8);
        ctx.fill();
    } else if (name_has(name, "指引") || name_has(name, "路标")) {
        ctx.set_fill(0xff5A5A5E);
        ctx.set_stroke(0xffC9A227);
        ctx.set_line_width(2.5f);
        ctx.begin_path();
        ctx.move_to(0, -22); ctx.line_to(16, -6); ctx.line_to(12, 22);
        ctx.line_to(0, 28); ctx.line_to(-12, 22); ctx.line_to(-16, -6);
        ctx.close_path();
        ctx.fill();
        ctx.stroke();
        ctx.set_fill(0xffA86B3C);
        ctx.begin_path();
        ctx.round_rect(-8, -4, 16, 12, 2);
        ctx.fill();
    } else if (name_has(name, "花圈") || name_has(name, "Ring")) {
        for (int i = 0; i < 8; ++i) {
            float a = (float) i * (float) M_PI / 4.f;
            float r = 22.f + (i & 1) * 6.f;
            ctx.set_fill((i & 1) ? 0xffE07080 : 0xffF0D050);
            ctx.begin_path();
            ctx.ellipse(cosf(a) * r, sinf(a) * r, 6, 4);
            ctx.fill();
        }
    } else {
        // Default seed / scrap / ember by map
        uint32_t c = 0xffC9A227;
        switch (map_id) {
        case MapID::kDesert: c = 0xffD8A45D; break;
        case MapID::kOcean: c = 0xff7CCDDC; break;
        case MapID::kPolar: c = 0xffBDE8F4; break;
        case MapID::kJungle: c = 0xff6FAF4A; break;
        case MapID::kAntHell: c = 0xffAD7852; break;
        case MapID::kVolcano: c = 0xffE66C40; break;
        case MapID::kRoyalCapital: c = 0xffC9A227; break;
        default: break;
        }
        ctx.set_fill(c);
        ctx.begin_path();
        ctx.ellipse(0, 2, 13, 17);
        ctx.fill();
        if (map_id == MapID::kVolcano || name_has(name, "余烬")) {
            ctx.set_fill(0xffF6BA59);
            ctx.begin_path();
            ctx.arc(0, 2, 6);
            ctx.fill();
            ctx.set_stroke(0xff8B2A18);
            ctx.set_line_width(2);
            ctx.begin_path();
            ctx.move_to(-8, -6); ctx.line_to(-2, 4);
            ctx.move_to(6, -8); ctx.line_to(2, 6);
            ctx.move_to(-4, 12); ctx.line_to(8, 8);
            ctx.stroke();
        } else {
            ctx.set_fill(0xff6FAF4A);
            ctx.begin_path();
            ctx.ellipse(5, -14, 9, 5);
            ctx.fill();
            if (name_has(name, "藤")) {
                ctx.set_stroke(0xff4A8A3A);
                ctx.set_line_width(2.5f);
                ctx.begin_path();
                ctx.arc(0, 2, 16);
                ctx.stroke();
            }
        }
    }
    ctx.set_global_alpha(1.f);
}

} // namespace

Visual resolve(MobID::T mob_id, std::string_view name, uint8_t map_id) {
    (void) map_id;
    if (mob_id == MobID::kYahahaPad) {
        if (name_has(name, "草丛") || name_has(name, "沙堆") || name_has(name, "沙丘")
            || name_has(name, "海草") || name_has(name, "雪堆") || name_has(name, "灌木")
            || (name_has(name, "蘑菇") && name_has(name, "伪装"))
            || name_has(name, "蚁丘") || name_has(name, "虫卵") || name_has(name, "灰堆")
            || name_has(name, "裂缝") || name_has(name, "旗帜") || name_has(name, "木箱影")
            || name_has(name, "Bush") || name_has(name, "Dune") || name_has(name, "Kelp")
            || name_has(name, "Banner"))
            return Visual::EventFlee;
        if (name_has(name, "菇") || name_has(name, "仙人掌") || name_has(name, "贝壳")
            || name_has(name, "冰晶") || name_has(name, "菌") || name_has(name, "硫")
            || name_has(name, "纹章") || name_has(name, "树脂菇")
            || name_has(name, "Mushroom") || name_has(name, "Cactus") || name_has(name, "Shell")
            || name_has(name, "Crest"))
            return Visual::EventHeight;
        if (!name.empty()
            && (name_has(name, "种子") || name_has(name, "残片") || name_has(name, "石碑")
                || name_has(name, "脚印") || name_has(name, "足迹") || name_has(name, "风痕")
                || name_has(name, "光纹") || name_has(name, "潮纹") || name_has(name, "遗迹")
                || name_has(name, "帐篷") || name_has(name, "休息") || name_has(name, "余烬")
                || name_has(name, "圣印") || name_has(name, "回响") || name_has(name, "花粉")
                || name_has(name, "沙尘") || name_has(name, "前兆") || name_has(name, "裂纹")
                || name_has(name, "寒光") || name_has(name, "藤种") || name_has(name, "古树")
                || name_has(name, "萤火") || name_has(name, "猴") || name_has(name, "巢芯")
                || name_has(name, "回声") || name_has(name, "落石") || name_has(name, "运输")
                || name_has(name, "迁徙") || name_has(name, "预警") || name_has(name, "气泡")
                || name_has(name, "飘落") || name_has(name, "游行") || name_has(name, "指引")
                || name_has(name, "花圈") || name_has(name, "蟹迹")
                || name_has(name, "Seed") || name_has(name, "Stele") || name_has(name, "Scrap")
                || name_has(name, "Echo") || name_has(name, "Camp") || name_has(name, "Omen")))
            return Visual::EventLore;
        // Timed marks / plain pads → themed pad.
        return Visual::Pad;
    }
    if (mob_id == MobID::kPushCrate) return Visual::Crate;
    if (mob_id == MobID::kLootChest) {
        if (name_has(name, "蜃楼") || name_has(name, "Mirage") || name_has(name, "幻影"))
            return Visual::Mirage;
        return Visual::Chest;
    }
    if (mob_id == MobID::kTreasureNpc) {
        if (name_has(name, "路旁叶语") || name_has(name, "沙路掮客")
            || name_has(name, "礁边向导") || name_has(name, "霜路残影")
            || name_has(name, "林缘贤影") || name_has(name, "穴道哨蚁")
            || name_has(name, "灰道囚人") || name_has(name, "街角掮客")
            || name_has(name, "Wayside") || name_has(name, "Sand Road")
            || name_has(name, "Reef Guide") || name_has(name, "Frost Road")
            || name_has(name, "Grove Wayside") || name_has(name, "Tunnel Scout")
            || name_has(name, "Ash Road") || name_has(name, "Street Broker"))
            return Visual::WaysideNpc;
        return Visual::MainNpc;
    }
    if (!name.empty() && (name_has(name, "迷路") || name_has(name, "掉队")
            || name_has(name, "Lost") || name_has(name, "Stray"))) {
        return Visual::Escort;
    }
    return Visual::None;
}

void draw(Renderer &ctx, Visual kind, uint8_t map_id, float radius,
          float animation, uint32_t seed, std::string_view name) {
    if (kind == Visual::None) return;
    // P3: slightly larger design scale for readable silhouettes at map zoom.
    float design_r = 46.f;
    if (kind == Visual::MainNpc || kind == Visual::WaysideNpc) design_r = 44.f;
    else if (kind == Visual::Pad || kind == Visual::EventLore) design_r = 42.f;
    else if (kind == Visual::Crate || kind == Visual::Chest || kind == Visual::Mirage)
        design_r = 34.f;
    else if (kind == Visual::Escort || kind == Visual::EventFlee
             || kind == Visual::EventHeight)
        design_r = 38.f;
    ctx.scale(radius / design_r);
    switch (kind) {
    case Visual::MainNpc: draw_main(ctx, map_id, animation, seed); break;
    case Visual::WaysideNpc: draw_wayside(ctx, map_id, animation, seed); break;
    case Visual::Escort: draw_escort(ctx, map_id, animation, seed); break;
    case Visual::Pad: draw_pad(ctx, map_id, animation, seed); break;
    case Visual::Crate: draw_crate(ctx, map_id, animation, seed); break;
    case Visual::Chest: chest_draw(ctx, map_id, 1.f, animation, seed); break;
    case Visual::Mirage: chest_draw(ctx, map_id, 0.52f, animation, seed); break;
    case Visual::EventFlee: draw_flee(ctx, map_id, animation, seed); break;
    case Visual::EventHeight: draw_height(ctx, map_id, animation, seed, name); break;
    case Visual::EventLore: draw_lore(ctx, map_id, animation, seed, name); break;
    default: break;
    }
}

}
