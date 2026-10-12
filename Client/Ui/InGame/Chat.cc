#include <Client/Ui/InGame/GameInfo.hh>

#include <Client/Ui/Button.hh>
#include <Client/Ui/Container.hh>
#include <Client/Ui/DynamicText.hh>
#include <Client/Ui/TextInput.hh>

#include <Client/Game.hh>
#include <Client/I18n.hh>
#include <Client/Input.hh>

#include <Shared/Binary.hh>
#include <Shared/Config.hh>

#include <Helpers/Bits.hh>

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
        default: return 0xffffffff;
    }
}

static std::string channel_bracket_label() {
    if (Game::chat_channel == ChatChannel::kChatParty)
        return std::string{"【"} + I18n::tr(I18n::kChatParty) + "】";
    if (Game::chat_channel == ChatChannel::kChatGuild)
        return std::string{"【"} + I18n::tr(I18n::kChatGuild) + "】";
    if (Game::chat_channel == ChatChannel::kChatWhisper && !Game::chat_whisper_to.empty())
        return "【" + Game::chat_whisper_to + "】";
    return std::string{"【"} + I18n::tr(I18n::kChatNearby) + "】";
}

class ChatTextInput : public TextInput {
public:
    ChatTextInput(std::string &r, float w, float h, uint32_t m, Style s)
        : TextInput(r, w, h, m, s) {}

    void on_event(uint8_t e) override {
        if (e == kMouseDown || e == kClick)
            Game::open_chat();
    }

    void on_render(Renderer &ctx) override {
        if (Game::chat_open) {
            TextInput::on_render(ctx);
            return;
        }
        // Inactive box: clickable, no DOM focus
        TextInput::on_render_skip(ctx);
        Element::on_render(ctx);
        ctx.set_fill(0xff888888);
        ctx.draw_text(I18n::tr(I18n::kChatPrompt), { .fill = 0xff888888, .size = height * 0.45f });
    }
};

class ChatLineElement : public Element {
    uint32_t index;
public:
    ChatLineElement(uint32_t i) : Element(0, 14, {
        .should_render = [i](){ return i < Game::chat_lines.size(); },
        .h_justify = Style::Left
    }), index(i) {}

    void refactor() override {
        if (index >= Game::chat_lines.size()) {
            width = 0;
            return;
        }
        Game::ChatLine const &line = Game::chat_lines[index];
        Game::renderer.set_text_size(height);
        std::string full;
        if (line.channel == ChatChannel::kChatSystem || line.sender.empty())
            full = line.text;
        else
            full = line.sender + ": " + line.text;
        width = Game::renderer.get_text_size(full.c_str());
    }

    void on_render(Renderer &ctx) override {
        if (index >= Game::chat_lines.size()) return;
        Game::ChatLine const &line = Game::chat_lines[index];
        uint32_t color = channel_color(line.channel);
        ctx.set_text_size(height);
        ctx.center_text_baseline();
        float left = -width / 2;
        if (line.channel == ChatChannel::kChatSystem || line.sender.empty()) {
            ctx.set_fill(color);
            ctx.translate(left + ctx.get_text_size(line.text.c_str()) / 2, 0);
            ctx.fill_text(line.text.c_str());
        } else {
            std::string name = line.sender;
            std::string rest = ": " + line.text;
            float name_w = ctx.get_text_size(name.c_str());
            float rest_w = ctx.get_text_size(rest.c_str());
            {
                RenderContext c(&ctx);
                ctx.set_fill(color);
                ctx.translate(left + name_w / 2, 0);
                ctx.fill_text(name.c_str());
            }
            {
                RenderContext c(&ctx);
                ctx.set_fill(0xffffffff);
                ctx.translate(left + name_w + rest_w / 2, 0);
                ctx.fill_text(rest.c_str());
            }
            if (BitMath::at(Input::mouse_buttons_pressed, Input::RightMouse)) {
                float mx = Input::mouse_x;
                float my = Input::mouse_y;
                float name_left = screen_x - width * Ui::scale / 2;
                float name_right = name_left + name_w * Ui::scale;
                float top = screen_y - height * Ui::scale / 2;
                float bot = screen_y + height * Ui::scale / 2;
                if (mx >= name_left && mx <= name_right && my >= top && my <= bot)
                    Game::open_chat_menu(name, mx, my);
            }
        }
    }
};

Element *Ui::make_chat_panel() {
    std::vector<Element *> lines;
    for (uint32_t i = 0; i < Game::CHAT_HISTORY; ++i)
        lines.push_back(new ChatLineElement(i));

    chat_input = new ChatTextInput(Game::chat_draft, 280, 28, MAX_CHAT_LENGTH, {
        .line_width = 3,
        .round_radius = 4,
        .should_render = [](){
            return Game::alive() && Game::should_render_game_ui();
        }
    });

    lines.push_back(new Ui::HContainer({
        new Ui::DynamicText(13, [](){ return channel_bracket_label(); }, {
            .fill = 0xffffffff,
            .animate = [](Element *elt, Renderer &) {
                elt->style.fill = channel_color(Game::chat_channel);
            },
            .should_render = [](){ return Game::alive(); }
        }),
        chat_input
    }, 0, 6, { .h_justify = Style::Left }));

    Element *elt = new Ui::VContainer(lines, 8, 3, {
        .fill = 0x66000000,
        .round_radius = 6,
        .should_render = [](){
            return Game::should_render_game_ui() && Game::alive();
        },
        .h_justify = Style::Left,
        .v_justify = Style::Bottom
    });
    // Right of vertical B/Z/C/V column (~44+pad), sit on bottom edge
    elt->x = 64;
    elt->y = -10;
    return elt;
}

Element *Ui::make_chat_context_menu() {
    Element *elt = new Ui::VContainer({
        new Ui::DynamicText(12, [](){ return Game::chat_menu_name; }, {
            .fill = 0xffdddddd
        }),
        new Ui::Button(150, 28,
            new Ui::DynamicText(13, [](){ return I18n::tr(I18n::kChatInviteParty); }),
            [](Element *, uint8_t e){
                if (e == Ui::kClick) Game::chat_menu_invite();
            },
            nullptr,
            { .fill = 0xff5a9fdb, .line_width = 3, .round_radius = 3 }
        ),
        new Ui::Button(150, 28,
            new Ui::DynamicText(13, [](){ return I18n::tr(I18n::kChatInviteGuild); }),
            [](Element *, uint8_t e){
                if (e == Ui::kClick) Game::chat_menu_invite_guild();
            },
            nullptr,
            { .fill = 0xff3d8bfd, .line_width = 3, .round_radius = 3 }
        ),
        new Ui::Button(150, 28,
            new Ui::DynamicText(13, [](){ return I18n::tr(I18n::kChatWhisper); }),
            [](Element *, uint8_t e){
                if (e == Ui::kClick) Game::chat_menu_whisper();
            },
            nullptr,
            { .fill = 0xff7a3d9b, .line_width = 3, .round_radius = 3 }
        )
    }, 8, 6, {
        .fill = 0xe0222222,
        .line_width = 3,
        .round_radius = 5,
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
