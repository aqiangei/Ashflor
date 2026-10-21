#pragma once

#include <cstdint>

class GameInstance;
class Client;
class Simulation;

namespace Treasure {
    // Spawn NPC + pads + crates once per map after zone fill
    void spawn_for_map(GameInstance *game);
    // Proximity hints, puzzle check, awards
    void tick(GameInstance *game);
}
