#include <Server/Server.hh>

#include <Server/AccountStore.hh>
#include <Server/Client.hh>
#include <Server/Game.hh>
#include <Server/LeaderboardStore.hh>

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
    static uint32_t lb_flush_ticks = 0;
    if (++lb_flush_ticks >= 25) { // ~1s at 25 TPS
        lb_flush_ticks = 0;
        LeaderboardStore::flush();
    }
    // Periodic full account snapshot for all logged-in clients (~5s)
    static uint32_t acc_flush_ticks = 0;
    if (++acc_flush_ticks >= 125) {
        acc_flush_ticks = 0;
        uint8_t any = 0;
        for (uint8_t i = 0; i < MapID::kNumMaps; ++i) {
            for (Client *c : Server::games[i].clients) {
                if (c == nullptr || c->account_name.empty()) continue;
                AccountStore::capture_nosave(c);
                any = 1;
            }
        }
        if (any) AccountStore::save();
    }
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> tick_time = end - start;
    if (tick_time > 5ms) std::cout << "tick took " << tick_time << '\n';
}

void Server::init() {
    AccountStore::load();
    LeaderboardStore::load();
    for (uint8_t i = 0; i < MapID::kNumMaps; ++i)
        Server::games[i].init(i);
    Server::run();
}
