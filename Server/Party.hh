#pragma once

#include <cstdint>
#include <string>

class Client;
class GameInstance;

namespace Party {
    constexpr uint32_t MAX_SIZE = 4;
    constexpr float REVIVE_RADIUS = 150.0f;
    constexpr float INVITE_RADIUS = 500.0f;
    // TPS is 20
    constexpr uint32_t REVIVE_TICKS = 60;
    constexpr uint32_t CORPSE_TTL = 60 * 20;

    void leave(Client *);
    // empty name = nearest alive player
    uint8_t invite(Client *from, std::string const &target_name);
    void on_death(Client *, float x, float y);
    void clear_corpse(Client *);
    void tick(GameInstance *);
    void apply_team(Client *);
    void send_update(Client *);
    void notify(Client *, std::string const &);
    uint8_t same_party(Client *, Client *);
};
