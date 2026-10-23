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
    // Tick telemetry for /health and /api/live (updated every Server::tick)
    extern double last_tick_ms;
    extern double max_tick_ms_window;
    extern uint32_t tick_overruns_window;
    extern uint32_t tick_count_window;
    extern void init();
    extern void run();
    extern void tick();
    extern void stop();
    extern GameInstance *game_for(Simulation *);
    extern Client *client_by_camera(Simulation *, EntityID const &);
    // JSON snapshot for /api/live (online roster + per-map counts)
    extern std::string live_status_json();
    // Compact liveness JSON for load balancers / ops probes
    extern std::string health_json();
    // Slim positions-only JSON for smooth minimap polling (~TPS)
    extern std::string live_positions_json();
    // Static per-map terrain/zones for live.html minimaps (cached)
    extern std::string live_map_layout_json();
    // Petal / mob id→name lists for the live admin UI
    extern std::string live_catalog_json();
    // Live admin actions — return compact JSON {ok,msg,...}
    extern std::string live_admin_announce(std::string const &text);
    // rarity: 0..kNumRarities-1 resolve same-name ladder; 255 = use petal_id as-is
    extern std::string live_admin_give(std::string const &who, uint32_t petal_id, uint32_t count,
                                      uint32_t rarity);
    extern std::string live_admin_spawn(uint8_t map_id, uint32_t mob_id, float x, float y,
                                       std::string const &near_who);
};
