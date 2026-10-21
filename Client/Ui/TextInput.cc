#include <Client/Ui/TextInput.hh>

#include <Client/Ui/Extern.hh>
#include <Client/DOM.hh>
#include <Client/Game.hh>
#include <Client/Input.hh>

#include <Helpers/UTF8.hh>

#include <cmath>

using namespace Ui;

static TextInput *g_active_text_input = nullptr;

static std::string _gen_nonce() {
    return "fjs" + std::to_string(static_cast<uint32_t>(frand() * 49263 + 672));
}

TextInput::TextInput(std::string &r, float w, float h, uint32_t m, Style s,
                     uint8_t password, uint8_t dark) :
    Element(w, h, s), name(_gen_nonce()), ref(r), max(m),
    is_password(password), password_masked(password ? 1 : 0), editing(0),
    dark_theme(dark) {
    style.fill = dark ? 0xff303B54 : 0xffeeeeee;
    style.stroke_hsv = 0;
    DOM::create_text_input(name.c_str(), max, is_password ? 1 : 0, dark);
    DOM::update_text(name.c_str(), r, max);
}

void TextInput::set_password_masked(uint8_t masked) {
    if (!is_password) return;
    password_masked = masked ? 1 : 0;
    DOM::set_input_password(name.c_str(), password_masked);
}

void TextInput::focus() {
    if (g_active_text_input != nullptr && g_active_text_input != this)
        g_active_text_input->blur();
    editing = 1;
    g_active_text_input = this;
    DOM::element_show(name.c_str());
    DOM::update_text(name.c_str(), ref, max);
    DOM::focus_element(name.c_str());
}

void TextInput::blur() {
    if (!editing && g_active_text_input != this) {
        DOM::element_hide(name.c_str());
        return;
    }
    sync_from_dom();
    DOM::blur_active();
    DOM::element_hide(name.c_str());
    editing = 0;
    if (g_active_text_input == this)
        g_active_text_input = nullptr;
}

void TextInput::sync_from_dom() {
    ref = DOM::retrieve_text(name.c_str(), max);
}

void TextInput::clear() {
    ref.clear();
    DOM::update_text(name.c_str(), ref, max);
}

void TextInput::set_text(std::string const &s) {
    ref = s;
    DOM::update_text(name.c_str(), ref, max);
}

uint8_t TextInput::contains_screen_point(float x, float y) const {
    return std::abs(x - screen_x) <= width * Ui::scale / 2
        && std::abs(y - screen_y) <= height * Ui::scale / 2;
}

void TextInput::on_event(uint8_t event) {
    if (event == kMouseDown || event == kClick)
        focus();
    // Do not blur on kFocusLost: while editing, the HTML <input> sits above the
    // canvas, so canvas focus flickers even though the user is still typing.
}

void TextInput::on_render(Renderer &ctx) {
    Element::on_render(ctx);
    if (!editing) {
        // Idle: canvas-only. Hiding the DOM overlay stops outside taps from
        // re-focusing a leftover <input> and popping the mobile keyboard.
        DOM::element_hide(name.c_str());
        if (!ref.empty()) {
            std::string shown = ref;
            if (is_password && password_masked) {
                UTF8Parser p(ref);
                uint32_t n = 0;
                while (!p.at_end()) { p.next_symbol(); ++n; }
                shown.assign(n, '*');
            }
            ctx.draw_text(shown.c_str(), {
                .fill = dark_theme ? 0xffF6F1FF : 0xff111111,
                .size = height * 0.45f,
                .stroke_scale = 0
            });
        }
        return;
    }
    DOM::element_show(name.c_str());
    DOM::update_pos_and_dimension(name.c_str(), screen_x, screen_y, width * Ui::scale, height * Ui::scale);
    ref = DOM::retrieve_text(name.c_str(), max);
    DOM::update_text(name.c_str(), ref, max);
}

void TextInput::on_render_skip(Renderer &ctx) {
    if (editing) blur();
    else DOM::element_hide(name.c_str());
}

void Ui::dismiss_active_text_input_at(float x, float y) {
    if (g_active_text_input == nullptr) return;
    if (g_active_text_input->contains_screen_point(x, y)) return;
    // Chat: only blur when press is still inside the chat panel (e.g. Send).
    // Presses outside the panel are handled by try_close_popups → close_chat().
    if (Game::chat_open && chat_panel_root != nullptr) {
        float hw = chat_panel_root->width * Ui::scale / 2;
        float hh = chat_panel_root->height * Ui::scale / 2;
        uint8_t in_panel = std::abs(x - chat_panel_root->screen_x) <= hw
            && std::abs(y - chat_panel_root->screen_y) <= hh;
        if (!in_panel) return;
    }
    g_active_text_input->blur();
}

TextInput *Ui::active_text_input() {
    return g_active_text_input;
}
