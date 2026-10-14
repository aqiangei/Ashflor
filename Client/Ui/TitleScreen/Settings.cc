#include <Client/Ui/TitleScreen/TitleScreen.hh>

#include <Client/Ui/Button.hh>
#include <Client/Ui/Chrome.hh>
#include <Client/Ui/Container.hh>
#include <Client/Ui/DynamicText.hh>
#include <Client/Ui/Extern.hh>
#include <Client/Ui/StaticText.hh>

#include <Client/DOM.hh>
#include <Client/Game.hh>
#include <Client/I18n.hh>
#include <Client/Input.hh>

using namespace Ui;

Element *Ui::make_settings_panel() {
    Element *elt = new Ui::VContainer({
        new Ui::DynamicText(25, [](){ return I18n::tr(I18n::kSettings); }, {
            .fill = Chrome::kTitle
        }),
        new Ui::HContainer({
            new Ui::ToggleButton(30, &I18n::use_chinese),
            new Ui::DynamicText(16, [](){ return I18n::tr(I18n::kLanguage); }, {
                .fill = Chrome::kBody
            })
        }, 0, 10, {.h_justify = Style::Left }),
        new Ui::HContainer({
            new Ui::ToggleButton(30, &Input::keyboard_movement),
            new Ui::DynamicText(16, [](){ return I18n::tr(I18n::kKeyboardMovement); }, {
                .fill = Chrome::kBody
            })
        }, 0, 10, {.h_justify = Style::Left }),
        new Ui::HContainer({
            new Ui::ToggleButton(30, &Input::movement_helper),
            new Ui::DynamicText(16, [](){ return I18n::tr(I18n::kMovementHelper); }, {
                .fill = Chrome::kBody
            })
        }, 0, 10, {.h_justify = Style::Left }),
        new Ui::HContainer({
            new Ui::ToggleButton(30, &Input::invert_attack),
            new Ui::DynamicText(16, [](){ return I18n::tr(I18n::kInvertAttack); }, {
                .fill = Chrome::kBody
            })
        }, 0, 10, {.h_justify = Style::Left }),
        new Ui::HContainer({
            new Ui::ToggleButton(30, &Input::request_mobile),
            new Ui::DynamicText(16, [](){ return I18n::tr(I18n::kRequestMobile); }, {
                .fill = Chrome::kBody
            })
        }, 0, 10, {.h_justify = Style::Left }),
        new Ui::HContainer({
            new Ui::ToggleButton(30, &Game::show_debug),
            new Ui::DynamicText(16, [](){ return I18n::tr(I18n::kDebugStats); }, {
                .fill = Chrome::kBody
            })
        }, 0, 10, {.h_justify = Style::Left }),
        new Ui::Button(220, 36,
            new Ui::DynamicText(16, [](){ return I18n::tr(I18n::kFullscreen); }),
            [](Element *, uint8_t e){
                if (e == Ui::kClick) DOM::toggle_fullscreen();
            },
            nullptr,
            { .fill = Chrome::kBtnSlate, .line_width = 4, .round_radius = Chrome::kRadius }
        ),
        new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kMadeBy); }, {
            .fill = Chrome::kMuted
        }),
        new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kAssetCredits); }, {
            .fill = Chrome::kMuted
        })
    }, 20, 10, {
        .fill = Chrome::kPanelDeep,
        .line_width = 6,
        .round_radius = Chrome::kRadius,
        .animate = [](Element *elt, Renderer &ctx){
            ctx.translate(0, (elt->animation - 1) * elt->height);
        },
        .should_render = [](){
            return Ui::panel_open == Panel::kSettings && Game::should_render_title_ui();
        },
        .h_justify = Style::Left,
        .v_justify = Style::Top
    });
    elt->x = 12;
    elt->y = 68;
    Ui::Panel::settings = elt;
    return elt;
}
