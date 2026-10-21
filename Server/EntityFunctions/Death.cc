#include <Server/EntityFunctions.hh>

#include <Server/AccountStore.hh>
#include <Server/Chat.hh>
#include <Server/Client.hh>
#include <Server/Game.hh>
#include <Server/Party.hh>
#include <Server/PetalTracker.hh>
#include <Server/Server.hh>
#include <Server/Spawn.hh>

#include <Shared/Craft.hh>
#include <Shared/Entity.hh>
#include <Shared/Map.hh>
#include <Shared/RarityScale.hh>
#include <Shared/Recipe.hh>
#include <Shared/Simulation.hh>
#include <Shared/SpawnRules.hh>
#include <Shared/StaticData.hh>

#include <Helpers/Math.hh>

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <string>

static std::string announce_player_name(Client *client) {
    if (client == nullptr || client->game == nullptr) return "无名";
    Simulation *sim = &client->game->simulation;
    if (!sim->ent_exists(client->camera)) return "无名";
    Entity &camera = sim->get_ent(client->camera);
    if (sim->ent_alive(camera.get_player())) {
        std::string n = sim->get_ent(camera.get_player()).get_name();
        if (!n.empty()) return n;
    }
    if (!client->account_name.empty()) return client->account_name;
    if (!client->play_name.empty()) return client->play_name;
    return "无名";
}

static std::string announce_party_names(GameInstance *game, Client *mvp) {
    if (mvp == nullptr) return "无名";
    if (game == nullptr || mvp->party_id == 0)
        return announce_player_name(mvp);
    std::string out;
    for (Client *c : game->clients) {
        if (c == nullptr || c->party_id != mvp->party_id) continue;
        if (!out.empty()) out += "、";
        out += announce_player_name(c);
    }
    if (out.empty()) return announce_player_name(mvp);
    return out;
}

static void credit_boss_kill_stats(GameInstance *game, Client *mvp) {
    if (mvp == nullptr) return;
    if (game == nullptr || mvp->party_id == 0) {
        if (mvp->stat_kills < 4000000000u) ++mvp->stat_kills;
        return;
    }
    for (Client *c : game->clients) {
        if (c == nullptr || c->party_id != mvp->party_id) continue;
        if (c->stat_kills < 4000000000u) ++c->stat_kills;
    }
}

void flush_flower_loadout_to_camera(Simulation *sim, Entity &camera, Entity const &flower) {
    if (sim == nullptr || !flower.has_component(kFlower)) return;
    uint32_t const total_slots = flower.get_loadout_count() + MAX_SLOT_COUNT;
    uint32_t level = score_to_level(flower.get_score());
    if (level < 1) level = 1;
    if (level < camera.get_respawn_level())
        level = camera.get_respawn_level();
    for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i)
        PetalTracker::remove_petal(sim, camera.get_inventory(i));
    camera.set_respawn_level(level);
    for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i)
        camera.set_inventory(i, PetalID::kNone);
    for (uint32_t i = 0; i < total_slots && i < 2 * MAX_SLOT_COUNT; ++i) {
        PetalID::T id = flower.get_loadout_ids(i);
        if (id == PetalID::kNone || id >= PetalID::kNumPetals) continue;
        camera.set_inventory(i, id);
        PetalTracker::add_petal(sim, id);
    }
}

static void _alloc_drops(Simulation *sim, std::vector<PetalID::T> &success_drops, float x, float y, EntityID team) {
    #ifdef DEBUG
    for (PetalID::T id : success_drops)
        assert(id != PetalID::kNone && id < PetalID::kNumPetals);
    #endif
    size_t count = success_drops.size();
    for (size_t i = count; i > 0; --i) {
        PetalID::T drop_id = success_drops[i - 1];
        if (PETAL_DATA[drop_id].rarity == RarityID::kUnique && PetalTracker::get_count(sim, drop_id) > 0) {
            success_drops[i - 1] = success_drops[count - 1];
            --count;
            success_drops.pop_back();
        }
    }
    DEBUG_ONLY(assert(success_drops.size() == count);)
    // Coalesce identical ids into stacked piles (bags mats / multi-rolls).
    std::array<uint16_t, PetalID::kNumPetals> piles = {};
    std::vector<PetalID::T> order;
    order.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        PetalID::T id = success_drops[i];
        if (id == PetalID::kNone || id >= PetalID::kNumPetals) continue;
        if (piles[id] == 0) order.push_back(id);
        if (piles[id] < 65535) ++piles[id];
    }
    size_t n = order.size();
    for (size_t i = 0; i < n; ++i) {
        PetalID::T id = order[i];
        Entity &drop = alloc_drop(sim, id, team, piles[id]);
        float dx = x, dy = y;
        if (n > 1) {
            float ang = (float) (i * 2 * M_PI / n);
            dx += std::cos(ang) * 28.f;
            dy += std::sin(ang) * 28.f;
            drop.velocity.unit_normal(ang).set_magnitude(25);
        }
        Map::resolve_circle_terrain(sim->map_id, dx, dy, drop.get_radius() + 8.f);
        dx = fclamp(dx, drop.get_radius(), ARENA_WIDTH - drop.get_radius());
        dy = fclamp(dy, drop.get_radius(), ARENA_HEIGHT - drop.get_radius());
        drop.set_x(dx);
        drop.set_y(dy);
    }
}

// Weighted pick from mob drop table (chances sum to 1). Prefer non-Unique if Unique taken.
// rarity_boost > 0 tilts the roll toward rarer petals (per-zone difficulty reward).
static PetalID::T _roll_mob_drop(Simulation *sim, MobID::T mob_id, float rarity_boost) {
    struct MobData const &mob_data = MOB_DATA[mob_id];
    StaticArray<float, MAX_DROPS_PER_MOB> const &chances = MOB_DROP_CHANCES[mob_id];
    if (mob_data.drops.size() == 0) return PetalID::kNone;
    float weights[MAX_DROPS_PER_MOB] = {};
    float total_weight = 0.f;
    for (uint32_t i = 0; i < mob_data.drops.size(); ++i) {
        float w = chances[i];
        PetalID::T drop_id = mob_data.drops[i];
        w *= Craft::drop_weight_mult(drop_id);
        if (rarity_boost > 0.f) {
            uint8_t r = PETAL_DATA[drop_id].rarity;
            // Zone tilt must not inflate Unique+ (Moon/Ygg) toward a first-kill guarantee.
            if (r < RarityID::kUnique)
                w *= std::pow(1.f + rarity_boost, (float)r);
        }
        weights[i] = w;
        total_weight += w;
    }
    if (total_weight <= 0.f) return mob_data.drops[0];
    for (uint32_t attempt = 0; attempt < 8; ++attempt) {
        float roll = frand() * total_weight;
        float acc = 0.f;
        PetalID::T chosen = mob_data.drops[0];
        for (uint32_t i = 0; i < mob_data.drops.size(); ++i) {
            acc += weights[i];
            chosen = mob_data.drops[i];
            if (roll < acc) break;
        }
        if (PETAL_DATA[chosen].rarity != RarityID::kUnique
            || PetalTracker::get_count(sim, chosen) == 0)
            return chosen;
    }
    // Fallback: first non-unique (or first drop)
    for (PetalID::T id : mob_data.drops)
        if (PETAL_DATA[id].rarity != RarityID::kUnique)
            return id;
    return mob_data.drops[0];
}

// Mob XP only (no PvP score-steal). Alive party mates of the killer each get
// a full copy of the mob's XP.
static void _add_mob_xp(Simulation *sim, EntityID const killer_id, Entity const &mob) {
    if (!mob.has_component(kMob)) return;
    uint32_t const xp = mob.score_reward;
    if (xp == 0 || !sim->ent_exists(killer_id)) return;
    Entity &killer = sim->get_ent(killer_id);
    if (!killer.has_component(kScore)) return;

    if (killer.has_component(kFlower) && sim->ent_alive(killer.get_parent())) {
        Client *kc = Server::client_by_camera(sim, killer.get_parent());
        if (kc != nullptr && kc->party_id != 0 && kc->game != nullptr) {
            uint8_t any = 0;
            for (Client *c : kc->game->clients) {
                if (c == nullptr || c->party_id != kc->party_id) continue;
                if (!c->alive() || c->game == nullptr) continue;
                if (&c->game->simulation != sim) continue;
                if (!sim->ent_exists(c->camera)) continue;
                Entity &cam = sim->get_ent(c->camera);
                if (!sim->ent_alive(cam.get_player())) continue;
                Entity &flower = sim->get_ent(cam.get_player());
                if (!flower.has_component(kScore)) continue;
                flower.set_score(flower.get_score() + xp);
                any = 1;
            }
            if (any) return;
        }
    }
    killer.set_score(killer.get_score() + xp);
}

void entity_on_death(Simulation *sim, Entity const &ent) {
    //don't do on_death for any despawned entity
    uint8_t natural_despawn = BitMath::at(ent.flags, EntityFlags::kIsDespawning) && ent.despawn_tick == 0;
    // XP from mobs only — never from flower death (old PvP score-steal removed).
    if (ent.has_component(kMob) && ent.score_reward > 0
        && sim->ent_exists(ent.last_damaged_by) && !natural_despawn) {
        EntityID killer_id = sim->get_ent(ent.last_damaged_by).base_entity;
        _add_mob_xp(sim, killer_id, ent);
    }
    if (ent.has_component(kFlower) && sim->ent_alive(ent.get_parent())) {
        Entity &camera = sim->get_ent(ent.get_parent());
        EntityID killer_id = sim->ent_exists(ent.last_damaged_by) ?
            sim->get_ent(ent.last_damaged_by).base_entity : NULL_ENTITY;
        if (sim->ent_alive(killer_id)) {
            Entity const &killer = sim->get_ent(killer_id);
            if (killer.has_component(kName)) camera.set_killed_by(killer.get_name());
            else camera.set_killed_by("");
            // No PvP kill counter — players cannot kill each other.
        } else if (ent.poison_ticks > 0) camera.set_killed_by("Poison");
        else camera.set_killed_by("");
    }
    if (ent.has_component(kMob)) {
        // Credit this-life mob kill to the damaging flower (heads only).
        if (!natural_despawn
            && !ent.has_component(kSegmented)
            && sim->ent_exists(ent.last_damaged_by)) {
            EntityID killer_id = sim->get_ent(ent.last_damaged_by).base_entity;
            if (sim->ent_alive(killer_id)) {
                Entity &killer = sim->get_ent(killer_id);
                if (killer.has_component(kFlower) && sim->ent_alive(killer.get_parent())) {
                    Client *owner = Server::client_by_camera(sim, killer.get_parent());
                    if (owner != nullptr)
                        client_credit_mob_kill(owner, ent.get_mob_id());
                }
            }
        }
        if (BitMath::at(ent.flags, EntityFlags::kSpawnedFromZone))
            Map::remove_mob(sim, ent);
        if (!natural_despawn && !(BitMath::at(ent.flags, EntityFlags::kNoDrops))) {
            struct MobData const &mob_data = MOB_DATA[ent.get_mob_id()];
            uint8_t const live_rarity = ent.get_rarity() < RarityID::kNumRarities
                ? ent.get_rarity() : mob_data.rarity;
            float rarity_boost = 0.f;
            {
                MapZones const &zones = map_zones(sim->map_id);
                if (ent.zone < zones.size())
                    rarity_boost = zones[ent.zone].drop_multiplier;
                else if (live_rarity >= RarityID::kEpic)
                    rarity_boost = 0.4f;
            }

            // Private loot: need ≥5% max HP damage (≥1% for bosses). Totals persist
            // across player respawns. Same-party claims coalesce. Party XP in _add_score.
            EntityID claim_teams[16];
            float claim_dmg[16];
            uint8_t claim_n = 0;
            {
                float const max_hp = ent.max_health > 1.f ? ent.max_health : 1.f;
                uint8_t const boss = spawn_class(ent.get_mob_id()) == SpawnClass::kBoss;
                float const need = max_hp * (boss ? 0.01f : 0.05f);
                uint8_t raw_n = ent.loot_claim_n;
                if (raw_n > 16) raw_n = 16;
                for (uint8_t i = 0; i < raw_n; ++i) {
                    if (ent.loot_claim_teams[i].null()) continue;
                    if (ent.loot_claim_damage[i] + 1e-3f < need) continue;
                    claim_teams[claim_n] = ent.loot_claim_teams[i];
                    claim_dmg[claim_n] = ent.loot_claim_damage[i];
                    ++claim_n;
                }
            }
            // Merge claim slots that belong to the same party (stale team ids after invite).
            GameInstance *game = Server::game_for(sim);
            auto client_for_team = [&](EntityID team) -> Client * {
                if (game == nullptr || team.null()) return nullptr;
                Client *c = Server::client_by_camera(sim, team);
                if (c != nullptr) return c;
                for (Client *x : game->clients) {
                    if (x == nullptr || x->game == nullptr) continue;
                    if (&x->game->simulation != sim) continue;
                    if (!sim->ent_exists(x->camera)) continue;
                    if (sim->get_ent(x->camera).get_team() == team) return x;
                }
                return nullptr;
            };
            for (uint8_t i = 0; i < claim_n; ++i) {
                Client *a = client_for_team(claim_teams[i]);
                for (uint8_t j = i + 1; j < claim_n; ) {
                    Client *b = client_for_team(claim_teams[j]);
                    if (Party::same_party(a, b)) {
                        claim_dmg[i] += claim_dmg[j];
                        claim_teams[j] = claim_teams[claim_n - 1];
                        claim_dmg[j] = claim_dmg[claim_n - 1];
                        --claim_n;
                    } else {
                        ++j;
                    }
                }
            }

            // Boss：全服播报造成最高伤害的小队/个人，排行榜击杀同步计入
            if (spawn_class(ent.get_mob_id()) == SpawnClass::kBoss
                && !ent.has_component(kSegmented)) {
                Client *mvp = nullptr;
                if (claim_n > 0) {
                    uint8_t bi = 0;
                    for (uint8_t i = 1; i < claim_n; ++i) {
                        if (claim_dmg[i] > claim_dmg[bi]) bi = i;
                    }
                    mvp = client_for_team(claim_teams[bi]);
                }
                if (mvp == nullptr && sim->ent_exists(ent.last_damaged_by)) {
                    EntityID kid = sim->get_ent(ent.last_damaged_by).base_entity;
                    if (sim->ent_alive(kid)) {
                        Entity &killer = sim->get_ent(kid);
                        if (killer.has_component(kFlower) && sim->ent_alive(killer.get_parent()))
                            mvp = Server::client_by_camera(sim, killer.get_parent());
                    }
                }
                if (mvp != nullptr) {
                    credit_boss_kill_stats(game, mvp);
                    char const *mob_nm = MOB_DATA[ent.get_mob_id()].name;
                    if (mob_nm == nullptr || mob_nm[0] == 0) mob_nm = "Boss";
                    std::string who = announce_party_names(game, mvp);
                    uint8_t is_party = mvp->party_id != 0;
                    std::string text = is_party
                        ? ("小队「" + who + "」合力击败了 " + mob_nm + "！")
                        : (who + " 击败了 " + mob_nm + "！");
                    Chat::broadcast_server("系统", text);
                }
            }

            auto team_has_bonus = [&](EntityID team) -> uint8_t {
                GameInstance *game = Server::game_for(sim);
                if (game == nullptr) return 0;
                for (Client *c : game->clients) {
                    if (c == nullptr || c->game == nullptr) continue;
                    if (&c->game->simulation != sim) continue;
                    if (!sim->ent_exists(c->camera)) continue;
                    if (!(sim->get_ent(c->camera).get_team() == team)) continue;
                    if (!c->account_name.empty() && c->bonus_enabled
                        && (c->daily_bonus_seconds || c->offline_bonus_seconds))
                        return 1;
                }
                return 0;
            };

            auto roll_for_team = [&](EntityID drop_team, float angle_off) {
                std::vector<PetalID::T> success_drops = {};
                PetalID::T chosen = _roll_mob_drop(sim, ent.get_mob_id(), rarity_boost);
                if (chosen != PetalID::kNone)
                    success_drops.push_back(chosen);
                float p2 = 0.26f + 0.045f * (float) live_rarity;
                if (success_drops.size() < MAX_DROPS_PER_MOB && frand() < p2) {
                    PetalID::T extra = _roll_mob_drop(sim, ent.get_mob_id(), rarity_boost);
                    if (extra != PetalID::kNone)
                        success_drops.push_back(extra);
                }
                if (live_rarity >= RarityID::kEpic
                    && success_drops.size() < MAX_DROPS_PER_MOB) {
                    float p3 = 0.10f + 0.025f * (float) live_rarity;
                    if (frand() < p3) {
                        PetalID::T extra = _roll_mob_drop(sim, ent.get_mob_id(), rarity_boost);
                        if (extra != PetalID::kNone)
                            success_drops.push_back(extra);
                    }
                }
                if (live_rarity >= RarityID::kLegendary
                    && success_drops.size() < MAX_DROPS_PER_MOB) {
                    float p4 = 0.05f + 0.02f * (float) (live_rarity - RarityID::kLegendary);
                    if (frand() < p4) {
                        PetalID::T extra = _roll_mob_drop(sim, ent.get_mob_id(), rarity_boost);
                        if (extra != PetalID::kNone)
                            success_drops.push_back(extra);
                    }
                }
                if (team_has_bonus(drop_team)) {
                    size_t bonus_rolls = std::min<size_t>(success_drops.size() * 2, 12);
                    for (size_t i = 0; i < bonus_rolls; ++i) {
                        PetalID::T extra = _roll_mob_drop(sim, ent.get_mob_id(), rarity_boost);
                        if (extra != PetalID::kNone
                            && PETAL_DATA[extra].rarity != RarityID::kUnique)
                            success_drops.push_back(extra);
                    }
                }
                float ox = ent.get_x() + std::cos(angle_off) * 12.f;
                float oy = ent.get_y() + std::sin(angle_off) * 12.f;
                _alloc_drops(sim, success_drops, ox, oy, drop_team);
                uint32_t coins = Craft::coins_from_mob(live_rarity);
                {
                    MapZones const &zones = map_zones(sim->map_id);
                    uint32_t diff = 0;
                    if (ent.zone < zones.size())
                        diff = zones[ent.zone].difficulty;
                    coins = (uint32_t) (coins * zone_coin_mult(diff)
                        * mob_spawn_threat_mult(mob_data.rarity, live_rarity) + 0.5f);
                    if (coins < 1) coins = 1;
                }
                if (coins > 0) {
                    uint16_t pile = coins > 65535 ? (uint16_t) 65535 : (uint16_t) coins;
                    Entity &drop = alloc_drop(sim, PetalID::kCoin, drop_team, pile);
                    float cx = ox, cy = oy;
                    Map::resolve_circle_terrain(sim->map_id, cx, cy, drop.get_radius() + 8.f);
                    cx = fclamp(cx, drop.get_radius(), ARENA_WIDTH - drop.get_radius());
                    cy = fclamp(cy, drop.get_radius(), ARENA_HEIGHT - drop.get_radius());
                    drop.set_x(cx);
                    drop.set_y(cy);
                    drop.velocity.unit_normal(frand() * 2 * (float) M_PI)
                        .set_magnitude(18 + frand() * 20);
                }
            };

            if (claim_n == 0) {
                // No player credit — unowned brief lock then FFA (rare edge).
                roll_for_team(NULL_ENTITY, 0.f);
            } else {
                for (uint8_t i = 0; i < claim_n; ++i) {
                    float ang = (claim_n == 1) ? 0.f
                        : (float) i * (2.f * (float) M_PI / (float) claim_n);
                    roll_for_team(claim_teams[i], ang);
                }
            }
        }
        if (ent.get_mob_id() == MobID::kAntHole && 
            BitMath::at(ent.flags, EntityFlags::kSpawnedFromZone) && 
            frand() < DIGGER_SPAWN_CHANCE) { 
            EntityID team = NULL_ENTITY;
            if (sim->ent_exists(ent.last_damaged_by))
                team = sim->get_ent(ent.last_damaged_by).get_team();
            alloc_mob(sim, MobID::kDigger, ent.get_x(), ent.get_y(), team);
        }

    } else if (ent.has_component(kPetal)) {
        if (petal_name_is(ent.get_petal_id(), "Web"))
            alloc_web(sim, 100, ent);
    } else if (ent.has_component(kFlower)) {
        // Peaceful title leave (kNoDrops): full inventory keep, no ground drops / corpse
        if (BitMath::at(ent.flags, EntityFlags::kNoDrops) && sim->ent_exists(ent.get_parent())) {
            Entity &camera = sim->get_ent(ent.get_parent());
            // Drop petal tracker refs from the flower slots (camera already holds them).
            uint32_t const total_slots = ent.get_loadout_count() + MAX_SLOT_COUNT;
            for (uint32_t i = 0; i < total_slots && i < 2 * MAX_SLOT_COUNT; ++i)
                PetalTracker::remove_petal(sim, ent.get_loadout_ids(i));
            flush_flower_loadout_to_camera(sim, camera, ent);
            // Fill empty primary with Basics only for title-leave respawn layout.
            uint32_t primary = loadout_slots_at_level(camera.get_respawn_level());
            for (uint32_t i = 0; i < primary; ++i) {
                if (camera.get_inventory(i) != PetalID::kNone) continue;
                camera.set_inventory(i, PetalID::kBasic);
                PetalTracker::add_petal(sim, PetalID::kBasic);
            }
            camera.set_killed_by("");
            if (Client *owner = Server::client_by_camera(sim, camera.id)) {
                AccountStore::capture_nosave(owner);
                Party::clear_corpse(owner);
                owner->has_death_pos = 0;
                owner->play_name.clear();
            }
            return;
        }
        // No ground drops on player death — keep full loadout + bag.
        // Slot layout preserved for respawn (camera[0..loadout)=primary, rest=bag).
        uint32_t const total_slots = ent.get_loadout_count() + MAX_SLOT_COUNT;
        std::vector<PetalID::T> slot_petals(total_slots, PetalID::kNone);
        for (uint32_t i = 0; i < total_slots; ++i) {
            DEBUG_ONLY(assert(ent.get_loadout_ids(i) < PetalID::kNumPetals));
            PetalID::T id = ent.get_loadout_ids(i);
            PetalTracker::remove_petal(sim, id);
            slot_petals[i] = id;
        }
        std::vector<PetalID::T> deleted_keep = {};
        for (uint32_t i = 0; i < ent.deleted_petals.size(); ++i) {
            DEBUG_ONLY(assert(ent.deleted_petals[i] < PetalID::kNumPetals));
            PetalTracker::remove_petal(sim, ent.deleted_petals[i]);
            if (ent.deleted_petals[i] != PetalID::kNone && ent.deleted_petals[i] != PetalID::kBasic)
                deleted_keep.push_back(ent.deleted_petals[i]);
        }
        if (!sim->ent_alive(ent.get_parent()))
            return;
        Entity &camera = sim->get_ent(ent.get_parent());
        float death_x = ent.get_x();
        float death_y = ent.get_y();
        // Level never regresses on death — keep the floor reached this life.
        uint32_t respawn_level = score_to_level(ent.get_score());
        if (respawn_level < 1) respawn_level = 1;
        if (respawn_level < camera.get_respawn_level())
            respawn_level = camera.get_respawn_level();
        camera.set_respawn_level(respawn_level);
        uint32_t primary_slots = loadout_slots_at_level(respawn_level);

        for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i)
            camera.set_inventory(i, PetalID::kNone);

        auto put_in_backpack = [&](PetalID::T id) {
            if (id == PetalID::kNone) return;
            uint32_t dest = primary_slots;
            while (dest < primary_slots + MAX_SLOT_COUNT
                && dest < 2 * MAX_SLOT_COUNT
                && camera.get_inventory(dest) != PetalID::kNone)
                ++dest;
            if (dest >= primary_slots + MAX_SLOT_COUNT || dest >= 2 * MAX_SLOT_COUNT)
                return;
            PetalTracker::add_petal(sim, id);
            camera.set_inventory(dest, id);
        };

        // 1) Primary petals that still fit stay in primary (bag-only → backpack)
        for (uint32_t i = 0; i < ent.get_loadout_count() && i < primary_slots; ++i) {
            if (slot_petals[i] == PetalID::kNone) continue;
            if (Recipe::is_bag_only(slot_petals[i])) {
                put_in_backpack(slot_petals[i]);
                continue;
            }
            PetalTracker::add_petal(sim, slot_petals[i]);
            camera.set_inventory(i, slot_petals[i]);
        }
        // 2) Overflow primary + backpack + deleted → backpack (order preserved)
        for (uint32_t i = primary_slots; i < ent.get_loadout_count(); ++i)
            put_in_backpack(slot_petals[i]);
        for (uint32_t i = ent.get_loadout_count(); i < total_slots; ++i)
            put_in_backpack(slot_petals[i]);
        for (PetalID::T id : deleted_keep)
            put_in_backpack(id);
        // 3) Fill empty primary with Basics
        for (uint32_t i = 0; i < primary_slots; ++i) {
            if (camera.get_inventory(i) != PetalID::kNone) continue;
            PetalTracker::add_petal(sim, PetalID::kBasic);
            camera.set_inventory(i, PetalID::kBasic);
        }
        if (Client *owner = Server::client_by_camera(sim, camera.id)) {
            // Nosave: sync write of accounts.dat on death stalls the whole server tick
            AccountStore::capture_nosave(owner);
            client_send_life_summary(owner);
            Party::on_death(owner, death_x, death_y);
        }
    } else if (ent.has_component(kDrop)) {
        if (BitMath::at(ent.flags, EntityFlags::kIsDespawning))
            PetalTracker::remove_petal(sim, ent.get_drop_id());
    }
}
