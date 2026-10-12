#include <Client/Render/RenderEntity.hh>

#include <Client/Render/Renderer.hh>

#include <Shared/Entity.hh>

#include <string>

void render_name(Renderer &ctx, Entity const &ent) {
    if (!ent.get_nametag_visible()) return;
    std::string const &name = ent.get_name();
    if (name.empty()) return;
    ctx.translate(0, -ent.get_radius() - 18);
    ctx.set_global_alpha(1 - ent.deletion_animation);
    ctx.scale(1 + 0.5 * ent.deletion_animation);
    ctx.draw_text(name.c_str(), {
        .fill = 0xffffffff,
        .stroke = 0xff000000,
        .size = 18,
        .stroke_scale = 0.14f
    });
}