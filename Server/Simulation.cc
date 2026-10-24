#include <Shared/Simulation.hh>

#include <Server/Process.hh>
#include <Server/Client.hh>
#include <Server/EntityFunctions.hh>
#include <Server/Server.hh>
#include <Server/Spawn.hh>
#include <Server/SpatialHash.hh>
#include <Server/TrainingDummy.hh>

#include <Server/LeaderboardStore.hh>
#include <Shared/Map.hh>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <vector>

static double _tick_ms_since(std::chrono::steady_clock::time_point const &t0) {
    return std::chrono::duration<double, std::milli>(
        std::chrono::steady_clock::now() - t0).count();
}

static std::string resolve_player_name(Simulation *sim, Entity &camera, Entity &player) {
    if (!player.get_name().empty()) return player.get_name();
    // Revive / rare spawn paths can leave the flower nameless; heal from session.
    Client *c = Server::client_by_camera(sim, camera.id);
    if (c == nullptr) return {};
    std::string n = !c->play_name.empty() ? c->play_name : c->account_name;
    if (!n.empty()) player.set_name(n);
    return n;
}

static void update_minimap(Simulation *sim, std::vector<Entity *> &players,
    std::vector<std::string> const &names)
{
    uint32_t online = (uint32_t) players.size();
    // Keep name tied to entity while sorting by score.
    std::vector<uint32_t> order(online);
    for (uint32_t i = 0; i < online; ++i) order[i] = i;
    std::stable_sort(order.begin(), order.end(), [&](uint32_t a, uint32_t b){
        return players[a]->get_score() > players[b]->get_score();
    });
    uint32_t map_n = std::min(online, MINIMAP_PLAYER_CAP);
    for (uint32_t i = 0; i < MINIMAP_PLAYER_CAP; ++i) {
        if (i < map_n) {
            uint32_t oi = order[i];
            Entity *p = players[oi];
            sim->arena_info.set_minimap_x(i, p->get_x());
            sim->arena_info.set_minimap_y(i, p->get_y());
            sim->arena_info.set_minimap_colors(i, p->get_color());
            sim->arena_info.set_minimap_names(i, names[oi]);
        } else {
            sim->arena_info.set_minimap_x(i, -1.f);
            sim->arena_info.set_minimap_y(i, -1.f);
            sim->arena_info.set_minimap_colors(i, 0);
            sim->arena_info.set_minimap_names(i, std::string{});
        }
    }
    // WS send can drop when bufferedAmount is high; force full slot snapshot
    // every 2s so emptied slots (x=-1) cannot leave ghost dots on clients.
    if (sim->tick_index % (2 * TPS) == 0)
        sim->arena_info.force_sync_minimap_slots();
}

static void calculate_leaderboard(Simulation *sim) {
    std::vector<Entity *> players;
    std::vector<std::string> names;
    sim->for_each<kCamera>([&](Simulation *sim, Entity &ent) {
        if (!sim->ent_alive(ent.get_player())) return;
        Entity &player = sim->get_ent(ent.get_player());
        std::string name = resolve_player_name(sim, ent, player);
        players.push_back(&player);
        names.push_back(name);
        // Score high-water → LB dirty. Do it with fill_top (~5 Hz), not every tick.
        if (!name.empty() && (sim->tick_index % 4 == 0))
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
    update_minimap(sim, players, names);
}

// 每图唯一的 Boss：由 MapGen 提供的锚点驱动，活着时不再生成，死了 45 秒重生。
static void ensure_boss(Simulation *sim) {
    float bx = 0.f, by = 0.f;
    uint8_t mob_id = 0;
    if (!Map::boss_anchor(sim->map_id, bx, by, mob_id)) return;
    // Cached id: avoid full mob scan every tick while boss is alive / on respawn CD.
    // Only the map-anchor boss (zone==0xFF) counts — nest trash queens must not
    // suppress Unique boss respawn.
    auto is_map_boss = [mob_id](Entity &m) -> bool {
        return m.get_mob_id() == mob_id && !m.pending_delete && m.zone == 0xFF;
    };
    if (sim->ent_alive(sim->boss_ent)) {
        Entity &b = sim->get_ent(sim->boss_ent);
        if (is_map_boss(b)) return;
    }
    // Stale boss_ent cache: reattach before burning respawn CD (avoids false
    // "dead" windows and duplicate spawn attempts after slot recycle).
    {
        bool alive = false;
        sim->for_each<kMob>([&](Simulation *, Entity &m) {
            if (is_map_boss(m)) {
                alive = true;
                sim->boss_ent = m.id;
            }
        });
        if (alive) {
            sim->boss_respawn_ticks = 0;
            return;
        }
    }
    if (sim->boss_ever_spawned) {
        if (sim->boss_respawn_ticks <= 0)
            sim->boss_respawn_ticks = 45 * (int32_t)TPS;
        if (--sim->boss_respawn_ticks > 0) return;
    }
    // CD finished — confirm still absent before allocating a Unique boss.
    {
        bool alive = false;
        sim->for_each<kMob>([&](Simulation *, Entity &m) {
            if (is_map_boss(m)) {
                alive = true;
                sim->boss_ent = m.id;
            }
        });
        if (alive) {
            sim->boss_respawn_ticks = 0;
            return;
        }
    }
    Map::resolve_circle_terrain(sim->map_id, bx, by, 100.f);
    std::cout << "[boss] map " << (int)sim->map_id << " spawning " << MOB_DATA[mob_id].name
              << " at (" << (int)bx << "," << (int)by << ")" << std::endl;
    Entity &boss = alloc_mob(sim, (MobID::T)mob_id, bx, by, NULL_ENTITY, [](Entity &mob) {
        MobID::T mid = mob.get_mob_id();
        uint8_t base_r = MOB_DATA[mid].rarity;
        // Map bosses are top-tier fights — never sit at the baked Common/Rare rung.
        uint8_t spawn_r = RarityID::kUnique;
        if (spawn_r < base_r) spawn_r = base_r;
        mob.set_rarity(spawn_r);
        float th = mob_spawn_threat_mult(base_r, spawn_r);
        float dmg = mob_spawn_damage_mult(base_r, spawn_r);
        float rs = mob_rarity_radius_mult(spawn_r);
        BitMath::set(mob.flags, EntityFlags::kHasCulling);
        mob.detection_radius = 1400.f;
        mob.zone = 0xFF;   // 不计入任何 Zone 的刷怪计数
        // Unique petals are baked on the ×2000 curve; rarity threat alone is not
        // enough — keep an extra boss pad so Unique loadouts still feel a fight.
        constexpr float kBossHpMult = 8.f;
        constexpr float kBossDmgMult = 1.35f;
        mob.max_health *= th * kBossHpMult;
        mob.health = mob.max_health;
        mob.damage *= dmg * kBossDmgMult;
        mob.score_reward = (uint32_t) (
            MOB_DATA[mid].xp * mob_spawn_xp_mult(base_r, spawn_r) * 8.f + 500.f);
        if (rs != 1.f) {
            float nr = mob.get_radius() * rs;
            mob.set_radius(nr);
            mob.mass = (1 + nr / BASE_FLOWER_RADIUS)
                * (MOB_DATA[mid].attributes.stationary ? 10000.f : 1.f);
        }
    });
    sim->boss_ent = boss.id;
    sim->boss_ever_spawned = 1;
    sim->boss_respawn_ticks = 0;
}

void Simulation::on_tick() {
    ++tick_index;
    // florr_clone / rrolf: no humans → no spawn pressure; skip heavy collide/AI.
    uint32_t flowers = 0;
    for_each<kFlower>([&](Simulation *, Entity &ent) {
        // Corpses have Flower but no Health — don't keep an empty map "occupied".
        if (!ent.has_component(kMob) && ent.has_component(kHealth)) ++flowers;
    });
    uint8_t const occupied = flowers > 0;

    auto t_hash0 = std::chrono::steady_clock::now();
    spatial_hash.refresh(ARENA_WIDTH, ARENA_HEIGHT);
    Server::phase_hash_ms += _tick_ms_since(t_hash0);
    // Always — empty-map early-out used to skip this and bosses never spawned
    // at boot / never counted respawn CD (live "Boss 存活" all dead).
    ensure_boss(this);
    TrainingDummy::ensure(this);

    // Idle map (Server already ~1 Hz): thin wild mobs slowly, skip collide/AI.
    // Only zone wildlife (kSpawnedFromZone). WorldEvent / Treasure / explore
    // props are never zone-spawned — do not wipe them (chat-only bug).
    if (!occupied) {
        for_each_entity([](Simulation *sim, Entity &ent) {
            if (!ent.has_component(kMob)) return;
            if (ent.zone == 0xFF) return; // boss / special
            if (!BitMath::at(ent.flags, EntityFlags::kSpawnedFromZone)) return;
            if (BitMath::at(ent.flags, EntityFlags::kIsDespawning)) return;
            // ~1.5% per idle tick → density drops without a stampede of deletes
            if (frand() < 0.015f) {
                BitMath::set(ent.flags, EntityFlags::kNoDrops);
                sim->request_delete(ent.id);
            }
        });
        for_each_entity(entity_clear_references);
        return;
    }

    // Occupied: refill near players. Soft-cap batches — linear 2+flowers up to 8
    // overfilled hub when a crowd's AOIs overlapped the Common belt.
    // Crowd scale: 32+ flowers on one map made spawn+collide the tick hog in
    // combat load tests — keep pockets filling for solo/small groups, throttle
    // when a hub is already packed.
    {
        auto t_spawn = std::chrono::steady_clock::now();
        float spawn_mul = 1.f;
        if (flowers >= 16u) spawn_mul = 0.55f;
        if (flowers >= 28u) spawn_mul = 0.35f;
        // Cap outer spawn calls per tick — each can still do 2 placement tries.
        // Unbounded 4+extra+burst used to stack with corridor probes → 90ms spikes.
        uint32_t budget = 5u;
        if (flowers >= 16u) budget = 3u;
        if (flowers >= 28u) budget = 2u;
        if (frand() < (2.5f * spawn_mul) / (float) TPS) {
            uint32_t extra = flowers;
            if (extra > 2u) extra = 2u;
            if (flowers >= 16u && extra > 1u) extra = 1u;
            uint32_t batches = 2u + extra;
            if (flowers >= 28u) batches = 2u;
            for (uint32_t i = 0; i < batches && budget > 0; ++i) {
                Map::spawn_zone_mob(this);
                --budget;
            }
        }
        // Underfilled player zones get a second burst so sparse rings/corners
        // do not stay empty after unseen-despawn cleared the pack.
        if (flowers < 16u && budget > 0 && frand() < 3.0f / (float) TPS) {
            uint32_t n = budget > 2u ? 2u : budget;
            for (uint32_t i = 0; i < n; ++i)
                Map::spawn_zone_mob(this);
        }
        Server::phase_spawn_ms += _tick_ms_since(t_spawn);
    }

    {
        auto t_hash = std::chrono::steady_clock::now();
        for_each_entity([](Simulation *sim, Entity &ent) {
            if (ent.has_component(kPhysics))
                sim->spatial_hash.insert(ent);
            if (BitMath::at(ent.flags, EntityFlags::kHasCulling))
                BitMath::set(ent.flags, EntityFlags::kIsCulled);
        });
        for_each<kCamera>(tick_culling_behavior);

        // florr unseenDespawn: zone mobs still culled after AOI → delete, no loot.
        // Stride ~5 ticks (~6Hz). unseen_ticks==0 = never in any camera AOI.
        // Plaza/soft longer than wild 30s so solo circling doesn't blink packs out;
        // not multi-minute — crowd AOIs keep mobs "seen" forever, so long timers
        // only delayed cleanup when people leave a pocket.
        if (tick_index % 5 == 0) {
            for_each<kMob>([](Simulation *sim, Entity &ent) {
                if (!BitMath::at(ent.flags, EntityFlags::kSpawnedFromZone)) return;
                if (ent.zone == 0xFF) return; // boss / special
                if (BitMath::at(ent.flags, EntityFlags::kIsDespawning)) return;
                if (!BitMath::at(ent.flags, EntityFlags::kHasCulling)) return;

                game_tick_t unseen_limit = (game_tick_t)(30 * TPS);
                game_tick_t never_seen_life = (game_tick_t)(60 * TPS);
                uint8_t in_plaza = Map::in_spawn_plaza(ent.get_x(), ent.get_y());
                uint32_t zdiff = RarityID::kNumRarities;
                if (ent.zone < MAX_ZONES_PER_MAP)
                    zdiff = map_zones(sim->map_id)[ent.zone].difficulty;
                if (in_plaza) {
                    unseen_limit = (game_tick_t)(55 * TPS);
                    never_seen_life = (game_tick_t)(70 * TPS);
                } else if (zdiff <= RarityID::kUnusual) {
                    unseen_limit = (game_tick_t)(40 * TPS);
                    never_seen_life = (game_tick_t)(75 * TPS);
                }

                if (BitMath::at(ent.flags, EntityFlags::kIsCulled)) {
                    if (ent.unseen_ticks == 0) {
                        if (ent.lifetime >= never_seen_life) {
                            BitMath::set(ent.flags, EntityFlags::kNoDrops);
                            sim->request_delete(ent.id);
                        }
                        return;
                    }
                    ent.unseen_ticks = (game_tick_t)(ent.unseen_ticks + 5);
                    if (ent.unseen_ticks >= unseen_limit) {
                        BitMath::set(ent.flags, EntityFlags::kNoDrops);
                        sim->request_delete(ent.id);
                    }
                } else {
                    // Mark seen: any positive value arms the despawn clock when culled again.
                    ent.unseen_ticks = 1;
                }
            });
            // After unseen pass: drain soft-zone surplus under crowd dens_mul.
            Map::thin_soft_zone_excess(this);
        }
        Server::phase_hash_ms += _tick_ms_since(t_hash);
    }

    {
        auto t_ai = std::chrono::steady_clock::now();
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
        Server::phase_ai_ms += _tick_ms_since(t_ai);
    }

    {
        auto t_col = std::chrono::steady_clock::now();
        spatial_hash.collide(on_collide);
        Server::phase_collide_ms += _tick_ms_since(t_col);
    }

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
        // Tagged drops stay private to their personal camera for their whole life.
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
