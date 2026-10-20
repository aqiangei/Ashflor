#include <Server/EntityFunctions.hh>

#include <Server/AccountStore.hh>
#include <Server/Client.hh>
#include <Server/Game.hh>
#include <Server/Party.hh>
#include <Server/PetalTracker.hh>
#include <Server/Server.hh>
#include <Server/Spawn.hh>

#include <Shared/Craft.hh>
#include <Shared/Entity.hh>
#include <Shared/Map.hh>
#include <Shared/Recipe.hh>
#include <Shared/Simulation.hh>
#include <Shared/StaticData.hh>

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>

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
        drop.set_x(x);
        drop.set_y(y);
        if (n > 1)
            drop.velocity.unit_normal(i * 2 * M_PI / n).set_magnitude(25);
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
        if (rarity_boost > 0.f) {
            uint8_t r = PETAL_DATA[mob_data.drops[i]].rarity;
            if (r < RarityID::kNumRarities)
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

static void _add_score(Simulation *sim, EntityID const killer_id, Entity const &target) {
    if (!sim->ent_exists(killer_id)) return;
    Entity &killer = sim->get_ent(killer_id);
    if (killer.has_component(kScore))
        killer.set_score(killer.get_score() + target.score_reward);
}

void entity_on_death(Simulation *sim, Entity const &ent) {
    //don't do on_death for any despawned entity
    uint8_t natural_despawn = BitMath::at(ent.flags, EntityFlags::kIsDespawning) && ent.despawn_tick == 0;
    if (ent.score_reward > 0 && sim->ent_exists(ent.last_damaged_by) && !natural_despawn) {
        EntityID killer_id = sim->get_ent(ent.last_damaged_by).base_entity;
        _add_score(sim, killer_id, ent);
    }
    if (ent.has_component(kFlower) && sim->ent_alive(ent.get_parent())) {
        Entity &camera = sim->get_ent(ent.get_parent());
        EntityID killer_id = sim->ent_exists(ent.last_damaged_by) ?
            sim->get_ent(ent.last_damaged_by).base_entity : NULL_ENTITY;
        if (sim->ent_alive(killer_id)) {
            Entity const &killer = sim->get_ent(killer_id);
            if (killer.has_component(kName)) camera.set_killed_by(killer.get_name());
            else camera.set_killed_by("");
            // Credit the kill to the killer's client (leaderboard board counter)
            EntityID killer_camera = killer.get_parent();
            if (sim->ent_exists(killer_camera)) {
                Client *killer_client = Server::client_by_camera(sim, killer_camera);
                if (killer_client != nullptr && killer_client->stat_kills < 4000000000u)
                    ++killer_client->stat_kills;
            }
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
            float rarity_boost = 0.f;
            {
                MapZones const &zones = map_zones(sim->map_id);
                if (ent.zone < zones.size())
                    rarity_boost = zones[ent.zone].drop_multiplier;
                else if (mob_data.rarity >= RarityID::kEpic)
                    rarity_boost = 0.4f;
            }

            // Private loot units: each damaging player (or party) settles their own roll.
            EntityID claim_teams[16];
            uint8_t claim_n = ent.loot_claim_n;
            if (claim_n > 16) claim_n = 16;
            for (uint8_t i = 0; i < claim_n; ++i)
                claim_teams[i] = ent.loot_claim_teams[i];
            if (claim_n == 0 && sim->ent_exists(ent.last_damaged_by)) {
                Entity &atk = sim->get_ent(ent.last_damaged_by);
                EntityID flower_id = atk.has_component(kFlower) ? ent.last_damaged_by
                    : (atk.has_component(kPetal) && sim->ent_alive(atk.get_parent())
                        ? atk.get_parent() : NULL_ENTITY);
                if (sim->ent_alive(flower_id)) {
                    EntityID team = sim->get_ent(flower_id).get_team();
                    if (!team.null()) {
                        claim_teams[0] = team;
                        claim_n = 1;
                    }
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
                float p2 = 0.26f + 0.045f * (float) mob_data.rarity;
                if (success_drops.size() < MAX_DROPS_PER_MOB && frand() < p2) {
                    PetalID::T extra = _roll_mob_drop(sim, ent.get_mob_id(), rarity_boost);
                    if (extra != PetalID::kNone)
                        success_drops.push_back(extra);
                }
                if (mob_data.rarity >= RarityID::kEpic
                    && success_drops.size() < MAX_DROPS_PER_MOB) {
                    float p3 = 0.10f + 0.025f * (float) mob_data.rarity;
                    if (frand() < p3) {
                        PetalID::T extra = _roll_mob_drop(sim, ent.get_mob_id(), rarity_boost);
                        if (extra != PetalID::kNone)
                            success_drops.push_back(extra);
                    }
                }
                if (mob_data.rarity >= RarityID::kLegendary
                    && success_drops.size() < MAX_DROPS_PER_MOB) {
                    float p4 = 0.05f + 0.02f * (float) (mob_data.rarity - RarityID::kLegendary);
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
                uint32_t coins = Craft::coins_from_mob(mob_data.rarity);
                if (coins > 0) {
                    uint16_t pile = coins > 65535 ? (uint16_t) 65535 : (uint16_t) coins;
                    Entity &drop = alloc_drop(sim, PetalID::kCoin, drop_team, pile);
                    drop.set_x(ox);
                    drop.set_y(oy);
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
        if (BitMath::at(ent.flags, EntityFlags::kNoDrops) && sim->ent_alive(ent.get_parent())) {
            Entity &camera = sim->get_ent(ent.get_parent());
            uint32_t const total_slots = ent.get_loadout_count() + MAX_SLOT_COUNT;
            uint32_t level = score_to_level(ent.get_score());
            if (level < 1) level = 1;
            if (level < camera.get_respawn_level())
                level = camera.get_respawn_level();
            for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i)
                PetalTracker::remove_petal(sim, camera.get_inventory(i));
            camera.set_respawn_level(level);
            for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i)
                camera.set_inventory(i, PetalID::kNone);
            for (uint32_t i = 0; i < total_slots && i < 2 * MAX_SLOT_COUNT; ++i) {
                PetalID::T id = ent.get_loadout_ids(i);
                PetalTracker::remove_petal(sim, id);
                if (id == PetalID::kNone || id >= PetalID::kNumPetals) continue;
                camera.set_inventory(i, id);
                PetalTracker::add_petal(sim, id);
            }
            uint32_t primary = loadout_slots_at_level(level);
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
