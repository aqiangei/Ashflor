#include <Server/EntityFunctions.hh>
#include <Server/PetalMechanics.hh>

#include <Shared/Map.hh>
#include <Shared/Simulation.hh>
#include <Shared/Entity.hh>
#include <Shared/StaticData.hh>

#include <Helpers/Bits.hh>
#include <Helpers/Vector.hh>

#include <cmath>

static bool _is_controlled_flower(Simulation *sim, Entity const &ent) {
    if (!ent.has_component(kFlower)) return false;
    if (!sim->ent_alive(ent.get_parent())) return false;
    return sim->get_ent(ent.get_parent()).has_component(kCamera);
}

bool is_player_faction(Simulation *sim, Entity const &ent) {
    if (_is_controlled_flower(sim, ent)) return true;
    if (ent.has_component(kPetal) && sim->ent_alive(ent.get_parent()))
        return _is_controlled_flower(sim, sim->get_ent(ent.get_parent()));
    // Egg / summon: base_entity points at owning flower
    if (!(ent.base_entity == ent.id) && sim->ent_alive(ent.base_entity)
        && _is_controlled_flower(sim, sim->get_ent(ent.base_entity)))
        return true;
    if (sim->ent_alive(ent.get_parent())) {
        Entity const &p = sim->get_ent(ent.get_parent());
        if (_is_controlled_flower(sim, p)) return true;
        if (p.has_component(kPetal) && sim->ent_alive(p.get_parent()))
            return _is_controlled_flower(sim, sim->get_ent(p.get_parent()));
    }
    return false;
}

static Entity const *_controlled_flower_of(Simulation *sim, Entity const &ent) {
    if (_is_controlled_flower(sim, ent)) return &ent;
    if (ent.has_component(kPetal) && sim->ent_alive(ent.get_parent())) {
        Entity const &p = sim->get_ent(ent.get_parent());
        if (_is_controlled_flower(sim, p)) return &p;
    }
    if (!(ent.base_entity == ent.id) && sim->ent_alive(ent.base_entity)) {
        Entity const &b = sim->get_ent(ent.base_entity);
        if (_is_controlled_flower(sim, b)) return &b;
    }
    if (sim->ent_alive(ent.get_parent())) {
        Entity const &p = sim->get_ent(ent.get_parent());
        if (_is_controlled_flower(sim, p)) return &p;
        if (p.has_component(kPetal) && sim->ent_alive(p.get_parent())) {
            Entity const &fp = sim->get_ent(p.get_parent());
            if (_is_controlled_flower(sim, fp)) return &fp;
        }
    }
    return nullptr;
}

bool flower_is_corrupted(Simulation *sim, Entity const &ent) {
    Entity const *flower = _controlled_flower_of(sim, ent);
    if (flower == nullptr) return false;
    if (flower->corruption_ticks > 0) return true;
    uint32_t n = flower->get_loadout_count();
    for (uint32_t i = 0; i < n; ++i) {
        // Resolve Mimic / behavior id — raw slot id missed Corruption copies.
        PetalID::T id = effective_petal_id(*flower, i);
        if (id == PetalID::kNone || id >= PetalID::kNumPetals) continue;
        if (PETAL_DATA[id].attributes.corruption) return true;
    }
    return false;
}

bool player_faction_pvp(Simulation *sim, Entity const &a, Entity const &b) {
    if (!is_player_faction(sim, a) || !is_player_faction(sim, b)) return false;
    return flower_is_corrupted(sim, a) || flower_is_corrupted(sim, b);
}

float flower_aggro_range_mult(Entity const &flower) {
    if (!flower.has_component(kFlower) || flower.has_component(kMob))
        return 1.f;
    float shrink = 0.f;
    for (uint32_t si = 0; si < flower.get_loadout_count(); ++si) {
        PetalID::T pid = effective_petal_id(flower, si);
        if (pid == PetalID::kNone || pid >= PetalID::kNumPetals) continue;
        float s = PETAL_DATA[pid].attributes.aggro_shrink;
        if (s > shrink) shrink = s;
    }
    if (shrink <= 0.f) return 1.f;
    return fmaxf(0.05f, 1.f - fminf(0.95f, shrink));
}

EntityID find_nearest_enemy(Simulation *simulation, Entity const &entity, float radius) {
    if ((entity.id.id - entity.lifetime) % (TPS / 5) != 0) return NULL_ENTITY;
    if (entity.immunity_ticks > 0) return NULL_ENTITY;
    EntityID ret;
    float min_dist = radius;
    bool const seeker_player = is_player_faction(simulation, entity);
    simulation->spatial_hash.query(entity.get_x(), entity.get_y(), radius, radius, [&](Simulation *sim, Entity &ent){
        if (!sim->ent_alive(ent.id)) return;
        // Auto-party shares one team id — Corruption PvP must still let summons
        // chase enemy flowers ("腐化时没被别人的召唤物攻击").
        if (ent.get_team() == entity.get_team()) {
            if (!(seeker_player && is_player_faction(sim, ent)
                  && player_faction_pvp(simulation, entity, ent)))
                return;
        }
        if (ent.immunity_ticks > 0) return;
        // Thrown honey uses kIsDetached + kIsDespawning (despawn timer).
        uint8_t const is_honey_bait = ent.has_component(kPetal)
            && petal_is_honey(ent.get_petal_id())
            && (BitMath::at(ent.flags, EntityFlags::kIsDetached)
                || BitMath::at(ent.flags, EntityFlags::kIsDespawning));
        if (!ent.has_component(kMob) && !ent.has_component(kFlower) && !is_honey_bait)
            return;
        // Party revive corpses are Flower visuals only — never aggro bait.
        if (ent.has_component(kFlower) && !ent.has_component(kHealth))
            return;
        // Practice dummies are not prey.
        if (ent.has_component(kMob) && ent.get_mob_id() == MobID::kTrainingDummy)
            return;
        // Never aggro own flower / own summons (Corruption PvP still opens
        // cross-player fights; "召唤物逮着我咬" was same-owner chase).
        if (seeker_player) {
            Entity const *home = _controlled_flower_of(simulation, entity);
            Entity const *prey_home = _controlled_flower_of(sim, ent);
            if (home != nullptr && prey_home != nullptr && home->id == prey_home->id)
                return;
        }
        // Players / bots / their summons never aggro other player-faction flowers
        // unless Corruption opens PvP.
        if (seeker_player && is_player_faction(sim, ent)
            && !player_faction_pvp(simulation, entity, ent)) return;
        // Hub respawn pad only: wild mobs never acquire aggro on players inside
        // (players may still attack). Larger plaza / roads are not sanctuaries.
        if (!seeker_player && is_player_faction(sim, ent)
            && Map::in_spawn_sanctuary(ent.get_x(), ent.get_y()))
            return;
        // Hatch pets: leash aggro to the owning flower, not an egg petal.
        EntityID home_id = entity.get_parent();
        if (!(entity.base_entity == entity.id) && sim->ent_alive(entity.base_entity)
            && sim->get_ent(entity.base_entity).has_component(kFlower))
            home_id = entity.base_entity;
        if (sim->ent_alive(home_id)) {
            Entity &home = sim->get_ent(home_id);
            float dist = Vector(ent.get_x()-home.get_x(),ent.get_y()-home.get_y()).magnitude();
            if (dist > SUMMON_RETREAT_RADIUS) return;
        }
        float dist = Vector(ent.get_x()-entity.get_x(),ent.get_y()-entity.get_y()).magnitude();
        // Poo: shrink flower aggro range from mob seekers
        if (!seeker_player && ent.has_component(kFlower) && !ent.has_component(kMob)) {
            float mult = flower_aggro_range_mult(ent);
            if (mult < 1.f && dist > radius * mult)
                return;
        }
        // Detached honey is preferred bait — treat as closer so mobs peel off the flower.
        float score = is_honey_bait ? dist * 0.28f : dist;
        if (score < min_dist) { min_dist = score; ret = ent.id; }
    });
    return ret;
}
