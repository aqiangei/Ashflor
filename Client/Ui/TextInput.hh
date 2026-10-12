#pragma once

#include <Client/Ui/Element.hh>

#include <string>

namespace Ui {
    class TextInput : public Element {
        std::string const name;
        std::string &ref;
        uint32_t max;
        uint8_t is_password = 0;
        uint8_t password_masked = 1;
    public:
        // is_password=1 → start as masked type=password
        TextInput(std::string &, float, float, uint32_t, Style = {}, uint8_t is_password = 0);

        void focus();
        void blur();
        void sync_from_dom();
        void set_password_masked(uint8_t masked);
        uint8_t password_is_masked() const { return password_masked; }
        uint8_t contains_screen_point(float x, float y) const;
        virtual void on_render(Renderer &) override;
        virtual void on_render_skip(Renderer &) override;
        virtual void on_event(uint8_t) override;
    };
}
