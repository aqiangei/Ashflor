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
#include <Shared/NetworkView.hh>

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
    if (sim->boss_ever_spawned) {
        if (sim->boss_respawn_ticks <= 0)
            sim->boss_respawn_ticks = 45 * (int32_t)TPS;
        if (--sim->boss_respawn_ticks > 0) return;
    }
    // Only scan when we are about to spawn (first time or CD finished).
    {
        bool alive = false;
        sim->for_each<kMob>([&](Simulation *, Entity &m) {
            if (is_map_boss(m)) {
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
        MobID::T mid = mob.get_mob_id();
        uint8_t base_r = MOB_DATA[mid].rarity;
        // Map bosses are top-tier fights — never sit at the baked Common/Rare rung.
        uint8_t spawn_r = RarityID::kUnique;
        if (spawn_r < base_r) spawn_r = base_r;
        mob.set_rarity(spawn_r);
        float th = mob_spawn_threat_mult(base_r, spawn_r);
        float dmg = mob_spawn_damage_mult(base_r, spawn_r);
        float rs = mob_spawn_radius_mult(base_r, spawn_r);
        BitMath::set(mob.flags, EntityFlags::kHasCulling);
        mob.detection_radius = 1400.f;
        mob.zone = 0xFF;   // 不计入任何 Zone 的刷怪计数
        // Extra boss pad on top of Unique HP ladder — was ×8 when ladder was
        // 2.6e7 and Unique petals felt helpless; ladder is flatter now so ×5.
        constexpr float kBossHpMult = 5.f;
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
    // Death Continue / revive wait still has a connected camera. Counting only
    // living flowers treated a solo death as an empty map: spatial_hash never
    // rebuilt, _update_client deleted every mob from the client's view, and
    // wildlife drained under the death screen.
    uint32_t viewers = 0;
    for_each<kCamera>([&](Simulation *sim, Entity &cam) {
        if (Server::client_by_camera(sim, cam.id) != nullptr)
            ++viewers;
    });
    uint8_t const occupied = flowers > 0 || viewers > 0;

    auto t_hash0 = std::chrono::steady_clock::now();
    spatial_hash.refresh(ARENA_WIDTH, ARENA_HEIGHT);
    Server::phase_hash_ms += _tick_ms_since(t_hash0);
    // Always — empty-map early-out used to skip this and bosses never spawned
    // at boot / never counted respawn CD (live "Boss 存活" all dead).
    ensure_boss(this);
    TrainingDummy::ensure(this);

    // Idle map (Server already ~1 Hz): drain wildlife fast — yesterday's
    // midnight collide storms were empty/near-empty biomes still holding
    // 1k+ ents (snow/forest). WorldEvent/Treasure/explore stay (non-zone).
    if (!occupied) {
        uint32_t budget = 96;
        uint32_t ents = entity_count();
        if (ents > 1200u) budget = 160;
        if (ents > 2000u) budget = 240;
        for_each_entity([&budget](Simulation *sim, Entity &ent) {
            if (budget == 0) return;
            if (!ent.has_component(kMob)) return;
            if (ent.zone == 0xFF) return; // boss / special
            if (!BitMath::at(ent.flags, EntityFlags::kSpawnedFromZone)) return;
            if (BitMath::at(ent.flags, EntityFlags::kIsDespawning)) return;
            // ~12%/idle-tick baseline; always honor budget so packed biomes drain
            if (frand() < 0.12f || budget > 48) {
                BitMath::set(ent.flags, EntityFlags::kNoDrops);
                sim->request_delete(ent.id);
                --budget;
            }
        });
        for_each_entity(entity_clear_references);
        return;
    }

    // Occupied: FOV-local refill (~19/viewport). HK 8c — aggressive refill,
    // only throttle near ENTITY_CAP (not mid-pool like old 4c BJ).
    {
        auto t_spawn = std::chrono::steady_clock::now();
        uint32_t ents = entity_count();
        // Align with Map kEntitySpawnStop (8050): reclaim before collide spikes.
        if (ents >= 7600u)
            emergency_reclaim_entities(ents >= 8050u ? 448u : 256u);
        ents = entity_count();
        if (ents < 8050u) {
            uint8_t underfilled = Map::any_combat_fov_underfilled(this, 0.75f);
            uint8_t headroom = ents < 6200u && flowers <= 4u;
            // Crowd: overlapping FOVs — soften vs BJ; 8c can refill multiplayer FOVs.
            float spawn_mul = 1.f;
            if (flowers >= 4u) spawn_mul = 0.85f;
            if (flowers >= 8u) spawn_mul = 0.70f;
            if (!underfilled && flowers >= 16u) spawn_mul = 0.55f;
            if (!underfilled && flowers >= 28u) spawn_mul = 0.40f;
            if (ents >= 7200u) spawn_mul *= 0.70f;
            if (ents >= 7800u) spawn_mul *= 0.55f;
            // Cap refill burst — post-restart FOV fill was blowing ticks to 80–120ms → 1006.
            uint32_t budget = underfilled ? (headroom ? 5u : 3u) : (headroom ? 3u : 2u);
            if (underfilled && flowers >= 4u) budget = 3u;
            if (underfilled && flowers >= 8u) budget = 2u;
            if (!underfilled && flowers >= 8u) budget = 2u;
            if (!underfilled && flowers >= 16u) budget = 1u;
            float under_chance = (flowers >= 8u) ? (3.0f / (float) TPS)
                                                 : (headroom ? 5.0f / (float) TPS
                                                             : 4.0f / (float) TPS);
            float chance = underfilled ? under_chance
                                       : ((3.0f * spawn_mul) / (float) TPS);
            if (frand() < chance) {
                uint32_t extra = (headroom && underfilled) ? 1u
                    : ((flowers > 2u && flowers < 8u) ? 1u : 0u);
                uint32_t batches = underfilled ? (2u + extra) : (1u + extra);
                if (!underfilled && flowers >= 12u) batches = 1u;
                for (uint32_t i = 0; i < batches && budget > 0; ++i) {
                    Map::spawn_zone_mob(this);
                    --budget;
                }
            }
            // Second pulse: light refill only — avoid stacking with first burst.
            if (underfilled && flowers < 4u && budget > 0
                && ents < 7200u && frand() < 3.0f / (float) TPS) {
                uint32_t n = budget > 2u ? 2u : budget;
                for (uint32_t i = 0; i < n; ++i)
                    Map::spawn_zone_mob(this);
            } else if (!underfilled && flowers < 6u && budget > 0
                       && ents < 6800u && frand() < 1.5f / (float) TPS) {
                Map::spawn_zone_mob(this);
            }
        }
        Server::phase_spawn_ms += _tick_ms_since(t_spawn);
    }

    {
        auto t_hash = std::chrono::steady_clock::now();
        // Only throttle wildlife outside every camera's network view. Death
        // cameras and wide vision still need a stable hash on every tick.
        struct View { float x, y, half_w, half_h; };
        std::vector<View> views;
        for_each<kCamera>([&](Simulation *, Entity &camera) {
            Client *owner = Server::client_by_camera(this, camera.id);
            float half_w, half_h;
            NetworkView::half_extents(camera.get_fov(), BASE_FOV,
                owner ? owner->view_half_w : 0, owner ? owner->view_half_h : 0,
                half_w, half_h);
            views.push_back({camera.get_camera_x(), camera.get_camera_y(),
                std::max(half_w, 5000.f), std::max(half_h, 5000.f)});
        });
        uint32_t const tick = tick_index;
        uint32_t const ents_now = entity_count();
        // Far wildlife hash LOD: stride 4 idle, 6 near ENTITY_CAP (8c headroom).
        uint32_t const hash_stride = (flowers >= 12u || ents_now >= 7200u) ? 6u : 4u;
        for_each_entity([tick, hash_stride, &views](Simulation *sim, Entity &ent) {
            if (ent.has_component(kPhysics)) {
                uint8_t skip = 0;
                if (ent.has_component(kMob)
                    && BitMath::at(ent.flags, EntityFlags::kSpawnedFromZone)
                    && ent.zone != 0xFF
                    && ((tick + ent.id.id) % hash_stride) != 0u) {
                    uint8_t in_view = 0;
                    float const mx = ent.get_x(), my = ent.get_y();
                    for (View const &view : views) {
                        if (std::abs(view.x - mx) <= view.half_w + ent.get_radius()
                            && std::abs(view.y - my) <= view.half_h + ent.get_radius()) {
                            in_view = 1;
                            break;
                        }
                    }
                    if (!in_view) skip = 1;
                }
                if (!skip) sim->spatial_hash.insert(ent);
            }
            if (BitMath::at(ent.flags, EntityFlags::kHasCulling))
                BitMath::set(ent.flags, EntityFlags::kIsCulled);
        });
        for_each<kCamera>(tick_culling_behavior);

        // florr unseenDespawn: culled zone wildlife → no-loot delete.
        // 8c: longer windows; load gates only near pool ceiling.
        if (tick_index % 5 == 0) {
            uint8_t const load_hi = ents_now >= 7200u || flowers >= 16u;
            uint8_t const load_panic = ents_now >= 7800u;
            for_each<kMob>([load_hi, load_panic](Simulation *sim, Entity &ent) {
                if (!BitMath::at(ent.flags, EntityFlags::kSpawnedFromZone)) return;
                if (ent.zone == 0xFF) return; // boss / special
                if (BitMath::at(ent.flags, EntityFlags::kIsDespawning)) return;
                if (!BitMath::at(ent.flags, EntityFlags::kHasCulling)) return;

                game_tick_t unseen_limit = (game_tick_t)(28 * TPS);
                game_tick_t never_seen_life = (game_tick_t)(55 * TPS);
                uint8_t in_plaza = Map::in_spawn_plaza(sim->map_id, ent.get_x(), ent.get_y());
                uint32_t zdiff = RarityID::kNumRarities;
                if (ent.zone < MAX_ZONES_PER_MAP)
                    zdiff = map_zones(sim->map_id)[ent.zone].difficulty;
                if (in_plaza) {
                    unseen_limit = (game_tick_t)(45 * TPS);
                    never_seen_life = (game_tick_t)(60 * TPS);
                } else if (zdiff <= RarityID::kUnusual) {
                    unseen_limit = (game_tick_t)(32 * TPS);
                    never_seen_life = (game_tick_t)(60 * TPS);
                }
                if (load_hi && !in_plaza) {
                    unseen_limit = (game_tick_t)(unseen_limit * 4 / 5);
                    never_seen_life = (game_tick_t)(never_seen_life * 5 / 6);
                }
                if (load_panic && !in_plaza) {
                    if (unseen_limit > (game_tick_t)(15 * TPS))
                        unseen_limit = (game_tick_t)(15 * TPS);
                    if (never_seen_life > (game_tick_t)(35 * TPS))
                        never_seen_life = (game_tick_t)(35 * TPS);
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
                    ent.unseen_ticks = 1;
                }
            });
            Map::thin_soft_zone_excess(this);
            // Console /live dumps sit outside zone bookkeeping — under load,
            // delete the oldest admin-spawned heads so FOV piles cannot stick.
            if (ents_now >= 1800u || load_panic) {
                uint32_t budget = load_panic ? 24u : (ents_now >= 3200u ? 16u : 8u);
                for_each<kMob>([&](Simulation *sim, Entity &ent) {
                    if (budget == 0) return;
                    if (ent.pending_delete) return;
                    if (!BitMath::at(ent.flags, EntityFlags::kAdminSpawned)) return;
                    if (ent.has_component(kSegmented)) return;
                    BitMath::set(ent.flags, EntityFlags::kNoDrops);
                    sim->request_delete(ent.id);
                    --budget;
                });
            }
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

    {
        auto t_motion = std::chrono::steady_clock::now();
        tick_curse_behavior(this);
        for_each<kPhysics>(tick_entity_motion);
        for_each<kSegmented>(tick_segment_behavior);
        for_each<kCamera>(tick_camera_behavior);
        for_each<kScore>(tick_score_behavior);
        for_each_entity(entity_clear_references);
        calculate_leaderboard(this);
        Server::phase_motion_ms += _tick_ms_since(t_motion);
    }
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
