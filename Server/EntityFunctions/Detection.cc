#include <Server/EntityFunctions.hh>

#include <Shared/Map.hh>
#include <Shared/Simulation.hh>
#include <Shared/Entity.hh>
#include <Shared/StaticData.hh>

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
        if (!ent.has_component(kMob) && !ent.has_component(kFlower)) return;
        // Players / bots / their summons never aggro other player-faction flowers
        if (seeker_player && is_player_faction(sim, ent)) return;
        // Spawn plaza: wild mobs never acquire aggro on players inside (players may still attack).
        if (!seeker_player && is_player_faction(sim, ent)
            && Map::in_spawn_plaza(ent.get_x(), ent.get_y()))
            return;
        if (sim->ent_alive(entity.get_parent())) {
            Entity &parent = sim->get_ent(entity.get_parent());
            float dist = Vector(ent.get_x()-parent.get_x(),ent.get_y()-parent.get_y()).magnitude();
            if (dist > SUMMON_RETREAT_RADIUS) return;
        }
        float dist = Vector(ent.get_x()-entity.get_x(),ent.get_y()-entity.get_y()).magnitude();
        if (dist < min_dist) { min_dist = dist; ret = ent.id; }
    });
    return ret;
}
