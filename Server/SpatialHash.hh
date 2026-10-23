#pragma once

#include <Shared/Entity.hh>
#include <Shared/StaticData.hh>
#include <Helpers/Math.hh>

#include <cstdint>
#include <cstring>
#include <utility>
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

// rrolf-style fixed cell: no per-tick vector growth under combat spikes.
// Densest 512px pocket is still << ENTITY_CAP; overflow drops (DEBUG assert).
static const uint32_t CELL_ENTITY_CAP = 1024;
// Must match Simulation::ENTITY_CAP (SpatialHash.hh is included before that const).
static const uint32_t SPATIAL_QUERY_ENTITY_CAP = 8192;
static float const SPATIAL_QUERY_MAX_HALF = 8192.f;
static float const SPATIAL_QUERY_MAX_RADIUS = 4096.f;

struct SpatialHashCell {
    uint32_t count = 0;
    EntityID entities[CELL_ENTITY_CAP];
};

using SpatialHashCollideFn = void (*)(Simulation *, Entity &, Entity &);

class SpatialHash {
    Simulation *simulation;
    SpatialHashCell cells[MAX_GRID_X][MAX_GRID_Y];
    // Packed cx<<16|cy for cells touched this tick (active-list clear).
    std::vector<uint32_t> active;
    // r > GRID_SIZE/2: fat-inserted into cells (for query) and collided
    // separately so every pair fires exactly once.
    std::vector<EntityID> big;
    uint32_t width;
    uint32_t height;

    void _push_cell(uint32_t cx, uint32_t cy, EntityID id);

public:
    SpatialHash(Simulation *);
    void refresh(uint32_t, uint32_t);
    void insert(Entity const &);
    void collide(SpatialHashCollideFn);

    // Template callback (inlined at call site) — no std::function per hit.
    // Definition lives in SpatialHash.inl, included after class Simulation is
    // complete (see Shared/Simulation.hh). Defining it here while Simulation
    // was incomplete broke em++ lambda instantiations on clean WASM builds.
    template <class F>
    void query(float x, float y, float w, float h, F &&cb);
};
