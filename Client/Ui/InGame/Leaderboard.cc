#include <Client/Ui/InGame/GameInfo.hh>

#include <Client/Ui/Chrome.hh>
#include <Client/Ui/Container.hh>
#include <Client/Ui/DynamicText.hh>
#include <Client/Ui/Extern.hh>

#include <Client/Game.hh>
#include <Client/I18n.hh>
#include <Client/StaticData.hh>

#include <Helpers/Math.hh>
#include <Shared/StaticData.hh>

#include <format>
#include <string>

using namespace Ui;

static float const LEADERBOARD_WIDTH = 220;

LeaderboardSlot::LeaderboardSlot(uint8_t p) : Element(LEADERBOARD_WIDTH, 20) , pos(p) {
    ratio.set(1);
    style.animate = [&](Element *, Renderer &){
        if (Game::simulation.arena_info.names[pos].empty()) return;
        float r = 1;
        if (((float) Game::simulation.arena_info.scores[0]) != 0)
            r = (float) Game::simulation.arena_info.scores[pos] / (float) Game::simulation.arena_info.scores[0];
        ratio.set(r);
        ratio.step(Ui::lerp_amount);
    };
}

void LeaderboardSlot::on_render(Renderer &ctx) {
    if (Game::simulation.arena_info.names[pos].empty()) return;
    ctx.set_stroke(0xff222222);
    ctx.set_line_width(height);
    ctx.round_line_cap();
    ctx.begin_path();
    ctx.move_to(-(width-height)/2,0);
    ctx.line_to((width-height)/2,0);
    ctx.stroke();
    ctx.set_stroke(FLOWER_COLORS[Game::simulation.arena_info.colors[pos]]);
    ctx.set_line_width(height * 0.8);
    ctx.begin_path();
    ctx.move_to(-(width-height)/2,0);
    ctx.line_to(-(width-height)/2+(width-height)*((float) ratio),0);
    ctx.stroke();
    std::string format_string = std::format("{} - {}",
        Game::simulation.arena_info.names[pos].size() == 0 ? I18n::tr(I18n::kUnnamed) : Game::simulation.arena_info.names[pos],
        format_score((float) Game::simulation.arena_info.scores[pos]));
    ctx.set_fill(0xffffffff);
    ctx.set_stroke(0xff222222);
    ctx.center_text_align();
    ctx.center_text_baseline();
    ctx.set_text_size(height * 0.75);
    ctx.set_line_width(height * 0.75 * 0.12);
    ctx.stroke_text(format_string.c_str());
    ctx.fill_text(format_string.c_str());
}

Element *Ui::make_leaderboard() {
    Element *elt = new Ui::VContainer({
        new Ui::DynamicText(22, [](){ return I18n::tr(I18n::kLeaderboardTitle); }, {
            .fill = Chrome::kTitle,
            .h_justify = Style::Left
        }),
        new Ui::DynamicText(13, [](){
            uint32_t reg = Game::simulation.arena_info.registered_count;
            uint32_t online = Game::simulation.arena_info.player_count;
            if (I18n::use_chinese)
                return std::format("记录 {} · 在线 {}", reg, online);
            return std::format("{} recorded · {} online", reg, online);
        }, { .fill = Chrome::kMuted, .h_justify = Style::Left }),
        new Ui::VContainer(
            Ui::make_range(0, LEADERBOARD_SIZE, [](uint32_t i){ return (Element *) (new Ui::LeaderboardSlot(i)); })
        , 8, 4, { .h_justify = Style::Left })
    }, 16, 8, {
        .fill = Chrome::kPanelDeep,
        .line_width = 0,
        .round_radius = Chrome::kRadius,
        .animate = [](Element *elt, Renderer &ctx){
            elt->y = Game::alive() ? 170.f : 84.f;
            ctx.translate(0, (elt->animation - 1) * elt->height);
        },
        .should_render = [](){
            if (Ui::panel_open != Panel::kLeaderboard) return false;
            return Game::should_render_title_ui()
                || (Game::should_render_game_ui() && Game::alive());
        },
        .h_justify = Style::Left,
        .v_justify = Style::Top
    });
    elt->x = 12;
    elt->y = 84;
    // Title copy first; in-game copy tracked as alt for click-outside hit tests
    if (Ui::Panel::leaderboard == nullptr)
        Ui::Panel::leaderboard = elt;
    else
        Ui::Panel::leaderboard_alt = elt;
    return elt;
}
