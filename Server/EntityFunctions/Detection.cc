#include <Server/EntityFunctions.hh>

#include <Shared/Map.hh>
#include <Shared/Simulation.hh>
#include <Shared/Entity.hh>
#include <Shared/StaticData.hh>

#include <Helpers/Bits.hh>
#include <Helpers/Vector.hh>

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

EntityID find_nearest_enemy(Simulation *simulation, Entity const &entity, float radius) {
    if ((entity.id.id - entity.lifetime) % (TPS / 5) != 0) return NULL_ENTITY;
    if (entity.immunity_ticks > 0) return NULL_ENTITY;
    EntityID ret;
    float min_dist = radius;
    bool const seeker_player = is_player_faction(simulation, entity);
    simulation->spatial_hash.query(entity.get_x(), entity.get_y(), radius, radius, [&](Simulation *sim, Entity &ent){
        if (!sim->ent_alive(ent.id)) return;
        if (ent.get_team() == entity.get_team()) return;
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
        // Players / bots / their summons never aggro other player-faction flowers
        if (seeker_player && is_player_faction(sim, ent)) return;
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
        // Detached honey is preferred bait — treat as closer so mobs peel off the flower.
        float score = is_honey_bait ? dist * 0.28f : dist;
        if (score < min_dist) { min_dist = score; ret = ent.id; }
    });
    return ret;
}
