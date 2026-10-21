#include <Client/Ui/InGame/GameInfo.hh>

#include <Client/Ui/Chrome.hh>
#include <Client/Ui/Container.hh>
#include <Client/Ui/DynamicText.hh>

#include <Client/Game.hh>
#include <Client/I18n.hh>
#include <Client/Ui/Extern.hh>
#include <Shared/StaticData.hh>
#include <Shared/StaticDefinitions.hh>
#include <Shared/Talent.hh>

#include <Helpers/Math.hh>

#include <format>
#include <string>

using namespace Ui;

namespace {
std::string xp_progress_label() {
    if (!Game::alive()) return {};
    uint32_t const total = (uint32_t) Game::score;
    uint32_t const lv = score_to_level(total);
    uint32_t const into = total - level_to_score(lv);
    uint32_t const need = score_to_pass_level(lv);
    return std::format("{}/{}", into, need);
}
} // namespace

// Thin XP strip only — name + HP sit above this in make_level_bar()
LevelBar::LevelBar() : Element(280, 18) {
    progress = 0;
    style.animate = [&](Element *elt, Renderer &ctx) {
        if (Game::alive()) {
            float xp = Game::score;
            level = score_to_level(xp);
            xp -= level_to_score(level);
            xp = fclamp(xp / score_to_pass_level(level), 0, 1);
            progress.set(xp);
        }
        progress.step(Ui::lerp_amount);
    };
    tooltip = new Ui::VContainer({
        new Ui::DynamicText(13, [](){
            if (!Game::alive()) return std::string{};
            return I18n::use_chinese
                ? std::format("经验 {}", xp_progress_label())
                : std::format("XP {}", xp_progress_label());
        }, { .fill = 0xffffffff })
    }, 8, 0);
    tooltip->style.fill = 0xb01c2418;
    tooltip->style.round_radius = 8;
}

void LevelBar::on_event(uint8_t event) {
    if (event != kFocusLost && Game::alive())
        rendering_tooltip = 1;
    else
        rendering_tooltip = 0;
}

void LevelBar::on_render(Renderer &ctx) {
    float const bar_h = 12.f;

    ctx.set_stroke(0xc0222222);
    ctx.round_line_cap();
    ctx.set_line_width(bar_h);
    ctx.begin_path();
    ctx.move_to(-width / 2, 0);
    ctx.line_to(width / 2, 0);
    ctx.stroke();
    ctx.set_stroke(0xfff9e496);
    ctx.set_line_width(bar_h * 0.78f);
    ctx.begin_path();
    ctx.move_to(-width / 2, 0);
    ctx.line_to(-width / 2 + width * ((float) progress), 0);
    ctx.stroke();
    // Hover: show current/target XP into this level; idle: level label.
    std::string text = (focus_state == kMouseHover || focus_state == kMouseDown)
        ? xp_progress_label()
        : I18n::format_lvl_flower(level);
    ctx.draw_text(text.c_str(), { .size = 11 });
}

HealthBar::HealthBar() : Element(280, 28) {
    ratio.set(1);
    lag.set(1);
    style.animate = [&](Element *elt, Renderer &) {
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
        // Room under the bar for guild name (bottom-right)
        elt->height = Game::guild_name.empty() ? 28.f : 42.f;
    };
}

void HealthBar::on_render(Renderer &ctx) {
    float const bar_h = 26.f;
    float const bar_y = Game::guild_name.empty() ? 0.f : -7.f;
    float r = fclamp((float) ratio, 0, 1);
    float l = fclamp((float) lag, 0, 1);

    ctx.translate(0, bar_y);
    ctx.round_line_cap();
    ctx.set_stroke(0xc0222222);
    ctx.set_line_width(bar_h);
    ctx.begin_path();
    ctx.move_to(-width / 2, 0);
    ctx.line_to(width / 2, 0);
    ctx.stroke();
    if (l > r) {
        ctx.set_stroke(0xffed2f31);
        ctx.set_line_width(bar_h * 0.72f);
        ctx.begin_path();
        ctx.move_to(-width / 2 + width * r, 0);
        ctx.line_to(-width / 2 + width * l, 0);
        ctx.stroke();
    }
    ctx.set_stroke(0xff75dd34);
    ctx.set_line_width(bar_h * 0.72f);
    ctx.begin_path();
    ctx.move_to(-width / 2, 0);
    ctx.line_to(-width / 2 + width * r, 0);
    ctx.stroke();
    std::string text = std::format("{:.0f}%", r * 100);
    ctx.draw_text(text.c_str(), { .size = bar_h * 0.55f });
    ctx.translate(0, -bar_y);

    if (!Game::guild_name.empty()) {
        float const sz = 11.f;
        ctx.set_text_size(sz);
        float tw = (float) ctx.get_text_size(Game::guild_name.c_str());
        // Anchor at bar bottom-right (text is center-aligned)
        ctx.translate(width / 2 - tw * 0.5f, bar_y + bar_h * 0.5f + sz * 0.65f);
        ctx.draw_text(Game::guild_name.c_str(), {
            .fill = Chrome::kGold,
            .stroke = 0xff000000,
            .size = sz,
            .stroke_scale = 0.12f
        });
    }
}

PartyHud::PartyHud() : Element(200, 0, {
    .should_render = [](){
        if (!Game::alive() || !Game::should_render_game_ui()) return false;
        for (Game::PartyMember const &m : Game::party_members)
            if (!(m.flags & 4)) return true;
        return false;
    }
}) {}

void PartyHud::refactor() {
    uint8_t n = 0;
    for (Game::PartyMember const &m : Game::party_members)
        if (!(m.flags & 4)) ++n;
    width = 200.f;
    height = n * 26.f;
    if (n == 0) height = 0;
}

void PartyHud::on_render(Renderer &ctx) {
    float y = -height * 0.5f + 13.f;
    for (Game::PartyMember const &m : Game::party_members) {
        if (m.flags & 4) continue;
        std::string label;
        if (m.flags & 1) label += '*';
        label += m.name;
        if (m.flags & 2) label += " [DOWN]";

        float const max_w = width - 8.f;
        ctx.set_text_size(12.f);
        if (ctx.get_text_size(label.c_str()) > max_w) {
            while (label.size() > 1 &&
                   ctx.get_text_size((label + "..").c_str()) > max_w)
                label.pop_back();
            label += "..";
        }
        float tw = (float) ctx.get_text_size(label.c_str());
        ctx.translate(-width * 0.5f + tw * 0.5f + 2.f, y - 4.f);
        ctx.draw_text(label.c_str(), {
            .fill = (m.flags & 2) ? 0xffff9aa0 : 0xffffc8e0,
            .stroke = 0xff000000,
            .size = 12.f,
            .stroke_scale = 0.12f
        });
        ctx.translate(width * 0.5f - tw * 0.5f - 2.f, -(y - 4.f));

        // Mini HP bar under name (pink fill matches minimap teammate dots)
        float const bar_w = width - 4.f;
        float const bar_h = 8.f;
        float const bar_y = y + 8.f;
        float hp = (m.flags & 2) ? 0.f : fclamp(m.hp_pct / 100.f, 0.f, 1.f);
        ctx.translate(0, bar_y);
        ctx.round_line_cap();
        ctx.set_stroke(0xc0222222);
        ctx.set_line_width(bar_h);
        ctx.begin_path();
        ctx.move_to(-bar_w * 0.5f, 0);
        ctx.line_to(bar_w * 0.5f, 0);
        ctx.stroke();
        if (hp > 0.f) {
            ctx.set_stroke(0xffff69b4);
            ctx.set_line_width(bar_h * 0.72f);
            ctx.begin_path();
            ctx.move_to(-bar_w * 0.5f, 0);
            ctx.line_to(-bar_w * 0.5f + bar_w * hp, 0);
            ctx.stroke();
        }
        ctx.translate(0, -bar_y);
        y += 26.f;
    }
}

// Full player status: name → HP (thick) → XP (thin) — top-left (under gear/exit)
Element *Ui::make_level_bar() {
    Element *status = new Ui::VContainer({
        new Ui::DynamicText(20, [](){
            if (Game::account_logged_in && !Game::username.empty())
                return Game::username;
            return std::string{Game::guest_play_name()};
        }, { .fill = 0xffffffff, .h_justify = Style::Left }),
        new Ui::HealthBar(),
        new Ui::LevelBar(),
        new Ui::DynamicText(12, [](){
            if (!Game::alive()) return std::string{};
            uint8_t slots = Talent::slot_count_from_ranks(Game::talent_ranks);
            if (slots >= MAX_SLOT_COUNT) return std::string{};
            return I18n::use_chinese
                ? std::format("槽位 {}/{}（天赋加点解锁）", (int) slots, (int) MAX_SLOT_COUNT)
                : std::format("Slots {}/{} (talent)", (int) slots, (int) MAX_SLOT_COUNT);
        }, { .fill = 0xffdddddd, .h_justify = Style::Left }),
        new Ui::PartyHud()
    }, 0, 4, {
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
