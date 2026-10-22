#pragma once

#include <Shared/EntityDef.hh>

#include <cstdint>
#include <string>

class Client;
class GameInstance;
class Simulation;

namespace Party {
    constexpr uint32_t MAX_SIZE = 4;
    // Party members' levels must stay within this span (max - min).
    constexpr uint32_t MAX_LEVEL_GAP = 50;
    // At/above this level the gap rule is waived (high-level free party).
    constexpr uint32_t LEVEL_GAP_FREE_FROM = 100;
    constexpr float REVIVE_RADIUS = 150.0f;
    constexpr float INVITE_RADIUS = 500.0f;
    // Nearby auto-squad (florr-style): join within this, leave beyond leave radius.
    constexpr float AUTO_JOIN_RADIUS = 1000.0f;
    constexpr float AUTO_LEAVE_RADIUS = 1700.0f; // hysteresis so edge flicker is rare
    constexpr uint32_t AUTO_PARTY_COOLDOWN = 20 * 5; // 5s after range-leave before rejoin
    // TPS is 20
    constexpr uint32_t REVIVE_TICKS = 60;
    constexpr uint32_t CORPSE_TTL = 60 * 20;

    constexpr uint32_t INVITE_TTL_TICKS = 20 * 30; // 30s

    void leave(Client *);
    // empty name = nearest alive player — sends pending invite (accept/reject)
    uint8_t invite(Client *from, std::string const &target_name);
    uint8_t respond(Client *target, std::string const &from_name, uint8_t accept);
    void on_death(Client *, float x, float y);
    void clear_corpse(Client *);
    void tick(GameInstance *);
    void apply_team(Client *);
    void send_update(Client *);
    void notify(Client *, std::string const &);
    uint8_t same_party(Client *, Client *);
    // Drop claim window: same team, or same party (team desync fallback).
    uint8_t can_take_drop(Client *picker, Simulation *, EntityID drop_team);
};
