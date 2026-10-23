#include <Client/Ui/Window.hh>

#include <Client/Ui/Extern.hh>

#include <Client/Game.hh>
#include <Client/Input.hh>

#include <cmath>

using namespace Ui;

Window::Window() : Container({}) {}

void Window::render(Renderer &ctx) {
    width = Ui::window_width;
    height = Ui::window_height;
    // Element::refactor skips invisible children, so a panel that just became
    // should_render() would keep a stale/zero size. Layout anything that wants
    // to show this frame (debug overlay, settings sheets, …).
    for (Element *elt : children) {
        if (elt->visible
            || elt->style.should_render == nullptr
            || elt->style.should_render())
            elt->refactor();
    }
    RenderContext context(&ctx);
    ctx.reset_transform();
    ctx.translate(width / 2, height / 2);
    Element::render(ctx);
}

void Window::on_render(Renderer &ctx) {
    RenderContext context(&ctx);
    for (uint32_t layer = 0; layer < 2; ++layer) {
        for (uint32_t i = 0; i < children.size(); ++i) {
            Element *elt = children[i];
            if (elt->style.layer != layer) continue;
            RenderContext context(&ctx);
            ctx.translate(elt->style.h_justify * width / 2, elt->style.v_justify * height / 2);
            ctx.scale(Ui::scale);
            ctx.translate(-elt->style.h_justify * elt->width / 2, -elt->style.v_justify * elt->height / 2);
            ctx.translate(elt->x, elt->y);
            elt->render(ctx);
        }
    }
    on_render_tooltip(ctx);
}

void Window::poll_events(ScreenEvent const &event) {
    if (style.no_polling) return;
    Element::poll_events(event);
    // Later children sit on top (drawers over loadout). Poll reverse so the
    // topmost hit claims focus; then stop so lower UI cannot steal it.
    for (size_t i = children.size(); i > 0; --i) {
        Element *elt = children[i - 1];
        if (!elt->visible) continue;
        Element *before = Ui::focused;
        elt->poll_events(event);
        if (Input::is_mobile) {
            auto touch = Input::touches.find(event.id);
            if (touch != Input::touches.end() && touch->second.saturated
                && Ui::focused != nullptr)
                break;
        } else if (Ui::focused != nullptr && Ui::focused != before) {
            break;
        }
    }
}

void Window::on_event(uint8_t event) {
    if (event == Ui::kMouseDown)
        Ui::panel_open = Ui::Panel::kNone;
}