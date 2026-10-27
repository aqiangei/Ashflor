#include <Client/Ui/InGame/GameInfo.hh>
#include <Client/Ui/InGame/HudIcons.hh>

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

using namespace Ui;

namespace {

// Scroll viewport tracks browser size so the pause sheet never runs off-screen.
class AdaptiveScroll final : public Ui::ScrollContainer {
public:
    float top_reserve = 0;   // title + sticky buttons above the scroller
    float bottom_pad = 24.f;
    float min_h = 120.f;

    AdaptiveScroll(Element *content, float initial_h)
        : Ui::ScrollContainer(content, initial_h) {
        always_capture_wheel = 1;
    }

    void refactor() override {
        float view_h = (float) (Ui::window_height / Ui::scale);
        float avail = view_h - top_reserve - bottom_pad;
        if (avail < min_h) avail = min_h;
        // Cap so a tall monitor does not leave a huge empty scroll well.
        float const cap = view_h * 0.72f;
        if (avail > cap) avail = cap;
        // Measure content first; shrink-wrap when it fits so the panel
        // does not paint a tall empty well under the last controls.
        Element *body = children[0];
        body->refactor();
        float const need = body->height;
        height = need > 0.f && need < avail ? need : avail;
        if (height < 1.f) height = 1.f;
        Ui::ScrollContainer::refactor();
    }
};

static Element *pause_toggle_row(uint8_t *flag, I18n::Str label) {
    uint8_t const m = Input::mobile_controls();
    float const toggle = 28.f;
    float const text = m ? 11.f : 15.f;
    float const gap = m ? 6.f : 10.f;
    return new Ui::HContainer({
        new Ui::ToggleButton(toggle, flag),
        new Ui::DynamicText(text, [label](){ return I18n::tr(label); }, {
            .fill = Chrome::kTitle,
            .h_justify = Style::Left
        })
    }, 0, gap, { .h_justify = Style::Left });
}

} // namespace

Element *Ui::make_pause_menu() {
    uint8_t const m = Input::mobile_controls();
    float const title = m ? 15.f : 22.f;
    float const btn_label = m ? 12.f : 16.f;
    float const row_label = m ? 11.f : 15.f;
    float const section = m ? 10.f : 13.f;
    float const hint = m ? 10.f : 12.f;
    float const btn_w = m ? 148.f : 260.f;
    float const btn_h = m ? 28.f : 38.f;
    float const pad = m ? 8.f : 16.f;
    float const gap = m ? 6.f : 10.f;
    float const panel_y = 56.f;

    Element *controls_col = new Ui::VContainer({
        new Ui::DynamicText(section, [](){ return I18n::tr(I18n::kSettingsControls); }, {
            .fill = Chrome::kGold, .h_justify = Style::Left
        }),
        pause_toggle_row(&Input::keyboard_movement, I18n::kKeyboardMovement),
        pause_toggle_row(&Input::movement_helper, I18n::kMovementHelper),
        pause_toggle_row(&Input::invert_attack, I18n::kInvertAttack),
        pause_toggle_row(&Input::invert_defend, I18n::kInvertDefend),
        pause_toggle_row(&Input::request_mobile, I18n::kRequestMobile),
        pause_toggle_row(&Input::disable_auto_party, I18n::kDisableAutoParty),
    }, 0, m ? 4.f : 8.f, { .h_justify = Style::Left, .no_animation = 1 });

    Element *display_col = new Ui::VContainer({
        new Ui::DynamicText(section, [](){ return I18n::tr(I18n::kSettingsDisplay); }, {
            .fill = Chrome::kGold, .h_justify = Style::Left
        }),
        pause_toggle_row(&I18n::use_chinese, I18n::kLanguage),
        pause_toggle_row(&Game::show_low_rarity_drops, I18n::kShowLowRarityDrops),
        pause_toggle_row(&Game::show_hitboxes, I18n::kShowHitboxes),
        pause_toggle_row(&Game::show_debug, I18n::kDebugStats),
    }, 0, m ? 4.f : 8.f, { .h_justify = Style::Left, .no_animation = 1 });

    // Mobile: always two compact columns. Desktop: two columns when wide.
    float const view_w = (float) (Ui::window_width / Ui::scale);
    uint8_t const two_col = m || view_w >= 720.f;
    Element *toggles_block = two_col
        ? (Element *) new Ui::HContainer({ controls_col, display_col }, 0, m ? 12.f : 24.f, {
            .h_justify = Style::Left, .no_animation = 1
        })
        : (Element *) new Ui::VContainer({ controls_col, display_col }, 0, 12.f, {
            .h_justify = Style::Left, .no_animation = 1
        });

    Element *scroll_body = new Ui::VContainer({
        new Ui::Button(btn_w, btn_h,
            new Ui::DynamicText(row_label, [](){ return I18n::tr(I18n::kPartyInviteNear); }),
            [](Element *, uint8_t e){
                if (e == Ui::kClick) Game::party_invite_nearest();
            },
            nullptr,
            { .fill = Chrome::kBtnSlate, .line_width = 0, .round_radius = Chrome::kRadius }
        ),
        new Ui::Button(btn_w, btn_h,
            new Ui::DynamicText(row_label, [](){ return I18n::tr(I18n::kPartyLeave); }),
            [](Element *, uint8_t e){
                if (e == Ui::kClick) Game::party_leave();
            },
            [](){ return Game::party_member_count == 0; },
            { .fill = Chrome::kBtnIron, .line_width = 0, .round_radius = Chrome::kRadius }
        ),
        toggles_block,
        new Ui::Button(btn_w, btn_h,
            new Ui::DynamicText(section, [](){ return I18n::tr(I18n::kFullscreen); }),
            [](Element *, uint8_t e){
                if (e == Ui::kClick) DOM::toggle_fullscreen();
            },
            nullptr,
            { .fill = Chrome::kBtnSlate, .line_width = 0, .round_radius = Chrome::kRadius }
        ),
        new Ui::Button(btn_w, btn_h,
            new Ui::DynamicText(section, [](){
                return I18n::use_chinese ? std::string{"给作者留言"} : std::string{"Feedback board"};
            }),
            [](Element *, uint8_t e){ if (e == Ui::kClick) DOM::open_feedback_board(); }, nullptr,
            { .fill = Chrome::kBtnMoss, .line_width = 0, .round_radius = Chrome::kRadius }),
        new Ui::DynamicText(hint, [](){ return I18n::tr(I18n::kEscHint); }, {
            .fill = Chrome::kBody,
            .h_justify = Style::Left
        })
    }, 0, gap, { .h_justify = Style::Left, .no_animation = 1 });

    // Title + Resume stay pinned; the rest scrolls to fit the browser window.
    float const sticky_h = title + gap + btn_h + pad * 2.f;
    auto *scroller = new AdaptiveScroll(scroll_body, 280.f);
    scroller->top_reserve = panel_y + sticky_h;
    scroller->bottom_pad = 36.f;

    Element *elt = new Ui::VContainer({
        new Ui::DynamicText(title, [](){ return I18n::tr(I18n::kPauseMenu); }, {
            .fill = Chrome::kTitle,
            .h_justify = Style::Left
        }),
        new Ui::Button(btn_w, btn_h,
            new Ui::DynamicText(btn_label, [](){ return I18n::tr(I18n::kResume); }),
            [](Element *, uint8_t e){
                if (e == Ui::kClick) Game::pause_menu_open = 0;
            },
            nullptr,
            { .fill = Chrome::kBtnMoss, .line_width = 0, .round_radius = Chrome::kRadius }
        ),
        scroller
    }, pad, gap, {
        .fill = Chrome::kPanelDeep, .line_width = 0, .round_radius = Chrome::kRadius,
        .should_render = [](){
            return Game::pause_menu_open && Game::should_render_game_ui()
                && (Game::alive() || Game::on_game_screen);
        },
        .h_justify = Style::Left,
        .v_justify = Style::Top
    });
    elt->x = m ? 10.f : 16.f;
    elt->y = panel_y;
    Ui::pause_menu_root = elt;
    return elt;
}

Element *Ui::make_game_topbar() {
    Element *gear = new Ui::Button(40, 40,
        make_hud_icon(hud_icon_settings, 26.f),
        [](Element *, uint8_t e){
            if (e == Ui::kClick) Game::toggle_pause();
        },
        nullptr,
        { .fill = Chrome::kBtnIron, .line_width = 0, .round_radius = Chrome::kRadius }
    );

    Element *mob_btn = new Ui::Button(40, 40,
        make_hud_icon(hud_icon_mob_gallery, 26.f),
        [](Element *, uint8_t e){
            if (e == Ui::kClick) {
                Game::pause_menu_open = 0;
                Ui::toggle_panel(Panel::kMobs);
            }
        },
        [](){ return Ui::panel_open == Panel::kMobs; },
        { .fill = Chrome::kBtnMoss, .line_width = 0, .round_radius = Chrome::kRadius }
    );

    Element *petal_btn = new Ui::Button(40, 40,
        make_hud_icon(hud_icon_petal_gallery, 26.f),
        [](Element *, uint8_t e){
            if (e == Ui::kClick) {
                Game::pause_menu_open = 0;
                Ui::toggle_panel(Panel::kPetals);
            }
        },
        [](){ return Ui::panel_open == Panel::kPetals; },
        { .fill = Chrome::kBtnBronze, .line_width = 0, .round_radius = Chrome::kRadius }
    );

    Element *trophy_btn = new Ui::Button(40, 40,
        make_hud_icon(hud_icon_leaderboard, 26.f),
        [](Element *, uint8_t e){
            if (e == Ui::kClick) {
                Game::pause_menu_open = 0;
                Ui::toggle_panel(Panel::kLeaderboard);
            }
        },
        [](){ return Ui::panel_open == Panel::kLeaderboard; },
        { .fill = Chrome::kBtnClay, .line_width = 0, .round_radius = Chrome::kRadius }
    );

    Element *exit_btn = new Ui::Button(40, 40,
        make_hud_icon(hud_icon_exit, 26.f),
        [](Element *, uint8_t e){
            if (e != Ui::kClick) return;
            Game::close_chat();
            Game::close_game_panel();
            Game::pause_menu_open = 0;
            Ui::panel_open = Panel::kNone;
            Game::leave_confirm_open = 1;
            Input::keys_held.clear();
            Input::game_inputs.x = 0;
            Input::game_inputs.y = 0;
            Input::game_inputs.flags = 0;
        },
        nullptr,
        { .fill = Chrome::kBtnDanger, .line_width = 0, .round_radius = Chrome::kRadius }
    );

    Element *guild_btn = Ui::make_guild_top_button();

    Element *boost_btn = new Ui::Button(40, 40,
        make_hud_icon(hud_icon_boost, 26.f),
        [](Element *, uint8_t e){
            if (e == Ui::kClick) Game::toggle_game_panel(Game::kPanelBoost);
        },
        [](){ return Game::game_panel == Game::kPanelBoost; },
        { .fill = 0xffd5ad24, .line_width = 0, .round_radius = Chrome::kRadius }
    );

    Element *elt = new Ui::HContainer({
        gear, guild_btn, mob_btn, petal_btn, trophy_btn, boost_btn, exit_btn
    }, 8, 12, {
        .should_render = [](){
            // Death screen: keep settings / galleries / leave usable.
            return Game::should_render_game_ui()
                && (Game::alive() || Game::on_game_screen);
        },
        .h_justify = Style::Left,
        .v_justify = Style::Top
    });
    elt->x = 8;
    elt->y = 8;
    Ui::game_topbar_root = elt;
    return elt;
}

Element *Ui::make_leave_confirm() {
    Element *elt = new Ui::VContainer({
        new Ui::DynamicText(18, [](){ return I18n::tr(I18n::kLeaveConfirm); }, {
            .fill = Chrome::kTitle
        }),
        new Ui::HContainer({
            new Ui::Button(110, 38,
                new Ui::DynamicText(15, [](){ return I18n::tr(I18n::kLeaveNo); }),
                [](Element *, uint8_t e){
                    if (e == Ui::kClick) Game::leave_confirm_open = 0;
                },
                nullptr,
                { .fill = Chrome::kBtnIron, .line_width = 0, .round_radius = Chrome::kRadius }
            ),
            new Ui::Button(110, 38,
                new Ui::DynamicText(15, [](){ return I18n::tr(I18n::kLeaveYes); }),
                [](Element *, uint8_t e){
                    if (e == Ui::kClick) Game::return_to_title();
                },
                nullptr,
                { .fill = Chrome::kBtnDanger, .line_width = 0, .round_radius = Chrome::kRadius }
            )
        }, 0, 10)
    }, 18, 12, {
        .fill = Chrome::kPanelDeep, .line_width = 0, .round_radius = Chrome::kRadius,
        .should_render = [](){
            return Game::leave_confirm_open && Game::should_render_game_ui()
                && (Game::alive() || Game::on_game_screen);
        }
    });
    Ui::leave_confirm_root = elt;
    return elt;
}
