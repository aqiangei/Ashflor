#pragma once

#include <Client/Ui/Element.hh>

namespace Ui {
    class LevelBar final : public Element {
    public:
        LerpFloat progress;
        uint32_t level;
        LevelBar();
        virtual void on_render(Renderer &) override;
    };

    class HealthBar final : public Element {
    public:
        LerpFloat ratio;
        LerpFloat lag;
        HealthBar();
        virtual void on_render(Renderer &) override;
    };

    class LeaderboardSlot final : public Element {
    public:
        uint8_t pos;
        LerpFloat ratio;
        LeaderboardSlot(uint8_t);

        virtual void on_render(Renderer &) override;
    };

    class Minimap final : public Element {
    public:
        Minimap(float);
        virtual void on_render(Renderer &) override;
        virtual void refactor() override;
    };

    class OverlevelTimer final : public Element {
    public:
        OverlevelTimer(float);
        virtual void on_render(Renderer &) override;
    };

    class MobileJoyStick final : public Element {
        float joystick_x;
        float joystick_y;
        float joystick_radius;
        uint32_t persistent_touch_id;
        uint8_t is_pressed;
    public:
        MobileJoyStick(float, float, float);
        virtual void on_render(Renderer &) override;
        virtual void on_event(uint8_t) override;
    };

    Element *make_leaderboard();
    Element *make_level_bar();
    Element *make_health_bar();
    Element *make_minimap();
    Element *make_overlevel_indicator();
    Element *make_mobile_attack_button();
    Element *make_mobile_defend_button();
    Element *make_mobile_joystick();
    Element *make_pause_menu();
    Element *make_game_topbar();
    Element *make_leave_confirm();
    Element *make_chat_panel();
    Element *make_chat_context_menu();
    Element *make_invite_toast();
    Element *make_guild_panel();
    Element *make_guild_top_button();
    Element *make_game_side_buttons();
    Element *make_inventory_panel();
    Element *make_talents_panel();
    Element *make_craft_panel();
    Element *make_special_craft_panel();
    class TextInput;
    TextInput *chat_text_input();
}