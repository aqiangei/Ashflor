#include <Client/Ui/InGame/GameInfo.hh>

#include <Client/Game.hh>
#include <Client/I18n.hh>
#include <Client/Ui/Extern.hh>

#include <Shared/Entity.hh>
#include <Shared/StaticData.hh>

#include <Helpers/Math.hh>

#include <cmath>
#include <format>
#include <string>

using namespace Ui;

// Large screen HP bar when the player is near a map boss.
namespace {
    constexpr float kBossBarRange = 2200.f;

    struct BossSighting {
        MobID::T mob = MobID::kNumMobs;
        float ratio = 1.f;
        float dist2 = 1e30f;
    };

    BossSighting find_nearby_boss() {
        BossSighting best;
        if (!Game::alive() || !Game::simulation_ready) return best;
        if (!Game::simulation.ent_exists(Game::player_id)) return best;
        Entity const &player = Game::simulation.get_ent(Game::player_id);
        float px = player.get_x();
        float py = player.get_y();
        float range2 = kBossBarRange * kBossBarRange;
        Game::simulation.for_each<kMob>([&](Simulation *, Entity &ent){
            if (!ent.has_component(kHealth)) return;
            // Worm segments share the fight — only head / non-segment.
            if (ent.has_component(kSegmented)) return;
            MobID::T mid = ent.get_mob_id();
            if (mid >= MobID::kNumMobs || !is_boss_mob(mid)) return;
            float dx = (float) ent.get_x() - px;
            float dy = (float) ent.get_y() - py;
            float d2 = dx * dx + dy * dy;
            if (d2 > range2 || d2 >= best.dist2) return;
            best.mob = mid;
            best.ratio = ent.get_health_ratio();
            best.dist2 = d2;
        });
        return best;
    }

    class BossHealthBar final : public Element {
        LerpFloat ratio;
        LerpFloat lag;
        LerpFloat show;
        MobID::T last_mob;
    public:
        BossHealthBar()
            : Element(640, 56, {
                .should_render = [](){
                    return Game::alive()
                        && Game::should_render_game_ui()
                        && find_nearby_boss().mob < MobID::kNumMobs;
                },
                .h_justify = Style::Center,
                .v_justify = Style::Top
            }), last_mob(MobID::kNumMobs) {
            ratio.set(1);
            lag.set(1);
            show.set(0);
            style.animate = [&](Element *elt, Renderer &ctx){
                BossSighting sight = find_nearby_boss();
                uint8_t visible = sight.mob < MobID::kNumMobs;
                show.set(visible ? 1.f : 0.f);
                show.step(Ui::lerp_amount * 2.5f);
                if (visible) {
                    if (sight.mob != last_mob) {
                        last_mob = sight.mob;
                        ratio = sight.ratio;
                        lag = sight.ratio;
                    }
                    ratio.set(sight.ratio);
                    ratio.step(Ui::lerp_amount);
                    if ((float) lag < (float) ratio) {
                        lag = (float) ratio;
                    } else {
                        lag.set((float) ratio);
                        lag.step(Ui::lerp_amount / 3);
                    }
                }
                float target_w = fminf(720.f, (float) (Ui::window_width / Ui::scale) * 0.72f);
                if (target_w < 280.f) target_w = 280.f;
                width = target_w;
                // Drop below aqa 全服 banner when that overlay is up.
                uint8_t announce_up = Game::screen_announce.expires_at > Game::timestamp
                    && !Game::screen_announce.text.empty();
                elt->y = announce_up ? 130.f : 68.f;
                ctx.set_global_alpha((float) show * (float) elt->animation);
                ctx.translate(0, -(1.f - (float) show) * 36.f);
            };
        }

        void on_render(Renderer &ctx) override {
            if (last_mob >= MobID::kNumMobs) return;
            float r = fclamp((float) ratio, 0, 1);
            float l = fclamp((float) lag, 0, 1);
            float bar_h = 22.f;

            // Name above the bar
            char const *name = I18n::mob_name(last_mob);
            if (name == nullptr || name[0] == '\0')
                name = MOB_DATA[last_mob].name;
            ctx.translate(0, -height / 2 + 14);
            ctx.draw_text(name, {
                .fill = 0xffffffff,
                .size = 18,
                .stroke_scale = 0.14f
            });
            ctx.translate(0, height / 2 - 14);

            // Bar track (centered vertically toward bottom of element)
            float bar_y = height / 2 - bar_h * 0.5f - 4.f;
            ctx.translate(0, bar_y);
            ctx.round_line_cap();

            ctx.set_stroke(0xdd1a1210);
            ctx.set_line_width(bar_h + 8.f);
            ctx.begin_path();
            ctx.move_to(-width / 2, 0);
            ctx.line_to(width / 2, 0);
            ctx.stroke();

            ctx.set_stroke(0xff5a3a28);
            ctx.set_line_width(bar_h + 4.f);
            ctx.begin_path();
            ctx.move_to(-width / 2, 0);
            ctx.line_to(width / 2, 0);
            ctx.stroke();

            if (l > r) {
                ctx.set_stroke(0xffed2f31);
                ctx.set_line_width(bar_h);
                ctx.begin_path();
                ctx.move_to(-width / 2 + width * r, 0);
                ctx.line_to(-width / 2 + width * l, 0);
                ctx.stroke();
            }

            uint32_t fill = r > 0.35f ? 0xffe23b3b : 0xffff7a3a;
            ctx.set_stroke(fill);
            ctx.set_line_width(bar_h);
            ctx.begin_path();
            ctx.move_to(-width / 2, 0);
            ctx.line_to(-width / 2 + width * r, 0);
            ctx.stroke();

            std::string pct = std::format("{:.0f}%", r * 100.f);
            ctx.draw_text(pct.c_str(), {
                .fill = 0xffffffff,
                .size = 14,
                .stroke_scale = 0.12f
            });
            ctx.translate(0, -bar_y);
        }
    };
}

Element *Ui::make_boss_health_bar() {
    Element *elt = new BossHealthBar();
    // Sit just under the topbar (topbar ~60px tall at y=6)
    elt->y = 68;
    return elt;
}
