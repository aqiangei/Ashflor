#pragma once

#include <cstdint>
#include <string>

namespace DOM {
    // password=1 → type=password (masked)
    void create_text_input(char const *, uint32_t, uint8_t password = 0);
    void set_input_password(char const *, uint8_t password);
    void element_show(char const *);
    void element_hide(char const *);
    void update_pos_and_dimension(char const *, float, float, float, float);
    std::string retrieve_text(char const *, uint32_t);
    void update_text(char const *, std::string const &, uint32_t);
    void open_page(char const *);
    void focus_element(char const *);
    void blur_active();
}