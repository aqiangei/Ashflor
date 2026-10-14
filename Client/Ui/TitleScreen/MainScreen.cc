#include <Client/Ui/TitleScreen/TitleScreen.hh>

#include <Client/Ui/Button.hh>
#include <Client/Ui/Chrome.hh>
#include <Client/Ui/Choose.hh>
#include <Client/Ui/Container.hh>
#include <Client/Ui/DynamicText.hh>
#include <Client/Ui/Extern.hh>
#include <Client/Ui/StaticIcon.hh>
#include <Client/Ui/TextInput.hh>

#include <Client/Debug.hh>
#include <Client/DOM.hh>
#include <Client/Game.hh>
#include <Client/I18n.hh>
#include <Client/Input.hh>

#include <Shared/Config.hh>
#include <Shared/StaticDefinitions.hh>

#include <cmath>
#include <format>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace Ui;

static TextInput *auth_password_input = nullptr;

static void select_map_garden(Element *, uint8_t e) {
    if (e == Ui::kClick) Game::selected_map = MapID::kGarden;
}
static void select_map_desert(Element *, uint8_t e) {
    if (e == Ui::kClick) Game::selected_map = MapID::kDesert;
}
static void select_map_ocean(Element *, uint8_t e) {
    if (e == Ui::kClick) Game::selected_map = MapID::kOcean;
}
static void select_map_polar(Element *, uint8_t e) {
    if (e == Ui::kClick) Game::selected_map = MapID::kPolar;
}
static void select_map_jungle(Element *, uint8_t e) {
    if (e == Ui::kClick) Game::selected_map = MapID::kJungle;
}
static void select_map_ant_hell(Element *, uint8_t e) {
    if (e == Ui::kClick) Game::selected_map = MapID::kAntHell;
}
static void select_map_volcano(Element *, uint8_t e) {
    if (e == Ui::kClick) Game::selected_map = MapID::kVolcano;
}
static void select_map_royal(Element *, uint8_t e) {
    if (e == Ui::kClick) Game::selected_map = MapID::kRoyalCapital;
}
static bool darken_map_garden() { return Game::selected_map != MapID::kGarden; }
static bool darken_map_desert() { return Game::selected_map != MapID::kDesert; }
static bool darken_map_ocean() { return Game::selected_map != MapID::kOcean; }
static bool darken_map_polar() { return Game::selected_map != MapID::kPolar; }
static bool darken_map_jungle() { return Game::selected_map != MapID::kJungle; }
static bool darken_map_ant_hell() { return Game::selected_map != MapID::kAntHell; }
static bool darken_map_volcano() { return Game::selected_map != MapID::kVolcano; }
static bool darken_map_royal() { return Game::selected_map != MapID::kRoyalCapital; }

static Element *make_map_select_button(
    uint8_t map_id,
    uint32_t fill,
    void (*on_click)(Element *, uint8_t),
    bool (*should_darken)()
) {
    return new Ui::Button(100, 36,
        new Ui::DynamicText(18, [map_id](){
            return std::string{I18n::map_name(map_id)};
        }),
        on_click,
        should_darken,
        { .fill = fill, .line_width = 5, .round_radius = 3 }
    );
}

static Element *make_title_how_to_play() {
    return new Ui::Choose(
        new Ui::VContainer({
            new Ui::DynamicText(28, [](){ return I18n::tr(I18n::kHowToPlay); }),
            new Ui::DynamicText(15, [](){ return I18n::tr(I18n::kMouseMove); }),
            new Ui::DynamicText(15, [](){ return I18n::tr(I18n::kLeftAttack); }),
            new Ui::DynamicText(15, [](){ return I18n::tr(I18n::kRightDefend); })
        }, 8, 6, { .no_animation = 1 }),
        new Ui::VContainer({
            new Ui::DynamicText(28, [](){ return I18n::tr(I18n::kHowToPlay); }),
            new Ui::DynamicText(15, [](){ return I18n::tr(I18n::kWasdMove); }),
            new Ui::DynamicText(15, [](){ return I18n::tr(I18n::kSpaceAttack); }),
            new Ui::DynamicText(15, [](){ return I18n::tr(I18n::kShiftDefend); }),
            new Ui::DynamicText(13, [](){
                return I18n::use_chinese
                    ? "Esc 菜单可组队；队友共享掉落；尸体旁站 3 秒可复活"
                    : "Esc menu: party up; share drops; stand 3s to revive";
            })
        }, 8, 6, { .no_animation = 1 }),
        [](){ return Input::keyboard_movement; },
        { .no_animation = 1, .no_polling = 1 }
    );
}

static Element *make_title_respawn_preview() {
    return new Ui::VContainer({
        new Ui::HContainer({
            new Ui::DynamicText(16, [](){
                return I18n::format_respawn_level(Game::respawn_level);
            }, { .fill = 0xffffffff, .no_animation = 1 }),
            new Ui::DynamicText(16, [](){ return I18n::tr(I18n::kWith); }, {
                .fill = 0xffffffff
            })
        }, 0, 6, { .h_justify = Style::Left }),
        new Ui::DynamicText(13, [](){
            return I18n::use_chinese
                ? "点击两个槽位交换装备（出生前可整理）"
                : "Tap two slots to swap before spawning";
        }, { .fill = 0xffcccccc, .h_justify = Style::Left }),
        new Ui::HContainer(
            Ui::make_range(0, MAX_SLOT_COUNT, [](uint32_t i){
                return (Element *) (new Ui::TitlePetalSlot((uint8_t) i));
            }),
            0, 8, { .h_justify = Style::Left }
        ),
        new Ui::HContainer(
            Ui::make_range(MAX_SLOT_COUNT, 2 * MAX_SLOT_COUNT, [](uint32_t i){
                return (Element *) (new Ui::TitlePetalSlot((uint8_t) i));
            }),
            0, 8, { .h_justify = Style::Left }
        )
    }, 10, 8, {
        .should_render = [](){
            return !Game::alive()
                && Game::simulation_ready
                && Game::simulation.ent_exists(Game::camera_id)
                && (Game::respawn_level > 1 || Game::guest_save_valid || Game::account_logged_in);
        },
        .no_animation = 1
    });
}

Element *Ui::make_title_input_box() {
    // Single vertical flow — no fixed-y sibling that can overlap when auth grows
    Ui::Element *title = new Ui::VContainer({
        new Ui::Element(0, 36),
        new Ui::Choose(
            new Ui::DynamicText(40, [](){ return I18n::tr(I18n::kConnecting); }),
            new Ui::VContainer({
                new Ui::DynamicText(19, [](){
                    if (Game::account_logged_in)
                        return I18n::use_chinese
                            ? std::string{"账号已登录："} + Game::username
                            : std::string{"Signed in: "} + Game::username;
                    if (Game::session_token != 0)
                        return I18n::use_chinese
                            ? std::string{"正在恢复账号："} + Game::username
                            : std::string{"Restoring account: "} + Game::username;
                    return I18n::use_chinese
                        ? std::string{"游客模式 · 本机进度"}
                        : std::string{"Guest mode · local progress"};
                }, { .fill = Chrome::kGold }),
                // Logged in: account name. Guest: fixed assigned id (no free-form name).
                new Ui::Choose(
                    new Ui::VContainer({
                        new Ui::DynamicText(18, [](){
                            return I18n::use_chinese
                                ? std::string{"游客固定编号（不可自定义）"}
                                : std::string{"Guest id (fixed, not editable)"};
                        }, { .fill = Chrome::kMuted }),
                        new Ui::HContainer({
                            new Ui::DynamicText(28, [](){
                                Game::ensure_guest_id();
                                return Game::guest_id;
                            }, { .fill = Chrome::kGold }),
                            new Ui::Button(110, 38,
                                new Ui::DynamicText(25, [](){ return I18n::tr(I18n::kSpawn); }),
                                [](Element *, uint8_t e){ if (e == Ui::kClick) Game::spawn_in(); },
                                [](){ return Game::in_game() != 0; },
                                { .fill = Chrome::kBtnMoss, .line_width = 5, .round_radius = Chrome::kRadius }
                            )
                        }, 0, 14, {})
                    }, 8, 8, { .no_animation = 1 }),
                    new Ui::VContainer({
                        new Ui::DynamicText(18, [](){
                            return I18n::use_chinese
                                ? std::string{"游戏名（账号）"}
                                : std::string{"Game name (account)"};
                        }, { .fill = Chrome::kMuted }),
                        new Ui::HContainer({
                            new Ui::DynamicText(32, [](){ return Game::username; }, {
                                .fill = Chrome::kTitle
                            }),
                            new Ui::Button(110, 38,
                                new Ui::DynamicText(25, [](){ return I18n::tr(I18n::kSpawn); }),
                                [](Element *, uint8_t e){ if (e == Ui::kClick) Game::spawn_in(); },
                                [](){ return Game::in_game() != 0; },
                                { .fill = Chrome::kBtnMoss, .line_width = 5, .round_radius = Chrome::kRadius }
                            )
                        }, 0, 14, {})
                    }, 8, 8, { .no_animation = 1 }),
                    [](){ return Game::account_logged_in != 0; }
                ),
                new Ui::DynamicText(14, [](){ return I18n::tr(I18n::kOrPressEnterSpawn); }),
                new Ui::DynamicText(14, [](){ return Game::account_status; }, {
                    .fill = 0xffff6666,
                    .should_render = [](){ return !Game::account_status.empty(); }
                }),
                new Ui::Element(0, 10),
                new Ui::VContainer({
                    new Ui::HContainer({
                        make_map_select_button(MapID::kGarden, 0xff1ea761, select_map_garden, darken_map_garden),
                        make_map_select_button(MapID::kDesert, 0xffdecf7c, select_map_desert, darken_map_desert),
                        make_map_select_button(MapID::kOcean, 0xff2e86ab, select_map_ocean, darken_map_ocean),
                        make_map_select_button(MapID::kPolar, 0xff8ecae6, select_map_polar, darken_map_polar)
                    }, 0, 10, {}),
                    new Ui::HContainer({
                        make_map_select_button(MapID::kJungle, 0xff2d6a4f, select_map_jungle, darken_map_jungle),
                        make_map_select_button(MapID::kAntHell, 0xffa98467, select_map_ant_hell, darken_map_ant_hell),
                        make_map_select_button(MapID::kVolcano, 0xffe76f51, select_map_volcano, darken_map_volcano),
                        make_map_select_button(MapID::kRoyalCapital, 0xffc4a35a, select_map_royal, darken_map_royal)
                    }, 0, 10, {})
                }, 10, 10, { .no_animation = 1 }),
                new Ui::Element(0, 14),
                new Ui::Choose(
                    new Ui::VContainer({
                        new Ui::Choose(
                            new Ui::HContainer({
                                new Ui::Button(100, 38,
                                    new Ui::DynamicText(16, [](){ return I18n::tr(I18n::kLogin); }),
                                    [](Element *, uint8_t e){
                                        if (e == Ui::kClick) {
                                            Game::auth_panel = 1;
                                            Game::account_status.clear();
                                        }
                                    },
                                    nullptr,
                                    { .fill = Chrome::kBtnSlate, .line_width = 5, .round_radius = Chrome::kRadius }
                                ),
                                new Ui::Button(100, 38,
                                    new Ui::DynamicText(16, [](){ return I18n::tr(I18n::kRegister); }),
                                    [](Element *, uint8_t e){
                                        if (e == Ui::kClick) {
                                            Game::auth_panel = 2;
                                            Game::account_status.clear();
                                        }
                                    },
                                    nullptr,
                                    { .fill = Chrome::kBtnMoss, .line_width = 5, .round_radius = Chrome::kRadius }
                                )
                            }, 0, 12, {}),
                            new Ui::HContainer({
                                new Ui::Button(120, 38,
                                    new Ui::DynamicText(16, [](){ return I18n::tr(I18n::kLogout); }),
                                    [](Element *, uint8_t e){
                                        if (e == Ui::kClick) Game::account_logout();
                                    },
                                    nullptr,
                                    { .fill = Chrome::kBtnDanger, .line_width = 5, .round_radius = Chrome::kRadius }
                                )
                            }, 0, 12, {}),
                            [](){ return Game::account_logged_in != 0; }
                        ),
                        new Ui::Element(0, 6),
                        new Ui::DynamicText(13, [](){
                            if (Game::account_logged_in)
                                return I18n::use_chinese
                                    ? std::string{"账号进度自动保存"}
                                    : std::string{"Account progress saved automatically"};
                            if (!Game::account_status.empty()) return Game::account_status;
                            return std::string{I18n::tr(I18n::kGuestHint)};
                        }, { .fill = Chrome::kMuted })
                    }, 10, 8, { .no_animation = 1 }),
                    new Ui::VContainer({
                        new Ui::DynamicText(16, [](){
                            return I18n::tr(Game::auth_panel == 2 ? I18n::kRegister : I18n::kLogin);
                        }),
                        new Ui::HContainer({
                            new Ui::DynamicText(14, [](){ return I18n::tr(I18n::kUsername); }),
                            new Ui::TextInput(Game::username, 220, 32, MAX_USERNAME_LENGTH, {
                                .line_width = 5,
                                .round_radius = Chrome::kRadius,
                                .should_render = [](){
                                    return Game::auth_panel != 0
                                        && !Game::in_game() && Game::transition_circle <= 0;
                                }
                            })
                        }, 0, 8, {}),
                        new Ui::HContainer({
                            new Ui::DynamicText(14, [](){ return I18n::tr(I18n::kPassword); }),
                            (auth_password_input = new Ui::TextInput(Game::password, 188, 32, MAX_PASSWORD_LENGTH, {
                                .line_width = 5,
                                .round_radius = Chrome::kRadius,
                                .should_render = [](){
                                    return Game::auth_panel != 0
                                        && !Game::in_game() && Game::transition_circle <= 0;
                                }
                            }, 1 /* password masked */)),
                            new Ui::Button(36, 32,
                                new Ui::DynamicText(12, [](){
                                    if (auth_password_input == nullptr || auth_password_input->password_is_masked())
                                        return I18n::use_chinese ? std::string{"显"} : std::string{"Show"};
                                    return I18n::use_chinese ? std::string{"隐"} : std::string{"Hide"};
                                }),
                                [](Element *, uint8_t e){
                                    if (e != Ui::kClick || auth_password_input == nullptr) return;
                                    auth_password_input->set_password_masked(
                                        !auth_password_input->password_is_masked());
                                },
                                nullptr,
                                { .fill = Chrome::kBtnIron, .line_width = 4, .round_radius = Chrome::kRadius }
                            )
                        }, 0, 6, {}),
                        new Ui::HContainer({
                            new Ui::Button(100, 38,
                                new Ui::DynamicText(16, [](){
                                    return I18n::tr(Game::auth_panel == 2 ? I18n::kRegister : I18n::kLogin);
                                }),
                                [](Element *, uint8_t e){
                                    if (e == Ui::kClick)
                                        Game::account_auth(Game::auth_panel == 2);
                                },
                                nullptr,
                                { .fill = Chrome::kBtnMoss, .line_width = 5, .round_radius = Chrome::kRadius }
                            ),
                            new Ui::Button(100, 38,
                                new Ui::DynamicText(16, [](){ return I18n::tr(I18n::kLeaveNo); }),
                                [](Element *, uint8_t e){
                                    if (e == Ui::kClick) Game::auth_panel = 0;
                                },
                                nullptr,
                                { .fill = Chrome::kBtnSlate, .line_width = 5, .round_radius = Chrome::kRadius }
                            )
                        }, 0, 12, {}),
                        new Ui::DynamicText(13, [](){
                            if (!Game::account_status.empty()) return Game::account_status;
                            return std::string{};
                        }, {
                            .fill = 0xffff6666,
                            .should_render = [](){ return !Game::account_status.empty(); }
                        })
                    }, 10, 8, { .no_animation = 1 }),
                    [](){ return Game::auth_panel != 0; }
                ),
                new Ui::Element(0, 18),
                // Respawn loadout OR how-to-play — in-flow, never fixed-y overlay
                new Ui::Choose(
                    make_title_how_to_play(),
                    make_title_respawn_preview(),
                    [](){ return Game::respawn_level > 1 ? 1 : 0; }
                )
            }, 12, 10, { .no_animation = 1 }),
            [](){ return Game::socket.ready; }
        )
    }, 0, 0, {
        .should_render = [](){ return Game::should_render_title_ui(); },
        .no_animation = 1
    });
    return title;
}

Element *Ui::make_title_info_box() {
    // Content moved into make_title_input_box flow to prevent overlap
    return new Ui::Element(0, 0, {
        .should_render = [](){ return false; }
    });
}

static void open_title_panel(uint8_t panel, Element *) {
    Ui::toggle_panel(panel);
}

Element *Ui::make_panel_buttons() {
    Element *settings_icon = new Ui::StaticIcon([](Element *elt, Renderer &ctx){
        float r = elt->width * 0.36f;
        ctx.set_fill(0xffffffff);
        ctx.begin_path();
        ctx.arc(0, 0, r);
        ctx.fill();
        ctx.set_fill(0xff6a6a6a);
        ctx.begin_path();
        ctx.arc(0, 0, r * 0.4f);
        ctx.fill();
        ctx.set_stroke(0xffffffff);
        ctx.set_line_width(elt->width * 0.16f);
        ctx.round_line_cap();
        for (int i = 0; i < 6; ++i) {
            float a = i * (2.f * (float)M_PI / 6.f);
            ctx.begin_path();
            ctx.move_to(cosf(a) * r * 0.55f, sinf(a) * r * 0.55f);
            ctx.line_to(cosf(a) * r * 0.95f, sinf(a) * r * 0.95f);
            ctx.stroke();
        }
    }, 28, 28);

    Element *petal_icon = new Ui::StaticIcon([](Element *elt, Renderer &ctx){
        float s = elt->width * 0.28f;
        ctx.set_fill(0xffffffff);
        for (int i = 0; i < 5; ++i) {
            float a = i * (2.f * (float)M_PI / 5.f) - (float)M_PI / 2.f;
            ctx.begin_path();
            ctx.arc(cosf(a) * s * 0.7f, sinf(a) * s * 0.7f, s * 0.55f);
            ctx.fill();
        }
        ctx.set_fill(0xffffe066);
        ctx.begin_path();
        ctx.arc(0, 0, s * 0.45f);
        ctx.fill();
    }, 28, 28);

    Element *mob_icon = new Ui::StaticIcon([](Element *elt, Renderer &ctx){
        // Paw / bestiary
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
    }, 28, 28);

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
    }, 28, 28);

    Element *book_icon = new Ui::StaticIcon([](Element *elt, Renderer &ctx){
        float w = elt->width * 0.55f;
        float h = elt->height * 0.65f;
        ctx.set_fill(0xffffffff);
        ctx.begin_path();
        ctx.round_rect(-w / 2, -h / 2, w, h, 2);
        ctx.fill();
        ctx.set_stroke(0xff5a9fdb);
        ctx.set_line_width(2);
        ctx.begin_path();
        ctx.move_to(0, -h / 2 + 2);
        ctx.line_to(0, h / 2 - 2);
        ctx.stroke();
        ctx.set_stroke(0xff88bbdd);
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
    }, 28, 28);

    Element *elt = new Ui::HContainer({
        new Ui::Button(50, 50, settings_icon,
            [](Element *, uint8_t e){
                if (e == Ui::kClick) open_title_panel(Panel::kSettings, Panel::settings);
            },
            [](){ return Ui::panel_open == Panel::kSettings; },
            { .fill = Chrome::kBtnIron, .line_width = 5, .round_radius = 3 }
        ),
        new Ui::Button(50, 50, mob_icon,
            [](Element *, uint8_t e){
                if (e == Ui::kClick) open_title_panel(Panel::kMobs, Panel::mob_gallery);
            },
            [](){ return Ui::panel_open == Panel::kMobs; },
            { .fill = Chrome::kBtnMoss, .line_width = 5, .round_radius = 3 }
        ),
        new Ui::Button(50, 50, trophy_icon,
            [](Element *, uint8_t e){
                if (e == Ui::kClick) open_title_panel(Panel::kLeaderboard, Panel::leaderboard);
            },
            [](){ return Ui::panel_open == Panel::kLeaderboard; },
            { .fill = Chrome::kBtnClay, .line_width = 5, .round_radius = 3 }
        ),
        new Ui::Button(50, 50, petal_icon,
            [](Element *, uint8_t e){
                if (e == Ui::kClick) open_title_panel(Panel::kPetals, Panel::petal_gallery);
            },
            [](){ return Ui::panel_open == Panel::kPetals; },
            { .fill = Chrome::kBtnBronze, .line_width = 5, .round_radius = 3 }
        ),
        new Ui::Button(50, 50, book_icon,
            [](Element *, uint8_t e){
                if (e == Ui::kClick) open_title_panel(Panel::kChangelog, Panel::changelog);
            },
            [](){ return Ui::panel_open == Panel::kChangelog; },
            { .fill = Chrome::kBtnSlate, .line_width = 5, .round_radius = 3 }
        ),
    }, 12, 10, {
        .should_render = [](){ return Game::should_render_title_ui(); },
        .h_justify = Style::Left,
        .v_justify = Style::Top
    });
    elt->x = 12;
    elt->y = 12;
    Ui::panel_buttons_root = elt;
    return elt;
}

Element *Ui::make_debug_stats() {
    Element *elt = new Ui::VContainer({
        new Ui::DynamicText(12, [](){
            float pxls = 0;
            for (Renderer *r : Renderer::renderers) pxls += r->width * r->height;
            return std::format("{} ctxs - {} pxls - {:.0f}x{:.0f}", Renderer::renderers.size(), format_score(pxls), Ui::window_width, Ui::window_height);
        }, { .fill = 0xffffffff, .h_justify = Style::Right }),
        new Ui::DynamicText(12, [](){
            if (Debug::frame_times.size() == 0) return std::string{I18n::tr(I18n::kDebugFail)};
            float min_dt = Debug::tick_times[0];
            float avg_dt = min_dt;
            float max_dt = min_dt;
            for (uint32_t i = 1; i < Debug::tick_times.size(); ++i) {
                float const frame_time = Debug::tick_times[i];
                if (min_dt > frame_time) min_dt = frame_time;
                else if (max_dt < frame_time) max_dt = frame_time;
                avg_dt += frame_time;
            }
            avg_dt /= Debug::tick_times.size();
            return I18n::use_chinese
                ? std::format("逻辑: {:.1f}/{:.1f}/{:.1f} ms (最小/平均/最大)", min_dt, avg_dt, max_dt)
                : std::format("tick: {:.1f}/{:.1f}/{:.1f} ms (min/avg/max)", min_dt, avg_dt, max_dt);
        }, { .fill = 0xffffffff, .h_justify = Style::Right }),
        new Ui::DynamicText(12, [](){
            if (Debug::frame_times.size() == 0) return std::string{I18n::tr(I18n::kDebugFail)};
            float min_dt = Debug::frame_times[0];
            float avg_dt = min_dt;
            float max_dt = min_dt;
            for (uint32_t i = 1; i < Debug::frame_times.size(); ++i) {
                float const frame_time = Debug::frame_times[i];
                if (min_dt > frame_time) min_dt = frame_time;
                else if (max_dt < frame_time) max_dt = frame_time;
                avg_dt += frame_time;
            }
            avg_dt /= Debug::frame_times.size();
            return I18n::use_chinese
                ? std::format("帧: {:.1f}/{:.1f}/{:.1f} ms (最小/平均/最大) - {:.1f} fps", min_dt, avg_dt, max_dt, 1000 / Ui::dt)
                : std::format("frame: {:.1f}/{:.1f}/{:.1f} ms (min/avg/max) - {:.1f} fps", min_dt, avg_dt, max_dt, 1000 / Ui::dt);
        }, { .fill = 0xffffffff, .h_justify = Style::Right })
    }, 5, 5, { .should_render = [](){ return Game::show_debug; }, .h_justify = Style::Right, .v_justify = Style::Top, .no_animation = 1 });
    elt->y = 8;
    return elt;
}

Element *Ui::make_github_link_button() {
    Element *elt = new Ui::HContainer({
        new Ui::Button(50, 50, 
            new Ui::StaticIcon([](Element *elt, Renderer &ctx){
                ctx.scale(elt->width / 20);
                ctx.set_fill(0xfff1f1f1);
                ctx.begin_path();
                ctx.move_to(0.00, -10.00);
                ctx.bcurve_to(5.52, -10.00, 10.00, -5.41, 10.00, 0.25);
                ctx.bcurve_to(10.00, 4.78, 7.14, 8.62, 3.17, 9.98);
                ctx.bcurve_to(2.66, 10.08, 2.48, 9.76, 2.48, 9.49);
                ctx.bcurve_to(2.48, 9.15, 2.49, 8.05, 2.49, 6.67);
                ctx.bcurve_to(2.49, 5.72, 2.17, 5.09, 1.81, 4.78);
                ctx.bcurve_to(4.04, 4.52, 6.38, 3.66, 6.38, -0.28);
                ctx.bcurve_to(6.38, -1.40, 5.99, -2.32, 5.35, -3.03);
                ctx.bcurve_to(5.45, -3.29, 5.80, -4.34, 5.25, -5.75);
                ctx.bcurve_to(5.25, -5.75, 4.41, -6.02, 2.50, -4.70);
                ctx.bcurve_to(1.71, -4.92, 0.85, -5.04, 0.00, -5.04);
                ctx.bcurve_to(-0.85, -5.04, -1.71, -4.92, -2.50, -4.70);
                ctx.bcurve_to(-4.41, -6.02, -5.25, -5.75, -5.25, -5.75);
                ctx.bcurve_to(-5.80, -4.34, -5.45, -3.29, -5.35, -3.03);
                ctx.bcurve_to(-5.99, -2.32, -6.38, -1.40, -6.38, -0.28);
                ctx.bcurve_to(-6.38, 3.65, -4.05, 4.53, -1.82, 4.79);
                ctx.bcurve_to(-2.11, 5.04, -2.37, 5.49, -2.46, 6.16);
                ctx.bcurve_to(-3.03, 6.42, -4.48, 6.87, -5.37, 5.30);
                ctx.bcurve_to(-5.37, 5.30, -5.90, 4.32, -6.90, 4.25);
                ctx.bcurve_to(-6.90, 4.25, -7.88, 4.23, -6.97, 4.87);
                ctx.bcurve_to(-6.97, 4.87, -6.32, 5.18, -5.86, 6.37);
                ctx.bcurve_to(-5.86, 6.37, -5.27, 8.20, -2.49, 7.58);
                ctx.bcurve_to(-2.49, 8.44, -2.48, 9.24, -2.48, 9.49);
                ctx.bcurve_to(-2.48, 9.76, -2.66, 10.08, -3.16, 9.98);
                ctx.bcurve_to(-7.13, 8.63, -10.00, 4.78, -10.00, 0.25);
                ctx.bcurve_to(-10.00, -5.41, -5.52, -10.00, 0.00, -10.00);
                ctx.fill();
            }, 40, 40),
            [](Element *elt, uint8_t e){
                if (e == Ui::kClick) DOM::open_page("https://github.com/aqiangei/ashflor");
            },
            nullptr,
            { .fill = Chrome::kBtnIron, .line_width = 5, .round_radius = Chrome::kRadius }
        )
    }, 10, 10, { .h_justify = Style::Right, .v_justify = Style::Top });
    return elt;
}
