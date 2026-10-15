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
#include <Shared/Simulation.hh>
#include <Shared/StaticData.hh>

#include <algorithm>
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
    if (count > 1) {
        for (size_t i = 0; i < count; ++i) {
            Entity &drop = alloc_drop(sim, success_drops[i], team);
            drop.set_x(x);
            drop.set_y(y);
            drop.velocity.unit_normal(i * 2 * M_PI / count).set_magnitude(25);
        }
    } else if (count == 1) {
        Entity &drop = alloc_drop(sim, success_drops[0], team);
        drop.set_x(x);
        drop.set_y(y);
    }
}

// Weighted pick from mob drop table (chances sum to 1). Prefer non-Unique if Unique taken.
static PetalID::T _roll_mob_drop(Simulation *sim, MobID::T mob_id) {
    struct MobData const &mob_data = MOB_DATA[mob_id];
    StaticArray<float, MAX_DROPS_PER_MOB> const &chances = MOB_DROP_CHANCES[mob_id];
    if (mob_data.drops.size() == 0) return PetalID::kNone;
    for (uint32_t attempt = 0; attempt < 8; ++attempt) {
        float roll = frand();
        float acc = 0.f;
        PetalID::T chosen = mob_data.drops[0];
        for (uint32_t i = 0; i < mob_data.drops.size(); ++i) {
            acc += chances[i];
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
        } else if (ent.poison_ticks > 0) camera.set_killed_by("Poison");
        else camera.set_killed_by("");
    }
    if (ent.has_component(kMob)) {
        if (BitMath::at(ent.flags, EntityFlags::kSpawnedFromZone))
            Map::remove_mob(sim, ent.zone);
        if (!natural_despawn && !(BitMath::at(ent.flags, EntityFlags::kNoDrops))) {
            struct MobData const &mob_data = MOB_DATA[ent.get_mob_id()];
            std::vector<PetalID::T> success_drops = {};
            // Guaranteed first roll + rarity-scaled bonus rolls (up to 4 on Unique)
            PetalID::T chosen = _roll_mob_drop(sim, ent.get_mob_id());
            if (chosen != PetalID::kNone)
                success_drops.push_back(chosen);
            float p2 = 0.26f + 0.045f * (float) mob_data.rarity;
            if (success_drops.size() < MAX_DROPS_PER_MOB && frand() < p2) {
                PetalID::T extra = _roll_mob_drop(sim, ent.get_mob_id());
                if (extra != PetalID::kNone)
                    success_drops.push_back(extra);
            }
            if (mob_data.rarity >= RarityID::kEpic
                && success_drops.size() < MAX_DROPS_PER_MOB) {
                float p3 = 0.10f + 0.025f * (float) mob_data.rarity;
                if (frand() < p3) {
                    PetalID::T extra = _roll_mob_drop(sim, ent.get_mob_id());
                    if (extra != PetalID::kNone)
                        success_drops.push_back(extra);
                }
            }
            if (mob_data.rarity >= RarityID::kLegendary
                && success_drops.size() < MAX_DROPS_PER_MOB) {
                float p4 = 0.05f + 0.02f * (float) (mob_data.rarity - RarityID::kLegendary);
                if (frand() < p4) {
                    PetalID::T extra = _roll_mob_drop(sim, ent.get_mob_id());
                    if (extra != PetalID::kNone)
                        success_drops.push_back(extra);
                }
            }
            EntityID drop_team = NULL_ENTITY;
            if (sim->ent_exists(ent.last_damaged_by))
                drop_team = sim->get_ent(ent.last_damaged_by).get_team();
            _alloc_drops(sim, success_drops, ent.get_x(), ent.get_y(), drop_team);
            // Coins: award killer by mob rarity (Common ants → Unique bosses)
            if (sim->ent_exists(ent.last_damaged_by)) {
                EntityID killer_id = sim->get_ent(ent.last_damaged_by).base_entity;
                if (sim->ent_alive(killer_id)) {
                    Entity &killer = sim->get_ent(killer_id);
                    if (killer.has_component(kFlower) && sim->ent_alive(killer.get_parent())) {
                        Client *owner = Server::client_by_camera(sim, killer.get_parent());
                        if (owner != nullptr)
                            client_award_coins(owner, Craft::coins_from_mob(mob_data.rarity));
                    }
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
            if (level > MAX_LEVEL) level = MAX_LEVEL;
            for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i)
                PetalTracker::remove_petal(sim, camera.get_inventory(i));
            camera.set_respawn_level((uint8_t) level);
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
                AccountStore::capture(owner);
                Party::clear_corpse(owner);
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
        uint32_t respawn_level = div_round_up(3 * score_to_level(ent.get_score()), 4);
        if (respawn_level > MAX_LEVEL) respawn_level = MAX_LEVEL;
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

        // 1) Primary petals that still fit stay in primary slots
        for (uint32_t i = 0; i < ent.get_loadout_count() && i < primary_slots; ++i) {
            if (slot_petals[i] == PetalID::kNone) continue;
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
            AccountStore::capture(owner);
            Party::on_death(owner, death_x, death_y);
        }
    } else if (ent.has_component(kDrop)) {
        if (BitMath::at(ent.flags, EntityFlags::kIsDespawning))
            PetalTracker::remove_petal(sim, ent.get_drop_id());
    }
}