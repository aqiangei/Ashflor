#include <Client/Ui/InGame/GameInfo.hh>

#include <Client/Ui/Button.hh>
#include <Client/Ui/Chrome.hh>
#include <Client/Ui/Container.hh>
#include <Client/Ui/DynamicText.hh>
#include <Client/Ui/StaticIcon.hh>
#include <Client/Ui/TextInput.hh>

#include <Client/Game.hh>
#include <Client/I18n.hh>
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
    Element *elt = new Ui::VContainer({
        new Ui::DynamicText(14, [](){
            if (Game::invite_type == InviteType::kInviteGuild)
                return I18n::tr(I18n::kInviteGuildTitle);
            return I18n::tr(I18n::kInvitePartyTitle);
        }, { .fill = Chrome::kTitle, .h_justify = Style::Left }),
        new Ui::DynamicText(13, [](){
            return Game::invite_from;
        }, { .fill = Chrome::kGold, .h_justify = Style::Left }),
        new Ui::HContainer({
            new Ui::Button(88, 30,
                new Ui::DynamicText(13, [](){ return I18n::tr(I18n::kInviteAccept); }),
                [](Element *, uint8_t e){
                    if (e == Ui::kClick) Game::invite_respond(1);
                },
                nullptr,
                { .fill = Chrome::kBtnMoss, .line_width = 4, .round_radius = Chrome::kRadius }),
            new Ui::Button(88, 30,
                new Ui::DynamicText(13, [](){ return I18n::tr(I18n::kInviteReject); }),
                [](Element *, uint8_t e){
                    if (e == Ui::kClick) Game::invite_respond(0);
                },
                nullptr,
                { .fill = Chrome::kBtnDanger, .line_width = 4, .round_radius = Chrome::kRadius })
        }, 0, 8, { .h_justify = Style::Left })
    }, 12, 8, {
        .fill = Chrome::kPanelDeep,
        .line_width = 5,
        .round_radius = Chrome::kRadius,
        .animate = [](Element *elt, Renderer &ctx){
            ctx.translate((1.f - (float) elt->animation) * -40.f, 0);
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
    // Below topbar row (gear ~40 + pad)
    elt->x = 12;
    elt->y = 58;
    return elt;
}

Element *Ui::make_guild_panel() {
    Element *name_input = new Ui::TextInput(Game::guild_create_draft, 160, 28, 12, {
        .line_width = 2,
        .round_radius = 4,
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
                { .fill = Chrome::kBtnDanger, .line_width = 4, .round_radius = Chrome::kRadius }),
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
                    .fill = Chrome::kBtnMoss,
                    .line_width = 4,
                    .round_radius = Chrome::kRadius,
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
                .fill = Chrome::kBtnIron,
                .line_width = 4,
                .round_radius = Chrome::kRadius,
                .should_render = [](){ return !Game::guild_name.empty(); }
            }),
        new Ui::DynamicText(11, [](){
            return I18n::use_chinese
                ? "聊天框右键玩家名 → 邀请入公会 / 组队 / 私信"
                : "Right-click a chat name → guild / party / whisper";
        }, { .fill = Chrome::kMuted, .h_justify = Style::Left })
    }, 14, 8, {
        .fill = Chrome::kPanelDeep,
        .line_width = 6,
        .round_radius = Chrome::kRadius,
        .animate = [](Element *elt, Renderer &ctx){
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
    elt->x = 12;
    elt->y = 58;
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
        { .fill = Chrome::kBtnSlate, .line_width = 5, .round_radius = Chrome::kRadius }
    );
}
