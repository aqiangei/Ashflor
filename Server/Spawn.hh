#pragma once

#include <Shared/Entity.hh>

#include <functional>
#include <string>

class Simulation;

Entity &alloc_drop(Simulation *, PetalID::T, EntityID const team = NULL_ENTITY, uint16_t count = 1);
Entity &alloc_corpse(
    Simulation *, EntityID const team, float x, float y, std::string const &name,
    uint8_t color = 0, uint8_t equip_flags = 0
);
Entity &alloc_mob(
    Simulation *, MobID::T, float, float, 
    EntityID const, std::function<void(Entity &)> = nullptr
);
Entity &alloc_player(Simulation *, EntityID const);
Entity &alloc_petal(Simulation *, PetalID::T, Entity const &);
Entity &alloc_web(Simulation *, float, Entity const &);
Entity &alloc_cpu_camera(Simulation *, EntityID const);

void player_spawn(Simulation *, Entity &, Entity &);
void player_spawn_at(Simulation *, Entity &, Entity &, float x, float y);