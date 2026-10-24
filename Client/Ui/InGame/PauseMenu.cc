#include <Client/Ui/InGame/GameInfo.hh>
#include <Client/Ui/InGame/HudIcons.hh>

#include <Client/Ui/Button.hh>
#include <Client/Ui/Chrome.hh>
#include <Client/Ui/Container.hh>
#include <Client/Ui/DynamicText.hh>
#include <Client/Ui/Extern.hh>

#include <Client/DOM.hh>
#include <Client/Game.hh>
#include <Client/I18n.hh>
#include <Client/Input.hh>

#include <Helpers/Math.hh>

#include <cmath>
#include <cstddef>
#include <string>
#include <vector>

using namespace Ui;

static Element *pause_toggle_row(uint8_t *flag, I18n::Str label) {
    uint8_t const m = Input::mobile_controls();
    float const toggle = m ? 32.f : 40.f;
    float const text = m ? 14.f : 22.f;
    float const gap = m ? 8.f : 14.f;
    return new Ui::HContainer({
        new Ui::ToggleButton(toggle, flag),
        new Ui::DynamicText(text, [label](){ return I18n::tr(label); }, {
            .fill = Chrome::kTitle,
            .h_justify = Style::Left
        })
    }, 0, gap, { .h_justify = Style::Left });
}

Element *Ui::make_pause_menu() {
    // Desktop keeps the large panel; mobile shrinks type so the sheet fits.
    uint8_t const m = Input::mobile_controls();
    float const title = m ? 20.f : 36.f;
    float const btn_label = m ? 15.f : 28.f;
    float const row_label = m ? 14.f : 22.f;
    float const section = m ? 12.f : 20.f;
    float const hint = m ? 11.f : 18.f;
    float const btn_w = m ? 220.f : 320.f;
    float const pad = m ? 14.f : 28.f;
    float const gap = m ? 8.f : 14.f;

    std::vector<Element *> toggles = {
        pause_toggle_row(&Input::keyboard_movement, I18n::kKeyboardMovement),
        pause_toggle_row(&Input::movement_helper, I18n::kMovementHelper),
        pause_toggle_row(&Input::invert_attack, I18n::kInvertAttack),
        pause_toggle_row(&Input::invert_defend, I18n::kInvertDefend),
        pause_toggle_row(&Input::disable_auto_party, I18n::kDisableAutoParty),
        pause_toggle_row(&Input::request_mobile, I18n::kRequestMobile),
        pause_toggle_row(&Game::show_debug, I18n::kDebugStats),
        pause_toggle_row(&Game::show_hitboxes, I18n::kShowHitboxes),
    };
    Element *toggles_block = nullptr;
    if (m) {
        // Same two-column sheet as title settings so it fits the phone viewport.
        size_t mid = (toggles.size() + 1) / 2;
        std::vector<Element *> left(toggles.begin(), toggles.begin() + (std::ptrdiff_t) mid);
        std::vector<Element *> right(toggles.begin() + (std::ptrdiff_t) mid, toggles.end());
        toggles_block = new Ui::HContainer({
            new Ui::VContainer(left, 0, 8, { .h_justify = Style::Left, .no_animation = 1 }),
            new Ui::VContainer(right, 0, 8, { .h_justify = Style::Left, .no_animation = 1 })
        }, 0, 12, { .h_justify = Style::Left, .no_animation = 1 });
    } else {
        toggles_block = new Ui::VContainer(toggles, 0, gap, {
            .h_justify = Style::Left, .no_animation = 1
        });
    }

    Element *elt = new Ui::VContainer({
        new Ui::DynamicText(title, [](){ return I18n::tr(I18n::kPauseMenu); }, {
            .fill = Chrome::kTitle,
            .h_justify = Style::Left
        }),
        new Ui::Button(btn_w, m ? 40.f : 64.f,
            new Ui::DynamicText(btn_label, [](){ return I18n::tr(I18n::kResume); }),
            [](Element *, uint8_t e){
                if (e == Ui::kClick) Game::pause_menu_open = 0;
            },
            nullptr,
            { .fill = Chrome::kBtnMoss, .line_width = 0, .round_radius = Chrome::kRadius }
        ),
        new Ui::Button(btn_w, m ? 36.f : 56.f,
            new Ui::DynamicText(row_label, [](){ return I18n::tr(I18n::kPartyInviteNear); }),
            [](Element *, uint8_t e){
                if (e == Ui::kClick) Game::party_invite_nearest();
            },
            nullptr,
            { .fill = Chrome::kBtnSlate, .line_width = 0, .round_radius = Chrome::kRadius }
        ),
        new Ui::Button(btn_w, m ? 36.f : 56.f,
            new Ui::DynamicText(row_label, [](){ return I18n::tr(I18n::kPartyLeave); }),
            [](Element *, uint8_t e){
                if (e == Ui::kClick) Game::party_leave();
            },
            [](){ return Game::party_member_count == 0; },
            { .fill = Chrome::kBtnIron, .line_width = 0, .round_radius = Chrome::kRadius }
        ),
        new Ui::DynamicText(section, [](){
            return I18n::use_chinese ? std::string{"—— 操作 ——"} : std::string{"— Controls —"};
        }, { .fill = Chrome::kMuted, .h_justify = Style::Left }),
        toggles_block,
        new Ui::Button(btn_w, m ? 34.f : 48.f,
            new Ui::DynamicText(section, [](){ return I18n::tr(I18n::kFullscreen); }),
            [](Element *, uint8_t e){
                if (e == Ui::kClick) DOM::toggle_fullscreen();
            },
            nullptr,
            { .fill = Chrome::kBtnSlate, .line_width = 0, .round_radius = Chrome::kRadius }
        ),
        new Ui::Button(btn_w, m ? 34.f : 48.f,
            new Ui::DynamicText(section, [](){ return I18n::use_chinese ? std::string{"给作者留言"} : std::string{"Feedback board"}; }),
            [](Element *, uint8_t e){ if (e == Ui::kClick) DOM::open_feedback_board(); }, nullptr,
            { .fill = Chrome::kBtnMoss, .line_width = 0, .round_radius = Chrome::kRadius }),
        new Ui::DynamicText(hint, [](){ return I18n::tr(I18n::kEscHint); }, {
            .fill = Chrome::kBody,
            .h_justify = Style::Left
        })
    }, pad, gap, {
        .fill = Chrome::kPanelDeep, .line_width = 0, .round_radius = Chrome::kRadius,
        .should_render = [](){
            return Game::pause_menu_open && Game::alive() && Game::should_render_game_ui();
        },
        .h_justify = Style::Left,
        .v_justify = Style::Top
    });
    elt->x = m ? 10.f : 16.f;
    elt->y = m ? 70.f : 64.f;
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

    // Boost sits at the right end of the top-left cluster (just before leave).
    Element *elt = new Ui::HContainer({
        gear,
        guild_btn,
        mob_btn,
        petal_btn,
        trophy_btn,
        boost_btn,
        exit_btn
    }, 10, 8, {
        .should_render = [](){
            return Game::alive() && Game::should_render_game_ui();
        },
        .h_justify = Style::Left,
        .v_justify = Style::Top
    });
    elt->x = 6;
    elt->y = 6;
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
            return Game::leave_confirm_open && Game::alive() && Game::should_render_game_ui();
        }
    });
    Ui::leave_confirm_root = elt;
    return elt;
}
