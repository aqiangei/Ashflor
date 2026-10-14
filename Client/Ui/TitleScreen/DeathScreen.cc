#include <Client/Ui/TitleScreen/TitleScreen.hh>

#include <Client/Game.hh>
#include <Client/I18n.hh>
#include <Client/Ui/Button.hh>
#include <Client/Ui/Chrome.hh>
#include <Client/Ui/DynamicText.hh>
#include <Client/Ui/Container.hh>
#include <Client/Ui/StaticText.hh>

#include <Client/Assets/Assets.hh>

#include <cmath>

using namespace Ui;

DeadFlowerIcon::DeadFlowerIcon(float width) : Element(width, width, {}) {}

void DeadFlowerIcon::on_render(Renderer &ctx) {
    if (Game::loadout_count == 0) return;
    StaticArray<PetalID::T, MAX_SLOT_COUNT> physical_loadout;
    uint8_t equip_flags = 0;
    for (uint32_t i = 0; i < Game::loadout_count; ++i) {
        PetalData const &data = PETAL_DATA[Game::cached_loadout[i]];
        if (data.count > 0) physical_loadout.push(Game::cached_loadout[i]);
        if (data.attributes.equipment != EquipmentFlags::kNone) 
            BitMath::set(equip_flags, data.attributes.equipment);
    }
    ctx.scale((width / 4) / 25);
    for (uint32_t i = 0; i < physical_loadout.size(); ++i) {
        float angle = 2 * M_PI * i / physical_loadout.size() + 0.5;
        float offset_x = cosf(angle) * 40;
        float offset_y = sinf(angle) * 25;
        if (offset_y > 0) continue;
        RenderContext c(&ctx);
        PetalID::T id = physical_loadout[i];
        ctx.translate(offset_x, offset_y);
        ctx.rotate(PETAL_DATA[id].attributes.icon_angle);
        if (PETAL_DATA[id].radius > 20) ctx.scale(20 / PETAL_DATA[id].radius);
        draw_static_petal_single(id, ctx);
    }
    {
        RenderContext c(&ctx);
        ctx.rotate(-0.2);
        float flower_radius = width / 3;
        draw_static_flower(ctx, { .radius = 25, .mouth = 5, .face_flags = (1<<FaceFlags::kDeadEyes), .equip_flags = equip_flags });
    }
    for (uint32_t i = 0; i < physical_loadout.size(); ++i) {
        float angle = 2 * M_PI * i / physical_loadout.size() + 0.5;
        float offset_x = cosf(angle) * 40;
        float offset_y = sinf(angle) * 25;
        if (offset_y <= 0) continue;
        RenderContext c(&ctx);
        PetalID::T id = physical_loadout[i];
        ctx.translate(offset_x, offset_y);
        ctx.rotate(PETAL_DATA[id].attributes.icon_angle);
        if (PETAL_DATA[id].radius > 20) ctx.scale(20 / PETAL_DATA[id].radius);
        draw_static_petal_single(id, ctx);
    }
}

Element *Ui::make_death_main_screen() {
    Ui::Element *continue_button = new Ui::Button(
        145,
        40,
        new Ui::DynamicText(28, [](){ return I18n::tr(I18n::kContinue); }),
        [](Element *elt, uint8_t e){
            if (e == Ui::kClick && !Game::alive() && !Game::leaving_to_title)
                Game::spawn_in();
        },
        [](){ return !Game::alive() && !Game::leaving_to_title; },
        {.fill = Chrome::kBtnMoss, .line_width = 5, .round_radius = Chrome::kRadius }
    );
    Ui::Element *container = new Ui::VContainer({
        new Ui::DynamicText(25, [](){ return I18n::tr(I18n::kKilledBy); }),
        new Ui::DynamicText(30, [](){
            if (!Game::simulation.ent_exists(Game::camera_id))
                return std::string{""};
            if (Game::simulation.get_ent(Game::camera_id).get_killed_by() == "") 
                return std::string{I18n::tr(I18n::kMysteriousEntity)};
            return Game::simulation.get_ent(Game::camera_id).get_killed_by();
        }),
        new Ui::DeadFlowerIcon(125),
        new Ui::DynamicText(14, [](){ return I18n::tr(I18n::kReviveHint); }, {
            .fill = 0xff88ff88,
            .should_render = [](){ return Game::awaiting_party_revive != 0; }
        }),
        new Ui::DynamicText(16, [](){
            return I18n::format_revive_progress(Game::revive_progress);
        }, {
            .fill = 0xffffff66,
            .should_render = [](){
                return Game::awaiting_party_revive && Game::revive_progress > 0;
            }
        }),
        continue_button,
        new Ui::DynamicText(14, [](){ return I18n::tr(I18n::kOrPressEnterContinue); })
    }, 0, 10, { .animate = [](Element *elt, Renderer &ctx) {
        ctx.translate(0, (elt->animation - 1) * ctx.height * 0.6);
    }, .should_render = [](){
        // Soft-leave keeps on_game_screen=0 so we never flash death → Continue → revive
        return !Game::alive() && Game::on_game_screen
            && !Game::leaving_to_title && Game::should_render_game_ui();
    } });
    return container;
}
