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

    // Teammate name + HP strip under self status (top-left)
    class PartyHud final : public Element {
    public:
        PartyHud();
        virtual void on_render(Renderer &) override;
        virtual void refactor() override;
    };

    class LeaderboardBoard final : public Element {
    public:
        LeaderboardBoard();

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
        float joystick_radius;
        float base_x;
        float base_y;
        float knob_x;
        float knob_y;
        uint32_t persistent_touch_id;
        uint8_t is_pressed;
    public:
        MobileJoyStick(float, float, float);
        virtual void on_render(Renderer &) override;
        virtual void on_event(uint8_t) override;
        virtual void poll_events(ScreenEvent const &) override;
    };

    Element *make_leaderboard();
    Element *make_level_bar();
    Element *make_health_bar();
    Element *make_boss_health_bar();
    Element *make_minimap();
    Element *make_overlevel_indicator();
    Element *make_mobile_attack_button();
    Element *make_mobile_defend_button();
    Element *make_mobile_swap_button(); // R: tap swap-all, long-press cycle preset
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
    Element *make_presets_panel();
    Element *make_talents_panel();
    Element *make_craft_panel();
    Element *make_special_craft_panel();
    Element *make_boost_panel();
    class TextInput;
    TextInput *chat_text_input();
}
