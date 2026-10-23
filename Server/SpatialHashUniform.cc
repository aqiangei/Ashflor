#include <Server/SpatialHash.hh>

#include <Shared/Simulation.hh>
#include <Shared/Entity.hh>
#include <Helpers/Math.hh>

SpatialHash::SpatialHash(Simulation *sim) : simulation(sim), width(1), height(1) {
    active.reserve(256);
    big.reserve(8);
}

void SpatialHash::_push_cell(uint32_t cx, uint32_t cy, EntityID id) {
    SpatialHashCell &cell = cells[cx][cy];
    if (cell.count == 0)
        active.push_back((cx << 16) | cy);
    if (cell.count >= CELL_ENTITY_CAP) {
        DEBUG_ONLY(assert(!"SpatialHash cell overflow");)
        return;
    }
    cell.entities[cell.count++] = id;
}

void SpatialHash::refresh(uint32_t _width, uint32_t _height) {
    DEBUG_ONLY(assert(_width <= ARENA_WIDTH && _height <= ARENA_HEIGHT));
    width = div_round_up(_width, GRID_SIZE);
    height = div_round_up(_height, GRID_SIZE);
    for (uint32_t i = 0; i < active.size(); ++i) {
        uint32_t const key = active[i];
        cells[key >> 16][key & 0xffffu].count = 0;
    }
    active.clear();
    big.clear();
}

void SpatialHash::insert(Entity const &ent) {
    DEBUG_ONLY(assert(ent.has_component(kPhysics));)
    // for the uniform grid to work, the max ent radius is GRID_SIZE/2
    DEBUG_ONLY(assert(ent.get_radius() <= GRID_SIZE / 2);)
    uint32_t x = (uint32_t) (fclamp(ent.get_x(), 0, ARENA_WIDTH - 1) / GRID_SIZE);
    uint32_t y = (uint32_t) (fclamp(ent.get_y(), 0, ARENA_HEIGHT - 1) / GRID_SIZE);
    if (x >= MAX_GRID_X) x = MAX_GRID_X - 1;
    if (y >= MAX_GRID_Y) y = MAX_GRID_Y - 1;
    _push_cell(x, y, ent.id);
}

void SpatialHash::collide(SpatialHashCollideFn on_collide) {
    // Forward neighbourhood over active cells only (same pair-once geometry as
    // scanning the full grid — inactive neighbours have count==0).
    for (uint32_t ai = 0; ai < active.size(); ++ai) {
        uint32_t const key = active[ai];
        uint32_t const x = key >> 16;
        uint32_t const y = key & 0xffffu;
        SpatialHashCell &cell = cells[x][y];
        for (uint32_t i = 0; i < cell.count; ++i) {
            EntityID const a = cell.entities[i];
            if (!simulation->ent_exists(a)) continue;
            Entity &ea = simulation->get_ent(a);
            for (uint32_t j = i + 1; j < cell.count; ++j) {
                EntityID const b = cell.entities[j];
                if (!simulation->ent_exists(b)) continue;
                on_collide(simulation, ea, simulation->get_ent(b));
            }
            auto pair_other = [&](SpatialHashCell &cell2) {
                for (uint32_t j = 0; j < cell2.count; ++j) {
                    EntityID const b = cell2.entities[j];
                    if (!simulation->ent_exists(b)) continue;
                    on_collide(simulation, ea, simulation->get_ent(b));
                }
            };
            if (x + 1 < MAX_GRID_X) {
                pair_other(cells[x + 1][y]);
                if (y > 0) pair_other(cells[x + 1][y - 1]);
                if (y + 1 < MAX_GRID_Y) pair_other(cells[x + 1][y + 1]);
            }
            if (y + 1 < MAX_GRID_Y) pair_other(cells[x][y + 1]);
        }
    }
}
