#include <Client/Ui/TitleScreen/TitleScreen.hh>

#include <Client/Ui/Button.hh>
#include <Client/Ui/Chrome.hh>
#include <Client/Ui/Container.hh>
#include <Client/Ui/DynamicText.hh>
#include <Client/Ui/Extern.hh>
#include <Client/Ui/ScrollContainer.hh>

#include <Client/DOM.hh>
#include <Client/Game.hh>
#include <Client/I18n.hh>
#include <Client/Input.hh>

#include <Helpers/Math.hh>

#include <cmath>
#include <string>
#include <vector>

using namespace Ui;

namespace {

class AdaptiveScroll final : public Ui::ScrollContainer {
public:
    float top_reserve = 0;
    float bottom_pad = 24.f;
    float min_h = 140.f;

    AdaptiveScroll(Element *content, float initial_h)
        : Ui::ScrollContainer(content, initial_h) {
        always_capture_wheel = 1;
    }

    void refactor() override {
        float view_h = (float) (Ui::window_height / Ui::scale);
        float avail = view_h - top_reserve - bottom_pad;
        if (avail < min_h) avail = min_h;
        float const cap = view_h * 0.78f;
        if (avail > cap) avail = cap;
        // Shrink-wrap short content; only fill avail when scrolling is needed.
        Element *body = children[0];
        body->refactor();
        float const need = body->height;
        height = need > 0.f && need < avail ? need : avail;
        if (height < 1.f) height = 1.f;
        Ui::ScrollContainer::refactor();
    }
};

static Element *settings_toggle_row(uint8_t *flag, I18n::Str label, float toggle, float text, float gap) {
    return new Ui::HContainer({
        new Ui::ToggleButton(toggle, flag),
        new Ui::DynamicText(text, [label](){ return I18n::tr(label); }, {
            .fill = Chrome::kBody,
            .h_justify = Style::Left
        })
    }, 0, gap, { .h_justify = Style::Left });
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

} // namespace

Element *Ui::make_settings_panel() {
    uint8_t const mobile = Input::is_mobile;
    float const toggle = 28.f;
    float const label = mobile ? 11.f : 15.f;
    float const pad = mobile ? 8.f : 18.f;
    float const row_gap = mobile ? 4.f : 10.f;
    float const section_gap = mobile ? 10.f : 20.f;
    float const title_sz = mobile ? 10.f : 13.f;
    float const row_gap_h = mobile ? 6.f : 10.f;
    float const panel_y = mobile ? 64.f : 84.f;
    float const header_h = (mobile ? 16.f : 24.f) + 10.f + pad * 2.f;

    auto row = [&](uint8_t *flag, I18n::Str text) {
        return settings_toggle_row(flag, text, toggle, label, row_gap_h);
    };

    Element *controls = settings_section(I18n::kSettingsControls, {
        row(&Input::keyboard_movement, I18n::kKeyboardMovement),
        row(&Input::movement_helper, I18n::kMovementHelper),
        row(&Input::invert_attack, I18n::kInvertAttack),
        row(&Input::invert_defend, I18n::kInvertDefend),
        row(&Input::request_mobile, I18n::kRequestMobile),
    }, title_sz, row_gap);

    Element *display = settings_section(I18n::kSettingsDisplay, {
        row(&I18n::use_chinese, I18n::kLanguage),
        row(&Game::show_low_rarity_drops, I18n::kShowLowRarityDrops),
        row(&Game::show_hitboxes, I18n::kShowHitboxes),
        row(&Game::show_debug, I18n::kDebugStats),
    }, title_sz, row_gap);

    Element *social = settings_section(I18n::kSettingsSocial, {
        row(&Input::disable_auto_party, I18n::kDisableAutoParty),
    }, title_sz, row_gap);

    // Mobile: always two compact columns. Desktop: two columns when wide.
    float const view_w = (float) (Ui::window_width / Ui::scale);
    uint8_t const two_col = mobile || view_w >= 720.f;
    Element *toggles_block = two_col
        ? (Element *) new Ui::HContainer({
            new Ui::VContainer({ controls, social }, 0, section_gap, {
                .h_justify = Style::Left, .no_animation = 1
            }),
            display
        }, 0, mobile ? 12.f : 28.f, { .h_justify = Style::Left, .no_animation = 1 })
        : (Element *) new Ui::VContainer({
            controls,
            display,
            social
        }, 0, section_gap, { .h_justify = Style::Left, .no_animation = 1 });

    Element *actions = new Ui::VContainer({
        new Ui::Button(mobile ? 140.f : 200.f, mobile ? 28.f : 34.f,
            new Ui::DynamicText(mobile ? 12.f : 15.f, [](){ return I18n::tr(I18n::kFullscreen); }),
            [](Element *, uint8_t e){
                if (e == Ui::kClick) DOM::toggle_fullscreen();
            },
            nullptr,
            { .fill = Chrome::kBtnSlate, .line_width = 0, .round_radius = Chrome::kRadius }
        ),
        new Ui::DynamicText(mobile ? 12.f : 14.f, [](){
            return I18n::use_chinese
                ? std::string{"交流 QQ 群：227990150"}
                : std::string{"QQ group: 227990150"};
        }, { .fill = Chrome::kGold, .h_justify = Style::Left }),
        new Ui::HContainer({
            new Ui::Button(mobile ? 100.f : 140.f, mobile ? 28.f : 32.f,
                new Ui::DynamicText(mobile ? 11.f : 13.f, [](){ return I18n::use_chinese ? std::string{"给作者留言"} : std::string{"Feedback"}; }),
                [](Element *, uint8_t e){ if (e == Ui::kClick) DOM::open_feedback_board(); }, nullptr,
                { .fill = Chrome::kBtnMoss, .line_width = 0, .round_radius = Chrome::kRadius }),
            new Ui::Button(mobile ? 90.f : 120.f, mobile ? 28.f : 32.f,
                new Ui::DynamicText(mobile ? 11.f : 13.f, [](){ return I18n::use_chinese ? std::string{"加入 QQ 群"} : std::string{"QQ group"}; }),
                [](Element *, uint8_t e){ if (e == Ui::kClick) DOM::open_qq_group(); }, nullptr,
                { .fill = Chrome::kBtnSlate, .line_width = 0, .round_radius = Chrome::kRadius })
        }, 0, mobile ? 8.f : 10.f, { .h_justify = Style::Left }),
        new Ui::DynamicText(mobile ? 11.f : 13.f, [](){ return I18n::tr(I18n::kMadeBy); }, {
            .fill = Chrome::kMuted, .h_justify = Style::Left
        }),
        new Ui::DynamicText(mobile ? 10.f : 12.f, [](){ return I18n::tr(I18n::kAssetCredits); }, {
            .fill = Chrome::kMuted, .h_justify = Style::Left
        }),
        new Ui::Button(mobile ? 170.f : 260.f, mobile ? 26.f : 30.f,
            new Ui::DynamicText(mobile ? 10.f : 12.f, [](){
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
        new Ui::DynamicText(mobile ? 10.f : 11.f, [](){
            return std::string{"github.com/trigonal-bacon/gardn"};
        }, { .fill = 0xff8ab4a0, .h_justify = Style::Left })
    }, 0, mobile ? 8.f : 12.f, { .h_justify = Style::Left, .no_animation = 1 });

    Element *scroll_body = new Ui::VContainer({
        toggles_block,
        actions
    }, 0, section_gap, { .h_justify = Style::Left, .no_animation = 1 });

    auto *scroller = new AdaptiveScroll(scroll_body, 280.f);
    scroller->top_reserve = panel_y + header_h;
    scroller->bottom_pad = mobile ? 24.f : 32.f;

    Element *elt = new Ui::VContainer({
        new Ui::DynamicText(mobile ? 16.f : 24.f, [](){ return I18n::tr(I18n::kSettings); }, {
            .fill = Chrome::kTitle
        }),
        scroller
    }, pad, mobile ? 8.f : 12.f, {
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
    elt->y = panel_y;
    Ui::Panel::settings = elt;
    return elt;
}
