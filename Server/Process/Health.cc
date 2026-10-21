#include <Server/Process.hh>

#include <Server/EntityFunctions.hh>
#include <Shared/Simulation.hh>
#include <Shared/Entity.hh>

#include <iostream>

void tick_health_behavior(Simulation *sim, Entity &ent) {
    ent.set_damaged(0);
    if (ent.poison_ticks > 0 && !ent.has_component(kPetal)) {
        ent.poison_ticks--;
        inflict_damage(sim, ent.poison_dealer, ent.id, ent.poison_inflicted, DamageType::kPoison);
        if (ent.poison_ticks % (TPS / 2) != 0) ent.set_damaged(0);
    } else {
        ent.poison_inflicted = 0;
        ent.poison_dealer = NULL_ENTITY;
    }
    if (ent.burn_ticks > 0 && !ent.has_component(kPetal)) {
        ent.burn_ticks--;
        inflict_damage(sim, ent.burn_dealer, ent.id, ent.burn_inflicted, DamageType::kBurn);
        if (ent.burn_ticks % (TPS / 2) != 0) ent.set_damaged(0);
    } else {
        ent.burn_inflicted = 0;
        ent.burn_dealer = NULL_ENTITY;
    }
    if (ent.dandy_ticks > 0) --ent.dandy_ticks;
    if (ent.health <= 0) sim->request_delete(ent.id);
    if (ent.max_health == 0) return;
    // Petals keep a dummy ratio; flowers and mobs publish real HP so the bar shows when hit.
    if (ent.has_component(kPetal))
        ent.set_health_ratio(1);
    else
        ent.set_health_ratio(ent.health / ent.max_health);
}