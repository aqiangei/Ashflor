#include <Client/Ui/InGame/HudIcons.hh>

#include <Client/Ui/StaticIcon.hh>

#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace Ui {
namespace {

constexpr float kLine = 2.4f;

void setup_24(Renderer &ctx, float button_w) {
    float s = button_w / 24.f;
    ctx.scale(s);
    ctx.translate(-12.f, -12.f);
    ctx.set_fill(0xffffffff);
    ctx.set_stroke(0xffffffff);
    ctx.set_line_width(kLine);
    ctx.round_line_cap();
    ctx.round_line_join();
}

// Soft oval petal; tip points along +rot (local -Y before rot = tip direction).
void fill_petal(Renderer &ctx, float cx, float cy, float rx, float ry, float tip_angle) {
    ctx.begin_path();
    for (int i = 0; i <= 20; ++i) {
        float t = (float) i / 20.f * 2.f * (float) M_PI;
        float lx = sinf(t) * rx;
        float ly = -cosf(t) * ry;
        float x = cx + lx * cosf(tip_angle) - ly * sinf(tip_angle);
        float y = cy + lx * sinf(tip_angle) + ly * cosf(tip_angle);
        if (i == 0) ctx.move_to(x, y);
        else ctx.line_to(x, y);
    }
    ctx.close_path();
    ctx.fill();
}

void add_ellipse_hole(Renderer &ctx, float cx, float cy, float rx, float ry, float rot) {
    ctx.move_to(cx + cosf(rot) * rx, cy + sinf(rot) * rx);
    for (int i = 1; i <= 14; ++i) {
        float t = (float) i / 14.f * 2.f * (float) M_PI;
        float along = cosf(t) * rx;
        float across = sinf(t) * ry;
        float x = cx + along * cosf(rot) - across * sinf(rot);
        float y = cy + along * sinf(rot) + across * cosf(rot);
        ctx.line_to(x, y);
    }
    ctx.close_path();
}

} // namespace

void hud_icon_settings(Renderer &ctx) {
    float const cx = 12.f, cy = 12.f;
    float const r_rim = 7.0f;
    float const r_tooth = 9.5f;
    float const half = 0.28f;

    ctx.begin_path();
    for (int i = 0; i < 6; ++i) {
        float amid = - (float) M_PI / 2.f + i * (float) M_PI / 3.f;
        float a0 = amid - half;
        float a1 = amid + half;
        float a_gap0 = amid + half;
        float a_gap1 = amid + (float) M_PI / 3.f - half;
        if (i == 0) ctx.move_to(cx + cosf(a0) * r_rim, cy + sinf(a0) * r_rim);
        else ctx.line_to(cx + cosf(a0) * r_rim, cy + sinf(a0) * r_rim);
        ctx.line_to(cx + cosf(amid - 0.14f) * r_tooth, cy + sinf(amid - 0.14f) * r_tooth);
        ctx.line_to(cx + cosf(amid + 0.14f) * r_tooth, cy + sinf(amid + 0.14f) * r_tooth);
        ctx.line_to(cx + cosf(a1) * r_rim, cy + sinf(a1) * r_rim);
        for (int s = 1; s <= 3; ++s) {
            float t = a_gap0 + (a_gap1 - a_gap0) * (s / 3.f);
            ctx.line_to(cx + cosf(t) * r_rim, cy + sinf(t) * r_rim);
        }
    }
    ctx.close_path();
    // Hub hole
    ctx.move_to(cx + 4.0f, cy);
    ctx.partial_arc(cx, cy, 4.0f, 0, 2.f * (float) M_PI, 0);
    ctx.close_path();
    ctx.fill(); // evenodd

    ctx.begin_path();
    ctx.round_rect(cx - 2.7f, cy - 3.0f, 1.9f, 6.0f, 0.6f);
    ctx.round_rect(cx + 0.8f, cy - 3.0f, 1.9f, 6.0f, 0.6f);
    ctx.fill();
}

void hud_icon_boost(Renderer &ctx) {
    fill_petal(ctx, 8.0f, 13.8f, 4.0f, 6.0f, 0.45f);

    ctx.set_line_width(2.5f);
    ctx.begin_path();
    ctx.move_to(16.0f, 17.0f);
    ctx.line_to(16.0f, 7.5f);
    ctx.move_to(12.6f, 11.2f);
    ctx.line_to(16.0f, 6.8f);
    ctx.line_to(19.4f, 11.2f);
    ctx.stroke();

    // Tiny spark
    ctx.begin_path();
    ctx.move_to(16.0f, 3.8f);
    ctx.line_to(16.6f, 5.3f);
    ctx.line_to(18.1f, 5.9f);
    ctx.line_to(16.6f, 6.5f);
    ctx.line_to(16.0f, 8.0f);
    ctx.line_to(15.4f, 6.5f);
    ctx.line_to(13.9f, 5.9f);
    ctx.line_to(15.4f, 5.3f);
    ctx.close_path();
    ctx.fill();
}

void hud_icon_guild(Renderer &ctx) {
    float const px = 5.8f;
    ctx.begin_path();
    // Banner with swallowtail
    ctx.move_to(px + 0.8f, 4.2f);
    ctx.line_to(19.2f, 4.2f);
    ctx.line_to(19.2f, 14.0f);
    ctx.line_to(14.6f, 17.6f);
    ctx.line_to(12.5f, 15.0f);
    ctx.line_to(10.4f, 17.6f);
    ctx.line_to(px + 0.8f, 14.0f);
    ctx.close_path();
    add_ellipse_hole(ctx, 11.0f, 9.6f, 3.0f, 1.45f, -0.75f);
    add_ellipse_hole(ctx, 15.2f, 9.6f, 3.0f, 1.45f, 0.75f);
    ctx.fill();

    ctx.set_line_width(2.5f);
    ctx.begin_path();
    ctx.move_to(px, 3.5f);
    ctx.line_to(px, 20.8f);
    ctx.stroke();
}

void hud_icon_mob_gallery(Renderer &ctx) {
    // Pages
    ctx.begin_path();
    ctx.move_to(12.0f, 12.0f);
    ctx.line_to(3.2f, 10.5f);
    ctx.line_to(2.8f, 20.0f);
    ctx.line_to(11.2f, 20.8f);
    ctx.close_path();
    ctx.move_to(12.0f, 12.0f);
    ctx.line_to(20.8f, 10.5f);
    ctx.line_to(21.2f, 20.0f);
    ctx.line_to(12.8f, 20.8f);
    ctx.close_path();
    ctx.fill();

    ctx.set_line_width(2.3f);
    ctx.begin_path();
    ctx.move_to(12.0f, 12.2f);
    ctx.line_to(12.0f, 20.5f);
    ctx.stroke();

    // Bug head + eye holes (evenodd)
    ctx.begin_path();
    ctx.arc(12.0f, 8.0f, 3.7f);
    ctx.close_path();
    ctx.move_to(10.6f + 0.9f, 7.8f);
    ctx.partial_arc(10.6f, 7.8f, 0.9f, 0, 2.f * (float) M_PI, 0);
    ctx.close_path();
    ctx.move_to(13.4f + 0.9f, 7.8f);
    ctx.partial_arc(13.4f, 7.8f, 0.9f, 0, 2.f * (float) M_PI, 0);
    ctx.close_path();
    ctx.fill();

    ctx.set_line_width(2.3f);
    ctx.begin_path();
    ctx.move_to(10.3f, 5.2f);
    ctx.line_to(8.6f, 3.0f);
    ctx.move_to(13.7f, 5.2f);
    ctx.line_to(15.4f, 3.0f);
    ctx.stroke();
}

void hud_icon_petal_gallery(Renderer &ctx) {
    // Card with center hole so flower hub reads as cutout to the button.
    ctx.begin_path();
    ctx.round_rect(5.0f, 3.2f, 14.0f, 17.6f, 2.2f);
    ctx.close_path();
    ctx.move_to(12.0f + 2.3f, 12.0f);
    ctx.partial_arc(12.0f, 12.0f, 2.3f, 0, 2.f * (float) M_PI, 0);
    ctx.close_path();
    ctx.fill();

    for (int i = 0; i < 5; ++i) {
        float a = i * (2.f * (float) M_PI / 5.f) - (float) M_PI / 2.f;
        float px = 12.0f + cosf(a) * 4.2f;
        float py = 12.0f + sinf(a) * 4.2f;
        fill_petal(ctx, px, py, 2.6f, 3.8f, a + (float) M_PI / 2.f);
    }
}

void hud_icon_leaderboard(Renderer &ctx) {
    ctx.begin_path();
    ctx.move_to(6.8f, 4.2f);
    ctx.line_to(17.2f, 4.2f);
    ctx.line_to(16.0f, 11.2f);
    ctx.qcurve_to(12.0f, 14.2f, 8.0f, 11.2f);
    ctx.close_path();
    ctx.fill();

    ctx.set_line_width(2.4f);
    ctx.begin_path();
    ctx.partial_arc(6.2f, 7.4f, 2.7f, (float) M_PI * 0.55f, (float) M_PI * 1.45f, 0);
    ctx.stroke();
    ctx.begin_path();
    ctx.partial_arc(17.8f, 7.4f, 2.7f, (float) M_PI * -0.45f, (float) M_PI * 0.45f, 0);
    ctx.stroke();

    ctx.begin_path();
    ctx.round_rect(10.7f, 12.8f, 2.6f, 2.3f, 0.5f);
    ctx.round_rect(8.4f, 15.0f, 7.2f, 1.7f, 0.5f);
    ctx.fill();

    ctx.begin_path();
    ctx.round_rect(3.4f, 19.4f, 5.0f, 2.0f, 0.5f);
    ctx.round_rect(9.0f, 17.4f, 6.0f, 4.0f, 0.5f);
    ctx.round_rect(15.6f, 18.6f, 5.0f, 2.8f, 0.5f);
    ctx.fill();
}

void hud_icon_exit(Renderer &ctx) {
    ctx.set_line_width(2.5f);
    ctx.begin_path();
    ctx.move_to(9.8f, 4.0f);
    ctx.line_to(5.2f, 4.0f);
    ctx.line_to(5.2f, 20.0f);
    ctx.line_to(9.8f, 20.0f);
    ctx.stroke();

    ctx.begin_path();
    ctx.move_to(8.2f, 12.0f);
    ctx.line_to(18.8f, 12.0f);
    ctx.move_to(13.8f, 7.4f);
    ctx.line_to(19.4f, 12.0f);
    ctx.line_to(13.8f, 16.6f);
    ctx.stroke();
}

void hud_icon_inventory(Renderer &ctx) {
    ctx.begin_path();
    // Body
    ctx.move_to(4.2f, 9.8f);
    ctx.qcurve_to(4.2f, 8.2f, 5.8f, 8.2f);
    ctx.line_to(18.2f, 8.2f);
    ctx.qcurve_to(19.8f, 8.2f, 19.8f, 9.8f);
    ctx.line_to(19.8f, 18.8f);
    ctx.qcurve_to(19.8f, 20.8f, 17.8f, 20.8f);
    ctx.line_to(6.2f, 20.8f);
    ctx.qcurve_to(4.2f, 20.8f, 4.2f, 18.8f);
    ctx.close_path();
    // Flap
    ctx.move_to(4.6f, 9.2f);
    ctx.qcurve_to(12.0f, 4.8f, 19.4f, 9.2f);
    ctx.line_to(19.4f, 11.6f);
    ctx.qcurve_to(12.0f, 8.8f, 4.6f, 11.6f);
    ctx.close_path();
    // Clasp hole
    ctx.move_to(12.0f + 1.55f, 10.3f);
    ctx.partial_arc(12.0f, 10.3f, 1.55f, 0, 2.f * (float) M_PI, 0);
    ctx.close_path();
    ctx.fill();

    ctx.set_line_width(2.4f);
    ctx.begin_path();
    ctx.move_to(8.6f, 7.0f);
    ctx.qcurve_to(12.0f, 3.6f, 15.4f, 7.0f);
    ctx.stroke();
}

void hud_icon_talents(Renderer &ctx) {
    float const nodes[3][2] = {{12.f, 19.f}, {5.2f, 6.2f}, {18.8f, 6.2f}};
    ctx.set_line_width(2.6f);
    ctx.begin_path();
    ctx.move_to(nodes[0][0], nodes[0][1]);
    ctx.line_to(12.0f, 12.2f);
    ctx.line_to(nodes[1][0], nodes[1][1]);
    ctx.move_to(12.0f, 12.2f);
    ctx.line_to(nodes[2][0], nodes[2][1]);
    ctx.stroke();

    for (auto const &n : nodes) {
        ctx.begin_path();
        ctx.arc(n[0], n[1], 2.5f);
        ctx.fill();
    }
}

void hud_icon_craft(Renderer &ctx) {
    fill_petal(ctx, 6.4f, 7.0f, 2.5f, 3.5f, 0.5f);
    fill_petal(ctx, 6.4f, 17.0f, 2.5f, 3.5f, -0.5f);
    fill_petal(ctx, 17.2f, 12.0f, 3.5f, 5.0f, (float) M_PI * 0.5f);

    ctx.set_line_width(2.3f);
    ctx.begin_path();
    ctx.move_to(9.4f, 8.0f);
    ctx.qcurve_to(12.5f, 10.0f, 13.6f, 11.2f);
    ctx.move_to(9.4f, 16.0f);
    ctx.qcurve_to(12.5f, 14.0f, 13.6f, 12.8f);
    ctx.stroke();
}

void hud_icon_special_craft(Renderer &ctx) {
    ctx.set_line_width(2.6f);
    ctx.begin_path();
    ctx.move_to(5.8f, 18.6f);
    ctx.line_to(13.2f, 9.8f);
    ctx.stroke();

    ctx.begin_path();
    ctx.move_to(10.2f, 5.4f);
    ctx.line_to(18.6f, 5.4f);
    ctx.line_to(19.0f, 8.8f);
    ctx.line_to(16.0f, 9.2f);
    ctx.line_to(16.0f, 11.0f);
    ctx.line_to(12.6f, 11.0f);
    ctx.line_to(12.6f, 9.2f);
    ctx.line_to(9.8f, 8.8f);
    ctx.close_path();
    ctx.fill();

    ctx.set_line_width(2.2f);
    ctx.begin_path();
    ctx.move_to(17.2f, 12.0f);
    ctx.line_to(19.4f, 14.2f);
    ctx.stroke();

    fill_petal(ctx, 17.4f, 17.6f, 2.7f, 3.6f, 0.7f);
}

void hud_icon_shop(Renderer &ctx) {
    // Mini garden shop: scalloped 3-section awning + storefront with door cutout.
    // Awning body (top bar + scalloped underside) with sparse stripe gaps.
    ctx.begin_path();
    ctx.move_to(3.0f, 6.0f);
    ctx.line_to(21.0f, 6.0f);
    ctx.line_to(21.0f, 9.2f);
    // Three rounded scallops along y≈11 (centers ~6, 12, 18)
    ctx.qcurve_to(19.5f, 11.2f, 18.0f, 11.0f);
    ctx.qcurve_to(16.5f, 10.6f, 15.0f, 11.0f);
    ctx.qcurve_to(13.5f, 11.2f, 12.0f, 11.0f);
    ctx.qcurve_to(10.5f, 10.6f, 9.0f, 11.0f);
    ctx.qcurve_to(7.5f, 11.2f, 6.0f, 11.0f);
    ctx.qcurve_to(4.5f, 10.6f, 3.0f, 9.2f);
    ctx.close_path();
    // Two thin stripe cutouts (negative space) — sparse so 24px stays readable
    ctx.move_to(8.2f, 6.6f);
    ctx.line_to(9.4f, 6.6f);
    ctx.line_to(9.4f, 10.2f);
    ctx.line_to(8.2f, 10.2f);
    ctx.close_path();
    ctx.move_to(14.6f, 6.6f);
    ctx.line_to(15.8f, 6.6f);
    ctx.line_to(15.8f, 10.2f);
    ctx.line_to(14.6f, 10.2f);
    ctx.close_path();
    ctx.fill(); // evenodd

    // Storefront with doorway hole
    ctx.begin_path();
    ctx.round_rect(5.0f, 11.0f, 14.0f, 10.0f, 1.6f);
    ctx.close_path();
    // Door cutout (x 10–14, y 15–21)
    ctx.move_to(10.0f, 15.0f);
    ctx.line_to(14.0f, 15.0f);
    ctx.line_to(14.0f, 21.0f);
    ctx.line_to(10.0f, 21.0f);
    ctx.close_path();
    ctx.fill();
}

Element *make_hud_icon(void (*draw)(Renderer &), float size) {
    return new StaticIcon([draw](Element *elt, Renderer &ctx) {
        setup_24(ctx, elt->width);
        draw(ctx);
    }, size, size);
}

} // namespace Ui
