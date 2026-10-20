#include <Client/Ui/InGame/GameInfo.hh>

#include <Client/Ui/Button.hh>
#include <Client/Ui/Chrome.hh>
#include <Client/Ui/Container.hh>
#include <Client/Ui/DynamicText.hh>
#include <Client/Ui/StaticIcon.hh>
#include <Client/Ui/TextInput.hh>

#include <Client/Game.hh>
#include <Client/I18n.hh>
#include <Client/Input.hh>
#include <Client/Render/Renderer.hh>

#include <Shared/Binary.hh>

#include <cmath>
#include <string>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace Ui;

// IconPark "every-user" spirit — two people (heads + torso arcs)
static void iconpark_guild(Renderer &ctx) {
    float s = 40.f / 48.f;
    ctx.scale(s);
    ctx.translate(-24.f, -24.f);
    ctx.set_fill(0xffffffff);
    ctx.set_stroke(0xffffffff);
    ctx.set_line_width(3.2f);
    ctx.round_line_cap();
    ctx.round_line_join();
    // left person
    ctx.begin_path();
    ctx.arc(15.f, 13.f, 5.5f);
    ctx.fill();
    ctx.begin_path();
    ctx.partial_arc(15.f, 36.f, 11.f, (float) M_PI, 0.f, 0);
    ctx.stroke();
    // right person
    ctx.begin_path();
    ctx.arc(33.f, 14.f, 5.f);
    ctx.fill();
    ctx.begin_path();
    ctx.partial_arc(33.f, 36.f, 10.f, (float) M_PI, 0.f, 0);
    ctx.stroke();
}

Element *Ui::make_invite_toast() {
    // One card only (new invite replaces). Mobile: tappable but not huge —
    // close stays <56px wide so Button won't inflate it to a 64² blob.
    uint8_t const mobile = Input::mobile_controls();
    float const title_sz = mobile ? 15.f : 14.f;
    float const name_sz = mobile ? 14.f : 13.f;
    float const btn_label = mobile ? 14.f : 13.f;
    float const close_w = mobile ? 48.f : 32.f;
    float const close_h = mobile ? 40.f : 28.f;
    float const act_w = mobile ? 100.f : 88.f;
    float const act_h = mobile ? 40.f : 32.f;
    float const pad = mobile ? 10.f : 12.f;
    float const gap = mobile ? 8.f : 8.f;

    Element *elt = new Ui::VContainer({
        new Ui::HFlexContainer(
            new Ui::DynamicText(title_sz, [](){
                if (Game::invite_type == InviteType::kInviteGuild)
                    return I18n::tr(I18n::kInviteGuildTitle);
                return I18n::tr(I18n::kInvitePartyTitle);
            }, { .fill = Chrome::kTitle, .h_justify = Style::Left }),
            new Ui::Button(close_w, close_h,
                new Ui::DynamicText(mobile ? 16.f : 14.f, [](){ return std::string{"X"}; }),
                [](Element *, uint8_t e){
                    if (e == Ui::kClick) Game::invite_dismiss();
                },
                nullptr,
                { .fill = Chrome::kBtnIron, .line_width = 0, .round_radius = Chrome::kRadius }),
            0, 8, {}
        ),
        new Ui::DynamicText(name_sz, [](){
            if (Game::invite_from.empty()) return std::string{};
            if (I18n::use_chinese)
                return std::string{"来自 "} + Game::invite_from;
            return std::string{"From "} + Game::invite_from;
        }, { .fill = Chrome::kGold, .h_justify = Style::Left }),
        new Ui::HContainer({
            new Ui::Button(act_w, act_h,
                new Ui::DynamicText(btn_label, [](){ return I18n::tr(I18n::kInviteAccept); }),
                [](Element *, uint8_t e){
                    if (e == Ui::kClick) Game::invite_respond(1);
                },
                nullptr,
                { .fill = Chrome::kBtnMoss, .line_width = 0, .round_radius = Chrome::kRadius }),
            new Ui::Button(act_w, act_h,
                new Ui::DynamicText(btn_label, [](){ return I18n::tr(I18n::kInviteReject); }),
                [](Element *, uint8_t e){
                    if (e == Ui::kClick) Game::invite_respond(0);
                },
                nullptr,
                { .fill = Chrome::kBtnDanger, .line_width = 0, .round_radius = Chrome::kRadius })
        }, 0, gap, { .h_justify = Style::Left })
    }, pad, gap, {
        .fill = Chrome::kPanelDeep, .line_width = 0, .round_radius = Chrome::kRadius,
        .animate = [](Element *elt, Renderer &ctx){
            // Slide in from left; re-anchor under topbar each frame (mobile toggle)
            uint8_t const m = Input::mobile_controls();
            elt->x = m ? 10.f : 12.f;
            // Topbar bottom ≈ 66; sit just under it, above / over HP strip briefly
            elt->y = m ? 70.f : 68.f;
            ctx.translate((1.f - (float) elt->animation) * -48.f, 0);
        },
        .should_render = [](){
            return Game::invite_open
                && Game::alive()
                && Game::should_render_game_ui();
        },
        .h_justify = Style::Left,
        .v_justify = Style::Top,
        .layer = 1
    });
    elt->x = mobile ? 10.f : 12.f;
    elt->y = mobile ? 70.f : 68.f;
    return elt;
}

Element *Ui::make_guild_panel() {
    Element *name_input = new Ui::TextInput(Game::guild_create_draft, 160, 32, 12, {
        .line_width = 0,
        .round_radius = Chrome::kRadius,
        .should_render = [](){
            return Game::guild_panel_open && Game::guild_name.empty();
        }
    });

    Element *elt = new Ui::VContainer({
        new Ui::HFlexContainer(
            new Ui::DynamicText(20, [](){ return I18n::tr(I18n::kGuildTitle); }, {
                .fill = Chrome::kTitle
            }),
            new Ui::Button(32, 28,
                new Ui::DynamicText(14, [](){ return std::string{"X"}; }),
                [](Element *, uint8_t e){
                    if (e == Ui::kClick) Game::close_guild_panel();
                },
                nullptr,
                { .fill = Chrome::kBtnDanger, .line_width = 0, .round_radius = Chrome::kRadius }),
            0, 8, {}
        ),
        new Ui::DynamicText(13, []() -> std::string {
            if (Game::guild_name.empty())
                return I18n::tr(I18n::kGuildEmpty);
            return Game::guild_name;
        }, { .fill = Chrome::kGold, .h_justify = Style::Left }),
        new Ui::DynamicText(12, [](){
            return std::string{I18n::tr(I18n::kGuildMembers)} + ": " + Game::guild_status;
        }, {
            .fill = Chrome::kBody,
            .should_render = [](){ return !Game::guild_name.empty(); },
            .h_justify = Style::Left
        }),
        new Ui::DynamicText(11, [](){
            return I18n::tr(I18n::kGuildCreateHint);
        }, {
            .fill = Chrome::kMuted,
            .should_render = [](){ return Game::guild_name.empty(); },
            .h_justify = Style::Left
        }),
        new Ui::HContainer({
            name_input,
            new Ui::Button(72, 28,
                new Ui::DynamicText(13, [](){ return I18n::tr(I18n::kGuildCreate); }),
                [](Element *, uint8_t e){
                    if (e == Ui::kClick) Game::guild_create();
                },
                nullptr,
                {
                    .fill = Chrome::kBtnMoss, .line_width = 0, .round_radius = Chrome::kRadius,
                    .should_render = [](){ return Game::guild_name.empty(); }
                })
        }, 0, 8, { .h_justify = Style::Left }),
        new Ui::Button(140, 30,
            new Ui::DynamicText(13, [](){ return I18n::tr(I18n::kGuildLeave); }),
            [](Element *, uint8_t e){
                if (e == Ui::kClick) Game::guild_leave();
            },
            nullptr,
            {
                .fill = Chrome::kBtnIron, .line_width = 0, .round_radius = Chrome::kRadius,
                .should_render = [](){ return !Game::guild_name.empty(); }
            }),
        new Ui::DynamicText(11, [](){
            return I18n::use_chinese
                ? "聊天框右键玩家名 → 邀请入公会 / 组队 / 私信"
                : "Right-click a chat name → guild / party / whisper";
        }, { .fill = Chrome::kMuted, .h_justify = Style::Left })
    }, 14, 8, {
        .fill = Chrome::kPanelDeep, .line_width = 0, .round_radius = Chrome::kRadius,
        .animate = [](Element *elt, Renderer &ctx){
            // Yield top-left to the invite toast when both would stack
            uint8_t const m = Input::mobile_controls();
            elt->x = m ? 10.f : 12.f;
            elt->y = (Game::invite_open ? (m ? 210.f : 190.f) : (m ? 70.f : 68.f));
            ctx.translate(0, (1.f - (float) elt->animation) * -20.f);
        },
        .should_render = [](){
            return Game::guild_panel_open
                && Game::alive()
                && Game::should_render_game_ui();
        },
        .h_justify = Style::Left,
        .v_justify = Style::Top,
        .layer = 1
    });
    elt->x = Input::mobile_controls() ? 10.f : 12.f;
    elt->y = Input::mobile_controls() ? 70.f : 68.f;
    return elt;
}

Element *Ui::make_guild_top_button() {
    return new Ui::Button(40, 40,
        new Ui::StaticIcon([](Element *elt, Renderer &ctx){
            iconpark_guild(ctx);
            (void) elt;
        }, 26, 26),
        [](Element *, uint8_t e){
            if (e == Ui::kClick) {
                Game::pause_menu_open = 0;
                Game::toggle_guild_panel();
            }
        },
        [](){ return Game::guild_panel_open != 0; },
        { .fill = Chrome::kBtnSlate, .line_width = 0, .round_radius = Chrome::kRadius }
    );
}
