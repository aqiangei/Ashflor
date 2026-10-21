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
#include <iostream>
#include <vector>

static void update_minimap(Simulation *sim, std::vector<Entity const *> &players) {
    uint32_t online = (uint32_t) players.size();
    std::stable_sort(players.begin(), players.end(), [](Entity const *a, Entity const *b){
        return a->get_score() > b->get_score();
    });
    uint32_t map_n = std::min(online, MINIMAP_PLAYER_CAP);
    for (uint32_t i = 0; i < MINIMAP_PLAYER_CAP; ++i) {
        if (i < map_n) {
            sim->arena_info.set_minimap_x(i, players[i]->get_x());
            sim->arena_info.set_minimap_y(i, players[i]->get_y());
            sim->arena_info.set_minimap_colors(i, players[i]->get_color());
            sim->arena_info.set_minimap_names(i, players[i]->get_name());
        } else {
            sim->arena_info.set_minimap_x(i, -1.f);
            sim->arena_info.set_minimap_y(i, -1.f);
            sim->arena_info.set_minimap_colors(i, 0);
            sim->arena_info.set_minimap_names(i, std::string{});
        }
    }
}

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

    // fill_top sorts the entire historical store — throttle to 5 Hz; minimap stays every tick.
    if (sim->tick_index % 4 == 0) {
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
    }
    update_minimap(sim, players);
}

// 每图唯一的 Boss：由 MapGen 提供的锚点驱动，活着时不再生成，死了 45 秒重生。
static void ensure_boss(Simulation *sim) {
    float bx = 0.f, by = 0.f;
    uint8_t mob_id = 0;
    if (!Map::boss_anchor(sim->map_id, bx, by, mob_id)) return;
    // Cached id: avoid full mob scan every tick while boss is alive / on respawn CD.
    if (sim->ent_alive(sim->boss_ent)) {
        Entity &b = sim->get_ent(sim->boss_ent);
        if (b.get_mob_id() == mob_id && !b.pending_delete) return;
    }
    if (sim->boss_ever_spawned) {
        if (sim->boss_respawn_ticks <= 0)
            sim->boss_respawn_ticks = 45 * (int32_t)TPS;
        if (--sim->boss_respawn_ticks > 0) return;
    }
    // Only scan when we are about to spawn (first time or CD finished).
    {
        bool alive = false;
        sim->for_each<kMob>([&](Simulation *, Entity &m) {
            if (m.get_mob_id() == mob_id && !m.pending_delete) {
                alive = true;
                sim->boss_ent = m.id;
            }
        });
        if (alive) return;
    }
    Map::resolve_circle_terrain(sim->map_id, bx, by, 100.f);
    std::cout << "[boss] map " << (int)sim->map_id << " spawning " << MOB_DATA[mob_id].name
              << " at (" << (int)bx << "," << (int)by << ")" << std::endl;
    Entity &boss = alloc_mob(sim, (MobID::T)mob_id, bx, by, NULL_ENTITY, [](Entity &mob) {
        mob.score_reward = MOB_DATA[mob.get_mob_id()].xp;
        BitMath::set(mob.flags, EntityFlags::kHasCulling);
        mob.detection_radius = 1400.f;
        mob.zone = 0xFF;   // 不计入任何 Zone 的刷怪计数
    });
    sim->boss_ent = boss.id;
    sim->boss_ever_spawned = 1;
    sim->boss_respawn_ticks = 0;
}

void Simulation::on_tick() {
    ++tick_index;
    spatial_hash.refresh(ARENA_WIDTH, ARENA_HEIGHT);
    ensure_boss(this);
    // Double the spawn attempts so the doubled zone population fills at the old pace.
    if (frand() < 1.0f / TPS) {
        uint32_t players = 0;
        for_each<kFlower>([&](Simulation *, Entity &ent) {
            if (!ent.has_component(kMob)) ++players;
        });
        uint32_t batches = 2u * (4u + players);
        if (batches > 24u) batches = 24u;
        for (uint32_t i = 0; i < batches; ++i)
            Map::spawn_zone_mob(this);
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
    for_each<kDrop>(tick_drop_behavior);
    for_each<kFlower>(tick_player_behavior);
    tick_ai_prepare(this);
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
        if (ent.immunity_ticks > 0) {
            --ent.immunity_ticks;
        }
        // Team-tagged drops stay private for their whole lifetime (owner + party).
        // Only unowned (manual discard) piles use immunity then go FFA — never clear team here.
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
