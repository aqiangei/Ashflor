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
