#pragma once

#include <Shared/Binary.hh>
#include <Shared/EntityDef.hh>
#include <Shared/StaticDefinitions.hh>

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
    uint8_t talent_hp = 0;
    uint8_t talent_rot = 0;
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