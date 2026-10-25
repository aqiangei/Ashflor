#include <Client/Render/RenderEntity.hh>

#include <Client/Assets/Assets.hh>
#include <Client/Render/Renderer.hh>

#include <Client/Game.hh>

#include <Helpers/Bits.hh>
#include <Shared/Entity.hh>
#include <Shared/StaticDefinitions.hh>

#include <cmath>

namespace {
// Party-revive marker: a rock beside the body so teammates spot downs at range.
void draw_downed_stone(Renderer &ctx, float flower_r) {
    float const r = flower_r * 0.92f;
    RenderContext stone(&ctx);
    // Sit slightly behind/beside the tipped flower.
    ctx.translate(flower_r * 1.05f, flower_r * 0.08f);
    ctx.set_fill(0xff8b8b8b);
    ctx.set_stroke(0xff4a4a4a);
    ctx.set_line_width(3.5f);
    ctx.round_line_cap();
    ctx.round_line_join();
    ctx.begin_path();
    ctx.move_to(r * 0.85f, r * 0.15f);
    ctx.line_to(r * 0.55f, -r * 0.70f);
    ctx.line_to(r * 0.05f, -r * 0.95f);
    ctx.line_to(-r * 0.55f, -r * 0.55f);
    ctx.line_to(-r * 0.90f, r * 0.05f);
    ctx.line_to(-r * 0.65f, r * 0.70f);
    ctx.line_to(r * 0.35f, r * 0.75f);
    ctx.close_path();
    ctx.fill();
    ctx.stroke();
    // Light facet so it reads as stone, not a flat blob.
    ctx.set_fill(0x55ffffff);
    ctx.begin_path();
    ctx.move_to(r * 0.15f, -r * 0.55f);
    ctx.line_to(r * 0.40f, -r * 0.25f);
    ctx.line_to(-r * 0.10f, -r * 0.15f);
    ctx.close_path();
    ctx.fill();
}
}

void render_flower(Renderer &ctx, Entity const &ent) {
    if (ent.has_component(kMob)) return;
    uint8_t face_flags = ent.get_face_flags();
    if (ent.deletion_animation > 0) BitMath::set(face_flags, FaceFlags::kDeadEyes);

    // Party revive corpse: Flower + DeadEyes, no Health.
    // Stone marker for range visibility; flower stays normal color with X eyes.
    uint8_t const is_corpse =
        BitMath::at(face_flags, FaceFlags::kDeadEyes) && !ent.has_component(kHealth);
    if (is_corpse) {
        RenderContext pose(&ctx);
        {
            RenderContext shadow(&ctx);
            ctx.set_global_alpha(0.28f);
            ctx.set_fill(0xff000000);
            ctx.begin_path();
            ctx.ellipse(4.f, ent.get_radius() * 0.55f,
                ent.get_radius() * 1.15f, ent.get_radius() * 0.40f);
            ctx.fill();
        }
        draw_downed_stone(ctx, ent.get_radius());
        ctx.translate(0.f, ent.get_radius() * 0.12f);
        ctx.rotate(ent.get_angle()); // server sets ~-0.55 fallen tip
        ctx.scale(1.f, 0.78f);
        draw_static_flower(ctx, {
            .radius = ent.get_radius(),
            .eye_x = ent.eye_x,
            .eye_y = ent.eye_y,
            .mouth = 5.f,
            .cutter_angle = 0.f,
            .face_flags = face_flags,
            .equip_flags = ent.get_equip_flags(),
            .color = ent.get_color()
        });
        return;
    }

    RenderContext effects(&ctx);
    if (ent.get_effect_flags() & FlowerEffects::kBurrow) {
        ctx.set_fill(0xff73513a);
        ctx.set_stroke(0xffc9a36a);
        ctx.set_line_width(3.f);
        ctx.begin_path();
        ctx.ellipse(0, ent.get_radius() * .5f, ent.get_radius() * 1.2f, ent.get_radius() * .45f);
        ctx.fill(); ctx.stroke();
        ctx.set_global_alpha(.35f);
        ctx.scale(.7f);
    } else if (ent.get_shield_ratio() > .001f) {
        ctx.set_stroke(0xff78c9ff);
        ctx.set_line_width(3.f);
        ctx.begin_path(); ctx.arc(0, 0, ent.get_radius() + 6.f); ctx.stroke();
    }
    // Lethal grace (Bandage firing): amber aura + thick layered gauze.
    if (ent.get_effect_flags() & FlowerEffects::kDeathGrace) {
        float pulse = 0.55f + 0.45f * sinf((float)Game::timestamp * 0.012f);
        float r = ent.get_radius();
        ctx.round_line_cap();
        ctx.round_line_join();
        // Outer amber glow ring
        ctx.set_global_alpha(0.28f + 0.32f * pulse);
        ctx.set_stroke(0xffffc878);
        ctx.set_line_width(5.5f);
        ctx.begin_path();
        ctx.arc(0, 0, r + 5.f + 4.f * pulse);
        ctx.stroke();
        // Inner warm rim
        ctx.set_global_alpha(0.45f + 0.25f * pulse);
        ctx.set_stroke(0xffffe0a0);
        ctx.set_line_width(2.4f);
        ctx.begin_path();
        ctx.arc(0, 0, r + 2.5f + 1.5f * pulse);
        ctx.stroke();
        ctx.set_global_alpha(1.f);
        // Layered bandage wraps (shadow → fill → seam)
        ctx.set_stroke(0x44000000);
        ctx.set_line_width(8.5f);
        ctx.begin_path();
        ctx.move_to(-r * 0.9f, -r * 0.12f);
        ctx.qcurve_to(0, -r * 1.12f, r * 0.9f, -r * 0.08f);
        ctx.move_to(-r * 0.85f, r * 0.22f);
        ctx.qcurve_to(0, r * 1.05f, r * 0.85f, r * 0.18f);
        ctx.move_to(-r * 0.55f, -r * 0.7f);
        ctx.qcurve_to(r * 0.35f, 0, r * 0.65f, r * 0.7f);
        ctx.stroke();
        ctx.set_stroke(0xfff7efe4);
        ctx.set_line_width(7.2f);
        ctx.begin_path();
        ctx.move_to(-r * 0.9f, -r * 0.12f);
        ctx.qcurve_to(0, -r * 1.12f, r * 0.9f, -r * 0.08f);
        ctx.move_to(-r * 0.85f, r * 0.22f);
        ctx.qcurve_to(0, r * 1.05f, r * 0.85f, r * 0.18f);
        ctx.move_to(-r * 0.55f, -r * 0.7f);
        ctx.qcurve_to(r * 0.35f, 0, r * 0.65f, r * 0.7f);
        ctx.stroke();
        ctx.set_stroke(0xffc8a878);
        ctx.set_line_width(1.8f);
        ctx.begin_path();
        ctx.move_to(-r * 0.9f, -r * 0.12f);
        ctx.qcurve_to(0, -r * 1.12f, r * 0.9f, -r * 0.08f);
        ctx.move_to(-r * 0.85f, r * 0.22f);
        ctx.qcurve_to(0, r * 1.05f, r * 0.85f, r * 0.18f);
        ctx.move_to(-r * 0.55f, -r * 0.7f);
        ctx.qcurve_to(r * 0.35f, 0, r * 0.65f, r * 0.7f);
        ctx.stroke();
        // Fray tips
        ctx.set_stroke(0xffd4c4a8);
        ctx.set_line_width(1.4f);
        ctx.begin_path();
        ctx.move_to(r * 0.9f, -r * 0.08f);
        ctx.line_to(r * 1.08f, -r * 0.18f);
        ctx.move_to(r * 0.92f, 0);
        ctx.line_to(r * 1.1f, 0.02f * r);
        ctx.move_to(r * 0.85f, r * 0.18f);
        ctx.line_to(r * 1.05f, r * 0.28f);
        ctx.stroke();
    }
    draw_static_flower(ctx, {
        .radius = ent.get_radius(),
        .eye_x = ent.eye_x,
        .eye_y = ent.eye_y,
        .mouth = ent.mouth,
        .cutter_angle = (float) (Game::timestamp / 200),
        .face_flags = face_flags,
        .equip_flags = ent.get_equip_flags(),
        .color = ent.get_color()
    });
}
