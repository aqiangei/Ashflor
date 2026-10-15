#include <Client/Ui/InGame/GameInfo.hh>

#include <Client/Ui/Extern.hh>
#include <Client/Game.hh>
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

static uint8_t mobile_hud() {
    return Input::mobile_controls() && Game::alive();
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
            // First frame of this press: nest appears under the finger
            if (!is_pressed) {
                base_x = px;
                base_y = py;
            }
        }
    } else if (is_pressed && BitMath::at(Input::mouse_buttons_state, Input::LeftMouse)) {
        px = Input::mouse_x;
        py = Input::mouse_y;
        held = 1;
    }

    if (!held) {
        is_pressed = 0;
        persistent_touch_id = (uint32_t)-1;
        knob_x = 0;
        knob_y = 0;
        Input::game_inputs.x = 0;
        Input::game_inputs.y = 0;
        return; // hide stick when idle (not a fixed pad)
    }
    is_pressed = 1;

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
    if (event == UiEvent::kMouseDown) {
        is_pressed = 0; // force re-anchor under this press in on_render
        if (Input::is_mobile) {
            persistent_touch_id = touch_id;
            auto it = Input::touches.find(touch_id);
            if (it != Input::touches.end()) {
                base_x = it->second.x;
                base_y = it->second.y;
                is_pressed = 1;
            }
        } else {
            base_x = Input::mouse_x;
            base_y = Input::mouse_y;
            is_pressed = 1;
        }
    } else if (event == UiEvent::kClick) {
        is_pressed = 0;
        persistent_touch_id = (uint32_t)-1;
    } else if (event == UiEvent::kFocusLost) {
        if (Input::is_mobile && persistent_touch_id != (uint32_t)-1
            && Input::touches.contains(persistent_touch_id))
            return;
        is_pressed = 0;
        persistent_touch_id = (uint32_t)-1;
    }
}

void MobileJoyStick::poll_events(ScreenEvent const &event) {
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
public:
    MobileCircleButton(float r, uint8_t flag, uint32_t fill)
        : Element(r * 2.5f, r * 2.5f, {
            .fill = fill,
            .line_width = 0,
            .round_radius = r * 1.25f,
            .should_render = [](){ return mobile_hud(); },
            .no_animation = 1
        }), radius(r), flag_bit(flag), hold_id((uint32_t)-1), held(0) {}

    void on_render(Renderer &ctx) override {
        float px = 0, py = 0;
        uint8_t down = 0;
        if (held) {
            if (Input::is_mobile) {
                auto it = Input::touches.find(hold_id);
                if (it != Input::touches.end()) {
                    px = it->second.x; py = it->second.y; down = 1;
                }
            } else if (BitMath::at(Input::mouse_buttons_state, Input::LeftMouse)) {
                px = Input::mouse_x; py = Input::mouse_y; down = 1;
            }
        }
        if (!down) {
            held = 0;
            hold_id = (uint32_t)-1;
            BitMath::unset(Input::game_inputs.flags, flag_bit);
        } else {
            float dx = px - screen_x;
            float dy = py - screen_y;
            float hit = radius * 1.35f * Ui::scale;
            if (dx * dx + dy * dy > hit * hit) {
                held = 0;
                hold_id = (uint32_t)-1;
                BitMath::unset(Input::game_inputs.flags, flag_bit);
            } else {
                BitMath::set(Input::game_inputs.flags, flag_bit);
            }
        }

        ctx.set_fill(held ? 0x77ffffff : style.fill);
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
            BitMath::set(Input::game_inputs.flags, flag_bit);
        } else if (event == UiEvent::kClick) {
            held = 0;
            hold_id = (uint32_t)-1;
            BitMath::unset(Input::game_inputs.flags, flag_bit);
        } else if (event == UiEvent::kFocusLost) {
            if (Input::is_mobile && hold_id != (uint32_t)-1
                && Input::touches.contains(hold_id))
                return;
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
