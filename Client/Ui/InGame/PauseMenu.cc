#include <Client/Ui/InGame/GameInfo.hh>

#include <Client/Ui/Button.hh>
#include <Client/Ui/Container.hh>
#include <Client/Ui/DynamicText.hh>
#include <Client/Ui/Extern.hh>
#include <Client/Ui/StaticIcon.hh>

#include <Client/Game.hh>
#include <Client/I18n.hh>
#include <Client/Input.hh>

#include <Helpers/Math.hh>

#include <cmath>
#include <string>

using namespace Ui;

static Element *pause_toggle_row(uint8_t *flag, I18n::Str label) {
    return new Ui::HContainer({
        new Ui::ToggleButton(40, flag),
        new Ui::DynamicText(22, [label](){ return I18n::tr(label); }, {
            .fill = 0xffffffff,
            .h_justify = Style::Left
        })
    }, 0, 14, { .h_justify = Style::Left });
}

Element *Ui::make_pause_menu() {
    // ~2x previous pause panel (was ~160px controls)
    Element *elt = new Ui::VContainer({
        new Ui::DynamicText(36, [](){ return I18n::tr(I18n::kPauseMenu); }, {
            .fill = 0xffffffff,
            .h_justify = Style::Left
        }),
        new Ui::Button(320, 64,
            new Ui::DynamicText(28, [](){ return I18n::tr(I18n::kResume); }),
            [](Element *, uint8_t e){
                if (e == Ui::kClick) Game::pause_menu_open = 0;
            },
            nullptr,
            { .fill = 0xff1dd129, .line_width = 5, .round_radius = 6 }
        ),
        new Ui::Button(320, 56,
            new Ui::DynamicText(22, [](){ return I18n::tr(I18n::kPartyInviteNear); }),
            [](Element *, uint8_t e){
                if (e == Ui::kClick) Game::party_invite_nearest();
            },
            nullptr,
            { .fill = 0xff5a9fdb, .line_width = 5, .round_radius = 6 }
        ),
        new Ui::Button(320, 56,
            new Ui::DynamicText(22, [](){ return I18n::tr(I18n::kPartyLeave); }),
            [](Element *, uint8_t e){
                if (e == Ui::kClick) Game::party_leave();
            },
            [](){ return Game::party_member_count == 0; },
            { .fill = 0xff888888, .line_width = 5, .round_radius = 6 }
        ),
        new Ui::DynamicText(20, [](){
            return I18n::use_chinese ? std::string{"—— 操作 ——"} : std::string{"— Controls —"};
        }, { .fill = 0xffaaaaaa, .h_justify = Style::Left }),
        pause_toggle_row(&Input::keyboard_movement, I18n::kKeyboardMovement),
        pause_toggle_row(&Input::movement_helper, I18n::kMovementHelper),
        pause_toggle_row(&Input::invert_attack, I18n::kInvertAttack),
        pause_toggle_row(&Input::invert_defend, I18n::kInvertDefend),
        new Ui::DynamicText(18, [](){ return I18n::tr(I18n::kEscHint); }, {
            .fill = 0xffdddddd,
            .h_justify = Style::Left
        })
    }, 28, 16, {
        .fill = 0xc0222222,
        .line_width = 5,
        .round_radius = 10,
        .should_render = [](){
            return Game::pause_menu_open && Game::alive() && Game::should_render_game_ui();
        },
        .h_justify = Style::Left,
        .v_justify = Style::Top
    });
    elt->x = 16;
    elt->y = 72;
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
        { .fill = 0xff5a5a5a, .line_width = 4, .round_radius = 8 }
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
        { .fill = 0xff6db36d, .line_width = 4, .round_radius = 8 }
    );

    Element *trophy_btn = new Ui::Button(40, 40, trophy_icon,
        [](Element *, uint8_t e){
            if (e == Ui::kClick) {
                Game::pause_menu_open = 0;
                Ui::toggle_panel(Panel::kLeaderboard);
            }
        },
        [](){ return Ui::panel_open == Panel::kLeaderboard; },
        { .fill = 0xffd94a4a, .line_width = 4, .round_radius = 8 }
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
        { .fill = 0xff7ec850, .line_width = 4, .round_radius = 8 }
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
        { .fill = 0xffe03a3a, .line_width = 4, .round_radius = 8 }
    );

    Element *elt = new Ui::HContainer({
        gear,
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
            .fill = 0xffffffff
        }),
        new Ui::HContainer({
            new Ui::Button(110, 36,
                new Ui::DynamicText(15, [](){ return I18n::tr(I18n::kLeaveNo); }),
                [](Element *, uint8_t e){
                    if (e == Ui::kClick) Game::leave_confirm_open = 0;
                },
                nullptr,
                { .fill = 0xff888888, .line_width = 4, .round_radius = 3 }
            ),
            new Ui::Button(110, 36,
                new Ui::DynamicText(15, [](){ return I18n::tr(I18n::kLeaveYes); }),
                [](Element *, uint8_t e){
                    if (e == Ui::kClick) Game::return_to_title();
                },
                nullptr,
                { .fill = 0xffe03a3a, .line_width = 4, .round_radius = 3 }
            )
        }, 0, 10)
    }, 16, 12, {
        .fill = 0xe0222222,
        .line_width = 4,
        .round_radius = 8,
        .should_render = [](){
            return Game::leave_confirm_open && Game::alive() && Game::should_render_game_ui();
        }
    });
    Ui::leave_confirm_root = elt;
    return elt;
}
