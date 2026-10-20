#include <Client/Render/RenderEntity.hh>

#include <Client/Render/Renderer.hh>

#include <Client/Assets/Assets.hh>

#include <Client/Particle.hh>

#include <Client/Ui/Ui.hh>

#include <Shared/Entity.hh>
#include <Shared/StaticData.hh>

#include <cmath>

void render_petal(Renderer &ctx, Entity const &ent) {
    PetalID::T const id = ent.get_petal_id();
    struct PetalData const &data = PETAL_DATA[id];
    // Rooted Monstera: soft green heal disc under the leaf (world units, before petal scale).
    if (data.attributes.aura_heal > 0 && ent.get_split_projectile()
        && data.attributes.aura_radius > 0) {
        RenderContext aura(&ctx);
        float const R = data.attributes.aura_radius;
        ctx.set_fill(0x2828A860);
        ctx.begin_path();
        ctx.arc(0, 0, R);
        ctx.fill();
        ctx.set_fill(0x1838B870);
        ctx.begin_path();
        ctx.arc(0, 0, R * 0.55f);
        ctx.fill();
        ctx.set_stroke(0x5540C878);
        ctx.set_line_width(2.5f);
        ctx.begin_path();
        ctx.arc(0, 0, R);
        ctx.stroke();
    }
    ctx.scale(ent.get_radius() / data.radius);
    // Monstera reuses split_projectile as "rooted" flag — always draw a single leaf.
    if (ent.get_split_projectile() && id != PetalID::kMonstera)
        draw_static_petal(id, ctx);
    else
        draw_static_petal_single(id, ctx);
    if (data.rarity == RarityID::kUnique && frand() < fclamp(0.25 * Ui::dt/16.67, 0, 1))
        Particle::add_unique_particle(ent.get_x(), ent.get_y());
}