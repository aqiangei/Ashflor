#include <Client/Ui/InGame/GameInfo.hh>

#include <Client/Ui/Container.hh>
#include <Client/Ui/DynamicText.hh>

#include <Client/Game.hh>
#include <Client/I18n.hh>
#include <Client/Ui/Extern.hh>
#include <Shared/StaticData.hh>

#include <Helpers/Math.hh>

#include <format>
#include <string>

using namespace Ui;

// Height includes name (top) + XP bar (bottom) so layout does not paint outside bounds
LevelBar::LevelBar() : Element(280, 56) {
    progress = 0;
    style.animate = [&](Element *elt, Renderer &ctx) {
        if (Game::alive()) {
            Entity &player = Game::simulation.get_ent(Game::player_id);
            float xp = Game::score;
            level = score_to_level(xp);
            xp -= level_to_score(level);
            xp = fclamp(xp / score_to_pass_level(level), 0, 1);
            progress.set(xp);
        }
        progress.step(Ui::lerp_amount);
    };
}

void LevelBar::on_render(Renderer &ctx) {
    float const bar_h = 32.f;
    float const bar_y = height / 2 - bar_h / 2; // bar sits in lower half

    char const *display = Game::nickname.c_str();
    if (Game::account_logged_in && !Game::username.empty())
        display = Game::username.c_str();
    ctx.translate(0, -height / 2 + 14);
    ctx.draw_text(display, { .size = 20 });
    ctx.translate(0, height / 2 - 14);

    ctx.translate(0, bar_y);
    ctx.set_stroke(0xc0222222);
    ctx.round_line_cap();
    ctx.set_line_width(bar_h);
    ctx.begin_path();
    ctx.move_to(-width / 2, 0);
    ctx.line_to(width / 2, 0);
    ctx.stroke();
    ctx.set_stroke(0xfff9e496);
    ctx.set_line_width(bar_h * 0.8f);
    ctx.begin_path();
    ctx.move_to(-width / 2, 0);
    ctx.line_to(-width / 2 + width * ((float) progress), 0);
    ctx.stroke();
    std::string text = I18n::format_lvl_flower(level);
    ctx.draw_text(text.c_str(), { .size = 15 });
    ctx.translate(0, -bar_y);
}

HealthBar::HealthBar() : Element(280, 16) {
    ratio.set(1);
    lag.set(1);
    style.animate = [&](Element *, Renderer &) {
        if (Game::alive()) {
            Entity &player = Game::simulation.get_ent(Game::player_id);
            float r = player.get_health_ratio();
            ratio.set(r);
            ratio.step(Ui::lerp_amount);
            if ((float) lag < r) {
                lag = r;
            } else {
                lag.set(r);
                lag.step(Ui::lerp_amount / 3);
            }
        }
    };
}

void HealthBar::on_render(Renderer &ctx) {
    float r = fclamp((float) ratio, 0, 1);
    float l = fclamp((float) lag, 0, 1);
    ctx.round_line_cap();
    ctx.set_stroke(0xc0222222);
    ctx.set_line_width(height);
    ctx.begin_path();
    ctx.move_to(-width / 2, 0);
    ctx.line_to(width / 2, 0);
    ctx.stroke();
    if (l > r) {
        ctx.set_stroke(0xffed2f31);
        ctx.set_line_width(height * 0.72f);
        ctx.begin_path();
        ctx.move_to(-width / 2 + width * r, 0);
        ctx.line_to(-width / 2 + width * l, 0);
        ctx.stroke();
    }
    ctx.set_stroke(0xff75dd34);
    ctx.set_line_width(height * 0.72f);
    ctx.begin_path();
    ctx.move_to(-width / 2, 0);
    ctx.line_to(-width / 2 + width * r, 0);
    ctx.stroke();
    std::string text = std::format("{:.0f}%", r * 100);
    ctx.draw_text(text.c_str(), { .size = height * 0.7f });
}

// Full player status: name / XP / HP — top-left (under gear/exit)
Element *Ui::make_level_bar() {
    Element *status = new Ui::VContainer({
        new Ui::LevelBar(),
        new Ui::DynamicText(12, [](){
            std::string format_string;
            if (Game::alive()) {
                uint32_t level = score_to_level(Game::score);
                if (loadout_slots_at_level(level) < MAX_SLOT_COUNT)
                    format_string = I18n::format_extra_slot(
                        div_round_up(level + 1, LEVELS_PER_EXTRA_SLOT) * LEVELS_PER_EXTRA_SLOT);
            }
            return format_string;
        }, { .fill = 0xffdddddd, .h_justify = Style::Left }),
        new Ui::HealthBar()
    }, 0, 6, {
        .should_render = []() {
            return Game::alive() && Game::should_render_game_ui();
        },
        .h_justify = Style::Left,
        .v_justify = Style::Top
    });
    status->x = 12;
    // Topbar: y=6, height = 40 + 2*outer_pad(10) = 60 → bottom at 66
    status->y = 76;
    status->style.animate = [](Element *elt, Renderer &ctx) {
        ctx.translate(-(1 - (float) elt->animation) * 1.5f * elt->width, 0);
    };
    return status;
}

// Kept for API compatibility — status is now entirely in make_level_bar()
Element *Ui::make_health_bar() {
    return new Ui::Element(0, 0, {
        .should_render = [](){ return false; }
    });
}
