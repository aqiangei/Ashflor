#include <Server/Game.hh>

#include <Server/AccountStore.hh>
#include <Server/LeaderboardStore.hh>
#include <Server/Client.hh>
#include <Server/Guild.hh>
#include <Server/Party.hh>
#include <Server/PetalTracker.hh>
#include <Server/Server.hh>
#include <Server/Spawn.hh>

#include <Shared/Binary.hh>
#include <Shared/Entity.hh>
#include <Shared/Map.hh>

#include <array>

static void _update_client(Simulation *sim, Client *client) {
    if (client == nullptr) return;
    if (!client->verified) return;
    if (sim == nullptr) return;
    if (!sim->ent_exists(client->camera)) return;
    std::set<EntityID> in_view;
    std::vector<EntityID> deletes;
    in_view.insert(client->camera);
    Entity &camera = sim->get_ent(client->camera);
    if (sim->ent_exists(camera.get_player())) 
        in_view.insert(camera.get_player());
    Writer writer(Server::OUTGOING_PACKET);
    writer.write<uint8_t>(Clientbound::kClientUpdate);
    writer.write<EntityID>(client->camera);
    sim->spatial_hash.query(camera.get_camera_x(), camera.get_camera_y(), 
    960 / camera.get_fov() + 50, 540 / camera.get_fov() + 50, 
    [&](Simulation *, Entity &ent){
        in_view.insert(ent.id);
    });

    for (EntityID const &i: client->in_view) {
        if (!in_view.contains(i)) {
            writer.write<EntityID>(i);
            deletes.push_back(i);
        }
    }

    for (EntityID const &i : deletes)
        client->in_view.erase(i);

    writer.write<EntityID>(NULL_ENTITY);
    //upcreates
    for (EntityID id: in_view) {
        DEBUG_ONLY(assert(sim->ent_exists(id));)
        Entity &ent = sim->get_ent(id);
        uint8_t create = !client->in_view.contains(id);
        writer.write<EntityID>(id);
        writer.write<uint8_t>(create | (ent.pending_delete << 1));
        ent.write(&writer, BitMath::at(create, 0));
        client->in_view.insert(id);
    }
    writer.write<EntityID>(NULL_ENTITY);
    //write arena stuff
    writer.write<uint8_t>(!client->seen_arena);
    sim->arena_info.write(&writer, !client->seen_arena);
    client->seen_arena = 1;
    client->send_packet(writer.packet, writer.at - writer.packet);
}

static void _teardown_camera(GameInstance *game, Client *client) {
    if (!game->simulation.ent_exists(client->camera)) return;
    Entity &c = game->simulation.get_ent(client->camera);
    if (game->simulation.ent_exists(c.get_team()))
        --game->simulation.get_ent(c.get_team()).player_count;
    if (game->simulation.ent_exists(c.get_player()))
        game->simulation.request_delete(c.get_player());
    for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i)
        PetalTracker::remove_petal(&game->simulation, c.get_inventory(i));
    game->simulation.request_delete(client->camera);
}

GameInstance::GameInstance() : simulation(), clients(), team_manager(&simulation) {}

void GameInstance::init(uint8_t id) {
    map_id = id;
    simulation.map_id = id;
    simulation.arena_info.set_map_id(id);
    for (uint32_t i = 0; i < ENTITY_CAP / 2; ++i)
        Map::spawn_random_mob(&simulation, frand() * ARENA_WIDTH, frand() * ARENA_HEIGHT);
    #ifdef GAMEMODE_TDM
    team_manager.add_team(ColorID::kBlue);
    team_manager.add_team(ColorID::kRed);
    #endif
}

void GameInstance::tick() {
    simulation.tick();
    Party::tick(this);
    Guild::tick(this);
    for (Client *client : clients)
        _update_client(&simulation, client);
    simulation.post_tick();
}

void GameInstance::broadcast(uint8_t const *packet, size_t size) {
    for (Client *client : clients) {
        if (client != nullptr && client->verified)
            client->send_packet(packet, size);
    }
}

void GameInstance::add_client(Client *client) {
    DEBUG_ONLY(assert(client->game != this);)
    if (client->game != nullptr)
        client->game->remove_client(client);
    client->game = this;
    clients.insert(client);
    Entity &ent = simulation.alloc_ent();
    ent.add_component(kCamera);
    ent.add_component(kRelations);
    #ifdef GAMEMODE_TDM
    EntityID team = team_manager.get_random_team();
    ent.set_team(team);
    ent.set_color(simulation.get_ent(team).get_color());
    ++simulation.get_ent(team).player_count;
    #else
    ent.set_team(ent.id);
    ent.set_color(ColorID::kYellow); 
    #endif
    
    ent.set_fov(BASE_FOV);
    ent.set_respawn_level(1);
    for (uint32_t i = 0; i < loadout_slots_at_level(ent.get_respawn_level()); ++i)
        ent.set_inventory(i, PetalID::kBasic);
    // Starter specials: Accelerator, Antennae, Magnet
    if (loadout_slots_at_level(ent.get_respawn_level()) > 1)
        ent.set_inventory(1, PetalID::kAccelerator);
    if (loadout_slots_at_level(ent.get_respawn_level()) > 2)
        ent.set_inventory(2, PetalID::kAntennae);
    if (loadout_slots_at_level(ent.get_respawn_level()) > 3)
        ent.set_inventory(3, PetalID::kMagnet);
    if (frand() < 0.001 && PetalTracker::get_count(&simulation, PetalID::kUniqueBasic) == 0)
        ent.set_inventory(0, PetalID::kUniqueBasic);
    for (uint32_t i = 0; i < loadout_slots_at_level(ent.get_respawn_level()); ++i)
        PetalTracker::add_petal(&simulation, ent.get_inventory(i));
    client->camera = ent.id;
    client->seen_arena = 0;
    client->in_view.clear();
}

Client *GameInstance::client_by_camera(EntityID const &id) {
    for (Client *client : clients) {
        if (client != nullptr && client->camera == id) return client;
    }
    return nullptr;
}

void GameInstance::remove_client(Client *client) {
    DEBUG_ONLY(assert(client->game == this);)
    // Flush best score before releasing the play name
    if (!client->play_name.empty() && simulation.ent_exists(client->camera)) {
        Entity &cam = simulation.get_ent(client->camera);
        if (simulation.ent_alive(cam.get_player())) {
            Entity &player = simulation.get_ent(cam.get_player());
            LeaderboardStore::record(client->play_name, player.get_score(), player.get_color());
            LeaderboardStore::flush();
        }
    }
    AccountStore::capture(client);
    AccountStore::logout(client);
    Party::leave(client);
    Guild::leave(client);
    clients.erase(client);
    _teardown_camera(this, client);
    client->game = nullptr;
    client->in_view.clear();
    client->play_name.clear();
}

void GameInstance::transfer_client(Client *client, GameInstance *dest) {
    if (client == nullptr || dest == nullptr || client->game == dest) return;
    DEBUG_ONLY(assert(client->game == this);)
    AccountStore::capture(client);
    Account *acc = client->account_name.empty()
        ? nullptr
        : AccountStore::find(client->account_name);

    // Snapshot camera progress so guests keep loadout across map switches
    uint8_t snap_level = 1;
    std::array<PetalID::T, 2 * MAX_SLOT_COUNT> snap_petals{};
    snap_petals.fill(PetalID::kNone);
    if (simulation.ent_exists(client->camera)) {
        Entity &cam = simulation.get_ent(client->camera);
        snap_level = cam.get_respawn_level();
        if (snap_level < 1) snap_level = 1;
        for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i)
            snap_petals[i] = cam.get_inventory(i);
    }

    Party::leave(client);
    clients.erase(client);
    _teardown_camera(this, client);
    client->game = nullptr;
    client->in_view.clear();

    // add_client without going through remove_client (game is null)
    dest->add_client(client);
    if (!dest->simulation.ent_exists(client->camera)) return;
    Entity &new_cam = dest->simulation.get_ent(client->camera);
    if (acc != nullptr) {
        AccountStore::apply_to_camera(&dest->simulation, new_cam, *acc);
    } else {
        for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i)
            PetalTracker::remove_petal(&dest->simulation, new_cam.get_inventory(i));
        new_cam.set_respawn_level(snap_level);
        for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i) {
            PetalID::T id = snap_petals[i];
            if (id >= PetalID::kNumPetals) id = PetalID::kNone;
            new_cam.set_inventory(i, id);
            PetalTracker::add_petal(&dest->simulation, id);
        }
    }
}
