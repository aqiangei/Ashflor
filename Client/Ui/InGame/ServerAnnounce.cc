#include <Client/Ui/InGame/GameInfo.hh>

#include <Client/Assets/Assets.hh>
#include <Client/Game.hh>
#include <Client/I18n.hh>
#include <Client/Render/Renderer.hh>
#include <Client/Ui/Chrome.hh>
#include <Client/Ui/Extern.hh>

#include <Helpers/Math.hh>
#include <Shared/StaticDefinitions.hh>

#include <cmath>
#include <format>
#include <string>

using namespace Ui;

namespace {

uint8_t announce_active() {
    return Game::screen_announce.expires_at > Game::timestamp
        && !Game::screen_announce.text.empty();
}

float announce_alpha() {
    if (!announce_active()) return 0.f;
    double left = Game::screen_announce.expires_at - Game::timestamp;
    double total = Game::SCREEN_ANNOUNCE_MS;
    // Soft fade in / out
    float a = 1.f;
    if (left < 900.0) a = (float) (left / 900.0);
    double age = total - left;
    if (age < 350.0) a = fminf(a, (float) (age / 350.0));
    return fclamp(a, 0.f, 1.f);
}

class ServerAnnounceBanner final : public Element {
    LerpFloat show;
public:
    ServerAnnounceBanner()
        : Element(560, 54, {
            .should_render = [](){
                return Game::should_render_game_ui()
                    && (Game::alive() || Game::on_game_screen)
                    && announce_active();
            },
            .h_justify = Style::Center,
            .v_justify = Style::Top
        }) {
        show.set(0);
        style.animate = [&](Element *elt, Renderer &ctx){
            uint8_t on = announce_active();
            show.set(on ? 1.f : 0.f);
            show.step(Ui::lerp_amount * 2.8f);
            float target_w = fminf(680.f, (float) (Ui::window_width / Ui::scale) * 0.7f);
            if (target_w < 260.f) target_w = 260.f;
            width = target_w;
            float a = (float) show * (float) elt->animation * announce_alpha();
            ctx.set_global_alpha(a);
            ctx.translate(0, -(1.f - (float) show) * 28.f);
        };
    }

    void on_render(Renderer &ctx) override {
        if (!announce_active()) return;
        std::string const &body = Game::screen_announce.text;

        // Soft warm plate (gentle cream, low-contrast edge — not a hard chrome card)
        float hw = width / 2.f;
        float hh = height / 2.f;
        ctx.set_fill(0x55241810);
        ctx.begin_path();
        ctx.round_rect(-hw + 2.f, -hh + 3.f, width, height, 12.f);
        ctx.fill();
        ctx.set_fill(0xccf3e2c4);
        ctx.begin_path();
        ctx.round_rect(-hw, -hh, width, height, 12.f);
        ctx.fill();
        ctx.set_fill(0xaafff8ec);
        ctx.begin_path();
        ctx.round_rect(-hw + 3.f, -hh + 3.f, width - 6.f, height - 6.f, 10.f);
        ctx.fill();

        // Title row
        std::string title = I18n::use_chinese
            ? std::string{"【全服】 "} + Game::screen_announce.sender
            : std::string{"[Server] "} + Game::screen_announce.sender;
        ctx.translate(0, -10.f);
        ctx.draw_text(title.c_str(), {
            .fill = 0xffb8860b,
            .size = 13,
            .stroke_scale = 0.08f
        });
        ctx.translate(0, 22.f);
        ctx.draw_text(body.c_str(), {
            .fill = 0xff3a2a1a,
            .size = 16,
            .stroke_scale = 0.06f
        });
        ctx.translate(0, -12.f);
    }
};

} // namespace

Element *Ui::make_server_announce_banner() {
    Element *elt = new ServerAnnounceBanner();
    // Same band as boss HP bar (under topbar)
    elt->y = 68;
    return elt;
}

namespace {

uint8_t loot_toast_active() {
    return Game::loot_toast.expires_at > Game::timestamp
        && (!Game::loot_toast.title.empty() || Game::loot_toast.petal_id != PetalID::kNone);
}

float loot_toast_alpha() {
    if (!loot_toast_active()) return 0.f;
    double left = Game::loot_toast.expires_at - Game::timestamp;
    double total = Game::LOOT_TOAST_MS;
    float a = 1.f;
    if (left < 700.0) a = (float) (left / 700.0);
    double age = total - left;
    if (age < 280.0) a = fminf(a, (float) (age / 280.0));
    return fclamp(a, 0.f, 1.f);
}

class LootToastBanner final : public Element {
    LerpFloat show;
public:
    LootToastBanner()
        : Element(320, 88, {
            .should_render = [](){
                return Game::should_render_game_ui()
                    && Game::alive()
                    && loot_toast_active();
            },
            .h_justify = Style::Center,
            .v_justify = Style::Top
        }) {
        show.set(0);
        style.animate = [&](Element *elt, Renderer &ctx){
            uint8_t on = loot_toast_active();
            show.set(on ? 1.f : 0.f);
            show.step(Ui::lerp_amount * 3.2f);
            float a = (float) show * (float) elt->animation * loot_toast_alpha();
            ctx.set_global_alpha(a);
            // Slide down slightly on appear — non-blocking overlay
            ctx.translate(0, -(1.f - (float) show) * 36.f);
        };
    }

    void on_render(Renderer &ctx) override {
        if (!loot_toast_active()) return;
        float hw = width / 2.f;
        float hh = height / 2.f;
        Chrome::draw_plate(ctx, width, height, 0xdd2a4038, Chrome::kRadius, 0.f);
        ctx.set_fill(0x44ffe08a);
        ctx.begin_path();
        ctx.round_rect(-hw + 3.f, -hh + 3.f, width - 6.f, height - 6.f, Chrome::kRadius - 2.f);
        ctx.fill();

        ctx.translate(0, -28.f);
        ctx.draw_text(Game::loot_toast.title.c_str(), {
            .fill = 0xffffe08a, .size = 15, .stroke_scale = 0.08f
        });
        ctx.translate(0, 28.f);

        if (Game::loot_toast.petal_id != PetalID::kNone
            && Game::loot_toast.petal_id < PetalID::kNumPetals) {
            RenderContext c(&ctx);
            ctx.translate(-36.f, 8.f);
            ctx.scale(0.85f);
            draw_loadout_background(ctx, Game::loot_toast.petal_id, 1.f, 0);
        }
        ctx.translate(28.f, 10.f);
        std::string body = Game::loot_toast.amount > 0
            ? (I18n::use_chinese
                ? std::format("×{} 已入背包", Game::loot_toast.amount)
                : std::format("x{} → bag", Game::loot_toast.amount))
            : (I18n::use_chinese ? "已入背包" : "Added to bag");
        ctx.draw_text(body.c_str(), {
            .fill = 0xffffffff, .size = 16, .stroke_scale = 0.08f
        });
    }
};

} // namespace

Element *Ui::make_loot_toast() {
    Element *elt = new LootToastBanner();
    elt->y = 130; // under server announce / boss bar
    return elt;
}
