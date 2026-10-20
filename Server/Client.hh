#pragma once

#include <Shared/Binary.hh>
#include <Shared/EntityDef.hh>
#include <Shared/StaticDefinitions.hh>
#include <Shared/LoadoutPreset.hh>

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
    // Leaderboard board counters, flushed into LeaderboardStore periodically
    uint32_t stat_playtime = 0; // seconds online
    uint32_t stat_messages = 0; // chat messages sent
    uint32_t stat_kills = 0;    // players killed
    uint32_t stat_crafts = 0;   // craft attempts
    // This-life tallies — reset on spawn, sent on death for the summary panels.
    std::array<uint16_t, MobID::kNumMobs> life_mob_kills = {};
    std::array<uint16_t, PetalID::kNumPetals> life_petal_picks = {};
    uint8_t talent_hp = 0;
    uint8_t talent_rot = 0;
    std::array<LoadoutPreset, LOADOUT_PRESET_COUNT> loadout_presets = {};
    // 0..19 active cloud preset; LOADOUT_PRESET_COUNT = none yet
    uint8_t active_loadout_preset = LOADOUT_PRESET_COUNT;
    uint32_t bonus_day = 0;
    uint32_t daily_bonus_seconds = 0;
    uint32_t offline_bonus_seconds = 0;
    uint8_t bonus_enabled = 1; // player can pause timers / triple drops anytime
    uint64_t last_bonus_tick = 0;
    uint64_t last_bonus_sync = 0;
    uint8_t rot_speed_setting = 7; // Q/E wheel 0..10
    uint64_t last_chat_ms = 0;
    uint32_t party_id = 0;
    uint8_t party_leader = 0;
    std::string pending_party_from;
    uint32_t pending_party_ttl = 0;
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
    Client();
    void init();
    void remove();
    void disconnect(int = CloseReason::kProtocol, std::string const & = "Protocol Error");
    uint8_t alive();

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
    Client *getUserData();
    void send(uint8_t const *, size_t);
    void end(int, std::string const &);
};
#endif
