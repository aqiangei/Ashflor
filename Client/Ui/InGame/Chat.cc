#include <Client/Ui/InGame/GameInfo.hh>

#include <Client/Ui/Button.hh>
#include <Client/Ui/Chrome.hh>
#include <Client/Ui/Container.hh>
#include <Client/Ui/DynamicText.hh>
#include <Client/Ui/Extern.hh>
#include <Client/Ui/ScrollContainer.hh>
#include <Client/Ui/TextInput.hh>

#include <Client/Game.hh>
#include <Client/I18n.hh>
#include <Client/Input.hh>

#include <Shared/Binary.hh>
#include <Shared/Config.hh>

#include <Helpers/Bits.hh>
#include <Helpers/Math.hh>

#include <cmath>
#include <string>

using namespace Ui;

static TextInput *chat_input = nullptr;

TextInput *Ui::chat_text_input() {
    return chat_input;
}

static uint32_t channel_color(uint8_t channel) {
    switch (channel) {
        case ChatChannel::kChatParty: return 0xffff69b4;
        case ChatChannel::kChatGuild: return 0xff40e0d0;
        case ChatChannel::kChatWhisper: return 0xff9b30ff;
        case ChatChannel::kChatSystem: return 0xffaaaaaa;
        case ChatChannel::kChatServer: return 0xffffd700;
        default: return 0xffffffff;
    }
}

static std::string channel_bracket_label() {
    if (Game::chat_channel == ChatChannel::kChatParty)
        return std::string{"【"} + I18n::tr(I18n::kChatParty) + "】";
    if (Game::chat_channel == ChatChannel::kChatGuild)
        return std::string{"【"} + I18n::tr(I18n::kChatGuild) + "】";
    if (Game::chat_channel == ChatChannel::kChatServer)
        return std::string{"【"} + I18n::tr(I18n::kChatServer) + "】";
    if (Game::chat_channel == ChatChannel::kChatWhisper && !Game::chat_whisper_to.empty())
        return "【" + Game::chat_whisper_to + "】";
    return std::string{"【"} + I18n::tr(I18n::kChatNearby) + "】";
}

// Opacity for a line while the panel is collapsed (0 = hide).
static float collapsed_line_alpha(Game::ChatLine const &line) {
    double age = Game::timestamp - line.created_at;
    if (age < 0) age = 0;
    if (age <= Game::CHAT_COLLAPSE_HOLD_MS) return 1.f;
    double fade_t = (age - Game::CHAT_COLLAPSE_HOLD_MS) / Game::CHAT_COLLAPSE_FADE_MS;
    if (fade_t >= 1.0) return 0.f;
    return (float) (1.0 - fade_t);
}

static uint8_t line_visible_now(uint32_t i) {
    if (i >= Game::chat_lines.size()) return 0;
    if (Game::chat_open) return 1;
    return collapsed_line_alpha(Game::chat_lines[i]) > 0.02f;
}

class ChatTextInput : public TextInput {
public:
    ChatTextInput(std::string &r, float w, float h, uint32_t m, Style s)
        : TextInput(r, w, h, m, s) {}

    void on_event(uint8_t e) override {
        if (e == kMouseDown || e == kClick) {
            Game::open_chat();
            TextInput::on_event(e);
        }
    }

    void on_render(Renderer &ctx) override {
        if (Game::chat_open) {
            TextInput::on_render(ctx);
            return;
        }
        TextInput::on_render_skip(ctx);
        Element::on_render(ctx);
        ctx.set_fill(0xff888888);
        ctx.draw_text(I18n::tr(I18n::kChatPrompt), { .fill = 0xff888888, .size = height * 0.45f });
    }
};

class ChatLineElement : public Element {
    uint32_t index;
public:
    ChatLineElement(uint32_t i) : Element(0, 16, {
        .should_render = [i](){ return line_visible_now(i); },
        .h_justify = Style::Left,
        .no_animation = 1
    }), index(i) {}

    void refactor() override {
        if (!line_visible_now(index)) {
            width = 0;
            height = 0;
            // So VContainer skips padding this frame (visible is normally
            // refreshed in render(), which runs after layout).
            visible = 0;
            return;
        }
        visible = 1;
        height = 16;
        Game::ChatLine const &line = Game::chat_lines[index];
        Game::renderer.set_text_size(height);
        std::string full;
        if (line.channel == ChatChannel::kChatSystem || line.sender.empty())
            full = line.text;
        else if (line.channel == ChatChannel::kChatWhisper && !line.whisper_to.empty())
            full = line.sender + " → " + line.whisper_to + ": " + line.text;
        else
            full = line.sender + ": " + line.text;
        width = Game::renderer.get_text_size(full.c_str());
        if (width < 40.f) width = 40.f;
    }

    void on_render(Renderer &ctx) override {
        if (!line_visible_now(index)) return;
        Game::ChatLine const &line = Game::chat_lines[index];
        float alpha = Game::chat_open ? 1.f : collapsed_line_alpha(line);
        if (alpha <= 0.02f) return;
        ctx.set_global_alpha(alpha);
        uint32_t color = channel_color(line.channel);
        ctx.set_text_size(height);
        ctx.center_text_baseline();
        float left = -width / 2;
        if (line.channel == ChatChannel::kChatSystem || line.sender.empty()) {
            ctx.set_fill(color);
            ctx.translate(left + ctx.get_text_size(line.text.c_str()) / 2, 0);
            ctx.fill_text(line.text.c_str());
        } else {
            uint8_t whisper = line.channel == ChatChannel::kChatWhisper
                && !line.whisper_to.empty();
            std::string name = line.sender;
            std::string arrow = whisper ? (" → " + line.whisper_to) : "";
            std::string rest = ": " + line.text;
            float name_w = ctx.get_text_size(name.c_str());
            float arrow_w = arrow.empty() ? 0.f : ctx.get_text_size(arrow.c_str());
            float rest_w = ctx.get_text_size(rest.c_str());
            {
                RenderContext c(&ctx);
                ctx.set_fill(color);
                ctx.translate(left + name_w / 2, 0);
                ctx.fill_text(name.c_str());
            }
            if (!arrow.empty()) {
                RenderContext c(&ctx);
                ctx.set_fill(color);
                ctx.translate(left + name_w + arrow_w / 2, 0);
                ctx.fill_text(arrow.c_str());
            }
            {
                RenderContext c(&ctx);
                ctx.set_fill(0xffffffff);
                ctx.translate(left + name_w + arrow_w + rest_w / 2, 0);
                ctx.fill_text(rest.c_str());
            }
            if (Game::chat_open
                && BitMath::at(Input::mouse_buttons_pressed, Input::RightMouse)) {
                float mx = Input::mouse_x;
                float my = Input::mouse_y;
                float name_left = screen_x - width * Ui::scale / 2;
                float top = screen_y - height * Ui::scale / 2;
                float bot = screen_y + height * Ui::scale / 2;
                float sender_right = name_left + name_w * Ui::scale;
                float arrow_right = sender_right + arrow_w * Ui::scale;
                if (my >= top && my <= bot) {
                    if (mx >= name_left && mx <= sender_right)
                        Game::open_chat_menu(line.sender, mx, my);
                    else if (whisper && mx > sender_right && mx <= arrow_right)
                        Game::open_chat_menu(line.whisper_to, mx, my);
                }
            }
        }
        ctx.set_global_alpha(1.f);
    }
};

class ChatScroll final : public Ui::ScrollContainer {
    float open_h;
public:
    ChatScroll(Element *content, float open_height)
        : Ui::ScrollContainer(content, open_height), open_h(open_height) {}

    void refactor() override {
        Element *content = children[0];
        Element *bar = children[1];
        always_capture_wheel = Game::chat_open ? 1 : 0;

        if (Game::chat_open) {
            height = open_h;
        } else {
            // Size toast window to currently-visible (fading) lines only.
            if (content != nullptr) content->refactor();
            float need = content != nullptr ? content->height : 0.f;
            height = need < 1.f ? 0.f : fminf(need, open_h * 0.55f);
        }

        Ui::ScrollContainer::refactor();

        if (!Game::chat_open) {
            // No scrollbar / wheel steal while collapsed — just a fading toast.
            if (bar != nullptr) {
                bar->visible = 0;
                bar->width = 0;
            }
            if (content != nullptr) {
                width = content->width;
                if (content->height > height + 0.5f) {
                    float bar_h = height * height / content->height;
                    lerp_scroll = height - bar_h;
                    float ratio = (content->height - height)
                        / std::max(0.001f, height - bar_h);
                    content->y = -ratio * lerp_scroll;
                } else {
                    lerp_scroll = 0;
                    content->y = 0;
                }
            }
            return;
        }

        if (!Game::chat_scroll_to_end) return;
        Game::chat_scroll_to_end = 0;
        if (content != nullptr && content->height > height + 0.5f) {
            float bar_h = height * height / content->height;
            lerp_scroll = height - bar_h;
            if (bar != nullptr) bar->y = lerp_scroll;
            float ratio = (content->height - height)
                / std::max(0.001f, height - bar_h);
            content->y = -ratio * lerp_scroll;
        } else {
            lerp_scroll = 0;
        }
    }
};

Element *Ui::make_chat_panel() {
    std::vector<Element *> line_els;
    for (uint32_t i = 0; i < Game::CHAT_HISTORY; ++i)
        line_els.push_back(new ChatLineElement(i));
    Element *history = new Ui::VContainer(line_els, 4, 3, {
        .no_animation = 1,
        .h_justify = Style::Left
    });

    uint8_t const mobile = Input::is_mobile;
    float const open_h = mobile ? 160.f : 200.f;
    auto *scroll = new ChatScroll(history, open_h);
    scroll->style.should_render = [](){
        if (!(Game::alive() && Game::should_render_game_ui())) return false;
        if (Game::chat_open) return true;
        for (uint32_t i = 0; i < Game::chat_lines.size(); ++i)
            if (line_visible_now(i)) return true;
        return false;
    };
    scroll->style.h_justify = Style::Left;

    float const input_w = mobile ? 300.f : 360.f;
    float const input_h = mobile ? 36.f : 32.f;
    chat_input = new ChatTextInput(Game::chat_draft, input_w, input_h, MAX_CHAT_LENGTH, {
        .line_width = 0,
        .round_radius = Chrome::kRadius,
        .should_render = [](){
            return Game::alive() && Game::should_render_game_ui();
        }
    });

    Element *send_btn = new Ui::Button(
        mobile ? 56.f : 52.f, input_h,
        new Ui::DynamicText(mobile ? 15.f : 14.f, [](){
            return I18n::use_chinese ? std::string{"发送"} : std::string{"Send"};
        }),
        [](Element *, uint8_t e){
            if (e != Ui::kClick) return;
            if (!Game::chat_open) Game::open_chat();
            Game::send_chat();
        },
        nullptr,
        {
            .fill = Chrome::kBtnMoss,
            .line_width = 0,
            .round_radius = Chrome::kRadius,
            .should_render = [](){
                return Game::alive() && Game::should_render_game_ui()
                    && Game::chat_open;
            }
        }
    );

    Element *input_row = new Ui::HContainer({
        new Ui::DynamicText(13, [](){ return channel_bracket_label(); }, {
            .fill = 0xffffffff,
            .animate = [](Element *elt, Renderer &) {
                elt->style.fill = channel_color(Game::chat_channel);
            },
            .should_render = [](){ return Game::alive(); }
        }),
        chat_input,
        send_btn
    }, 0, 6, { .h_justify = Style::Left });

    Element *tab_hint = new Ui::DynamicText(12, [](){
        return Game::chat_completion_hint;
    }, {
        .fill = 0xffc8b0ff,
        .h_justify = Style::Left,
        .should_render = [](){
            return Game::chat_open && !Game::chat_completion_hint.empty();
        }
    });

    Element *elt = new Ui::VContainer({ scroll, input_row, tab_hint }, 10, 4, {
        .fill = 0x88000000,
        .round_radius = Chrome::kRadius,
        .animate = [](Element *elt, Renderer &) {
            // Solid panel while focused; transparent when only fading toasts show.
            if (Game::chat_open)
                elt->style.fill = 0x88000000;
            else {
                uint8_t any = 0;
                for (uint32_t i = 0; i < Game::chat_lines.size(); ++i) {
                    if (line_visible_now(i)) { any = 1; break; }
                }
                elt->style.fill = any ? 0x44000000 : 0x00000000;
            }
        },
        .should_render = [](){
            return Game::should_render_game_ui() && Game::alive();
        },
        .h_justify = Style::Left,
        .v_justify = Style::Bottom
    });
    elt->x = mobile ? 64.f : 72.f;
    elt->y = -16;
    Ui::chat_panel_root = elt;
    return elt;
}

Element *Ui::make_chat_context_menu() {
    Element *elt = new Ui::VContainer({
        new Ui::DynamicText(12, [](){ return Game::chat_menu_name; }, {
            .fill = Chrome::kBody
        }),
        new Ui::Button(150, 28,
            new Ui::DynamicText(13, [](){ return I18n::tr(I18n::kChatInviteParty); }),
            [](Element *, uint8_t e){
                if (e == Ui::kClick) Game::chat_menu_invite();
            },
            nullptr,
            { .fill = Chrome::kBtnSlate, .line_width = 0, .round_radius = Chrome::kRadius }
        ),
        new Ui::Button(150, 28,
            new Ui::DynamicText(13, [](){ return I18n::tr(I18n::kChatInviteGuild); }),
            [](Element *, uint8_t e){
                if (e == Ui::kClick) Game::chat_menu_invite_guild();
            },
            nullptr,
            { .fill = Chrome::kBtnBronze, .line_width = 0, .round_radius = Chrome::kRadius }
        ),
        new Ui::Button(150, 28,
            new Ui::DynamicText(13, [](){ return I18n::tr(I18n::kChatWhisper); }),
            [](Element *, uint8_t e){
                if (e == Ui::kClick) Game::chat_menu_whisper();
            },
            nullptr,
            { .fill = Chrome::kBtnIron, .line_width = 0, .round_radius = Chrome::kRadius }
        )
    }, 8, 6, {
        .fill = Chrome::kPanelDeep, .line_width = 0, .round_radius = Chrome::kRadius,
        .animate = [](Element *elt, Renderer &) {
            elt->x = Game::chat_menu_x / Ui::scale;
            elt->y = Game::chat_menu_y / Ui::scale;
        },
        .should_render = [](){
            return Game::chat_menu_open && Game::alive() && Game::should_render_game_ui();
        },
        .h_justify = Style::Left,
        .v_justify = Style::Top,
        .layer = 1
    });
    return elt;
}
