#include <Client/Render/RenderEntity.hh>

#include <Client/Assets/Assets.hh>
#include <Client/Render/Renderer.hh>

#include <Client/Game.hh>

#include <Helpers/Bits.hh>
#include <Shared/Entity.hh>
#include <Shared/StaticDefinitions.hh>

#include <cmath>

void render_flower(Renderer &ctx, Entity const &ent) {
    if (ent.has_component(kMob)) return;
    uint8_t face_flags = ent.get_face_flags();
    if (ent.deletion_animation > 0) BitMath::set(face_flags, FaceFlags::kDeadEyes);

    // Party revive corpse: Flower + DeadEyes, no Health. Tip it over and mute
    // the palette so teammates can spot the body at a glance.
    uint8_t const is_corpse =
        BitMath::at(face_flags, FaceFlags::kDeadEyes) && !ent.has_component(kHealth);
    if (is_corpse) {
        RenderContext pose(&ctx);
        // Soft ground shadow under the tipped body.
        {
            RenderContext shadow(&ctx);
            ctx.set_global_alpha(0.28f);
            ctx.set_fill(0xff000000);
            ctx.begin_path();
            ctx.ellipse(4.f, ent.get_radius() * 0.55f, ent.get_radius() * 1.05f, ent.get_radius() * 0.38f);
            ctx.fill();
        }
        ctx.set_global_alpha(0.88f);
        ctx.translate(0.f, ent.get_radius() * 0.12f);
        ctx.rotate(ent.get_angle()); // server sets ~-0.55 fallen tip
        ctx.scale(1.f, 0.78f); // squash → lying on the ground
        // Cool wash so corpse ≠ living flower at distance.
        ctx.add_color_filter(0xff4a5a6a, 0.35f);
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
