#include <Server/Game.hh>

#include <Server/AccountStore.hh>
#include <Server/LeaderboardStore.hh>
#include <Server/Loot.hh>
#include <Server/Client.hh>
#include <Server/EntityFunctions.hh>
#include <Server/Guild.hh>
#include <Server/Party.hh>
#include <Server/Treasure.hh>
#include <Server/WorldEvent.hh>
#include <Server/WorldExplore.hh>
#include <Server/PetalTracker.hh>
#include <Server/Server.hh>
#include <Server/Spawn.hh>

#include <Shared/Binary.hh>
#include <Shared/Entity.hh>
#include <Shared/Map.hh>
#include <Shared/NetworkView.hh>
#include <Shared/StaticData.hh>

#include <algorithm>
#include <array>
#include <chrono>
#include <iostream>
#include <string>
#include <vector>

static void _park_player_still(Simulation *sim, Client *client) {
    if (!sim->ent_exists(client->camera)) return;
    Entity &camera = sim->get_ent(client->camera);
    if (!sim->ent_alive(camera.get_player())) return;
    Entity &player = sim->get_ent(camera.get_player());
    player.acceleration.set(0, 0);
    player.input = 0;
}

static void _update_client(Simulation *sim, Client *client) {
    if (client == nullptr) return;
    if (!client->verified) return;
    if (sim == nullptr) return;
    if (!sim->ent_exists(client->camera)) return;
    // Frozen / background tabs stop reading. Keep the flower standing still and
    // skip pushes so backpressure cannot kill the socket (was: dropped → kick).
    uint64_t now_ms = (uint64_t) std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    uint8_t silent = client->last_rx_ms > 0 && (now_ms - client->last_rx_ms) > 4000;
    if (silent || !client->can_accept_update()) {
        _park_player_still(sim, client);
        // Delta flags are cleared each tick, including ticks we cannot send.
        // Recreate the view on resume so a missed camera/player change recovers.
        client->needs_full_update = 1;
        return;
    }
    if (client->needs_full_update) {
        client->in_view.clear();
        client->seen_arena = 0;
        client->needs_full_update = 0;
    }
    // A temporary tree allocated one node per visible entity on every tick.
    // Collect contiguously, then keep the same sorted, unique wire order.
    std::vector<EntityID> in_view;
    in_view.reserve(client->in_view.size() + 16);
    std::vector<EntityID> deletes;
    in_view.push_back(client->camera);
    Entity &camera = sim->get_ent(client->camera);
    if (sim->ent_exists(camera.get_player())) 
        in_view.push_back(camera.get_player());
    Writer writer(Server::OUTGOING_PACKET);
    writer.write<uint8_t>(Clientbound::kClientUpdate);
    writer.write<EntityID>(client->camera);
    float half_w, half_h;
    NetworkView::half_extents(camera.get_fov(), BASE_FOV,
        client->view_half_w, client->view_half_h, half_w, half_h);
    sim->spatial_hash.query(camera.get_camera_x(), camera.get_camera_y(),
        half_w, half_h,
        [&](Simulation *, Entity &ent){
            // Private drops are tagged to a personal camera id. Party join
            // rewrites camera.get_team() to the leader camera — never use that
            // for loot visibility (mates would see the leader's piles but fail
            // pickup). Only the owning camera may see / take tagged drops.
            if (ent.has_component(kDrop) && !Loot::can_access_drop(ent, client->camera)) return;
            in_view.push_back(ent.id);
        });
    std::sort(in_view.begin(), in_view.end());
    in_view.erase(std::unique(in_view.begin(), in_view.end()), in_view.end());
    // No per-client entity count cap — matches gardn / rrolf / florr_clone-web.
    // Cost is controlled by FOV AABB interest, density caps, and AI cull.

    for (EntityID const &i: client->in_view) {
        if (!std::binary_search(in_view.begin(), in_view.end(), i)) {
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
        if (create) client->in_view.insert(id);
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
    // Density-capped fill (was ENTITY_CAP/2 = 4096 attempts × 8 maps → boot hog).
    // Zone caps inside spawn_zone_mob still apply; this only limits attempts.
    // HK 8c: fuller boot so first join is not an empty biome (was 900 on 4c).
    constexpr uint32_t kInitSpawnAttempts = 1400;
    for (uint32_t i = 0; i < kInitSpawnAttempts; ++i)
        Map::spawn_zone_mob(&simulation);
    // Startup diagnostics: which mob species actually spawned in this biome.
    // Cheap (one pass) and makes "did my new monsters actually spawn?" checkable
    // from `journalctl -u ashflor` without a live client.
    {
        // active_entities is only rebuilt inside tick(), so refresh it first.
        simulation.refresh_active_entities();
        uint32_t counts[MobID::kNumMobs] = {};
        simulation.for_each<kMob>([&](Simulation *, Entity &mob) {
            MobID::T mid = mob.get_mob_id();
            if (mid < MobID::kNumMobs) ++counts[mid];
        });
        std::string line = "[mobs] map " + std::to_string((int)id) + ":";
        for (MobID::T mid = 0; mid < MobID::kNumMobs; ++mid) {
            if (counts[mid] == 0) continue;
            line += " " + std::string(MOB_DATA[mid].name) + "=" + std::to_string(counts[mid]);
        }
        std::cout << line << std::endl;
    }
    // Bots are full camera+AI flowers; 8 maps × N was the main tick budget eater.
    // Disable until the single-thread WASM sim has headroom again.
    constexpr uint32_t kBotCount = 0;
    for (uint32_t i = 0; i < kBotCount; ++i) {
        Entity &cam = alloc_cpu_camera(&simulation, NULL_ENTITY);
        Entity &player = alloc_player(&simulation, cam.get_team());
        player_spawn(&simulation, cam, player);
        static char const *names[] = {
            "Bot-Alpha", "Bot-Beta", "Bot-Gamma", "Bot-Delta",
            "Bot-Echo", "Bot-Foxtrot"
        };
        player.set_name(names[i % 6]);
    }
    #ifdef GAMEMODE_TDM
    team_manager.add_team(ColorID::kBlue);
    team_manager.add_team(ColorID::kRed);
    #endif
    Treasure::spawn_for_map(this);
    WorldExplore::spawn_for_map(this);
    WorldEvent::spawn_for_map(this);
}

static void process_map_portals(GameInstance *game) {
    if (game == nullptr) return;
    // Snapshot clients — transfer mutates the set
    std::vector<Client *> snap(game->clients.begin(), game->clients.end());
    for (Client *client : snap) {
        if (client == nullptr || client->game != game) continue;
        if (!game->simulation.ent_exists(client->camera)) continue;
        Entity &cam = game->simulation.get_ent(client->camera);
        if (!game->simulation.ent_alive(cam.get_player())) continue;
        Entity &player = game->simulation.get_ent(cam.get_player());
        if (player.immunity_ticks > 0) continue;
        float px = player.get_x();
        float py = player.get_y();
        for (MapPortalDef const &p : MAP_PORTALS) {
            if (p.from_map != game->map_id) continue;
            float dx = px - p.x;
            float dy = py - p.y;
            if (dx * dx + dy * dy > p.radius * p.radius) continue;
            if (p.to_map >= MapID::kNumMaps || !map_is_open(p.to_map)) continue;
            GameInstance *dest = &Server::games[p.to_map];
            std::string name = player.get_name();
            // Do NOT reuse old-map team EntityID — teams are per-simulation.
            game->transfer_client(client, dest);
            if (client->game != dest) break;
            if (!dest->simulation.ent_exists(client->camera)) break;
            Entity &ncam = dest->simulation.get_ent(client->camera);
            if (dest->simulation.ent_exists(ncam.get_player()))
                dest->simulation.request_delete(ncam.get_player());
            Entity &nplayer = alloc_player(&dest->simulation, ncam.get_team());
            float dxp = p.dest_x, dyp = p.dest_y;
            Map::resolve_circle_terrain(dest->map_id, dxp, dyp, BASE_FLOWER_RADIUS + 8);
            player_spawn_at(&dest->simulation, ncam, nplayer, dxp, dyp);
            nplayer.set_name(name);
            nplayer.set_guild(client->guild_name);
            client->play_name = name;
            // Long enough to walk off hub before another portal can fire
            nplayer.immunity_ticks = (game_tick_t)(4.0f * TPS);
            Party::apply_team(client);
            break;
        }
    }
}

void GameInstance::tick() {
    simulation.tick();
    auto t_sync = std::chrono::steady_clock::now();
    process_map_portals(this);
    Party::tick(this);
    Guild::tick(this);
    Treasure::tick(this);
    WorldExplore::tick(this);
    WorldEvent::tick(this);
    for (Client *client : clients) {
        TrainingDummy::tick_client(&simulation, client);
        _update_client(&simulation, client);
    }
    Server::phase_sync_ms += std::chrono::duration<double, std::milli>(
        std::chrono::steady_clock::now() - t_sync).count();
    auto t_post = std::chrono::steady_clock::now();
    simulation.post_tick();
    Server::phase_post_ms += std::chrono::duration<double, std::milli>(
        std::chrono::steady_clock::now() - t_post).count();
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
    WorldExplore::on_player_spawned(client);
    WorldEvent::clear_client(client);
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
        }
    }
    // Move the session stat counters into the leaderboard store
    client_flush_stats(client);
    AccountStore::capture(client); // sync disk on disconnect
    LeaderboardStore::flush();
    AccountStore::logout(client);
    Party::leave(client);
    // Keep guild roster membership on disconnect so others see them offline.
    // Explicit leave / account logout still call Guild::leave.
    std::string gname = client->guild_name;
    clients.erase(client);
    if (!gname.empty()) {
        for (Client *c : clients) {
            if (c != nullptr && c->guild_name == gname)
                Guild::send_update(c);
        }
    }
    WorldExplore::clear_client(client);
    WorldEvent::clear_client(client);
    _teardown_camera(this, client);
    client->game = nullptr;
    client->in_view.clear();
    client->play_name.clear();
}

void GameInstance::transfer_client(Client *client, GameInstance *dest) {
    if (client == nullptr || dest == nullptr || client->game == dest) return;
    DEBUG_ONLY(assert(client->game == this);)

    WorldExplore::clear_client(client);
    WorldEvent::clear_client(client);

    // Snapshot from the live flower (camera inventory is empty while alive).
    uint32_t snap_level = 1;
    std::array<PetalID::T, 2 * MAX_SLOT_COUNT> snap_petals{};
    snap_petals.fill(PetalID::kNone);
    if (simulation.ent_exists(client->camera)) {
        Entity &cam = simulation.get_ent(client->camera);
        EntityID pid = cam.get_player();
        if (simulation.ent_exists(pid) && simulation.get_ent(pid).has_component(kFlower)) {
            Entity &player = simulation.get_ent(pid);
            flush_flower_loadout_to_camera(&simulation, cam, player);
            uint32_t sc = player.get_score();
            if (client->pending_score > sc) sc = client->pending_score;
            uint32_t lv = score_to_level(sc);
            if (lv < 1) lv = 1;
            if (lv < cam.get_respawn_level()) lv = cam.get_respawn_level();
            snap_level = lv;
            client->pending_score = sc;
            for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i)
                snap_petals[i] = player.get_loadout_ids(i);
        } else {
            snap_level = cam.get_respawn_level();
            if (snap_level < 1) snap_level = 1;
            uint32_t floor = level_to_score(snap_level);
            if (client->pending_score < floor) client->pending_score = floor;
            for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i)
                snap_petals[i] = cam.get_inventory(i);
        }
    }

    // Memory snapshot only — portal hops must not sync-write 150KB accounts.dat.
    // Periodic flush (~15s) and disconnect still persist.
    AccountStore::capture_nosave(client);
    Account *acc = client->account_name.empty()
        ? nullptr
        : AccountStore::find(client->account_name);

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
        new_cam.set_respawn_level(snap_level < 1 ? 1 : snap_level);
        for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i) {
            PetalID::T id = snap_petals[i];
            if (id >= PetalID::kNumPetals) id = PetalID::kNone;
            new_cam.set_inventory(i, id);
            PetalTracker::add_petal(&dest->simulation, id);
        }
    }
}
