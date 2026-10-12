#pragma once

#include <Shared/StaticData.hh>

#include <array>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>

class Client;
class Simulation;
class Entity;

// Bound to a registered account and written to accounts.dat:
// level, 16 loadout/backpack petals, per-petal craft_bank, coins, craft_attempts.
struct Account {
    std::string username;
    uint64_t pass_hash = 0;
    uint8_t level = 1;
    std::array<PetalID::T, 2 * MAX_SLOT_COUNT> petals = {};
    std::array<uint32_t, PetalID::kNumPetals> craft_bank = {};
    uint32_t coins = 0;
    uint32_t craft_attempts = 0; // successful+failed 5-in-1 batches
};

namespace AccountStore {
    void load();
    // Atomic replace of accounts.dat (+ accounts.dat.bak + periodic backups/)
    void save();
    // Copy live client progress into the Account row (no disk write)
    void capture_nosave(Client *);
    // capture_nosave + save()
    void capture(Client *);
    uint32_t registered_count();
    uint64_t hash_password(std::string const &username, std::string const &password);
    uint8_t valid_username(std::string const &);
    uint8_t valid_password(std::string const &);
    Account *find(std::string const &username);
    uint8_t register_account(Client *, std::string const &username, std::string const &password);
    uint8_t login(Client *, std::string const &username, std::string const &password);
    void logout(Client *);
    void apply_to_camera(Simulation *, Entity &, Account const &);
};
