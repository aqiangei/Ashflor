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
            PetalID::T chosen = _roll_mob_drop(sim, ent.get_mob_id());
            if (chosen != PetalID::kNone)
                success_drops.push_back(chosen);
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
        if (ent.get_petal_id() == PetalID::kWeb || ent.get_petal_id() == PetalID::kTriweb)
            alloc_web(sim, 100, ent);
    } else if (ent.has_component(kFlower)) {
        // Preserve slot indices so backpack survives respawn (spawn reads
        // camera[0..loadout) = primary, camera[loadout..loadout+8) = bag).
        uint32_t const total_slots = ent.get_loadout_count() + MAX_SLOT_COUNT;
        std::vector<PetalID::T> slot_petals(total_slots, PetalID::kNone);
        std::vector<uint8_t> slot_keep(total_slots, 0);
        for (uint32_t i = 0; i < total_slots; ++i) {
            DEBUG_ONLY(assert(ent.get_loadout_ids(i) < PetalID::kNumPetals));
            PetalID::T id = ent.get_loadout_ids(i);
            PetalTracker::remove_petal(sim, id);
            slot_petals[i] = id;
            if (id != PetalID::kNone && id != PetalID::kBasic && frand() < 0.95)
                slot_keep[i] = 1;
        }
        // Deleted/overwritten petals: keep as extra bag candidates (append later)
        std::vector<PetalID::T> deleted_keep = {};
        for (uint32_t i = 0; i < ent.deleted_petals.size(); ++i) {
            DEBUG_ONLY(assert(ent.deleted_petals[i] < PetalID::kNumPetals));
            PetalTracker::remove_petal(sim, ent.deleted_petals[i]);
            if (ent.deleted_petals[i] != PetalID::kNone && ent.deleted_petals[i] != PetalID::kBasic && frand() < 0.95)
                deleted_keep.push_back(ent.deleted_petals[i]);
        }

        // Ground drops: up to 3 rarest kept petals (prefer higher rarity)
        std::vector<uint32_t> drop_candidates = {};
        for (uint32_t i = 0; i < total_slots; ++i)
            if (slot_keep[i]) drop_candidates.push_back(i);
        std::sort(drop_candidates.begin(), drop_candidates.end(), [&](uint32_t a, uint32_t b) {
            return PETAL_DATA[slot_petals[a]].rarity < PETAL_DATA[slot_petals[b]].rarity;
        });
        std::vector<PetalID::T> success_drops = {};
        uint32_t numDrops = drop_candidates.size();
        if (numDrops > 3) numDrops = 3;
        for (uint32_t n = 0; n < numDrops; ++n) {
            uint32_t slot = drop_candidates.back();
            drop_candidates.pop_back();
            PetalID::T p_id = slot_petals[slot];
            slot_keep[slot] = 0;
            if (PETAL_DATA[p_id].rarity >= RarityID::kRare && frand() < 0.05) p_id = PetalID::kPollen;
            success_drops.push_back(p_id);
        }
        // Also allow deleted petals to drop (same rarity sort, fill remaining)
        if (success_drops.size() < 3 && !deleted_keep.empty()) {
            std::sort(deleted_keep.begin(), deleted_keep.end(), [](PetalID::T a, PetalID::T b) {
                return PETAL_DATA[a].rarity < PETAL_DATA[b].rarity;
            });
            while (success_drops.size() < 3 && !deleted_keep.empty()) {
                PetalID::T p_id = deleted_keep.back();
                deleted_keep.pop_back();
                if (PETAL_DATA[p_id].rarity >= RarityID::kRare && frand() < 0.05) p_id = PetalID::kPollen;
                success_drops.push_back(p_id);
            }
        }
        _alloc_drops(sim, success_drops, ent.get_x(), ent.get_y(), ent.get_team());
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

        // 1) Kept primary petals that still fit stay in primary slots
        for (uint32_t i = 0; i < ent.get_loadout_count() && i < primary_slots; ++i) {
            if (!slot_keep[i]) continue;
            PetalTracker::add_petal(sim, slot_petals[i]);
            camera.set_inventory(i, slot_petals[i]);
        }
        // 2) Overflow primary + old backpack + leftover deleted → backpack (order preserved)
        for (uint32_t i = primary_slots; i < ent.get_loadout_count(); ++i)
            if (slot_keep[i]) put_in_backpack(slot_petals[i]);
        for (uint32_t i = ent.get_loadout_count(); i < total_slots; ++i)
            if (slot_keep[i]) put_in_backpack(slot_petals[i]);
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