#pragma once

#include <Shared/EntityDef.hh>

class Simulation;
class Entity;
class EntityID;

void inflict_damage(Simulation *, EntityID const, EntityID const, float, uint8_t);
void inflict_heal(Simulation *, Entity &, float);

void entity_on_death(Simulation *, Entity const &);

// Copy flower loadout → parent camera inventory (respawn layout). Call before
// request_delete when capture / respawn must not see an empty camera.
void flush_flower_loadout_to_camera(Simulation *, Entity &camera, Entity const &flower);

// True for real players / CPU bots, their petals, and flower-owned summons.
// False for wild mobs and digger (kFlower without camera parent).
bool is_player_faction(Simulation *, Entity const &);

// Corruption petal / Unique aura: enables player-faction PvP.
bool flower_is_corrupted(Simulation *, Entity const &);
bool player_faction_pvp(Simulation *, Entity const &, Entity const &);

EntityID find_nearest_enemy(Simulation *, Entity const &, float);

// Poo: multiplier on mob aggro / deaggro range vs this flower (1 = full range).
float flower_aggro_range_mult(Entity const &);

void entity_set_despawn_tick(Entity &, game_tick_t);
void entity_clear_references(Simulation *, Entity &);
