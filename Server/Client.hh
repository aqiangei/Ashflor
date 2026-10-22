#pragma once

#include <Shared/Binary.hh>
#include <Shared/EntityDef.hh>
#include <Shared/StaticDefinitions.hh>
#include <Shared/LoadoutPreset.hh>
#include <Shared/Talent.hh>

#include <array>
#include <cstdint>
#include <set>
#include <string>

#ifdef WASM_SERVER
class WebSocket;
#else
#include <App.h>
class Client;
typedef uWS::WebSocket<false, true, Client> WebSocket;
#endif

class GameInstance;

class Client {
public:
    GameInstance *game;
    EntityID camera;
    std::set<EntityID> in_view;
    WebSocket *ws;
    uint8_t verified = 0;
    uint8_t seen_arena = 0;
    std::string account_name;
    // Display name claimed on last spawn (empty = unnamed); used for unique-name checks
    std::string play_name;
    std::array<uint32_t, PetalID::kNumPetals> craft_bank = {};
    uint32_t coins = 0;
    uint32_t craft_attempts = 0;
    // Infinite petal levels (top-rung families only); 0 = base stats.
    std::array<uint32_t, PetalID::kNumPetals> petal_levels = {};
    // Leaderboard board counters, flushed into LeaderboardStore periodically
    uint32_t stat_playtime = 0; // seconds online
    uint32_t stat_messages = 0; // chat messages sent
    uint32_t stat_kills = 0;    // boss MVP kills (most damage; whole party credited)
    uint32_t stat_crafts = 0;   // Unique+ craft successes (matches 全服合成播报)
    // This-life tallies — reset on spawn, sent on death for the summary panels.
    std::array<uint16_t, MobID::kNumMobs> life_mob_kills = {};
    std::array<uint16_t, PetalID::kNumPetals> life_petal_picks = {};
    uint8_t talent_ranks[TalentID::kNum] = {};
    std::array<LoadoutPreset, LOADOUT_PRESET_COUNT> loadout_presets = {};
    // 0..19 active cloud preset; LOADOUT_PRESET_COUNT = none yet
    uint8_t active_loadout_preset = LOADOUT_PRESET_COUNT;
    uint32_t bonus_day = 0;
    uint32_t daily_bonus_seconds = 0;
    uint32_t offline_bonus_seconds = 0;
    uint8_t bonus_enabled = 0; // off by default; player must toggle on
    uint64_t last_bonus_tick = 0;
    uint64_t last_bonus_sync = 0;
    uint8_t rot_speed_setting = Talent::kRotSettingDefault; // Q/E wheel 0..10
    uint64_t last_chat_ms = 0;
    uint64_t last_server_chat_ms = 0; // 全服频道单独冷却
    std::string last_whisper_from; // for /r /reply
    // Per-map ritual claim bits (bit ritual_idx) — session only
    uint8_t treasure_ritual_claimed[8] = {};
    // Per-map chest claim bits (bit chest_idx) — session only
    uint8_t treasure_chest_claimed[8] = {};
    uint32_t treasure_hint_cd = 0; // ticks until next NPC hint
    // WorldExplore: per-map follow + 3-touch (session). Step is for current map only.
    uint8_t explore_follow_done[8] = {};
    uint8_t explore_puzzle_done[8] = {};
    uint8_t explore_puzzle_step = 0;
    uint32_t explore_follow_cd = 0;
    EntityID explore_follower = NULL_ENTITY;
    // WorldEvent: per-map claim bits (slot 0..15) — session
    uint16_t world_event_claimed[8] = {};
    // Timed-touch attempt (current map only)
    uint8_t timed_touch_mask = 0;
    uint8_t timed_touch_event = 255; // index into map event list
    uint32_t timed_touch_ticks = 0;
    // Height-order mushrooms (G06)
    uint8_t height_order_step = 0;
    uint8_t height_order_event = 255;
    uint32_t party_id = 0;
    uint8_t party_leader = 0;
    std::string pending_party_from;
    uint32_t pending_party_ttl = 0;
    uint32_t auto_party_cd = 0; // ticks until nearby auto-join can fire again
    std::string guild_name;
    uint8_t guild_leader = 0;
    std::string pending_guild_from;
    uint32_t pending_guild_ttl = 0;
    uint8_t awaiting_revive = 0;
    float corpse_x = 0;
    float corpse_y = 0;
    uint32_t corpse_ttl = 0;
    uint32_t revive_progress = 0;
    EntityID corpse_id = NULL_ENTITY;
    // Solo/party: death site for Continue respawn (kept after corpse TTL expires)
    uint8_t has_death_pos = 0;
    float death_x = 0;
    float death_y = 0;
    uint8_t death_map_id = 0;
    // Full score carried across death/map transfer (not just level floor).
    // Spawn restores this so mid-level XP is not wiped.
    uint32_t pending_score = 0;
    // Rate-limit / backpressure (Native + Wasm)
    std::string remote_ip;
    uint8_t conn_counted = 0;
    uint64_t last_rx_ms = 0;
    Client();
    void init();
    void remove();
    void disconnect(int = CloseReason::kProtocol, std::string const & = "Protocol Error");
    uint8_t alive();
    uint8_t can_accept_update();

    void send_packet(uint8_t const *, size_t);
    //takes in a bool expr
    //if true, packet reading should be terminated
    //optionally, the client can also be disconnected
    bool check_invalid(bool);
    static void on_message(WebSocket *, std::string_view, uint64_t);
    static void on_disconnect(WebSocket *, int, std::string_view);
};

// Award mob-kill coins and sync craft/coin bank to the client
void client_award_coins(Client *, uint32_t amount);
// Put one petal in bag/bank and sync (treasure / shop / admin)
uint8_t client_award_petal(Client *, PetalID::T id);
void client_send_loot_toast(Client *, PetalID::T id, uint16_t amount, std::string const &title);
void client_sync_craft(Client *);
void client_tick_bonus(Client *);
// Move the session counters of a client into the leaderboard store
void client_flush_stats(Client *);
void client_clear_life_stats(Client *);
void client_credit_mob_kill(Client *, MobID::T);
void client_credit_petal_pick(Client *, PetalID::T);
void client_send_life_summary(Client *);

#ifdef WASM_SERVER
class WebSocket {
    int ws_id;
public:
    Client client;
    WebSocket(int);
    int id() const { return ws_id; }
    Client *getUserData();
    void send(uint8_t const *, size_t);
    void end(int, std::string const &);
};
#endif
