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
#include <Shared/RarityScale.hh>
#include <Shared/StaticDefinitions.hh>

#include <Helpers/Bits.hh>
#include <Helpers/Math.hh>
#include <Helpers/UTF8.hh>

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

using namespace Ui;

static TextInput *chat_input = nullptr;
static constexpr uint32_t kWhisperAccent = 0xffD89AFF;

TextInput *Ui::chat_text_input() {
    return chat_input;
}

static uint32_t channel_color(uint8_t channel) {
    switch (channel) {
        case ChatChannel::kChatParty: return 0xffff69b4;
        case ChatChannel::kChatGuild: return 0xff40e0d0;
        case ChatChannel::kChatWhisper: return kWhisperAccent;
        case ChatChannel::kChatSystem: return 0xffaaaaaa;
        case ChatChannel::kChatServer: return 0xffffd700;
        default: return 0xffffffff;
    }
}

// whisper_to "craft:N" from high-craft broadcast → Super/Unique rarity tint
static uint32_t craft_line_color(Game::ChatLine const &line) {
    if (line.whisper_to.size() < 7 || line.whisper_to.compare(0, 6, "craft:") != 0)
        return 0;
    unsigned r = 0;
    for (size_t i = 6; i < line.whisper_to.size(); ++i) {
        char c = line.whisper_to[i];
        if (c < '0' || c > '9') break;
        r = r * 10u + (unsigned) (c - '0');
    }
    if (r >= RarityID::kNumRarities) return 0;
    // Unique petal tint is dark gunmetal — unreadable on chat. Use bright gold.
    if (r == RarityID::kUnique) return 0xffffd24a;
    return RARITY_COLORS[r] | 0xff000000;
}

static std::string channel_bracket_label() {
    if (Game::chat_channel == ChatChannel::kChatParty)
        return std::string{"【"} + I18n::tr(I18n::kChatParty) + "】";
    if (Game::chat_channel == ChatChannel::kChatGuild)
        return std::string{"【"} + I18n::tr(I18n::kChatGuild) + "】";
    if (Game::chat_channel == ChatChannel::kChatServer)
        return std::string{"【"} + I18n::tr(I18n::kChatServer) + "】";
    if (Game::chat_channel == ChatChannel::kChatWhisper && !Game::chat_whisper_to.empty())
        return I18n::use_chinese
            ? "[私聊 " + Game::chat_whisper_to + "]"
            : "[Whisper " + Game::chat_whisper_to + "]";
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

struct ChatLineParts {
    std::string tag;
    std::string lead;
    std::string name;
    std::string tail;
    std::string message;
};

static ChatLineParts chat_line_parts(Game::ChatLine const &line) {
    if (line.channel == ChatChannel::kChatWhisper && !line.sender.empty()) {
        bool outgoing = line.sender == Game::nickname || line.sender == Game::username;
        return {
            I18n::use_chinese ? "[私聊] " : "[Whisper] ",
            outgoing ? (I18n::use_chinese ? "发给 " : "To ") : std::string{},
            outgoing && !line.whisper_to.empty() ? line.whisper_to : line.sender,
            outgoing ? ": " : (I18n::use_chinese ? " 对你说: " : " to you: "),
            line.text
        };
    }
    // Server channel must not look like nearby player chat (ops used to spoof "aqa").
    if (line.channel == ChatChannel::kChatServer && !line.sender.empty()) {
        uint8_t is_craft = line.whisper_to.size() >= 6
            && line.whisper_to.compare(0, 6, "craft:") == 0;
        uint8_t is_banner = line.whisper_to == "banner";
        return {
            is_craft
                ? (I18n::use_chinese ? "[合成] " : "[Craft] ")
                : (is_banner
                    ? (I18n::use_chinese ? "[广播] " : "[Announce] ")
                    : (I18n::use_chinese ? "[全服] " : "[Server] ")),
            {},
            line.sender,
            I18n::use_chinese ? "：" : ": ",
            line.text
        };
    }
    return { {}, {}, line.sender, ": ", line.text };
}

class ChatTextInput : public TextInput {
public:
    ChatTextInput(std::string &r, float w, float h, uint32_t m, Style s)
        : TextInput(r, w, h, m, s, 0, 1) {}

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
        // Keep unsent draft visible while collapsed (blur / Esc / outside click)
        if (!Game::chat_draft.empty()) {
            ctx.draw_text(Game::chat_draft.c_str(), {
                .fill = 0xffF6F1FF,
                .size = Input::is_mobile ? 12.5f : height * 0.45f,
                .stroke_scale = 0
            });
            return;
        }
        ctx.draw_text(I18n::use_chinese ? "按回车键或点击此处聊天"
                                        : (Input::is_mobile ? "Tap or Enter to chat"
                                                            : "Press Enter or click to chat"), {
            .fill = 0xffF6F1FF, .size = Input::is_mobile ? 12.5f : height * 0.42f
        });
    }
};

static float chat_line_max_w() {
    // Match ChatScroll floor; stay clear of bottom-right loadout chrome.
    return Input::is_mobile ? 300.f : 460.f;
}

// UTF-8 wrap (CJK-safe). No ellipsis — chat should show the full message.
static void wrap_chat_text(float size, std::string const &text, float max_w,
                           std::vector<std::string> &out) {
    out.clear();
    if (text.empty()) return;
    Game::renderer.set_text_size(size);
    UTF8Parser p(text);
    std::string cur;
    while (!p.at_end()) {
        uint32_t start = p.offset();
        if (p.next_symbol() == 0) break;
        uint32_t end = p.offset();
        std::string next = cur;
        next.append(text, start, end - start);
        if (!cur.empty() && Game::renderer.get_text_size(next.c_str()) > max_w) {
            out.push_back(std::move(cur));
            cur.assign(text, start, end - start);
        } else {
            cur = std::move(next);
        }
    }
    if (!cur.empty()) out.push_back(std::move(cur));
}

class ChatLineElement : public Element {
    uint32_t index;
    std::vector<std::string> wrap_lines;
    float prefix_w = 0.f; // tag+lead+name+tail on first visual row
    float line_h = 17.f;
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
            wrap_lines.clear();
            // So VContainer skips padding this frame (visible is normally
            // refreshed in render(), which runs after layout).
            visible = 0;
            return;
        }
        visible = 1;
        line_h = 17.f;
        float const max_w = chat_line_max_w();
        Game::ChatLine const &line = Game::chat_lines[index];
        Game::renderer.set_text_size(line_h);
        if (line.channel == ChatChannel::kChatSystem || line.sender.empty()) {
            prefix_w = 0.f;
            wrap_chat_text(line_h, line.text, max_w, wrap_lines);
            if (wrap_lines.empty()) wrap_lines.push_back({});
            width = 0.f;
            for (auto const &ln : wrap_lines)
                width = fmax(width, Game::renderer.get_text_size(ln.c_str()));
        } else {
            ChatLineParts p = chat_line_parts(line);
            prefix_w = Game::renderer.get_text_size((p.tag + p.lead + p.name + p.tail).c_str());
            float msg_budget = fmax(40.f, max_w - prefix_w);
            // If prefix alone is huge, wrap the whole string as one block.
            if (prefix_w > max_w * 0.85f) {
                prefix_w = 0.f;
                wrap_chat_text(line_h, p.tag + p.lead + p.name + p.tail + p.message,
                               max_w, wrap_lines);
            } else {
                wrap_chat_text(line_h, p.message, msg_budget, wrap_lines);
                if (wrap_lines.empty()) wrap_lines.push_back({});
            }
            width = prefix_w;
            for (size_t i = 0; i < wrap_lines.size(); ++i) {
                float w = Game::renderer.get_text_size(wrap_lines[i].c_str());
                if (i == 0 && prefix_w > 0.f) w += prefix_w;
                if (w > width) width = w;
            }
        }
        if (width < 40.f) width = 40.f;
        if (width > max_w) width = max_w;
        uint32_t rows = (uint32_t) wrap_lines.size();
        if (rows < 1) rows = 1;
        height = line_h * (float) rows + 2.f * (float) (rows - 1);
    }

    void on_render(Renderer &ctx) override {
        if (!line_visible_now(index)) return;
        Game::ChatLine const &line = Game::chat_lines[index];
        float alpha = Game::chat_open ? 1.f : collapsed_line_alpha(line);
        if (alpha <= 0.02f) return;
        ctx.set_global_alpha(alpha);
        uint32_t color = channel_color(line.channel);
        uint32_t craft_col = craft_line_color(line);
        if (craft_col != 0) color = craft_col;
        ctx.set_text_size(line_h);
        ctx.center_text_baseline();
        float left = -width / 2;
        float top = -height / 2 + line_h * 0.5f;

        if (line.channel == ChatChannel::kChatSystem || line.sender.empty()
            || prefix_w <= 0.f) {
            ctx.set_fill(color);
            for (size_t i = 0; i < wrap_lines.size(); ++i) {
                RenderContext c(&ctx);
                float tw = ctx.get_text_size(wrap_lines[i].c_str());
                ctx.translate(left + tw * 0.5f, top + (float) i * (line_h + 2.f));
                ctx.fill_text(wrap_lines[i].c_str());
            }
        } else {
            ChatLineParts p = chat_line_parts(line);
            float tag_w = ctx.get_text_size(p.tag.c_str());
            float lead_w = ctx.get_text_size(p.lead.c_str());
            float name_w = ctx.get_text_size(p.name.c_str());
            float tail_w = ctx.get_text_size(p.tail.c_str());
            float pos = left;
            float y0 = top;
            if (!p.tag.empty()) {
                RenderContext c(&ctx);
                ctx.set_fill(craft_col != 0 ? craft_col : kWhisperAccent);
                ctx.translate(pos + tag_w / 2, y0);
                ctx.fill_text(p.tag.c_str());
            }
            pos += tag_w;
            if (!p.lead.empty()) {
                RenderContext c(&ctx);
                ctx.set_fill(0xffffffff);
                ctx.translate(pos + lead_w / 2, y0);
                ctx.fill_text(p.lead.c_str());
            }
            pos += lead_w;
            {
                RenderContext c(&ctx);
                ctx.set_fill(line.channel == ChatChannel::kChatWhisper ? 0xffffffff : color);
                ctx.translate(pos + name_w / 2, y0);
                ctx.fill_text(p.name.c_str());
            }
            pos += name_w;
            if (!p.tail.empty()) {
                RenderContext c(&ctx);
                ctx.set_fill(craft_col != 0 ? craft_col : 0xffffffff);
                ctx.translate(pos + tail_w / 2, y0);
                ctx.fill_text(p.tail.c_str());
            }
            pos += tail_w;
            for (size_t i = 0; i < wrap_lines.size(); ++i) {
                RenderContext c(&ctx);
                float x = (i == 0) ? pos : left;
                float y = top + (float) i * (line_h + 2.f);
                float tw = ctx.get_text_size(wrap_lines[i].c_str());
                ctx.set_fill(craft_col != 0 ? craft_col : 0xffffffff);
                ctx.translate(x + tw * 0.5f, y);
                ctx.fill_text(wrap_lines[i].c_str());
            }
            if (Game::chat_open
                && BitMath::at(Input::mouse_buttons_pressed, Input::RightMouse)) {
                float mx = Input::mouse_x;
                float my = Input::mouse_y;
                float name_left = screen_x - width * Ui::scale / 2;
                float row_top = screen_y - height * Ui::scale / 2;
                float row_bot = row_top + line_h * Ui::scale;
                float sender_left = name_left + (tag_w + lead_w) * Ui::scale;
                float sender_right = sender_left + name_w * Ui::scale;
                if (my >= row_top && my <= row_bot) {
                    if (mx >= sender_left && mx <= sender_right)
                        Game::open_chat_menu(p.name, mx, my);
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

        // ScrollContainer width followed the longest line, so the thumb sat mid
        // panel. Stretch to the input-row width floor — content stays Left,
        // bar stays Right → thumb flush with the chat panel edge.
        float const want = chat_line_max_w() + (Input::is_mobile ? 36.f : 24.f);
        if (want > width) width = want;

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
        .h_justify = Style::Left,
        .no_animation = 1
    });

    uint8_t const mobile = Input::is_mobile;
    float const open_h = mobile ? 320.f : 260.f;
    auto *scroll = new ChatScroll(history, open_h);
    scroll->style.should_render = [](){
        if (!(Game::can_chat() && Game::should_render_game_ui())) return false;
        if (Game::chat_open) return true;
        for (uint32_t i = 0; i < Game::chat_lines.size(); ++i)
            if (line_visible_now(i)) return true;
        return false;
    };
    scroll->style.h_justify = Style::Left;

    float const input_w = mobile ? 180.f : 360.f;
    float const input_h = mobile ? 36.f : 32.f;
    chat_input = new ChatTextInput(Game::chat_draft, input_w, input_h, MAX_CHAT_LENGTH, {
        .line_width = 0,
        .round_radius = 10.f,
        .should_render = [](){
            return Game::can_chat() && Game::should_render_game_ui();
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
                return Game::can_chat() && Game::should_render_game_ui()
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
            .should_render = [](){ return Game::can_chat(); }
        }),
        chat_input,
        send_btn
    }, 0, 6, { .h_justify = Style::Left });

    Element *tab_hint = new Ui::DynamicText(12, [](){
        return Game::chat_completion_hint;
    }, {
        .fill = 0xffc8b0ff,
        .should_render = [](){
            return Game::chat_open && !Game::chat_completion_hint.empty();
        },
        .h_justify = Style::Left
    });

    Element *elt = new Ui::VContainer({ scroll, input_row, tab_hint }, 10, 4, {
        .fill = 0x990F2038,
        .round_radius = 12.f,
        .animate = [](Element *elt, Renderer &) {
            // Cool translucent chat overlay; whisper mode gets a violet tint.
            uint8_t whisper = Game::chat_channel == ChatChannel::kChatWhisper;
            if (Game::chat_open)
                elt->style.fill = whisper ? 0xaa24203F : 0x990F2038;
            else {
                uint8_t any = 0;
                for (uint32_t i = 0; i < Game::chat_lines.size(); ++i) {
                    if (line_visible_now(i)) { any = 1; break; }
                }
                elt->style.fill = any
                    ? (whisper ? 0x8824203F : 0x770F2038) : 0x00000000;
            }
        },
        .should_render = [](){
            return Game::should_render_game_ui() && Game::can_chat();
        },
        .h_justify = Style::Left,
        .v_justify = Style::Bottom
    });
    elt->x = mobile ? 52.f : 72.f;
    elt->y = -16;
    Ui::chat_panel_root = elt;
    return elt;
}

class ChatContextMenu final : public Ui::VContainer {
public:
    ChatContextMenu(std::vector<Element *> const &children, float opad, float ipad, Style s)
        : Ui::VContainer(children, opad, ipad, s) {}
    void refactor() override {
        Ui::VContainer::refactor();
        // Left/Top window justify: x/y are UI-space offsets from screen top-left.
        // Clamp so the whole menu stays on-screen (RMB near bottom was clipped).
        float sx = (float) (Ui::window_width / Ui::scale);
        float sy = (float) (Ui::window_height / Ui::scale);
        float mx = Game::chat_menu_x / (float) Ui::scale;
        float my = Game::chat_menu_y / (float) Ui::scale;
        float const margin = 8.f;
        if (mx + width > sx - margin) mx = sx - margin - width;
        if (my + height > sy - margin) my = sy - margin - height;
        if (mx < margin) mx = margin;
        if (my < margin) my = margin;
        x = mx;
        y = my;
    }
};

Element *Ui::make_chat_context_menu() {
    Element *elt = new ChatContextMenu({
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
        .should_render = [](){
            return Game::chat_menu_open && Game::can_chat() && Game::should_render_game_ui();
        },
        .h_justify = Style::Left,
        .v_justify = Style::Top,
        .layer = 1
    });
    return elt;
}
