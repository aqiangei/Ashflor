#pragma once

#include <Client/Render/Renderer.hh>
#include <Client/Socket.hh>
#include <Client/Ui/Ui.hh>
#include <Client/StaticData.hh>

#include <Shared/Binary.hh>
#include <Shared/Simulation.hh>
#include <Shared/StaticDefinitions.hh>
#include <Shared/LoadoutPreset.hh>

#include <array>
#include <string>
#include <unordered_map>
#include <vector>

namespace Game {
    struct ChatLine {
        uint8_t channel = ChatChannel::kChatNearby;
        std::string sender;
        std::string text;
        double expires_at = 0;
    };

    struct SpeechBubble {
        std::string text;
        double expires_at = 0;
    };

    constexpr uint32_t CHAT_HISTORY = 8;

    // One row of the leaderboard board (name + value in the row's category)
    struct LeaderboardStatRow {
        std::string name;
        uint64_t value = 0;
    };

    // Multi-category leaderboard board: one column per LeaderboardCategory
    struct LeaderboardBoard {
        uint32_t recorded = 0;
        std::array<std::array<LeaderboardStatRow, LEADERBOARD_STAT_SIZE>, kLbCategoryCount> tops{};
        std::array<uint8_t, kLbCategoryCount> has_self{};
        std::array<std::string, kLbCategoryCount> self_name{};
        std::array<uint32_t, kLbCategoryCount> self_rank{};
        std::array<uint64_t, kLbCategoryCount> self_value{};
    };

    extern Simulation simulation;
    extern Renderer renderer;
    extern Renderer game_ui_renderer;
    extern Socket socket;
    extern Ui::Window title_ui_window;
    extern Ui::Window game_ui_window;
    extern Ui::Window other_ui_window;
    extern EntityID camera_id;
    extern EntityID player_id;
    extern std::string nickname; // in-game display; guests always use guest_id
    // Fixed guest id "G#XXXXXX" (localStorage). Guests cannot pick custom names.
    extern std::string guest_id;
    void ensure_guest_id();
    std::string const &guest_play_name();
    extern std::string username;
    extern std::string password;
    extern std::string account_status;
    extern uint64_t session_token; // browser-local; auto-resume after reconnect
    extern uint8_t pending_auto_login;
    extern uint8_t spawn_after_auth;
    extern uint8_t account_logged_in;
    extern uint8_t auth_panel; // 0 closed, 1 login, 2 register
    extern std::string disconnect_message;
    extern std::array<uint8_t, PetalID::kNumPetals> seen_petals;
    extern std::array<uint8_t, MobID::kNumMobs> seen_mobs;

    // This-life summary (filled by kLifeSummary on death; shown top-right).
    struct LifeEntry {
        uint16_t id = 0;
        uint16_t count = 0;
    };
    extern std::vector<LifeEntry> life_mob_kills;
    extern std::vector<LifeEntry> life_petal_picks;
    extern uint8_t life_summary_ready;
    
    extern double timestamp;

    // Leaderboard board state (filled by the server, drawn by the leaderboard panel)
    extern LeaderboardBoard leaderboard_board;
    extern double leaderboard_board_at;
    void request_leaderboard_board();
    
    extern double score;
    extern float overlevel_timer;
    extern float slot_indicator_opacity;
    extern float transition_circle;
    // Spawn watchdog: timestamp of the last spawn request + retry count.
    // If the server never puts us back in the game we re-send the request.
    extern double spawn_request_at;
    extern uint8_t spawn_retries;

    extern uint32_t respawn_level;

    extern std::array<PetalID::T, 2 * MAX_SLOT_COUNT> cached_loadout;

    extern uint8_t loadout_count;
    extern uint8_t simulation_ready;
    extern uint8_t on_game_screen;
    extern uint8_t leaving_to_title; // soft-leave in progress — don't force game UI / auto-respawn
    extern uint8_t show_debug;
    extern uint8_t selected_map; // MapID for title-screen world select
    extern uint8_t minimap_expanded; // 0 compact full-map, 1 larger full-map (toggle M)
    extern float camera_zoom; // client wheel zoom multiplier (1 = default)
    extern uint8_t pause_menu_open;
    extern uint8_t leave_confirm_open;
    extern uint8_t game_panel; // 0 none, see kPanel*
    enum : uint8_t {
        kPanelNone = 0,
        kPanelInventory = 1, // B
        kPanelTalents = 2,   // Z
        kPanelCraft = 3,     // C
        kPanelSpecial = 4,   // V
        kPanelPresets = 5,   // reserved; switch via K/L+digit, no side button
        kPanelBoost = 6      // daily / offline drop bonus
    };
    extern uint8_t chat_open;
    extern uint8_t chat_channel;
    extern std::string chat_whisper_to;
    extern std::string chat_draft;
    extern std::vector<ChatLine> chat_lines;
    extern std::unordered_map<std::string, SpeechBubble> speech_bubbles;
    extern uint8_t chat_menu_open;
    extern std::string chat_menu_name;
    extern float chat_menu_x;
    extern float chat_menu_y;
    struct PartyMember {
        std::string name;
        uint8_t flags = 0; // 1=leader 2=down 4=self
        uint8_t hp_pct = 100;
        float x = -1.f;
        float y = -1.f;
    };
    extern std::string party_status;
    extern std::vector<PartyMember> party_members;
    extern uint8_t party_member_count;
    extern uint8_t revive_progress;
    extern uint8_t awaiting_party_revive;
    // Pending invite toast (top-left); type = InviteType
    extern uint8_t invite_open;
    extern uint8_t invite_type;
    extern std::string invite_from;
    extern std::string guild_name;
    extern std::string guild_status; // member list summary
    extern uint8_t guild_member_count;
    extern uint8_t guild_panel_open;
    extern std::string guild_create_draft;
    extern std::array<uint32_t, PetalID::kNumPetals> craft_bank;
    extern uint32_t coins;
    extern uint8_t craft_selected_rarity; // shop / specials
    extern PetalID::T craft_tray_id;
    extern uint32_t craft_tray_count;
    extern std::string craft_status;
    extern uint32_t daily_bonus_seconds;
    extern uint32_t offline_bonus_seconds;
    extern uint8_t bonus_enabled; // 1 = burn timers + triple drops
    extern double bonus_synced_at;
    void set_bonus_enabled(uint8_t on);

    // Guest local progress (browser localStorage)
    extern uint8_t guest_save_valid;
    extern uint32_t guest_level;
    extern std::array<PetalID::T, 2 * MAX_SLOT_COUNT> guest_petals;
    extern std::array<uint32_t, PetalID::kNumPetals> guest_craft_bank;
    extern uint32_t guest_coins;
    extern uint8_t guest_talent_hp;
    extern uint8_t guest_talent_rot;
    extern uint8_t talent_hp;
    extern uint8_t talent_rot;
    extern uint8_t talent_pending;
    extern uint8_t talent_pending_id;
    extern uint8_t talent_pending_rank;
    extern std::string talent_status;
    extern std::array<LoadoutPreset, LOADOUT_PRESET_COUNT> loadout_presets;
    extern uint8_t active_loadout_preset; // 0..19, 20 = no preset applied
    extern std::string loadout_preset_status;
    // Mobile: sticky after long-press R — tap primary slot i → apply L(i+1)
    extern uint8_t mobile_l_preset_pick;
    extern uint8_t rot_speed_setting; // quantized 0..10 (network)
    extern float rot_speed_f;         // smooth 0..10 (hold Q/E / touch)
    extern float rot_hint_opacity;    // world/UI hint ring while adjusting
    extern double rot_hint_stamp;     // timestamp of last adjust

    // Apply continuous spin adjust (delta levels); refreshes hint ring
    void nudge_rot_speed(float delta);
    void set_rot_speed_from_frac(float frac01);
    void save_loadout_preset(uint8_t index);
    void apply_loadout_preset(uint8_t index);
    // Equip one copy from backpack stacks onto the loadout bar.
    void withdraw_bank_petal(PetalID::T id);
    // Unequip primary/secondary into backpack stacks.
    void deposit_bank_petal(uint8_t static_pos);
    // Next non-empty cloud preset after the active one (wraps). Returns 1 if sent.
    uint8_t cycle_next_loadout_preset();
    // Mobile L-pick: apply L bank preset for primary slot index; clears sticky. 1 if applied.
    uint8_t try_mobile_l_preset_slot(uint8_t primary_index);

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
    void chat_menu_invite_guild();
    void chat_menu_whisper();
    void invite_respond(uint8_t accept);
    // Hide toast without Accept/Reject (server invite still expires on its own)
    void invite_dismiss();
    void toggle_guild_panel();
    void close_guild_panel();
    void guild_create();
    void guild_invite_name(std::string const &);
    void guild_leave();
    void toggle_pause();
    void toggle_game_panel(uint8_t panel);
    void close_game_panel();
    uint8_t game_panel_blocking();
    void return_to_title();
    void party_invite_nearest();
    void party_invite_name(std::string const &);
    void party_leave();
    void spawn_in();
    // Send the raw spawn request (also used by the spawn watchdog)
    void send_spawn_request();
    void account_auth(uint8_t is_register);
    void account_resume();
    void account_logout();
    void talent_spend(uint8_t talent_id);
    void delete_petal(uint8_t);
    void swap_petals(uint8_t, uint8_t);
    void swap_all_petals();
    void craft_petals(uint8_t mode, PetalID::T petal_id);
    void craft_buy_rarity(uint8_t rarity);
    void craft_buy_special(PetalID::T id);
    void craft_recipe(uint16_t recipe_index);
    // Secondary + pure bag slot copies (not primary)
    uint32_t craft_bag_count(PetalID::T id);
    // Backpack total: slot copies + stacks
    uint32_t craft_owned(PetalID::T id);
    uint32_t craft_available(PetalID::T id);
    void craft_tray_add(PetalID::T id, uint32_t amount);
    void craft_tray_clear();
    void on_message(uint8_t *, uint32_t);
    void poll_ui_event(Ui::ScreenEvent const &);
};
