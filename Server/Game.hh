#pragma once

#include <Server/TeamManager.hh>

#include <Shared/Simulation.hh>
#include <Shared/StaticDefinitions.hh>

#include <set>

class Client;

class GameInstance {
    TeamManager team_manager;
public:
    uint8_t map_id = MapID::kGarden;
    std::set<Client *> clients;
    Simulation simulation;
    GameInstance();
    void init(uint8_t map_id);
    void tick();
    void add_client(Client *);
    void remove_client(Client *);
    // Move client to another map without logging out the account session
    void transfer_client(Client *, GameInstance *dest);
    Client *client_by_camera(EntityID const &);
    void broadcast(uint8_t const *, size_t);
};
