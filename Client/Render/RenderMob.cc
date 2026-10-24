#include <Client/Render/RenderEntity.hh>

#include <Client/Render/Renderer.hh>

#include <Client/Assets/Assets.hh>
#include <Client/Assets/ThemeArt.hh>
#include <Client/Particle.hh>
#include <Client/Ui/Extern.hh>

#include <Shared/Entity.hh>
#include <Shared/EntityDef.hh>
#include <Shared/StaticData.hh>

#include <Client/Game.hh>

#include <Helpers/Math.hh>

#include <cmath>
#include <string_view>
#include <unordered_map>

namespace {
struct SpiritTrailPrev {
    float x = 0;
    float y = 0;
    double t = 0;
    uint8_t ok = 0;
};
std::unordered_map<uint32_t, SpiritTrailPrev> spirit_trail_prev;
}

void render_mob(Renderer &ctx, Entity const &ent) {
    uint32_t flags = 0;
    if (ent.get_team() == Game::simulation.get_ent(Game::camera_id).get_team()) BitMath::set(flags, 0);
    if (ent.has_component(kSegmented)) BitMath::set(flags, 1);
    MobRenderAttributes attrs = {ent.animation, ent.get_radius(), ent.id.id, flags, ent.get_color()};
    if (ent.has_component(kFlower)) {
        attrs.flower_attrs = {
            .radius = ent.get_radius(),
            .eye_x = ent.eye_x,
            .eye_y = ent.eye_y,
            .mouth = ent.mouth,
            .cutter_angle = (float) (Game::timestamp / 200),
            .face_flags = ent.get_face_flags(),
            .equip_flags = ent.get_equip_flags(),
            .color = ent.get_color()
        };
    }
    uint8_t const map_id = Game::simulation.arena_info.map_id;
    std::string_view const nm = (ent.has_component(kName) && !ent.get_name().empty())
        ? std::string_view(ent.get_name()) : std::string_view{};
    ThemeArt::Visual const theme = ThemeArt::resolve(ent.get_mob_id(), nm, map_id);
    if (theme != ThemeArt::Visual::None) {
        ThemeArt::draw(ctx, theme, map_id, attrs.radius, attrs.animation, attrs.seed, nm);
    } else {
        draw_static_mob(ent.get_mob_id(), ctx, attrs);
        // E: lightweight sleep overlay (no new AI) — Z marks above sleeping event mobs.
        if (!nm.empty() && (nm.find("睡觉") != std::string_view::npos
                || nm.find("Sleep") != std::string_view::npos
                || nm.find("Asleep") != std::string_view::npos)) {
            float r = attrs.radius;
            ctx.set_global_alpha(0.55f + 0.2f * sinf(attrs.animation * 0.7f));
            ctx.set_stroke(0xffffffff);
            ctx.set_line_width(2.5f);
            float ox = r * 0.55f, oy = -r * 0.85f;
            for (int i = 0; i < 3; ++i) {
                float s = 0.55f + i * 0.25f;
                ctx.begin_path();
                ctx.move_to(ox + i * 6.f, oy - i * 8.f);
                ctx.line_to(ox + i * 6.f + 7.f * s, oy - i * 8.f);
                ctx.line_to(ox + i * 6.f, oy - i * 8.f + 7.f * s);
                ctx.line_to(ox + i * 6.f + 7.f * s, oy - i * 8.f + 7.f * s);
                ctx.stroke();
            }
            ctx.set_global_alpha(1.f);
        }
    }
    // Guide-spirit trail (ThemeArt Escort, or name fallback).
    uint8_t const is_spirit = (theme == ThemeArt::Visual::Escort)
        || (!nm.empty() && (nm.find("迷路") != std::string_view::npos
            || nm.find("掉队") != std::string_view::npos
            || nm.find("Lost") != std::string_view::npos
            || nm.find("Stray") != std::string_view::npos));
    if (is_spirit) {
        float const x = ent.get_x();
        float const y = ent.get_y();
        uint32_t const key = EntityID::make_hash(ent.id);
        SpiritTrailPrev &prev = spirit_trail_prev[key];
        // Emit ~every 70ms, not every frame: per-frame emission at 60–144fps
        // hit the particle cap and starved the ribbon. Back direction = last
        // emit point → now, so it points along the walked path.
        if (!prev.ok || Game::timestamp - prev.t >= 70.0) {
            float bdx = prev.ok ? prev.x - x : 0.f;
            float bdy = prev.ok ? prev.y - y : 0.f;
            Particle::add_spirit_trail(x, y, bdx, bdy);
            prev.x = x;
            prev.y = y;
            prev.t = Game::timestamp;
            prev.ok = 1;
        }
    }
    // Unlock gallery cell for this live rarity (spotting or death).
    Game::mark_mob_seen(ent.get_mob_id(), ent.get_rarity());
    /*
    #ifdef DEBUG
    ctx.set_stroke(0x80ff0000);
    ctx.set_line_width(1);
    ctx.begin_path();
    ctx.arc(0,0,MOB_DATA[ent.get_mob_id()].attributes.aggro_radius);
    ctx.stroke();
    #endif
    */
}