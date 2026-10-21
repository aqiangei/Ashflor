#include <Client/Ui/Element.hh>

#include <Client/Ui/Chrome.hh>
#include <Client/Ui/Extern.hh>

#include <Client/Game.hh>
#include <Client/Input.hh>

#include <Helpers/Math.hh>

#include <cmath>
#include <iostream>

using namespace Ui;

static void default_animate(Element *elt, Renderer &ctx) {
    ctx.scale((float) elt->animation);
}

Element::Element(float w, float h, Style s) : width(w), height(h), style(s) {
    //so the animation doesnt set
    //animation.set(1);
    //animate = [&](Renderer &ctx){ ctx.scale((float) animation); };
    if (style.animate == nullptr) 
        style.animate = default_animate;
    if (style.should_render == nullptr)
        style.should_render = [](){ return 1; };
}

void Element::add_child(Element *elt) {
    if (elt == nullptr) return;
    children.push_back(elt);
    elt->parent = this;
    //parent/child?
}

void Element::render(Renderer &ctx) {
    animation.set(style.should_render());
    if (style.no_animation) animation.step(1);
    else animation.step(Ui::lerp_amount);
    float curr_animation = (float) animation;
    visible = curr_animation > 0.01;
    /*
    #ifdef DEBUG
    if (visible) {
        RenderContext context(&ctx);
        if (focus_state != kFocusLost)
            ctx.set_stroke(0x80ff0000);
        else
            ctx.set_stroke(0x80000000);
        ctx.set_line_width(1.5);
        ctx.begin_path();
        ctx.round_rect(-width / 2, -height / 2, width, height, 6);
        ctx.stroke();
    }
    #endif
    */
    if (visible) {
        style.animate(this, ctx);
        //get abs x, y;
        screen_x = ctx.context.transform_matrix[2];
        screen_y = ctx.context.transform_matrix[5];
        float eff_w = ctx.context.transform_matrix[0] * width;
        float eff_h = ctx.context.transform_matrix[4] * height;
        if (std::abs(screen_x - ctx.context.clip_x) <= (eff_w + ctx.context.clip_w) / 2 &&
            std::abs(screen_y - ctx.context.clip_y) <= (eff_h + ctx.context.clip_h) / 2)
            on_render(ctx);
        else
            on_render_skip(ctx);
        showed = 1;
    } else 
        on_render_skip(ctx);
    //event emitter
    if (focused) {
        uint8_t pressed = 0;
        uint8_t released = 0;
        if (Input::is_mobile) {
            pressed = Input::touches.contains(touch_id);
            released = !pressed;
            focused = pressed;
        }
        else {
            pressed = BitMath::at(Input::mouse_buttons_pressed, Input::LeftMouse);
            released = BitMath::at(Input::mouse_buttons_released, Input::LeftMouse);
            focused = 0;
        }
        
        if (pressed) {
            focus_state = kMouseDown;
            on_event(kMouseDown);
        }
        if (released && focus_state == kMouseDown) {
            focus_state = kClick;
            on_event(kClick);
        }
        else if (!Input::is_mobile && focus_state != kMouseDown) {
            focus_state = kMouseHover;
            on_event(kMouseHover);
        }
    } else if (focus_state != kFocusLost) {
        focus_state = kFocusLost;
        on_event(kFocusLost);
    }
}

void Element::on_render(Renderer &ctx) {
    if (style.fill != 0x00000000) {
        // Opaque rounded surfaces use soft plate; thin/transparent fills stay flat
        uint8_t use_plate = (style.fill & 0xff000000) >= 0xc0000000
            && (style.round_radius >= 6.f || style.line_width >= 3.f);
        if (use_plate) {
            float radius = style.round_radius > 0 ? style.round_radius : Chrome::kRadius;
            if (radius > 18.f) radius = 18.f;
            Chrome::draw_plate(ctx, width, height, style.fill | 0xff000000,
                               radius, style.line_width);
        } else {
            float radius = style.round_radius;
            ctx.set_fill(Renderer::HSV(style.fill, style.stroke_hsv));
            ctx.begin_path();
            ctx.round_rect(-width / 2, -height / 2, width, height, radius);
            ctx.fill();
            if (style.fill >= 0xff000000 && style.line_width > 0.f) {
                float inset = style.line_width;
                ctx.set_fill(style.fill);
                ctx.begin_path();
                ctx.round_rect(-width / 2 + inset, -height / 2 + inset,
                               width - 2 * inset, height - 2 * inset,
                               fmax(0.f, radius - inset * 0.5f));
                ctx.fill();
            }
        }
    }
}

void Element::on_render_tooltip(Renderer &ctx) {
    tooltip_animation.set(rendering_tooltip);
    tooltip_animation.step(Ui::lerp_amount * 1.5);
    if (tooltip_animation < 0.01 && !rendering_tooltip)
        tooltip = nullptr;
    if (tooltip != nullptr) {
        tooltip->refactor();
        ctx.reset_transform();
        float const s = (float) Ui::scale;
        float const tip_w = tooltip->width * s;
        float const tip_h = tooltip->height * s;
        float const half_h = height * s * 0.5f;
        float const gap = 5.f * s;
        float const margin = 8.f;
        // Keep horizontally on-screen (galleries sit in the left column).
        float cx = screen_x;
        if (cx < tip_w * 0.5f + margin) cx = tip_w * 0.5f + margin;
        if (cx > (float) Ui::window_width - tip_w * 0.5f - margin)
            cx = (float) Ui::window_width - tip_w * 0.5f - margin;
        // Prefer above the cell; tall mob tooltips near the top panel would
        // otherwise render entirely off-screen on mobile.
        float cy_above = screen_y - half_h - gap - tip_h * 0.5f;
        float cy_below = screen_y + half_h + gap + tip_h * 0.5f;
        float cy = cy_above;
        if (cy_above - tip_h * 0.5f < margin) {
            cy = cy_below;
            if (cy + tip_h * 0.5f > (float) Ui::window_height - margin)
                cy = fclamp(cy, tip_h * 0.5f + margin,
                    (float) Ui::window_height - tip_h * 0.5f - margin);
        }
        ctx.translate(cx, cy);
        ctx.scale(s);
        ctx.set_global_alpha((float) tooltip_animation);
        tooltip->on_render(ctx);
    }
    for (Element *elt : children) {
        RenderContext context(&ctx);
        if (!elt->visible) continue;
        elt->on_render_tooltip(ctx);
    }
}

void Element::on_render_skip(Renderer &ctx) {
    if (focus_state != kFocusLost) {
        focus_state = kFocusLost;
        on_event(kFocusLost);
    }
    for (Element *elt : children)
        elt->on_render_skip(ctx);
    focused = 0;
}

void Element::on_event(uint8_t event) {}

void Element::refactor() {
    for (Element *elt : children)
        if (elt->visible) elt->refactor();
}

void Element::poll_events(ScreenEvent const &event) {
    if (style.no_polling) {
        DEBUG_ONLY(assert(Ui::focused != this);)
        touch_id = (uint32_t)-1;
        return;
    }
    if (Input::is_mobile) {
        auto iter = Input::touches.find(touch_id);
        if (iter == Input::touches.end())
            touch_id = (uint32_t)-1;
        else {
            Input::Touch const &touch = iter->second;
            if (std::abs(touch.x - screen_x) < width * Ui::scale / 2
            && std::abs(touch.y - screen_y) < height * Ui::scale / 2) {
                if (std::abs(event.x - screen_x) > width * Ui::scale / 2
                || std::abs(event.y - screen_y) > height * Ui::scale / 2) return;
                Ui::focused = this;
                return;
            }
            touch_id = (uint32_t)-1;
            focused = 0;
        }
        if (touch_id == (uint32_t)-1) {
            if (std::abs(event.x - screen_x) > width * Ui::scale / 2
            || std::abs(event.y - screen_y) > height * Ui::scale / 2) return;
            // Only a fresh (unsaturated) touch may newly capture an element;
            // saturated touches are re-delivered solely for their owner above.
            auto claim = Input::touches.find(event.id);
            if (claim == Input::touches.end() || claim->second.saturated) return;
            Ui::focused = this;
            touch_id = event.id;
        }
    } else {
        if (std::abs(Input::mouse_x - screen_x) < width * Ui::scale / 2
        && std::abs(Input::mouse_y - screen_y) < height * Ui::scale / 2)
            Ui::focused = this;
    }
}

std::vector<Element *> const Ui::make_range(uint32_t start, uint32_t end, Element *gen(uint32_t)) {
    std::vector<Element *> elts;
    for (uint32_t i = start; i < end; ++i)
        elts.push_back(gen(i));
    return elts; 
}