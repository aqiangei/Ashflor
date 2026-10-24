#pragma once

#include <cstdint>

class GameInstance;
class Client;
class Simulation;
class Entity;

namespace Treasure {
    // Spawn NPC + pads + crates once per map after zone fill
    void spawn_for_map(GameInstance *game);
    // Proximity hints, puzzle check, awards
    void tick(GameInstance *game);
    // Kill-to-open loot chests (called from entity_on_death)
    void on_loot_chest_killed(Simulation *sim, Entity const &chest);
}
