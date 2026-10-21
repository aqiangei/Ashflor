#pragma once

#include <Client/Ui/Container.hh>
#include <Client/Ui/Element.hh>

namespace Ui {

    class ScrollBar : public Element {
    public:
        ScrollBar();

        virtual void on_event(uint8_t) override;
    };

    class ScrollContainer : public HContainer {
    public:
        float lerp_scroll;
        // When set, wheel scrolls this list whenever visible (not only on hover)
        uint8_t always_capture_wheel = 0;
        ScrollContainer(Element *, float);

        virtual void on_render(Renderer &) override;
        virtual void poll_events(ScreenEvent const &) override;
        virtual void refactor() override;
    private:
        float touch_press_x = 0;
        float touch_press_y = 0;
        uint8_t scroll_dragging = 0;
    };
}