#include <Client/Ui/ScrollContainer.hh>

#include <Client/Ui/Extern.hh>
#include <Client/Ui/InGame/Loadout.hh>
#include <Client/Input.hh>

#include <Helpers/Bits.hh>
#include <Helpers/Macros.hh>

#include <cmath>

using namespace Ui;

ScrollBar::ScrollBar() : Element(10, 100, {
    .fill = 0xaa5a3a1a,
    .round_radius = 5
}) {
    style.animate = [this](Element *, Renderer &){
        if (!style.layer) return;
        // Desktop: clear grab on mouse up. Mobile: clear when the finger lifts.
        if (!Input::is_mobile) {
            if (BitMath::at(Input::mouse_buttons_released, Input::LeftMouse))
                style.layer = 0;
            return;
        }
        if (touch_id == (uint32_t) -1 || !Input::touches.contains(touch_id))
            style.layer = 0;
    };
}

void ScrollBar::on_event(uint8_t event) {
    if (event == kMouseDown)
        style.layer = 1;
}

ScrollContainer::ScrollContainer(Element *content, float max_height) : HContainer({content, new ScrollBar()}, 0, 10, {}) {
    height = max_height;
    lerp_scroll = 0;
}

void ScrollContainer::on_render(Renderer &ctx) {
    DEBUG_ONLY(assert(children.size() == 2));
    Element *scroll = children[1];
    Element *content = children[0];
    if (height < content->height) {
        scroll->height = height * height / content->height;
        float ratio = height == scroll->height ? 1 : (content->height - height) / (height - scroll->height);
        uint8_t hovered = std::abs(Input::mouse_x - screen_x) < width * Ui::scale / 2
            && std::abs(Input::mouse_y - screen_y) < height * Ui::scale / 2;
        // always_capture_wheel: e.g. inventory open — wheel focuses this list
        if ((hovered || always_capture_wheel) && visible && Input::wheel_delta != 0) {
            // Map wheel to content pixels, then to scrollbar track
            lerp_scroll += Input::wheel_delta / ratio;
            Input::wheel_delta = 0; // consume so camera zoom does not also fire
        }

        // Thumb drag: desktop uses mouse delta; mobile follows the finger
        // that grabbed the bar (same touch_id as this container).
        uint8_t thumb_dragging = scroll->style.layer ? 1 : 0;
        if (thumb_dragging) {
            if (Input::is_mobile) {
                auto it = Input::touches.find(touch_id);
                if (it != Input::touches.end())
                    lerp_scroll += it->second.dy;
            } else {
                lerp_scroll += (Input::mouse_y - Input::prev_mouse_y);
            }
            // Thumb owns the gesture — never also apply content swipe.
            scroll_dragging = 0;
        }

        // Content-area swipe. Soft petal presses must not block scrolling —
        // only a committed petal/bag drag (past slop, finger left the bag)
        // does. Once we scroll, cancel the soft press so release does not
        // equip/swap. Skip entirely while the right-side thumb is dragged.
        if (Input::is_mobile && !thumb_dragging && !Ui::petal_drag_committed()) {
            auto iter = Input::touches.find(touch_id);
            if (iter != Input::touches.end()) {
                float mx = iter->second.x, my = iter->second.y;
                if (!scroll_dragging) {
                    float dist = std::hypot(mx - touch_press_x, my - touch_press_y);
                    // Prefer vertical list scroll over accidental petal-drag
                    // when the swipe is mostly up/down inside this viewport.
                    float adx = std::fabs(mx - touch_press_x);
                    float ady = std::fabs(my - touch_press_y);
                    uint8_t vertical_bias = ady >= adx * 0.85f
                        && ady >= Input::pointer_slop() * 0.55f;
                    // Bag→loadout (down toward the bar) must not become scroll.
                    uint8_t aiming_equip = Ui::bag_gesture_busy()
                        && Ui::bag_gesture_wants_equip(mx, my);
                    if (!aiming_equip && (dist >= Input::pointer_slop() || vertical_bias)) {
                        scroll_dragging = 1;
                        if (Ui::bag_gesture_busy() || Ui::UiLoadout::num_petals_selected != 0)
                            Ui::cancel_petal_drags();
                    }
                }
                if (scroll_dragging)
                    lerp_scroll -= iter->second.dy / ratio;
            } else {
                touch_id = (uint32_t)-1;
                scroll_dragging = 0;
            }
        } else if (Input::is_mobile && (thumb_dragging || Ui::petal_drag_committed())) {
            // Thumb or petal owns the gesture — do not also content-scroll.
            scroll_dragging = 0;
        }

        lerp_scroll = fclamp(lerp_scroll, 0, height - scroll->height);
        scroll->y = lerp(scroll->y, lerp_scroll, Ui::lerp_amount);
        content->y = lerp(content->y, -ratio * lerp_scroll, Ui::lerp_amount);
    } else
        content->y = scroll->y = 0;
    RenderContext c(&ctx);
    ctx.clip_rect(0,0,width,height);
    for (Element *elt : children) {
        if (!elt->visible) continue;
        RenderContext context(&ctx);
        ctx.translate(elt->x, elt->y);
        ctx.translate(elt->style.h_justify * (width - elt->width) / 2, elt->style.v_justify * (height - elt->height) / 2);
        elt->render(ctx);
    }
} 

void ScrollContainer::refactor() {
    DEBUG_ONLY(assert(children.size() == 2));
    // Content left-aligned, scrollbar flush right. Hide bar when content fits.
    Element *content = children[0];
    Element *bar = children[1];
    content->refactor();
    bar->refactor();
    content->style.h_justify = Style::Left;
    content->style.v_justify = Style::Top;
    bar->style.h_justify = Style::Right;
    bar->style.v_justify = Style::Top;
    content->x = 0;
    bar->x = 0;
    uint8_t need_bar = content->height > height + 0.5f;
    bar->visible = need_bar;
    if (!need_bar) {
        bar->width = 0;
        lerp_scroll = 0;
        content->y = 0;
        bar->y = 0;
        width = content->width;
    } else {
        // Desktop thin track; mobile needs a fat hit target for thumb drag.
        bar->width = Input::is_mobile ? 28.f : 10.f;
        width = content->width + inner_pad + bar->width;
    }
}

void ScrollContainer::poll_events(ScreenEvent const &event) {
    uint32_t prev_touch = touch_id;
    Element::poll_events(event);
    if (touch_id != prev_touch && touch_id != (uint32_t) -1) {
        touch_press_x = event.x;
        touch_press_y = event.y;
        scroll_dragging = 0;
    }
    if (touch_id == (uint32_t) -1)
        scroll_dragging = 0;
    if (Ui::focused != this) return;
    // children[0]=content, children[1]=scrollbar. Container::poll_events only
    // walks descendants when Ui::focused == content; if the touch sits in the
    // clip but outside content's AABB (scroll offset / padding), bag cells
    // never see the tap. Force-deliver into content while we own the gesture.
    DEBUG_ONLY(assert(children.size() == 2));
    Element *content = children[0];
    Element *bar = children[1];
    if (content != nullptr) {
        Ui::focused = content;
        content->poll_events(event);
        if (Ui::focused == content)
            Ui::focused = this;
    }
    if (bar != nullptr)
        bar->poll_events(event);
}
