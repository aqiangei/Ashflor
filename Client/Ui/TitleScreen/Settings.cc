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

#include <string>
#include <vector>

using namespace Ui;

static Element *settings_toggle_row(uint8_t *flag, I18n::Str label, float toggle, float text) {
    return new Ui::HContainer({
        new Ui::ToggleButton(toggle, flag),
        new Ui::DynamicText(text, [label](){ return I18n::tr(label); }, {
            .fill = Chrome::kBody,
            .h_justify = Style::Left
        })
    }, 0, 8, { .h_justify = Style::Left });
}

static Element *settings_section(I18n::Str title, std::vector<Element *> rows,
                                 float title_sz, float gap) {
    std::vector<Element *> kids;
    kids.push_back(new Ui::DynamicText(title_sz, [title](){ return I18n::tr(title); }, {
        .fill = Chrome::kGold,
        .h_justify = Style::Left
    }));
    kids.insert(kids.end(), rows.begin(), rows.end());
    return new Ui::VContainer(kids, 0, gap, {
        .h_justify = Style::Left, .no_animation = 1
    });
}

Element *Ui::make_settings_panel() {
    uint8_t const mobile = Input::is_mobile;
    float const toggle = mobile ? 28.f : 30.f;
    float const label = mobile ? 14.f : 16.f;
    float const pad = mobile ? 14.f : 20.f;
    float const gap = mobile ? 6.f : 8.f;
    float const section_gap = mobile ? 12.f : 14.f;
    float const title_sz = mobile ? 13.f : 14.f;

    auto row = [&](uint8_t *flag, I18n::Str text) {
        return settings_toggle_row(flag, text, toggle, label);
    };

    Element *controls = settings_section(I18n::kSettingsControls, {
        row(&Input::keyboard_movement, I18n::kKeyboardMovement),
        row(&Input::movement_helper, I18n::kMovementHelper),
        row(&Input::invert_attack, I18n::kInvertAttack),
        row(&Input::invert_defend, I18n::kInvertDefend),
        row(&Input::request_mobile, I18n::kRequestMobile),
    }, title_sz, gap);

    Element *display = settings_section(I18n::kSettingsDisplay, {
        row(&I18n::use_chinese, I18n::kLanguage),
        row(&Game::show_low_rarity_drops, I18n::kShowLowRarityDrops),
        row(&Game::show_hitboxes, I18n::kShowHitboxes),
        row(&Game::show_debug, I18n::kDebugStats),
    }, title_sz, gap);

    Element *social = settings_section(I18n::kSettingsSocial, {
        row(&Input::disable_auto_party, I18n::kDisableAutoParty),
    }, title_sz, gap);

    Element *toggles_block = nullptr;
    if (mobile) {
        toggles_block = new Ui::HContainer({
            new Ui::VContainer({ controls, social }, 0, section_gap, {
                .h_justify = Style::Left, .no_animation = 1
            }),
            display
        }, 0, 14, { .h_justify = Style::Left, .no_animation = 1 });
    } else {
        toggles_block = new Ui::VContainer({ controls, display, social }, 0, section_gap, {
            .h_justify = Style::Left, .no_animation = 1
        });
    }

    Element *elt = new Ui::VContainer({
        new Ui::DynamicText(mobile ? 22.f : 25.f, [](){ return I18n::tr(I18n::kSettings); }, {
            .fill = Chrome::kTitle
        }),
        toggles_block,
        new Ui::Button(mobile ? 160.f : 220.f, mobile ? 34.f : 36.f,
            new Ui::DynamicText(mobile ? 15.f : 16.f, [](){ return I18n::tr(I18n::kFullscreen); }),
            [](Element *, uint8_t e){
                if (e == Ui::kClick) DOM::toggle_fullscreen();
            },
            nullptr,
            { .fill = Chrome::kBtnSlate, .line_width = 0, .round_radius = Chrome::kRadius }
        ),
        new Ui::DynamicText(mobile ? 17.f : 16.f, [](){
            return I18n::use_chinese
                ? std::string{"交流 QQ 群：227990150"}
                : std::string{"QQ group: 227990150"};
        }, { .fill = Chrome::kGold, .h_justify = Style::Left }),
        new Ui::HContainer({
            new Ui::Button(mobile ? 130.f : 150.f, 34,
                new Ui::DynamicText(14, [](){ return I18n::use_chinese ? std::string{"给作者留言"} : std::string{"Feedback"}; }),
                [](Element *, uint8_t e){ if (e == Ui::kClick) DOM::open_feedback_board(); }, nullptr,
                { .fill = Chrome::kBtnMoss, .line_width = 0, .round_radius = Chrome::kRadius }),
            new Ui::Button(mobile ? 110.f : 130.f, 34,
                new Ui::DynamicText(14, [](){ return I18n::use_chinese ? std::string{"加入 QQ 群"} : std::string{"QQ group"}; }),
                [](Element *, uint8_t e){ if (e == Ui::kClick) DOM::open_qq_group(); }, nullptr,
                { .fill = Chrome::kBtnSlate, .line_width = 0, .round_radius = Chrome::kRadius })
        }, 0, 8, {}),
        new Ui::DynamicText(mobile ? 15.f : 14.f, [](){ return I18n::tr(I18n::kMadeBy); }, {
            .fill = Chrome::kMuted, .h_justify = Style::Left
        }),
        new Ui::DynamicText(mobile ? 13.f : 13.f, [](){ return I18n::tr(I18n::kAssetCredits); }, {
            .fill = Chrome::kMuted, .h_justify = Style::Left
        }),
        new Ui::Button(mobile ? 220.f : 280.f, mobile ? 32.f : 34.f,
            new Ui::DynamicText(mobile ? 13.f : 14.f, [](){
                return I18n::use_chinese
                    ? std::string{"打开上游 gardn 仓库"}
                    : std::string{"Open upstream gardn"};
            }),
            [](Element *, uint8_t e){
                if (e == Ui::kClick)
                    DOM::open_page("https://github.com/trigonal-bacon/gardn");
            },
            nullptr,
            { .fill = Chrome::kBtnMoss, .line_width = 0, .round_radius = Chrome::kRadius }
        ),
        new Ui::DynamicText(mobile ? 12.f : 12.f, [](){
            return std::string{"github.com/trigonal-bacon/gardn"};
        }, { .fill = 0xff8ab4a0, .h_justify = Style::Left })
    }, pad, gap, {
        .fill = Chrome::kPanelDeep,
        .line_width = 0,
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
    elt->y = mobile ? 72.f : 84.f;
    Ui::Panel::settings = elt;
    return elt;
}
