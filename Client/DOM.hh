#pragma once

#include <cstdint>
#include <string>

namespace DOM {
    // Set by overlay <input> Enter / soft-keyboard Send; consumed each tick.
    extern uint8_t text_enter_pressed;
    // password=1 → type=password (masked)
    void create_text_input(char const *, uint32_t, uint8_t password = 0,
                           uint8_t dark = 0);
    void set_input_password(char const *, uint8_t password);
    void element_show(char const *);
    void element_hide(char const *);
    void update_pos_and_dimension(char const *, float, float, float, float);
    std::string retrieve_text(char const *, uint32_t);
    void update_text(char const *, std::string const &, uint32_t);
    void open_page(char const *);
    void open_feedback_board();
    void feedback_result(uint8_t result, uint32_t id);
    void feedback_resolve_result(uint8_t result, uint32_t id, uint8_t resolved);
    void open_qq_group();
    void focus_element(char const *);
    void blur_active();
    // Persist VERSION_HASH so locateFile / script URLs keep matching after first load
    void cache_client();
    // Clear hash + hard reload (outdated client / bad wasm)
    void update_client(float delay_ms);
    void toggle_fullscreen();
}
