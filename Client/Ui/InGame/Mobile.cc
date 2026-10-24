#include <Client/Ui/InGame/GameInfo.hh>
#include <Client/Ui/InGame/Loadout.hh>

#include <Client/Ui/Extern.hh>
#include <Client/Game.hh>
#include <Client/I18n.hh>
#include <Client/Input.hh>

#include <Helpers/Bits.hh>
#include <Helpers/Math.hh>

#include <Shared/StaticData.hh>
#include <Shared/StaticDefinitions.hh>

#include <cmath>

using namespace Ui;

// Dynamic (touch-origin) joystick + larger circular A/B — gardn-style float base.

static constexpr float kJoyBaseR = 88.f;
static constexpr float kJoyKnobR = 40.f;
static constexpr float kJoyDead = 0.12f;
static constexpr float kAttackR = 72.f;
static constexpr float kDefendR = 56.f;

static uint32_t g_mobile_joy_touch = (uint32_t) -1;
static uint32_t g_mobile_atk_touch = (uint32_t) -1;
static uint32_t g_mobile_def_touch = (uint32_t) -1;
static uint32_t g_mobile_swap_touch = (uint32_t) -1;

static uint8_t mobile_hud() {
    // Soft keyboard / chat typing: stand still, hide stick + A/B
    return Input::mobile_controls() && Game::alive() && !Game::chat_open;
}

uint8_t Ui::mobile_touch_is_control(uint32_t touch_id) {
    if (touch_id == (uint32_t) -1) return 0;
    if (Ui::rotation_wheel_touch_busy(touch_id)) return 1;
    return touch_id == g_mobile_joy_touch
        || touch_id == g_mobile_atk_touch
        || touch_id == g_mobile_def_touch
        || touch_id == g_mobile_swap_touch;
}

MobileJoyStick::MobileJoyStick(float w, float h, float r)
    : Element(w, h, {}), joystick_radius(r),
      base_x(0), base_y(0), knob_x(0), knob_y(0),
      persistent_touch_id((uint32_t)-1), is_pressed(0) {
    style.should_render = [](){ return mobile_hud(); };
    style.no_animation = 1;
    style.no_polling = 0;
}

void MobileJoyStick::on_render(Renderer &ctx) {
    float px = 0, py = 0;
    uint8_t held = 0;
    if (Input::is_mobile) {
        auto it = Input::touches.find(persistent_touch_id);
        if (it != Input::touches.end()) {
            px = it->second.x;
            py = it->second.y;
            held = 1;
        }
    } else if (is_pressed && BitMath::at(Input::mouse_buttons_state, Input::LeftMouse)) {
        px = Input::mouse_x;
        py = Input::mouse_y;
        held = 1;
    }

    if (!held) {
        is_pressed = 0;
        if (persistent_touch_id == g_mobile_joy_touch)
            g_mobile_joy_touch = (uint32_t) -1;
        persistent_touch_id = (uint32_t)-1;
        knob_x = 0;
        knob_y = 0;
        Input::game_inputs.x = 0;
        Input::game_inputs.y = 0;
        return; // hide stick when idle (not a fixed pad)
    }
    is_pressed = 1;
    g_mobile_joy_touch = persistent_touch_id;

    float dx = (px - base_x) / Ui::scale;
    float dy = (py - base_y) / Ui::scale;
    float dist = std::sqrt(dx * dx + dy * dy);
    if (!is_finite(dist)) dist = 0;
    if (dist > joystick_radius && dist > 1e-3f) {
        dx *= joystick_radius / dist;
        dy *= joystick_radius / dist;
        dist = joystick_radius;
    }
    knob_x = dx;
    knob_y = dy;
    float dead = joystick_radius * kJoyDead;
    if (dist <= dead) {
        Input::game_inputs.x = 0;
        Input::game_inputs.y = 0;
    } else {
        // Server: magnitude >= 200 → full PLAYER_ACCELERATION
        float scale = 200.f / joystick_radius;
        Input::game_inputs.x = dx * scale;
        Input::game_inputs.y = dy * scale;
    }

    // Draw in screen space at the touch-origin nest (not the Element center)
    RenderContext c(&ctx);
    ctx.reset_transform();
    ctx.translate(base_x, base_y);
    ctx.scale(Ui::scale);

    ctx.set_fill(0x35ffffff);
    ctx.set_stroke(0x77ffffff);
    ctx.set_line_width(4);
    ctx.begin_path();
    ctx.arc(0, 0, joystick_radius);
    ctx.fill();
    ctx.stroke();

    ctx.set_fill(0xa0ffffff);
    ctx.begin_path();
    ctx.arc(knob_x, knob_y, kJoyKnobR);
    ctx.fill();
}

void MobileJoyStick::on_event(uint8_t event) {
    // kMouseDown is delivered every frame the touch is held. Anchor the nest
    // only on the first frame; later frames must keep base_x/base_y fixed
    // or the knob offset stays ~0 and the flower never moves.
    if (event == UiEvent::kMouseDown) {
        uint8_t fresh = !is_pressed;
        if (Input::is_mobile) {
            if (persistent_touch_id != touch_id) fresh = 1;
            persistent_touch_id = touch_id;
            if (!fresh) return;
            auto it = Input::touches.find(touch_id);
            if (it != Input::touches.end()) {
                base_x = it->second.x;
                base_y = it->second.y;
                is_pressed = 1;
            }
        } else if (fresh) {
            base_x = Input::mouse_x;
            base_y = Input::mouse_y;
            is_pressed = 1;
        }
    } else if (event == UiEvent::kClick) {
        is_pressed = 0;
        if (persistent_touch_id == g_mobile_joy_touch)
            g_mobile_joy_touch = (uint32_t) -1;
        persistent_touch_id = (uint32_t)-1;
    } else if (event == UiEvent::kFocusLost) {
        if (Input::is_mobile && persistent_touch_id != (uint32_t)-1
            && Input::touches.contains(persistent_touch_id))
            return;
        is_pressed = 0;
        if (persistent_touch_id == g_mobile_joy_touch)
            g_mobile_joy_touch = (uint32_t) -1;
        persistent_touch_id = (uint32_t)-1;
    }
}

void MobileJoyStick::poll_events(ScreenEvent const &event) {
    // Bag/craft drawer sits on the left pad — never steal its taps.
    if (Game::game_panel != Game::kPanelNone && !is_pressed)
        return;
    // The floating pad must also leave the bottom equipment bar / spin wheel alone.
    if (Input::is_mobile && !is_pressed
        && (Ui::loadout_bar_hit(event.x, event.y)
            || Ui::rotation_wheel_hit(event.x, event.y)))
        return;
    if (Input::is_mobile && is_pressed && event.id != persistent_touch_id)
        return;
    // On narrow screens the large joystick pad reaches beneath A/B.
    if (Input::is_mobile && !is_pressed && event.x >= Ui::window_width / 2)
        return;
    // Whole element is the capture pad; stick itself floats under the finger
    if (is_pressed) {
        Element::poll_events(event);
        return;
    }
    float ex = Input::is_mobile ? event.x : Input::mouse_x;
    float ey = Input::is_mobile ? event.y : Input::mouse_y;
    float hx = width * 0.5f * Ui::scale;
    float hy = height * 0.5f * Ui::scale;
    if (std::abs(ex - screen_x) <= hx && std::abs(ey - screen_y) <= hy)
        Element::poll_events(event);
}

// Circular hold button (attack / defend)
class MobileCircleButton : public Element {
    float radius;
    uint8_t flag_bit;
    uint32_t hold_id;
    uint8_t held;
    double pulse_until;
public:
    MobileCircleButton(float r, uint8_t flag, uint32_t fill)
        : Element(r * 2.5f, r * 2.5f, {
            .fill = fill,
            .line_width = 0,
            .round_radius = r * 1.25f,
            .should_render = [](){ return mobile_hud(); },
            .no_animation = 1
        }), radius(r), flag_bit(flag), hold_id((uint32_t)-1), held(0), pulse_until(0) {}

    void on_render(Renderer &ctx) override {
        uint8_t down = 0;
        if (Input::is_mobile) {
            // A fast tap can begin and end between two animation frames.
            // Keep that action for two server ticks so it is actually seen.
            for (auto const &[id, start] : Input::began_touches) {
                if (!Input::ended_touches.contains(id)) continue;
                float dx = start.x - screen_x;
                float dy = start.y - screen_y;
                float hit = radius * 1.35f * Ui::scale;
                if (dx * dx + dy * dy <= hit * hit)
                    pulse_until = Game::timestamp + 80;
            }
        }
        if (held) {
            if (Input::is_mobile) {
                down = Input::touches.contains(hold_id);
            } else if (BitMath::at(Input::mouse_buttons_state, Input::LeftMouse)) {
                down = 1;
            }
        }
        // Invert: hold = cancel always-on. Pulse after a tap must NOT keep the
        // flag latched or invert reads "held" until you press again.
        uint8_t const invert = (flag_bit == InputFlags::kAttacking)
            ? Input::invert_attack : Input::invert_defend;
        uint8_t const pulse_active = !invert && Game::timestamp < pulse_until;
        if (!down && !pulse_active) {
            if (flag_bit == InputFlags::kAttacking && hold_id == g_mobile_atk_touch)
                g_mobile_atk_touch = (uint32_t) -1;
            if (flag_bit == InputFlags::kDefending && hold_id == g_mobile_def_touch)
                g_mobile_def_touch = (uint32_t) -1;
            held = 0;
            hold_id = (uint32_t)-1;
            BitMath::unset(Input::game_inputs.flags, flag_bit);
        } else {
            // A held action belongs to its starting finger until touchend.
            // Sliding while aiming must not release it or transfer it to a
            // second finger passing over this button.
            BitMath::set(Input::game_inputs.flags, flag_bit);
            if (flag_bit == InputFlags::kAttacking) g_mobile_atk_touch = hold_id;
            if (flag_bit == InputFlags::kDefending) g_mobile_def_touch = hold_id;
        }

        ctx.set_fill(down || Game::timestamp < pulse_until ? 0x77ffffff : style.fill);
        ctx.begin_path();
        ctx.arc(0, 0, radius);
        ctx.fill();
        ctx.set_stroke(0xaaffffff);
        ctx.set_line_width(4);
        ctx.begin_path();
        ctx.arc(0, 0, radius);
        ctx.stroke();
        ctx.set_fill(0xffffffff);
        char const *label = (flag_bit == InputFlags::kAttacking) ? "A" : "B";
        ctx.draw_text(label, { .size = radius * 0.72f });
    }

    void on_event(uint8_t event) override {
        if (event == UiEvent::kMouseDown) {
            held = 1;
            hold_id = Input::is_mobile ? touch_id : 0;
            if (flag_bit == InputFlags::kAttacking) g_mobile_atk_touch = hold_id;
            if (flag_bit == InputFlags::kDefending) g_mobile_def_touch = hold_id;
            BitMath::set(Input::game_inputs.flags, flag_bit);
        } else if (event == UiEvent::kClick) {
            if (flag_bit == InputFlags::kAttacking && hold_id == g_mobile_atk_touch)
                g_mobile_atk_touch = (uint32_t) -1;
            if (flag_bit == InputFlags::kDefending && hold_id == g_mobile_def_touch)
                g_mobile_def_touch = (uint32_t) -1;
            held = 0;
            hold_id = (uint32_t)-1;
            BitMath::unset(Input::game_inputs.flags, flag_bit);
        } else if (event == UiEvent::kFocusLost) {
            if (Input::is_mobile && hold_id != (uint32_t)-1
                && Input::touches.contains(hold_id))
                return;
            if (flag_bit == InputFlags::kAttacking && hold_id == g_mobile_atk_touch)
                g_mobile_atk_touch = (uint32_t) -1;
            if (flag_bit == InputFlags::kDefending && hold_id == g_mobile_def_touch)
                g_mobile_def_touch = (uint32_t) -1;
            held = 0;
            hold_id = (uint32_t)-1;
            BitMath::unset(Input::game_inputs.flags, flag_bit);
        }
    }

    void poll_events(ScreenEvent const &event) override {
        float ex = Input::is_mobile ? event.x : Input::mouse_x;
        float ey = Input::is_mobile ? event.y : Input::mouse_y;
        float dx = ex - screen_x;
        float dy = ey - screen_y;
        float hit = radius * 1.35f * Ui::scale;
        if (Input::is_mobile) {
            // Generic Element focus has one owner for the whole UI. Capture
            // the action directly so joystick, inventory, and another finger
            // cannot replace this button's held touch.
            auto touch = Input::touches.find(event.id);
            if (!held && touch != Input::touches.end()
                && !touch->second.saturated
                && dx * dx + dy * dy <= hit * hit) {
                held = 1;
                hold_id = event.id;
                if (flag_bit == InputFlags::kAttacking) g_mobile_atk_touch = hold_id;
                if (flag_bit == InputFlags::kDefending) g_mobile_def_touch = hold_id;
                touch->second.saturated = 1;
                BitMath::set(Input::game_inputs.flags, flag_bit);
            }
            return;
        }
        if (held || dx * dx + dy * dy <= hit * hit)
            Element::poll_events(event);
    }
};

Element *Ui::make_mobile_attack_button() {
    Element *elt = new MobileCircleButton(kAttackR, InputFlags::kAttacking, 0x50dc3c3c);
    elt->style.h_justify = Style::Right;
    elt->style.v_justify = Style::Bottom;
    elt->x = -48;
    elt->y = -48;
    return elt;
}

Element *Ui::make_mobile_defend_button() {
    Element *elt = new MobileCircleButton(kDefendR, InputFlags::kDefending, 0x503c6edc);
    elt->style.h_justify = Style::Right;
    elt->style.v_justify = Style::Bottom;
    elt->x = -48 - kAttackR * 2.1f - 28.f;
    elt->y = -48 - 12.f;
    return elt;
}

// R: short tap = swap primary↔secondary (or cancel L-pick);
// hold = arm L-bank pick — then tap equip slots 1..n for L1..Ln.
class MobileSwapButton final : public Element {
    static constexpr float kRadius = 44.f;
    static constexpr double kLongMs = 380.0;
    uint32_t hold_id;
    uint8_t held;
    uint8_t long_fired;
    double press_at;

    static void short_tap() {
        if (Game::mobile_l_preset_pick) {
            Game::mobile_l_preset_pick = 0;
            Game::loadout_preset_status.clear();
            return;
        }
        Game::swap_all_petals();
    }

    static void arm_l_pick() {
        Game::mobile_l_preset_pick = 1;
        Game::loadout_preset_status = I18n::use_chinese
            ? "已显示 L1～L0 首花瓣预览，松手后点主槽切配装；点 R 取消"
            : "L1-L0 previews shown: release, then tap a primary slot; R cancels";
    }
public:
    MobileSwapButton()
        : Element(kRadius * 2.4f, kRadius * 2.4f, {
            .fill = 0x50404040,
            .line_width = 0,
            .round_radius = kRadius * 1.2f,
            .should_render = [](){
                return Input::mobile_controls() && (Game::alive() || Game::on_game_screen)
                    && !Game::chat_open && !Game::pause_menu_open
                    && !Game::leave_confirm_open && !Game::game_panel_blocking();
            },
            .no_animation = 1
        }), hold_id((uint32_t)-1), held(0), long_fired(0), press_at(0) {}

    void finish_press() {
        if (!held) return;
        if (!long_fired) short_tap();
        if (hold_id == g_mobile_swap_touch)
            g_mobile_swap_touch = (uint32_t) -1;
        held = 0;
        hold_id = (uint32_t)-1;
        long_fired = 0;
    }

    void on_render_skip(Renderer &ctx) override {
        if (hold_id == g_mobile_swap_touch) g_mobile_swap_touch = (uint32_t)-1;
        held = long_fired = 0;
        hold_id = (uint32_t)-1;
        Game::mobile_l_preset_pick = 0;
        Element::on_render_skip(ctx);
    }

    void on_render(Renderer &ctx) override {
        // Short taps can start and end between animation frames. Those never
        // reach poll_events, so recognize the completed gesture here.
        if (!held && Input::is_mobile) {
            float hit = kRadius * 1.35f * Ui::scale;
            for (auto const &[id, start] : Input::began_touches) {
                auto end = Input::ended_touches.find(id);
                if (end == Input::ended_touches.end()) continue;
                float sx = start.x - screen_x, sy = start.y - screen_y;
                float ex = end->second.x - screen_x, ey = end->second.y - screen_y;
                if (sx * sx + sy * sy <= hit * hit
                    && ex * ex + ey * ey <= hit * hit) {
                    short_tap();
                    break;
                }
            }
        }
        uint8_t down = 0;
        if (held) {
            if (Input::is_mobile)
                down = Input::touches.contains(hold_id);
            else
                down = BitMath::at(Input::mouse_buttons_state, Input::LeftMouse);
        }
        if (held && down && !long_fired
            && Game::timestamp - press_at >= kLongMs) {
            long_fired = 1;
            arm_l_pick();
        }
        // Direct-capture mobile path never gets Element kClick — release here.
        if (held && !down)
            finish_press();
        float fill_t = 0.f;
        if (held && down && !long_fired) {
            fill_t = (float) ((Game::timestamp - press_at) / kLongMs);
            if (fill_t > 1.f) fill_t = 1.f;
        }
        uint8_t armed = Game::mobile_l_preset_pick;
        ctx.set_fill(armed ? 0x99c9a227 : (down || long_fired ? 0x77ffffff : style.fill));
        ctx.begin_path();
        ctx.arc(0, 0, kRadius);
        ctx.fill();
        if (fill_t > 0.02f) {
            ctx.set_stroke(0xffc9a227);
            ctx.set_line_width(5);
            ctx.begin_path();
            ctx.arc(0, 0, kRadius + 3.f);
            ctx.stroke();
            ctx.set_global_alpha(0.35f + 0.45f * fill_t);
            ctx.set_fill(0xffc9a227);
            ctx.begin_path();
            ctx.arc(0, 0, kRadius * (0.55f + 0.45f * fill_t));
            ctx.fill();
            ctx.set_global_alpha(1.f);
        }
        if (armed) {
            ctx.set_stroke(0xffffe08a);
            ctx.set_line_width(5);
            ctx.begin_path();
            ctx.arc(0, 0, kRadius + 2.f);
            ctx.stroke();
        }
        ctx.set_stroke(0xaaffffff);
        ctx.set_line_width(4);
        ctx.begin_path();
        ctx.arc(0, 0, kRadius);
        ctx.stroke();
        ctx.set_fill(0xffffffff);
        ctx.draw_text(armed ? "L" : "R", { .size = kRadius * 0.72f });
    }

    void on_event(uint8_t event) override {
        if (event == UiEvent::kMouseDown) {
            held = 1;
            long_fired = 0;
            press_at = Game::timestamp;
            hold_id = Input::is_mobile ? touch_id : 0;
            g_mobile_swap_touch = hold_id;
        } else if (event == UiEvent::kClick || event == UiEvent::kFocusLost) {
            if (event == UiEvent::kFocusLost && Input::is_mobile
                && hold_id != (uint32_t)-1
                && Input::touches.contains(hold_id))
                return;
            finish_press();
        }
    }

    void poll_events(ScreenEvent const &event) override {
        float ex = Input::is_mobile ? event.x : Input::mouse_x;
        float ey = Input::is_mobile ? event.y : Input::mouse_y;
        float dx = ex - screen_x;
        float dy = ey - screen_y;
        float hit = kRadius * 1.35f * Ui::scale;
        if (Input::is_mobile) {
            auto touch = Input::touches.find(event.id);
            if (!held && touch != Input::touches.end()
                && !touch->second.saturated
                && dx * dx + dy * dy <= hit * hit) {
                held = 1;
                long_fired = 0;
                press_at = Game::timestamp;
                hold_id = event.id;
                g_mobile_swap_touch = hold_id;
                touch->second.saturated = 1;
            }
            return;
        }
        if (held || dx * dx + dy * dy <= hit * hit)
            Element::poll_events(event);
    }
};

Element *Ui::make_mobile_swap_button() {
    Element *elt = new MobileSwapButton();
    elt->style.h_justify = Style::Right;
    elt->style.v_justify = Style::Bottom;
    // Above defend, clear of A/B cluster
    elt->x = -48 - kAttackR * 2.1f - 28.f;
    elt->y = -48 - kDefendR * 2.2f - 36.f;
    return elt;
}

Element *Ui::make_mobile_joystick() {
    // Large left-bottom pad: stick appears under finger (not a fixed nest).
    // Offset past side plaques (~80–100 UI) so bag/talent buttons stay tappable.
    Element *elt = new Ui::MobileJoyStick(920.f, 720.f, kJoyBaseR);
    elt->style.h_justify = Style::Left;
    elt->style.v_justify = Style::Bottom;
    elt->x = 70.f;
    elt->y = -20.f;
    return elt;
}
