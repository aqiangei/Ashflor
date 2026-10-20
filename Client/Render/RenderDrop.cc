#include <Client/Render/RenderEntity.hh>

#include <Client/Render/Renderer.hh>

#include <Client/Assets/Assets.hh>

#include <Client/Game.hh>

#include <Shared/Entity.hh>
#include <Shared/StaticData.hh>

#include <cmath>
#include <cstdio>

void render_drop(Renderer &ctx, Entity const &ent) {
    float animation_value = sinf(Game::timestamp / 100);
    ctx.set_global_alpha(1 - ent.deletion_animation);
    ctx.rotate(M_PI * 2 * ent.deletion_animation);
    ctx.scale(1 - ent.deletion_animation);
    ctx.scale(1 + animation_value * 0.03);
    ctx.scale(ent.get_radius() / 30);
    ctx.set_fill(0x40000000);
    ctx.begin_path();
    ctx.round_rect(-33, -33, 66, 66, 4);
    ctx.fill();
    draw_loadout_background(ctx, ent.get_drop_id());
    uint16_t qty = ent.get_drop_count();
    if (qty > 1) {
        // Stacked pile badge (coins / multi-drops)
        char buf[16];
        if (qty >= 1000)
            std::snprintf(buf, sizeof(buf), "x%uk", (unsigned) (qty / 1000));
        else
            std::snprintf(buf, sizeof(buf), "x%u", (unsigned) qty);
        ctx.translate(18, -22);
        ctx.set_fill(0xe0000000);
        ctx.begin_path();
        ctx.round_rect(-16, -10, 32, 20, 8);
        ctx.fill();
        ctx.draw_text(buf, { .fill = 0xffffe066, .size = 14, .stroke_scale = 0.1f });
    }
}