#include <Shared/Simulation.hh>

#include <Server/Process.hh>
#include <Server/Client.hh>
#include <Server/EntityFunctions.hh>
#include <Server/Server.hh>
#include <Server/Spawn.hh>
#include <Server/SpatialHash.hh>

#include <Server/LeaderboardStore.hh>
#include <Shared/Map.hh>

#include <algorithm>
#include <vector>

static void calculate_leaderboard(Simulation *sim) {
    std::vector<Entity const *> players;
    sim->for_each<kCamera>([&](Simulation *sim, Entity &ent) {
        if (!sim->ent_alive(ent.get_player())) return;
        Entity const &player = sim->get_ent(ent.get_player());
        players.push_back(&player);
        // Persist best score for every named entrant
        std::string const &name = player.get_name();
        if (!name.empty())
            LeaderboardStore::record(name, player.get_score(), player.get_color());
    });
    uint32_t online = (uint32_t) players.size();
    sim->arena_info.set_player_count(online);
    sim->arena_info.set_registered_count(LeaderboardStore::recorded_count());

    std::vector<LeaderboardEntry> top;
    LeaderboardStore::fill_top(top);
    for (uint32_t i = 0; i < LEADERBOARD_SIZE; ++i) {
        if (i < top.size()) {
            sim->arena_info.set_names(i, top[i].name);
            sim->arena_info.set_scores(i, top[i].score);
            sim->arena_info.set_colors(i, top[i].color);
        } else {
            sim->arena_info.set_names(i, std::string{});
            sim->arena_info.set_scores(i, 0);
            sim->arena_info.set_colors(i, 0);
        }
    }
    // Minimap: every living player on this map (not only leaderboard top)
    std::stable_sort(players.begin(), players.end(), [](Entity const *a, Entity const *b){
        return a->get_score() > b->get_score();
    });
    uint32_t map_n = std::min(online, MINIMAP_PLAYER_CAP);
    for (uint32_t i = 0; i < MINIMAP_PLAYER_CAP; ++i) {
        if (i < map_n) {
            sim->arena_info.set_minimap_x(i, players[i]->get_x());
            sim->arena_info.set_minimap_y(i, players[i]->get_y());
            sim->arena_info.set_minimap_colors(i, players[i]->get_color());
        } else {
            sim->arena_info.set_minimap_x(i, -1.f);
            sim->arena_info.set_minimap_y(i, -1.f);
            sim->arena_info.set_minimap_colors(i, 0);
        }
    }
}

void Simulation::on_tick() {
    spatial_hash.refresh(ARENA_WIDTH, ARENA_HEIGHT);
    if (frand() < 1.0f / TPS) {
        for (uint32_t i = 0; i < 10; ++i) {
            Vector v;
            if (Map::find_spawn_location(this, 500, v))
                Map::spawn_random_mob(this, v.x, v.y);
        }
    }
    for_each_entity([](Simulation *sim, Entity &ent) {
        if (ent.has_component(kPhysics))
            sim->spatial_hash.insert(ent);
        if (BitMath::at(ent.flags, EntityFlags::kHasCulling))
            BitMath::set(ent.flags, EntityFlags::kIsCulled);
    });
    for_each<kCamera>(tick_culling_behavior);
    // Magnet pulls rewrite drop acceleration each tick
    for_each<kDrop>([](Simulation *, Entity &drop) {
        drop.acceleration.set(0, 0);
    });
    for_each<kFlower>(tick_player_behavior);
    for_each<kMob>(tick_ai_behavior);
    for_each<kCamera>(tick_player_ai_behavior);
    for_each<kPetal>(tick_petal_behavior);
    for_each<kHealth>(tick_health_behavior);
    spatial_hash.collide(on_collide);
    tick_curse_behavior(this);
    for_each<kPhysics>(tick_entity_motion);
    for_each<kSegmented>(tick_segment_behavior);
    for_each<kCamera>(tick_camera_behavior);
    for_each<kScore>(tick_score_behavior);
    for_each_entity(entity_clear_references);
    calculate_leaderboard(this);
}

void Simulation::post_tick() {
    arena_info.reset_protocol();
    for_each_entity([](Simulation *sim, Entity &ent) {
        //no deletions mid tick
        ent.reset_protocol();
        ++ent.lifetime;
        if (BitMath::at(ent.flags, EntityFlags::kIsDespawning)) {
            if (ent.despawn_tick == 0) sim->request_delete(ent.id);
            else --ent.despawn_tick;
        }
        if (ent.immunity_ticks > 0) --ent.immunity_ticks;
    });
    for_each_entity([](Simulation *sim, Entity &ent) {
        if (!ent.pending_delete) return;
        if (!ent.has_component(kPhysics)) 
            return sim->_delete_ent(ent.id);
        if (ent.deletion_tick >= TPS / 5) 
            return sim->_delete_ent(ent.id);
        if (ent.deletion_tick == 0)
            entity_on_death(sim, ent);
        ++ent.deletion_tick;
    });
}
