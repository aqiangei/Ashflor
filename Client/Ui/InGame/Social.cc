#include <Client/Ui/InGame/GameInfo.hh>
#include <Client/Ui/InGame/HudIcons.hh>

#include <Client/Ui/Button.hh>
#include <Client/Ui/Chrome.hh>
#include <Client/Ui/Container.hh>
#include <Client/Ui/DynamicText.hh>
#include <Client/Ui/ScrollContainer.hh>
#include <Client/Ui/StaticIcon.hh>
#include <Client/Ui/TextInput.hh>

#include <Client/Game.hh>
#include <Client/I18n.hh>
#include <Client/Input.hh>
#include <Client/Render/Renderer.hh>

#include <Shared/Binary.hh>

#include <cmath>
#include <format>
#include <string>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace Ui;

static TextInput *g_guild_anno_input = nullptr;

TextInput *Ui::guild_anno_text_input() {
    return g_guild_anno_input;
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

static constexpr uint8_t kGuildMemberSlots = 16;

class GuildMemberRow final : public Element {
    uint8_t index;
public:
    explicit GuildMemberRow(uint8_t i)
        : Element(280, 30, {
            .should_render = [i](){
                return Game::guild_panel_open
                    && !Game::guild_name.empty()
                    && i < Game::guild_members.size();
            },
            .h_justify = Style::Left,
            .no_animation = 1
        }), index(i) {}

    void refactor() override {
        width = Input::mobile_controls() ? 320.f : 360.f;
        height = 30.f;
        // Layout runs before render. If we keep a stale visible=0 (panel was
        // closed / skipped), VContainer collapses and ScrollContainer clips at
        // width 0 — rows never get Element::render, so they stay stuck blank.
        uint8_t want = style.should_render ? style.should_render() : 1;
        visible = want;
        animation.set(want);
    }

    void on_render(Renderer &ctx) override {
        if (index >= Game::guild_members.size()) return;
        Game::GuildMember const &m = Game::guild_members[index];
        uint8_t leader = (m.flags & 1) != 0;
        uint8_t online = (m.flags & 2) != 0;
        uint8_t self = (m.flags & 4) != 0;

        // Row plate: leader tinted gold, online greenish, offline muted
        uint32_t plate = online ? 0xff3a5a48 : 0xff3a3a3a;
        if (leader) plate = online ? 0xff5a4a28 : 0xff4a4030;
        Chrome::draw_plate(ctx, width, height, plate, 6.f, 0.f);

        float x = -width / 2 + 8;
        auto draw_label = [&](char const *t, uint32_t col, float size) {
            ctx.set_text_size(size);
            float w = ctx.get_text_size(t);
            {
                RenderContext c(&ctx);
                ctx.translate(x + w / 2, 0);
                ctx.draw_text(t, { .fill = col, .size = size, .stroke_scale = 0.08f });
            }
            x += w + 6;
        };

        if (leader)
            draw_label(I18n::tr(I18n::kGuildLeader), Chrome::kGold, 11);
        draw_label(m.name.c_str(), 0xffffffff, 13);
        {
            std::string lv = std::format("Lv{}", m.level);
            draw_label(lv.c_str(), 0xffc8d0d8, 11);
        }
        draw_label(
            online ? I18n::tr(I18n::kGuildOnline) : I18n::tr(I18n::kGuildOffline),
            online ? 0xff7dff9a : 0xffaaaaaa, 11);

        // Party invite — online + not self
        if (online && !self) {
            float bx = width / 2 - 36;
            {
                RenderContext c(&ctx);
                ctx.translate(bx, 0);
                Chrome::draw_plate(ctx, 56, 24, Chrome::kBtnMoss, 5.f, 0.f);
                ctx.draw_text(I18n::tr(I18n::kGuildPartyBtn), {
                    .fill = 0xffffffff, .size = 12, .stroke_scale = 0.08f
                });
            }
        }
    }

    void on_event(uint8_t event) override {
        if (event != kClick) return;
        if (index >= Game::guild_members.size()) return;
        Game::GuildMember const &m = Game::guild_members[index];
        if (!(m.flags & 2) || (m.flags & 4)) return;
        float local_x = (Input::mouse_x - screen_x) / (float) Ui::scale;
        if (local_x > width / 2 - 70.f)
            Game::party_invite_name(m.name);
    }
};

class GuildMemberScroll final : public Ui::ScrollContainer {
    float viewport_h;
public:
    GuildMemberScroll(Element *content, float h)
        : Ui::ScrollContainer(content, h), viewport_h(h) {
        always_capture_wheel = 1;
        style.should_render = [](){ return !Game::guild_name.empty(); };
        style.h_justify = Style::Left;
        style.no_animation = 1;
    }
    void refactor() override {
        if (style.should_render) {
            uint8_t want = style.should_render();
            visible = want;
            animation.set(want);
        }
        Ui::ScrollContainer::refactor();
        height = viewport_h;
        // Never collapse to a zero-width clip while the panel is open.
        float min_w = Input::mobile_controls() ? 320.f : 360.f;
        if (visible && width < min_w) width = min_w;
    }
};

static Element *make_guild_member_list() {
    std::vector<Element *> rows;
    for (uint8_t i = 0; i < kGuildMemberSlots; ++i)
        rows.push_back(new GuildMemberRow(i));
    Element *list = new Ui::VContainer(rows, 0, 4, {
        .should_render = [](){ return !Game::guild_name.empty(); },
        .h_justify = Style::Left,
        .no_animation = 1
    });
    float h = Input::mobile_controls() ? 140.f : 180.f;
    return new GuildMemberScroll(list, h);
}

Element *Ui::make_guild_panel() {
    uint8_t const mobile = Input::mobile_controls();
    float const panel_w = mobile ? 340.f : 400.f;

    Element *name_input = new Ui::TextInput(Game::guild_create_draft, mobile ? 200.f : 220.f, 32, 12, {
        .line_width = 0,
        .round_radius = Chrome::kRadius,
        .should_render = [](){
            return Game::guild_panel_open && Game::guild_name.empty();
        }
    });

    g_guild_anno_input = new Ui::TextInput(Game::guild_anno_draft, mobile ? 220.f : 260.f, 30, 80, {
        .line_width = 0,
        .round_radius = Chrome::kRadius,
        .should_render = [](){
            return Game::guild_panel_open
                && !Game::guild_name.empty()
                && Game::guild_self_is_leader();
        }
    });
    Element *anno_input = g_guild_anno_input;

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

        // Announcement (read-only for members)
        new Ui::DynamicText(11, [](){
            return std::string{I18n::tr(I18n::kGuildAnno)};
        }, {
            .fill = Chrome::kMuted,
            .should_render = [](){ return !Game::guild_name.empty(); },
            .h_justify = Style::Left
        }),
        new Ui::DynamicText(12, [](){
            if (Game::guild_announcement.empty())
                return I18n::use_chinese ? std::string{"（暂无公告）"} : std::string{"(no announcement)"};
            return Game::guild_announcement;
        }, {
            .fill = Chrome::kBody,
            .should_render = [](){
                return !Game::guild_name.empty() && !Game::guild_self_is_leader();
            },
            .h_justify = Style::Left
        }),
        new Ui::HContainer({
            anno_input,
            new Ui::Button(56, 28,
                new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kGuildAnnoSave); }),
                [](Element *, uint8_t e){
                    if (e == Ui::kClick) Game::guild_set_announcement();
                },
                nullptr,
                {
                    .fill = Chrome::kBtnBronze, .line_width = 0, .round_radius = Chrome::kRadius,
                    .should_render = [](){
                        return !Game::guild_name.empty() && Game::guild_self_is_leader();
                    }
                })
        }, 0, 6, {
            .should_render = [](){
                return !Game::guild_name.empty() && Game::guild_self_is_leader();
            },
            .h_justify = Style::Left
        }),
        new Ui::DynamicText(10, [](){ return I18n::tr(I18n::kGuildAnnoHint); }, {
            .fill = Chrome::kMuted,
            .should_render = [](){
                return !Game::guild_name.empty() && Game::guild_self_is_leader();
            },
            .h_justify = Style::Left
        }),

        new Ui::DynamicText(12, [](){
            // Prefer live vector size so the header matches rendered rows.
            uint32_t n = (uint32_t) Game::guild_members.size();
            if (n == 0) n = Game::guild_member_count;
            return std::format("{} ({})", I18n::tr(I18n::kGuildMembers), n);
        }, {
            .fill = Chrome::kTitle,
            .should_render = [](){ return !Game::guild_name.empty(); },
            .h_justify = Style::Left
        }),
        new Ui::DynamicText(10, [](){ return I18n::tr(I18n::kGuildPartyLevelHint); }, {
            .fill = Chrome::kMuted,
            .should_render = [](){ return !Game::guild_name.empty(); },
            .h_justify = Style::Left
        }),
        make_guild_member_list(),

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
    }, 14, 7, {
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
    (void) panel_w;
    elt->x = mobile ? 10.f : 12.f;
    elt->y = mobile ? 70.f : 68.f;
    return elt;
}

Element *Ui::make_guild_top_button() {
    return new Ui::Button(40, 40,
        make_hud_icon(hud_icon_guild, 26.f),
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
