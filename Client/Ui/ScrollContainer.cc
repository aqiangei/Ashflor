#include <Client/Ui/ScrollContainer.hh>

#include <Client/Ui/Extern.hh>
#include <Client/Ui/InGame/Loadout.hh>
#include <Client/Input.hh>

#include <Helpers/Bits.hh>
#include <Helpers/Macros.hh>

using namespace Ui;

ScrollBar::ScrollBar() : Element(10, 100, { .fill = 0xaa5a3a1a, .round_radius = 5 }) {
    style.animate = [&](Element *elt, Renderer &ctx){
        if (elt->style.layer && BitMath::at(Input::mouse_buttons_released, Input::LeftMouse))
            elt->style.layer = 0;
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
        if (scroll->style.layer) lerp_scroll += (Input::mouse_y - Input::prev_mouse_y);
        // Don't scroll while a bag petal tap/drag is active — mobile scroll
        // was eating click-to-equip and turning taps into no-op "drags".
        if (Input::is_mobile
            && Ui::UiLoadout::num_petals_selected == 0
            && !Ui::bag_gesture_busy()) {
            auto iter = Input::touches.find(touch_id);
            if (iter != Input::touches.end())
                lerp_scroll -= iter->second.dy / ratio;
            else
                touch_id = (uint32_t)-1;
        }
        lerp_scroll = fclamp(lerp_scroll, 0, height - scroll->height);
        scroll->y = lerp(scroll->y, lerp_scroll, Ui::lerp_amount);
        content->y = lerp(content->y, -ratio * lerp_scroll, Ui::lerp_amount);
    } else
        content->y = scroll->y = 0;
    RenderContext c(&ctx);
    ctx.clip_rect(0,0,width,height);
    for (Element *elt : children) {
        RenderContext context(&ctx);
        ctx.translate(elt->x, elt->y);
        ctx.translate(elt->style.h_justify * (width - elt->width) / 2, elt->style.v_justify * (height - elt->height) / 2);
        elt->render(ctx);
    }
} 

void ScrollContainer::refactor() {
    DEBUG_ONLY(assert(children.size() == 2));
    // Content left-aligned, scrollbar flush to the right edge of the viewport.
    // (Both Left would park the bar near the middle via justify math.)
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
    width = content->width + inner_pad + bar->width;
}

void ScrollContainer::poll_events(ScreenEvent const &event) {
    Element::poll_events(event);
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
