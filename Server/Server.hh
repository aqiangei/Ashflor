#pragma once

#include <Server/Game.hh>

#include <Shared/StaticDefinitions.hh>

#include <array>
#include <csignal>
#include <set>

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
};
