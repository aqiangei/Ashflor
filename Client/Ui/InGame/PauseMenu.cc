#include <Client/Ui/InGame/GameInfo.hh>

#include <Client/Ui/Button.hh>
#include <Client/Ui/Chrome.hh>
#include <Client/Ui/Container.hh>
#include <Client/Ui/DynamicText.hh>
#include <Client/Ui/Extern.hh>
#include <Client/Ui/StaticIcon.hh>

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
        pause_toggle_row(&Input::request_mobile, I18n::kRequestMobile),
        pause_toggle_row(&Game::show_debug, I18n::kDebugStats),
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
        new Ui::StaticIcon([](Element *elt, Renderer &ctx){
            float r = elt->width * 0.38f;
            ctx.set_fill(0xffffffff);
            ctx.begin_path();
            ctx.arc(0, 0, r);
            ctx.fill();
            ctx.set_fill(0xff5a5a5a);
            ctx.begin_path();
            ctx.arc(0, 0, r * 0.42f);
            ctx.fill();
            ctx.set_stroke(0xffffffff);
            ctx.set_line_width(elt->width * 0.18f);
            ctx.round_line_cap();
            for (int i = 0; i < 6; ++i) {
                float a = i * (2 * M_PI / 6);
                ctx.begin_path();
                ctx.move_to(cosf(a) * r * 0.55f, sinf(a) * r * 0.55f);
                ctx.line_to(cosf(a) * r * 0.95f, sinf(a) * r * 0.95f);
                ctx.stroke();
            }
        }, 22, 22),
        [](Element *, uint8_t e){
            if (e == Ui::kClick) Game::toggle_pause();
        },
        nullptr,
        { .fill = Chrome::kBtnIron, .line_width = 0, .round_radius = Chrome::kRadius }
    );

    Element *mob_icon = new Ui::StaticIcon([](Element *elt, Renderer &ctx){
        float s = elt->width * 0.22f;
        ctx.set_fill(0xffffffff);
        ctx.begin_path();
        ctx.arc(0, s * 0.55f, s * 1.15f);
        ctx.fill();
        ctx.begin_path();
        ctx.arc(-s * 1.1f, -s * 0.35f, s * 0.55f);
        ctx.arc(s * 1.1f, -s * 0.35f, s * 0.55f);
        ctx.arc(-s * 0.4f, -s * 1.05f, s * 0.5f);
        ctx.arc(s * 0.4f, -s * 1.05f, s * 0.5f);
        ctx.fill();
    }, 22, 22);

    Element *trophy_icon = new Ui::StaticIcon([](Element *elt, Renderer &ctx){
        float s = elt->width * 0.32f;
        ctx.set_fill(0xffffffff);
        ctx.begin_path();
        ctx.move_to(-s * 0.7f, -s * 0.85f);
        ctx.line_to(s * 0.7f, -s * 0.85f);
        ctx.line_to(s * 0.5f, s * 0.05f);
        ctx.line_to(0, s * 0.35f);
        ctx.line_to(-s * 0.5f, s * 0.05f);
        ctx.close_path();
        ctx.fill();
        ctx.begin_path();
        ctx.round_rect(-s * 0.18f, s * 0.2f, s * 0.36f, s * 0.4f, 1);
        ctx.fill();
        ctx.begin_path();
        ctx.round_rect(-s * 0.55f, s * 0.55f, s * 1.1f, s * 0.28f, 2);
        ctx.fill();
    }, 22, 22);

    Element *mob_btn = new Ui::Button(40, 40, mob_icon,
        [](Element *, uint8_t e){
            if (e == Ui::kClick) {
                Game::pause_menu_open = 0;
                Ui::toggle_panel(Panel::kMobs);
            }
        },
        [](){ return Ui::panel_open == Panel::kMobs; },
        { .fill = Chrome::kBtnMoss, .line_width = 0, .round_radius = Chrome::kRadius }
    );

    Element *trophy_btn = new Ui::Button(40, 40, trophy_icon,
        [](Element *, uint8_t e){
            if (e == Ui::kClick) {
                Game::pause_menu_open = 0;
                Ui::toggle_panel(Panel::kLeaderboard);
            }
        },
        [](){ return Ui::panel_open == Panel::kLeaderboard; },
        { .fill = Chrome::kBtnClay, .line_width = 0, .round_radius = Chrome::kRadius }
    );

    Element *changelog_icon = new Ui::StaticIcon([](Element *elt, Renderer &ctx){
        float w = elt->width * 0.55f;
        float h = elt->height * 0.65f;
        ctx.set_fill(0xffffffff);
        ctx.begin_path();
        ctx.round_rect(-w / 2, -h / 2, w, h, 2);
        ctx.fill();
        ctx.set_stroke(0xff5a8a3a);
        ctx.set_line_width(2);
        ctx.begin_path();
        ctx.move_to(0, -h / 2 + 2);
        ctx.line_to(0, h / 2 - 2);
        ctx.stroke();
        ctx.set_stroke(0xffa8d878);
        ctx.set_line_width(1.5f);
        for (int i = 0; i < 3; ++i) {
            float y = -h * 0.2f + i * h * 0.22f;
            ctx.begin_path();
            ctx.move_to(-w * 0.32f, y);
            ctx.line_to(-w * 0.08f, y);
            ctx.move_to(w * 0.08f, y);
            ctx.line_to(w * 0.32f, y);
            ctx.stroke();
        }
    }, 22, 22);

    Element *changelog_btn = new Ui::Button(40, 40, changelog_icon,
        [](Element *, uint8_t e){
            if (e == Ui::kClick) {
                Game::pause_menu_open = 0;
                Ui::toggle_panel(Panel::kChangelog);
            }
        },
        [](){ return Ui::panel_open == Panel::kChangelog; },
        { .fill = Chrome::kBtnBronze, .line_width = 0, .round_radius = Chrome::kRadius }
    );

    Element *exit_btn = new Ui::Button(40, 40,
        new Ui::StaticIcon([](Element *elt, Renderer &ctx){
            float s = elt->width * 0.32f;
            ctx.set_stroke(0xffffffff);
            ctx.set_line_width(elt->width * 0.18f);
            ctx.round_line_cap();
            ctx.begin_path();
            ctx.move_to(-s, -s);
            ctx.line_to(s, s);
            ctx.move_to(s, -s);
            ctx.line_to(-s, s);
            ctx.stroke();
        }, 22, 22),
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
        new Ui::StaticIcon([](Element *, Renderer &ctx){
            ctx.set_fill(0xffffffff);
            ctx.begin_path();
            for (int i = 0; i < 10; ++i) {
                float a = -M_PI / 2 + i * M_PI / 5;
                float r = (i & 1) ? 8.f : 18.f;
                float x = cosf(a) * r, y = sinf(a) * r;
                if (i == 0) ctx.move_to(x, y);
                else ctx.line_to(x, y);
            }
            ctx.close_path();
            ctx.fill();
        }, 25, 25),
        [](Element *, uint8_t e){
            if (e == Ui::kClick) Game::toggle_game_panel(Game::kPanelBoost);
        },
        [](){ return Game::game_panel == Game::kPanelBoost; },
        { .fill = 0xffd5ad24, .line_width = 0, .round_radius = Chrome::kRadius }
    );

    Element *elt = new Ui::HContainer({
        gear,
        boost_btn,
        guild_btn,
        mob_btn,
        trophy_btn,
        changelog_btn,
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
