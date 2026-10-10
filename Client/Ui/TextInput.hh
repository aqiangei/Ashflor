#pragma once

#include <Client/Ui/Element.hh>

#include <string>

namespace Ui {
    class TextInput : public Element {
        std::string const name;
        std::string &ref;
        uint32_t max;
    public:
        TextInput(std::string &, float, float, uint32_t, Style = {});

        void focus();
        void blur();
        void sync_from_dom();
        uint8_t contains_screen_point(float x, float y) const;
        virtual void on_render(Renderer &) override;
        virtual void on_render_skip(Renderer &) override;
        virtual void on_event(uint8_t) override;
    };
}