#pragma once

#include <Shared/Entity.hh>
#include <Shared/StaticData.hh>

#include <cstdint>
#include <functional>
#include <vector>

class Simulation;
class Entity;

// Was 200: fat-insert + small cells made large-rarity mobs occupy dozens of
// cells and explode collide cost. 512 matches florr_clone CELL_SIZE / rrolf
// order-of-magnitude — center-insert + adjacent collide stays correct for
// mild rarity radius scales (Unique ~7×).
static const uint32_t GRID_SIZE = 512;
static const uint32_t MAX_GRID_X = div_round_up(ARENA_WIDTH, GRID_SIZE);
static const uint32_t MAX_GRID_Y = div_round_up(ARENA_HEIGHT, GRID_SIZE);

class SpatialHash {
    Simulation *simulation;
    std::vector<EntityID> cells[MAX_GRID_X][MAX_GRID_Y];
    // r > GRID_SIZE/2: fat-inserted into cells (for query) and collided
    // separately so every pair fires exactly once.
    std::vector<EntityID> big;
    uint32_t width;
    uint32_t height;
public:
    SpatialHash(Simulation *);
    void refresh(uint32_t, uint32_t);
    void insert(Entity const &);
    void collide(std::function<void(Simulation *, Entity &, Entity &)>);
    void query(float, float, float, float, std::function<void(Simulation *, Entity &)>);
};
