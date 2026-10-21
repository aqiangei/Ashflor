#pragma once

#include <Server/Game.hh>

#include <Shared/StaticDefinitions.hh>

#include <array>
#include <csignal>
#include <set>
#include <string>

class Client;

size_t const MAX_PACKET_LEN = 64 * 1024;

#ifdef WASM_SERVER
class WebSocketServer {
public:
    WebSocketServer();
};
#else
#include <App.h>
typedef uWS::App WebSocketServer;
#endif

namespace Server {
    extern uint8_t OUTGOING_PACKET[MAX_PACKET_LEN];
    // Heap-allocated in Server::init (static std::array OOBs WASM stack/BSS at 8 maps)
    extern GameInstance *games;
    extern WebSocketServer server;
    // Zero-downtime deploy: SIGUSR2 → drain → stop when empty
    extern volatile sig_atomic_t is_draining;
    extern bool is_stopping;
    extern uint32_t player_count;
    extern void init();
    extern void run();
    extern void tick();
    extern void stop();
    extern GameInstance *game_for(Simulation *);
    extern Client *client_by_camera(Simulation *, EntityID const &);
    // JSON snapshot for /api/live (online roster + per-map counts)
    extern std::string live_status_json();
    // Petal / mob id→name lists for the live admin UI
    extern std::string live_catalog_json();
    // Live admin actions — return compact JSON {ok,msg,...}
    extern std::string live_admin_announce(std::string const &text);
    extern std::string live_admin_give(std::string const &who, uint32_t petal_id, uint32_t count);
    extern std::string live_admin_spawn(uint8_t map_id, uint32_t mob_id, float x, float y,
                                       std::string const &near_who);
};
