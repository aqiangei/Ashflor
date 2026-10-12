#pragma once

#include <Client/Render/Renderer.hh>
#include <Client/Socket.hh>
#include <Client/Ui/Ui.hh>
#include <Client/StaticData.hh>

#include <Shared/Binary.hh>
#include <Shared/Simulation.hh>
#include <Shared/StaticDefinitions.hh>

#include <array>
#include <string>
#include <vector>

namespace Game {
    struct ChatLine {
        uint8_t channel = ChatChannel::kChatNearby;
        std::string sender;
        std::string text;
        double expires_at = 0;
    };

    constexpr uint32_t CHAT_HISTORY = 8;

    extern Simulation simulation;
    extern Renderer renderer;
    extern Renderer game_ui_renderer;
    extern Socket socket;
    extern Ui::Window title_ui_window;
    extern Ui::Window game_ui_window;
    extern Ui::Window other_ui_window;
    extern EntityID camera_id;
    extern EntityID player_id;
    extern std::string nickname;
    extern std::string username;
    extern std::string password;
    extern std::string account_status;
    extern uint8_t account_logged_in;
    extern uint8_t auth_panel; // 0 closed, 1 login, 2 register
    extern std::string disconnect_message;
    extern std::array<uint8_t, PetalID::kNumPetals> seen_petals;
    extern std::array<uint8_t, MobID::kNumMobs> seen_mobs;
    
    extern double timestamp;
    
    extern double score;
    extern float overlevel_timer;
    extern float slot_indicator_opacity;
    extern float transition_circle;

    extern uint32_t respawn_level;

    extern std::array<PetalID::T, 2 * MAX_SLOT_COUNT> cached_loadout;

    extern uint8_t loadout_count;
    extern uint8_t simulation_ready;
    extern uint8_t on_game_screen;
    extern uint8_t show_debug;
    extern uint8_t selected_map; // MapID for title-screen world select
    extern uint8_t minimap_expanded; // 0 compact full-map, 1 larger full-map (toggle M)
    extern uint8_t pause_menu_open;
    extern uint8_t leave_confirm_open;
    extern uint8_t game_panel; // 0 none, see kPanel*
    enum : uint8_t {
        kPanelNone = 0,
        kPanelInventory = 1, // B
        kPanelTalents = 2,   // Z
        kPanelCraft = 3,     // C
        kPanelSpecial = 4    // V
    };
    extern uint8_t chat_open;
    extern uint8_t chat_channel;
    extern std::string chat_whisper_to;
    extern std::string chat_draft;
    extern std::vector<ChatLine> chat_lines;
    extern uint8_t chat_menu_open;
    extern std::string chat_menu_name;
    extern float chat_menu_x;
    extern float chat_menu_y;
    extern std::string party_status;
    extern uint8_t party_member_count;
    extern uint8_t revive_progress;
    extern uint8_t awaiting_party_revive;
    extern std::array<uint32_t, PetalID::kNumPetals> craft_bank;
    extern uint32_t coins;
    extern uint8_t craft_selected_rarity; // shop / specials
    extern PetalID::T craft_tray_id;
    extern uint32_t craft_tray_count;
    extern std::string craft_status;

    // Guest local progress (browser localStorage)
    extern uint8_t guest_save_valid;
    extern uint8_t guest_level;
    extern std::array<PetalID::T, 2 * MAX_SLOT_COUNT> guest_petals;
    extern std::array<uint32_t, PetalID::kNumPetals> guest_craft_bank;
    extern uint32_t guest_coins;

    void init();
    void reset();
    void send_guest_progress();
    void sync_guest_progress();
    uint8_t alive();
    uint8_t in_game();
    uint8_t should_render_title_ui();
    uint8_t should_render_game_ui();
    void tick(double);
    void render_game();
    void render_title_screen();
    void send_inputs();
    void send_chat();
    void open_chat();
    void close_chat();
    void push_chat_line(uint8_t channel, std::string const &sender, std::string const &text);
    void cycle_chat_channel();
    void open_chat_menu(std::string const &name, float x, float y);
    void close_chat_menu();
    void chat_menu_invite();
    void chat_menu_whisper();
    void toggle_pause();
    void toggle_game_panel(uint8_t panel);
    void close_game_panel();
    uint8_t game_panel_blocking();
    void return_to_title();
    void party_invite_nearest();
    void party_invite_name(std::string const &);
    void party_leave();
    void spawn_in();
    void account_auth(uint8_t is_register);
    void account_logout();
    void delete_petal(uint8_t);
    void swap_petals(uint8_t, uint8_t);
    void swap_all_petals();
    void craft_petals(uint8_t mode, PetalID::T petal_id);
    void craft_buy_rarity(uint8_t rarity);
    void craft_buy_special(PetalID::T id);
    uint32_t craft_available(PetalID::T id);
    void craft_tray_add(PetalID::T id, uint32_t amount);
    void craft_tray_clear();
    void on_message(uint8_t *, uint32_t);
    void poll_ui_event(Ui::ScreenEvent const &);
};
