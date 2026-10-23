#include <Client/Ui/TitleScreen/TitleScreen.hh>

#include <Client/Assets/Assets.hh>
#include <Client/Game.hh>
#include <Client/I18n.hh>
#include <Client/Input.hh>
#include <Client/StaticData.hh>
#include <Client/Ui/Chrome.hh>
#include <Client/Ui/Container.hh>
#include <Client/Ui/DynamicText.hh>
#include <Client/Ui/Extern.hh>

#include <Shared/StaticData.hh>

#include <Helpers/Math.hh>

#include <cmath>
#include <format>
#include <string>
#include <vector>

using namespace Ui;

namespace {
    constexpr float kPanelW = 280.f;
    constexpr float kCell = 52.f;
    constexpr float kGap = 6.f;
    constexpr int kCols = 4;
    constexpr float kGridMaxH = 168.f; // ~3 rows

    // 0 = mobs killed, 1 = petals picked
    class LifeSummaryGrid final : public Element {
        uint8_t kind;
        float scroll = 0;
    public:
        explicit LifeSummaryGrid(uint8_t k)
            : Element(kPanelW - 24.f, kGridMaxH, { .no_animation = 1 }), kind(k) {}

        std::vector<Game::LifeEntry> const &entries() const {
            return kind == 0 ? Game::life_mob_kills : Game::life_petal_picks;
        }

        void refactor() override {
            width = kPanelW - 24.f;
            auto const &list = entries();
            if (list.empty()) {
                height = 36.f;
                return;
            }
            int rows = ((int) list.size() + kCols - 1) / kCols;
            float content_h = rows * kCell + (rows - 1) * kGap;
            height = content_h > kGridMaxH ? kGridMaxH : content_h;
            float max_scroll = content_h > height ? content_h - height : 0.f;
            if (scroll > max_scroll) scroll = max_scroll;
            if (scroll < 0) scroll = 0;
        }

        void on_render(Renderer &ctx) override {
            auto const &list = entries();
            if (list.empty()) {
                ctx.draw_text(I18n::use_chinese ? "无" : "None", {
                    .fill = Chrome::kHint, .size = 14, .stroke_scale = 0.1f
                });
                return;
            }
            int rows = ((int) list.size() + kCols - 1) / kCols;
            float content_h = rows * kCell + (rows - 1) * kGap;
            float max_scroll = content_h > height ? content_h - height : 0.f;

            uint8_t hovered = fabsf(Input::mouse_x - screen_x) <= width * Ui::scale / 2
                && fabsf(Input::mouse_y - screen_y) <= height * Ui::scale / 2;
            if (hovered && visible && Input::wheel_delta != 0 && max_scroll > 0) {
                scroll -= (float) Input::wheel_delta * 0.35f;
                if (scroll < 0) scroll = 0;
                if (scroll > max_scroll) scroll = max_scroll;
            }

            RenderContext clip(&ctx);
            ctx.begin_path();
            ctx.round_rect(-width / 2, -height / 2, width, height, 4);
            ctx.clip();

            float grid_w = kCols * kCell + (kCols - 1) * kGap;
            float origin_x = -grid_w / 2 + kCell / 2;
            float origin_y = -height / 2 + kCell / 2 - scroll;

            for (size_t i = 0; i < list.size(); ++i) {
                int col = (int) (i % kCols);
                int row = (int) (i / kCols);
                float cx = origin_x + col * (kCell + kGap);
                float cy = origin_y + row * (kCell + kGap);
                if (cy + kCell / 2 < -height / 2 || cy - kCell / 2 > height / 2)
                    continue;

                Game::LifeEntry const &e = list[i];
                RenderContext cell(&ctx);
                ctx.translate(cx, cy);

                uint32_t fill = Chrome::kPanelInset;
                if (kind == 0 && e.id < MobID::kNumMobs)
                    fill = RARITY_COLORS[MOB_DATA[e.id].rarity];
                else if (kind == 1 && e.id < PetalID::kNumPetals)
                    fill = RARITY_COLORS[PETAL_DATA[e.id].rarity];

                ctx.set_fill(fill);
                ctx.begin_path();
                ctx.round_rect(-kCell / 2, -kCell / 2, kCell, kCell, 6);
                ctx.fill();
                ctx.set_stroke(0x66000000);
                ctx.set_line_width(1.5f);
                ctx.begin_path();
                ctx.round_rect(-kCell / 2 + 1, -kCell / 2 + 1, kCell - 2, kCell - 2, 5);
                ctx.stroke();

                if (kind == 0 && e.id < MobID::kNumMobs) {
                    struct MobData const &data = MOB_DATA[e.id];
                    float radius = (data.radius.upper + data.radius.lower) / 2;
                    RenderContext icon(&ctx);
                    if (e.id != MobID::kDigger)
                        ctx.rotate(-3 * (float) M_PI / 4);
                    if (radius > kCell * 0.42f)
                        ctx.scale(0.42f * kCell / radius);
                    ctx.scale(0.45f);
                    draw_static_mob((MobID::T) e.id, ctx, {
                        .radius = radius,
                        .flower_attrs = { .color = ColorID::kGray }
                    });
                } else if (kind == 1 && e.id < PetalID::kNumPetals) {
                    RenderContext icon(&ctx);
                    ctx.scale(0.72f);
                    if (PETAL_DATA[e.id].radius > 20)
                        ctx.scale(20.f / PETAL_DATA[e.id].radius);
                    draw_static_petal((PetalID::T) e.id, ctx);
                }

                if (e.count > 1) {
                    std::string badge = format_stack(e.count);
                    float const sz = 12.f;
                    ctx.set_text_size(sz);
                    float tw = (float) ctx.get_text_size(badge.c_str());
                    float bw = std::max(22.f, tw + 8.f);
                    ctx.translate(kCell / 2 - bw * 0.5f, -kCell / 2 + 10);
                    ctx.set_fill(0xe0000000);
                    ctx.begin_path();
                    ctx.round_rect(-bw * 0.5f, -8, bw, 16, 6);
                    ctx.fill();
                    ctx.draw_text(badge.c_str(), {
                        .fill = Chrome::kGold, .size = sz, .stroke_scale = 0.1f
                    });
                }
            }
        }
    };

    Element *make_summary_panel(uint8_t kind) {
        char const *title_zh = kind == 0 ? "本次击杀" : "本次收集";
        char const *title_en = kind == 0 ? "Kills this life" : "Petals this life";
        Element *body = new Ui::VContainer({
            new Ui::DynamicText(16, [title_zh, title_en](){
                return I18n::use_chinese ? std::string{title_zh} : std::string{title_en};
            }, { .fill = Chrome::kTitle, .h_justify = Style::Left }),
            new LifeSummaryGrid(kind)
        }, 10, 8, {
            .fill = 0xcc2a1c14,
            .line_width = 0,
            .round_radius = Chrome::kRadius,
            .h_justify = Style::Right,
            .v_justify = Style::Top,
            .should_render = [](){ return Game::life_summary_ready; },
            .no_animation = 1
        });
        body->width = kPanelW;
        return body;
    }
}

Element *Ui::make_death_life_summary() {
    // Score / level (was bottom-left StatScreen; petals row removed — overlaps side UI)
    Element *score_panel = new Ui::VContainer({
        new Ui::DynamicText(16, [](){ return I18n::tr(I18n::kStats); }, {
            .fill = Chrome::kTitle, .h_justify = Style::Left
        }),
        new Ui::DynamicText(14, [](){
            return I18n::format_score_line(format_score(Game::score));
        }, { .fill = Chrome::kBody, .h_justify = Style::Left }),
        new Ui::DynamicText(14, [](){
            return I18n::format_level_line(score_to_level(Game::score));
        }, { .fill = Chrome::kBody, .h_justify = Style::Left }),
    }, 10, 6, {
        .fill = 0xcc2a1c14,
        .line_width = 0,
        .round_radius = Chrome::kRadius,
        .h_justify = Style::Right,
        .v_justify = Style::Top,
        .no_animation = 1
    });
    score_panel->width = kPanelW;

    Element *elt = new Ui::VContainer({
        make_summary_panel(0),
        make_summary_panel(1),
        score_panel
    }, 0, 10, {
        .should_render = [](){
            return !Game::alive()
                && Game::on_game_screen
                && !Game::leaving_to_title
                && Game::spawn_request_at <= 0
                && Game::should_render_game_ui();
        },
        .h_justify = Style::Right,
        .v_justify = Style::Top,
        .no_animation = 1
    });
    elt->x = -12;
    elt->y = 72; // below topbar
    elt->style.animate = [](Element *elt, Renderer &ctx){
        ctx.set_global_alpha((float) elt->animation);
        ctx.translate((1.f - (float) elt->animation) * 40.f, 0);
    };
    return elt;
}
