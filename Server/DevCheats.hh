#pragma once

#include <cstdint>

class Client;
class Simulation;
class Entity;

// Local QA cheats (godmode + infinite petal bank). Off unless enabled
// via ASHFLOR_DEV_CHEATS=1 (or ashflor_dev_cheats_set). Only account "dev".
namespace DevCheats {
    void set_enabled(uint8_t on);
    uint8_t enabled();
    uint8_t active(Client const *client);
    // After successful login/register/resume — fill bank, coins, level, talents.
    void apply_kit(Client *client);
    // Per-tick: keep flower unkillable + top up craft bank.
    void tick_client(Client *client);
}
