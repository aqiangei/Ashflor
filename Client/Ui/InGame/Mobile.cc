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

// Fixed-base virtual joystick + circular A/B (florr_clone-web layout).

static constexpr float kJoyBaseR = 55.f;
static constexpr float kJoyKnobR = 26.f;
static constexpr float kJoyHitR = kJoyBaseR * 1.35f;
static constexpr float kJoyDead = 0.12f;
static constexpr float kAttackR = 48.f;
static constexpr float kDefendR = 36.f;

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
    // Fixed base: element center is the joystick nest (screen coords)
    base_x = screen_x;
    base_y = screen_y;

    float px = 0, py = 0;
    uint8_t held = 0;
    if (is_pressed) {
        if (Input::is_mobile) {
            auto it = Input::touches.find(persistent_touch_id);
            if (it != Input::touches.end()) {
                px = it->second.x;
                py = it->second.y;
                held = 1;
            }
        } else if (BitMath::at(Input::mouse_buttons_state, Input::LeftMouse)) {
            px = Input::mouse_x;
            py = Input::mouse_y;
            held = 1;
        }
    }

    if (!held) {
        is_pressed = 0;
        persistent_touch_id = (uint32_t)-1;
        knob_x = 0;
        knob_y = 0;
        Input::game_inputs.x = 0;
        Input::game_inputs.y = 0;
    } else {
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
            Input::game_inputs.x = dx;
            Input::game_inputs.y = dy;
        }
    }

    RenderContext c(&ctx);
    ctx.set_fill(0x28ffffff);
    ctx.set_stroke(0x59ffffff);
    ctx.set_line_width(3);
    ctx.begin_path();
    ctx.arc(0, 0, joystick_radius);
    ctx.fill();
    ctx.stroke();

    ctx.set_fill(is_pressed ? 0x8cffffff : 0x66ffffff);
    ctx.begin_path();
    ctx.arc(knob_x, knob_y, kJoyKnobR);
    ctx.fill();
}

void MobileJoyStick::on_event(uint8_t event) {
    if (event == UiEvent::kMouseDown) {
        is_pressed = 1;
        if (Input::is_mobile)
            persistent_touch_id = touch_id;
    } else if (event == UiEvent::kFocusLost || event == UiEvent::kClick) {
        is_pressed = 0;
        persistent_touch_id = (uint32_t)-1;
    }
}

void MobileJoyStick::poll_events(ScreenEvent const &event) {
    // Circular hit vs square Element box
    float hx = width * 0.5f * Ui::scale;
    float hy = height * 0.5f * Ui::scale;
    float r = (std::min)(hx, hy);
    float ex = event.x;
    float ey = event.y;
    if (!Input::is_mobile) {
        ex = Input::mouse_x;
        ey = Input::mouse_y;
    }
    float dx = ex - screen_x;
    float dy = ey - screen_y;
    if (is_pressed || dx * dx + dy * dy <= r * r)
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
        : Element(r * 2.4f, r * 2.4f, {
            .fill = fill,
            .line_width = 0,
            .round_radius = r * 1.2f,
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
            float hit = radius * 1.25f * Ui::scale;
            if (dx * dx + dy * dy > hit * hit) {
                // Allow slide-off to cancel
                held = 0;
                hold_id = (uint32_t)-1;
                BitMath::unset(Input::game_inputs.flags, flag_bit);
            } else {
                BitMath::set(Input::game_inputs.flags, flag_bit);
            }
        }

        ctx.set_fill(held ? 0x66ffffff : style.fill);
        ctx.begin_path();
        ctx.arc(0, 0, radius);
        ctx.fill();
        ctx.set_stroke(0x99ffffff);
        ctx.set_line_width(3);
        ctx.begin_path();
        ctx.arc(0, 0, radius);
        ctx.stroke();
        ctx.set_fill(0xffffffff);
        char const *label = (flag_bit == InputFlags::kAttacking) ? "A" : "B";
        ctx.draw_text(label, { .size = radius * 0.7f });
    }

    void on_event(uint8_t event) override {
        if (event == UiEvent::kMouseDown) {
            held = 1;
            hold_id = Input::is_mobile ? touch_id : 0;
            BitMath::set(Input::game_inputs.flags, flag_bit);
        } else if (event == UiEvent::kFocusLost || event == UiEvent::kClick) {
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
        float hit = radius * 1.25f * Ui::scale;
        if (held || dx * dx + dy * dy <= hit * hit)
            Element::poll_events(event);
    }
};

Element *Ui::make_mobile_attack_button() {
    Element *elt = new MobileCircleButton(kAttackR, InputFlags::kAttacking, 0x40dc3c3c);
    elt->style.h_justify = Style::Right;
    elt->style.v_justify = Style::Bottom;
    elt->x = -36;
    elt->y = -36;
    return elt;
}

Element *Ui::make_mobile_defend_button() {
    Element *elt = new MobileCircleButton(kDefendR, InputFlags::kDefending, 0x403c6edc);
    elt->style.h_justify = Style::Right;
    elt->style.v_justify = Style::Bottom;
    elt->x = -36 - kAttackR * 2.f - 20.f;
    elt->y = -36 - 8.f;
    return elt;
}

Element *Ui::make_mobile_joystick() {
    // Nest clear of left side-button column (≈64+pad) — florr_clone JOYSTICK_LEFT_CENTER
    Element *elt = new Ui::MobileJoyStick(kJoyHitR * 2.f, kJoyHitR * 2.f, kJoyBaseR);
    elt->style.h_justify = Style::Left;
    elt->style.v_justify = Style::Bottom;
    elt->x = 150.f;
    elt->y = -90.f;
    return elt;
}
