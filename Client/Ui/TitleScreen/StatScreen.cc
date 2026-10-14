#include <Client/Ui/TitleScreen/TitleScreen.hh>

#include <Client/Ui/InGame/Loadout.hh>
#include <Client/Ui/Container.hh>
#include <Client/Ui/StaticText.hh>
#include <Client/Ui/DynamicText.hh>
#include <Client/Assets/Assets.hh>
#include <Client/Game.hh>
#include <Client/I18n.hh>

#include <Shared/Entity.hh>
#include <Shared/StaticData.hh>

#include <format>
#include <string>

using namespace Ui;

static uint8_t death_swap_from = 255;
static uint8_t title_swap_from = 255;

StatPetalSlot::StatPetalSlot(uint8_t p) : Element(60,60,{ .no_animation = 1 }), pos(p) {
    //style.v_justify = Style::Bottom;
    style.should_render = [=](){
        return p < Game::loadout_count + MAX_SLOT_COUNT && Game::cached_loadout[p] != PetalID::kNone;
    };
}

void StatPetalSlot::refactor() {
    if (pos < Game::loadout_count) {
        width = height = 60;
    } else if (Game::cached_loadout[pos] != PetalID::kNone) {
        width = height = 50;
    } else {
        width = height = 0;
    }
}

void StatPetalSlot::on_render(Renderer &ctx) {
    uint8_t id = Game::cached_loadout[pos];
    if (id == PetalID::kNone) return;
    ctx.scale(width / 60);
    draw_loadout_background(ctx, id);
    ctx.scale(60.f / width);
    if (death_swap_from == pos) {
        ctx.set_stroke(0xffffffff);
        ctx.set_line_width(3);
        ctx.begin_path();
        ctx.round_rect(-width / 2 - 2, -height / 2 - 2, width + 4, height + 4, 5);
        ctx.stroke();
    }
}

void StatPetalSlot::on_event(uint8_t event) {
    uint8_t id = Game::cached_loadout[pos];
    if (event == kClick && !Game::alive() && Game::on_game_screen) {
        if (death_swap_from == 255) {
            death_swap_from = pos;
        } else if (death_swap_from == pos) {
            death_swap_from = 255;
        } else {
            // Optimistic cache + server (camera inventory when dead)
            PetalID::T a = Game::cached_loadout[death_swap_from];
            PetalID::T b = Game::cached_loadout[pos];
            Game::cached_loadout[death_swap_from] = b;
            Game::cached_loadout[pos] = a;
            Game::swap_petals(death_swap_from, pos);
            death_swap_from = 255;
        }
    }
    if (event != kFocusLost && id != PetalID::kNone) {
        rendering_tooltip = 1;
        tooltip = Ui::UiLoadout::petal_tooltips[id];
    } else {
        rendering_tooltip = 0;
    }
}

TitlePetalSlot::TitlePetalSlot(uint8_t p) : Element(50,50,{}), pos(p) {
    style.v_justify = Style::Bottom;
    style.should_render = [=](){
        if (Game::alive()) return false;
        if (!Game::simulation.ent_exists(Game::camera_id))
            return false;
        Entity const &camera = Game::simulation.get_ent(Game::camera_id);
        uint32_t lim = loadout_slots_at_level(camera.get_respawn_level()) + MAX_SLOT_COUNT;
        if (lim > 2 * MAX_SLOT_COUNT) lim = 2 * MAX_SLOT_COUNT;
        return p < lim; // show empties so bag prep is possible
    };
}

void TitlePetalSlot::on_render(Renderer &ctx) {
    if (!Game::simulation.ent_exists(Game::camera_id))
        return;
    Entity const &camera = Game::simulation.get_ent(Game::camera_id);
    uint8_t id = camera.get_inventory(pos);
    if (id != PetalID::kNone) {
        ctx.scale(width / 60);
        draw_loadout_background(ctx, id);
        ctx.scale(60.f / width);
    } else {
        ctx.set_fill(0x40ffffff);
        ctx.begin_path();
        ctx.round_rect(-width / 2, -height / 2, width, height, 4);
        ctx.fill();
    }
    if (title_swap_from == pos) {
        ctx.set_stroke(0xffffffff);
        ctx.set_line_width(3);
        ctx.begin_path();
        ctx.round_rect(-width / 2 - 2, -height / 2 - 2, width + 4, height + 4, 5);
        ctx.stroke();
    }
}

void TitlePetalSlot::on_event(uint8_t event) {
    if (!Game::simulation.ent_exists(Game::camera_id))
        return;
    Entity const &camera = Game::simulation.get_ent(Game::camera_id);
    uint8_t id = camera.get_inventory(pos);
    if (event == kClick && !Game::alive()) {
        if (title_swap_from == 255) {
            title_swap_from = pos;
        } else if (title_swap_from == pos) {
            title_swap_from = 255;
        } else {
            PetalID::T a = Game::cached_loadout[title_swap_from];
            PetalID::T b = Game::cached_loadout[pos];
            if (title_swap_from < 2 * MAX_SLOT_COUNT && pos < 2 * MAX_SLOT_COUNT) {
                Game::cached_loadout[title_swap_from] = b;
                Game::cached_loadout[pos] = a;
            }
            Game::swap_petals(title_swap_from, pos);
            title_swap_from = 255;
        }
    }
    if (event != kFocusLost && id != PetalID::kNone) {
        rendering_tooltip = 1;
        tooltip = Ui::UiLoadout::petal_tooltips[id];
    } else {
        rendering_tooltip = 0;
    }
}

Element *Ui::make_stat_screen() {
    Element *elt = new Ui::VContainer({
        new Ui::DynamicText(25, [](){ return I18n::tr(I18n::kStats); }, { .fill = 0xffffffff, .h_justify = Style::Left }),
        new Ui::DynamicText(16, [](){
            return I18n::format_score_line(format_score(Game::score));
        }, { .fill = 0xffffffff, .h_justify = Style::Left }),
        new Ui::DynamicText(16, [](){
            return I18n::format_level_line(score_to_level(Game::score));
        }, { .fill = 0xffffffff, .h_justify = Style::Left }),
        new Ui::HContainer(
            Ui::make_range(0, 2 * MAX_SLOT_COUNT, [](uint32_t i){ return (Ui::Element *) new StatPetalSlot(i); })
        , 0, 10)
    }, 20, 7);
    elt->style.animate = [](Element *elt, Renderer &ctx) {
        ctx.translate(0, (1 - elt->animation) * elt->height);
    };
    elt->style.should_render = [](){
        return !Game::alive() && Game::should_render_game_ui();
    };
    elt->animation.set(0);
    elt->style.h_justify = Style::Left;
    elt->style.v_justify = Style::Bottom;
    return elt;
}