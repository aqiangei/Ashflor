#include <Server/Server.hh>

#include <Server/AccountStore.hh>
#include <Server/Client.hh>
#include <Server/Game.hh>
#include <Server/Guild.hh>
#include <Server/LeaderboardStore.hh>
#include <Server/RateLimit.hh>

#include <Shared/Binary.hh>
#include <Shared/Entity.hh>
#include <Shared/StaticData.hh>
#include <Shared/StaticDefinitions.hh>

#include <chrono>
#include <iostream>
#include <new>
#include <string>
#include <vector>

static bool was_draining = false;

namespace Server {
    uint8_t OUTGOING_PACKET[MAX_PACKET_LEN] = {0};
    GameInstance *games = nullptr;
    volatile sig_atomic_t is_draining = 0;
    bool is_stopping = false;
    uint32_t player_count = 0;
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
    static uint32_t tick_n = 0;
    ++tick_n;
    if ((tick_n % 75) == 0) // ~3s at 25 TPS
        RateLimit::tick_prune();
    uint32_t online = 0;
    for (uint8_t i = 0; i < MapID::kNumMaps; ++i) {
        // Empty maps (no WS clients): tick at 1/6 rate. 8 maps at full rate
        // still push tick >40ms and make move/attack feel delayed.
        if (Server::games[i].clients.empty() && ((tick_n + i) % 6) != 0) {
            continue;
        }
        Server::games[i].tick();
        for (Client *c : Server::games[i].clients)
            client_tick_bonus(c);
        online += (uint32_t) Server::games[i].clients.size();
    }
    Server::player_count = online;
    // Online roster under minimap (~2s): name, map, guild, level
    if ((tick_n % 50) == 0 && online > 0) {
        struct Row {
            std::string name;
            uint8_t map_id;
            std::string guild;
            uint32_t level;
        };
        std::vector<Row> rows;
        rows.reserve(online);
        for (uint8_t mi = 0; mi < MapID::kNumMaps; ++mi) {
            for (Client *c : Server::games[mi].clients) {
                if (c == nullptr || c->play_name.empty()) continue;
                uint32_t level = 1;
                if (c->game != nullptr && c->game->simulation.ent_exists(c->camera)) {
                    Entity &cam = c->game->simulation.get_ent(c->camera);
                    if (c->game->simulation.ent_alive(cam.get_player()))
                        level = score_to_level(c->game->simulation.get_ent(cam.get_player()).get_score());
                    else
                        level = cam.get_respawn_level();
                }
                if (level < 1) level = 1;
                rows.push_back({ c->play_name, mi, c->guild_name, level });
            }
        }
        if (rows.size() > 64) rows.resize(64);
        Writer writer(Server::OUTGOING_PACKET);
        writer.write<uint8_t>(Clientbound::kOnlineRoster);
        writer.write<uint8_t>((uint8_t) rows.size());
        for (Row const &r : rows) {
            writer.write<std::string>(r.name);
            writer.write<uint8_t>(r.map_id);
            writer.write<std::string>(r.guild);
            writer.write<uint32_t>(r.level);
        }
        size_t plen = (size_t) (writer.at - writer.packet);
        for (uint8_t mi = 0; mi < MapID::kNumMaps; ++mi) {
            for (Client *c : Server::games[mi].clients) {
                if (c == nullptr) continue;
                c->send_packet(writer.packet, plen);
            }
        }
    }
    static uint32_t lb_flush_ticks = 0;
    if (++lb_flush_ticks >= 25) { // ~1s at 25 TPS
        lb_flush_ticks = 0;
        LeaderboardStore::flush();
        // Play time counter: one second per connected, spawned player
        for (uint8_t i = 0; i < MapID::kNumMaps; ++i) {
            for (Client *c : Server::games[i].clients) {
                if (c == nullptr || c->play_name.empty()) continue;
                if (c->stat_playtime < 4000000000u)
                    ++c->stat_playtime;
            }
        }
    }
    // Push the session stat counters into the leaderboard store (~5s)
    static uint32_t stat_flush_ticks = 0;
    if (++stat_flush_ticks >= 125) {
        stat_flush_ticks = 0;
        uint8_t any = 0;
        for (uint8_t i = 0; i < MapID::kNumMaps; ++i) {
            for (Client *c : Server::games[i].clients) {
                if (c == nullptr) continue;
                client_flush_stats(c);
                any = 1;
            }
        }
        if (any) LeaderboardStore::flush();
    }
    // Periodic full account snapshot for all logged-in clients (~5s)
    static uint32_t acc_flush_ticks = 0;
    if (++acc_flush_ticks >= 125) {
        acc_flush_ticks = 0;
        AccountStore::prune_stale_sessions();
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
    // 20 TPS budget ≈ 50ms; log only when we actually miss the frame
    if (tick_time > 40ms) std::cout << "tick took " << tick_time << '\n';

    if (Server::is_draining && !was_draining) {
        was_draining = true;
        std::cout << "draining...\n";
        AccountStore::save();
        Guild::save();
        LeaderboardStore::flush();
    }
    if (Server::is_draining && !Server::is_stopping && Server::player_count == 0)
        Server::stop();
}

void Server::init() {
    AccountStore::load();
    Guild::load();
    LeaderboardStore::load();
    // Seed the ranked "level" column from the saved accounts
    AccountStore::seed_leaderboard_levels();
    LeaderboardStore::flush();
    if (Server::games == nullptr)
        Server::games = new GameInstance[MapID::kNumMaps];
    for (uint8_t i = 0; i < MapID::kNumMaps; ++i)
        Server::games[i].init(i);
    Server::run();
}
