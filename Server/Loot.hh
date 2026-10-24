#pragma once

#include <Shared/Entity.hh>
#include <Shared/Simulation.hh>

namespace Loot {
// Team ids change when players join/leave parties. Ground loot is owned by a
// personal camera for its whole life; only deliberate discards are unowned.
inline bool can_access_drop(Entity const &drop, EntityID camera) {
    return !camera.null() && (drop.get_team().null() || drop.get_team() == camera);
}

// Resolve petals, summoned mobs and their projectiles through their personal
// parent chain. Never fall back to the shared party team/leader camera.
inline EntityID owner_camera(Simulation *sim, EntityID source) {
    for (uint8_t depth = 0; depth < 16 && sim->ent_exists(source); ++depth) {
        Entity const &ent = sim->get_ent(source);
        if (ent.has_component(kCamera)) return ent.id;
        EntityID next = ent.has_component(kRelations) ? ent.get_parent() : NULL_ENTITY;
        if (next.null() || next == source) next = ent.base_entity;
        if (next.null() || next == source) break;
        source = next;
    }
    return NULL_ENTITY;
}
}
