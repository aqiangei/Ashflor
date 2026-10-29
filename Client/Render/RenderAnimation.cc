#include <Client/Render/RenderEntity.hh>

#include <Client/Render/Renderer.hh>
#include <Client/Game.hh>

#include <Shared/Entity.hh>
#include <Shared/Simulation.hh>

#include <Helpers/Math.hh>
#include <Helpers/Vector.hh>

#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void render_animation(Simulation *sim, Renderer &ctx, Entity const &ent) {
    // Only roots draw; children hang off Relations.parent.
    if (sim->ent_exists(ent.get_parent())) {
        Entity const &p = sim->get_ent(ent.get_parent());
        if (p.has_component(kAnimation)) return;
    }

    auto find_child = [&](EntityID pid) -> Entity const * {
        Entity const *found = nullptr;
        sim->for_each<kAnimation>([&](Simulation *, Entity &e) {
            if (found != nullptr) return;
            if (e.id == ent.id) return;
            if (e.get_parent() == pid) found = &e;
        });
        return found;
    };

    ctx.set_global_alpha(0.9f * (1.f - ent.deletion_animation));
    ctx.set_stroke(0xffffffff);
    ctx.set_line_width(ent.get_radius() > 0.f ? ent.get_radius() : 3.5f);
    ctx.begin_path();
    ctx.move_to(ent.get_x(), ent.get_y());

    Entity const *last = &ent;
    uint8_t hops = 0;
    while (hops < 16) {
        Entity const *child = find_child(last->id);
        if (child == nullptr) break;
        Vector last_pos(last->get_x(), last->get_y());
        Vector curr_pos(child->get_x(), child->get_y());
        SeedGenerator gen((uint32_t)(ent.id.id * 374572u + last->id.id * 46237u + 918273u));
        for (;;) {
            Vector delta = curr_pos - last_pos;
            float magnitude = delta.magnitude();
            float disp = gen.binext() * 5.f + 30.f;
            if (disp >= magnitude) break;
            float angle = delta.angle() + gen.binext() * (float)M_PI / 6.f;
            Vector step;
            step.unit_normal(angle).set_magnitude(disp);
            Vector new_pos = last_pos + step;
            ctx.line_to(new_pos.x, new_pos.y);
            last_pos = new_pos;
        }
        ctx.line_to(child->get_x(), child->get_y());
        last = child;
        ++hops;
    }
    ctx.stroke();
    ctx.set_stroke(0xffccccff);
    ctx.set_line_width((ent.get_radius() > 0.f ? ent.get_radius() : 3.5f) * 0.5f);
    ctx.stroke();
}
