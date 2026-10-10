#include <Server/Server.hh>

#include <Server/AccountStore.hh>
#include <Server/Game.hh>
#include <Server/Client.hh>

#include <Shared/Binary.hh>

#include <chrono>
#include <iostream>

namespace Server {
    uint8_t OUTGOING_PACKET[MAX_PACKET_LEN] = {0};
    std::array<GameInstance, MapID::kNumMaps> games;
}

using namespace Server;

GameInstance *Server::game_for(Simulation *sim) {
    if (sim == nullptr) return nullptr;
    for (uint8_t i = 0; i < MapID::kNumMaps; ++i) {
        if (&games[i].simulation == sim) return &games[i];
    }
    return nullptr;
}

Client *Server::client_by_camera(Simulation *sim, EntityID const &id) {
    GameInstance *game = game_for(sim);
    if (game == nullptr) return nullptr;
    return game->client_by_camera(id);
}

void Server::tick() {
    using namespace std::chrono_literals;
    auto start = std::chrono::steady_clock::now();
    for (uint8_t i = 0; i < MapID::kNumMaps; ++i)
        Server::games[i].tick();
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> tick_time = end - start;
    if (tick_time > 5ms) std::cout << "tick took " << tick_time << '\n';
}

void Server::init() {
    AccountStore::load();
    for (uint8_t i = 0; i < MapID::kNumMaps; ++i)
        Server::games[i].init(i);
    Server::run();
}
