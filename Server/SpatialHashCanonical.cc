#include <Server/SpatialHash.hh>

#include <Shared/Simulation.hh>
#include <Shared/Entity.hh>

#include <algorithm>
#include <cstring>

static void _cell_range(Entity const &ent, uint32_t &sx, uint32_t &sy, uint32_t &ex, uint32_t &ey) {
    sx = (uint32_t) (fclamp(ent.get_x() - ent.get_radius(), 0, ARENA_WIDTH - 1) / GRID_SIZE);
    sy = (uint32_t) (fclamp(ent.get_y() - ent.get_radius(), 0, ARENA_HEIGHT - 1) / GRID_SIZE);
    ex = (uint32_t) (fclamp(ent.get_x() + ent.get_radius(), 0, ARENA_WIDTH - 1) / GRID_SIZE);
    ey = (uint32_t) (fclamp(ent.get_y() + ent.get_radius(), 0, ARENA_HEIGHT - 1) / GRID_SIZE);
}

SpatialHash::SpatialHash(Simulation *sim) : simulation(sim), width(1), height(1) {}

void SpatialHash::refresh(uint32_t _width, uint32_t _height) {
    DEBUG_ONLY(assert(_width <= ARENA_WIDTH && _height <= ARENA_HEIGHT));
    width = div_round_up(_width, GRID_SIZE);
    height = div_round_up(_height, GRID_SIZE);
    for (uint32_t x = 0; x < MAX_GRID_X; ++x)
        for (uint32_t y = 0; y < MAX_GRID_Y; ++y)
            cells[x][y].clear();
}

void SpatialHash::insert(Entity const &ent) {
    DEBUG_ONLY(assert(ent.has_component(kPhysics));)
    uint32_t sx, sy, ex, ey;
    _cell_range(ent, sx, sy, ex, ey);
    for (uint32_t x = sx; x <= ex; ++x)
        for (uint32_t y = sy; y <= ey; ++y)
            cells[x][y].push_back(ent.id);
}

void SpatialHash::collide(std::function<void(Simulation *, Entity &, Entity &)> on_collide) {
    // Same pair set as unordered_set dedupe: only fire in the lexicographically
    // smallest cell both entities occupy (classic canonical-grid filter).
    for (uint32_t x = 0; x < MAX_GRID_X; ++x) {
        for (uint32_t y = 0; y < MAX_GRID_Y; ++y) {
            std::vector<EntityID> const &cell = cells[x][y];
            for (uint32_t i = 0; i < cell.size(); ++i) {
                Entity &a = simulation->get_ent(cell[i]);
                uint32_t asx, asy, aex, aey;
                _cell_range(a, asx, asy, aex, aey);
                for (uint32_t j = i + 1; j < cell.size(); ++j) {
                    Entity &b = simulation->get_ent(cell[j]);
                    uint32_t bsx, bsy, bex, bey;
                    _cell_range(b, bsx, bsy, bex, bey);
                    uint32_t ox = std::max(asx, bsx);
                    uint32_t oy = std::max(asy, bsy);
                    if (x != ox || y != oy) continue;
                    on_collide(simulation, a, b);
                }
            }
        }
    }
}

void SpatialHash::query(float x, float y, float w, float h, std::function<void(Simulation *, Entity &)> cb) {
    // Generation stamp: same dedupe as unordered_set, no heap traffic on magnet/AOI.
    static uint32_t seen_stamp[ENTITY_CAP] = {};
    static uint32_t query_gen = 0;
    if (++query_gen == 0) {
        std::memset(seen_stamp, 0, sizeof(seen_stamp));
        query_gen = 1;
    }
    uint32_t sx = (uint32_t) (fclamp(x - w, 0, ARENA_WIDTH - 1) / GRID_SIZE);
    uint32_t sy = (uint32_t) (fclamp(y - h, 0, ARENA_HEIGHT - 1) / GRID_SIZE);
    uint32_t ex = (uint32_t) (fclamp(x + w, 0, ARENA_WIDTH - 1) / GRID_SIZE);
    uint32_t ey = (uint32_t) (fclamp(y + h, 0, ARENA_HEIGHT - 1) / GRID_SIZE);
    for (uint32_t _x = sx; _x <= ex; ++_x) {
        for (uint32_t _y = sy; _y <= ey; ++_y) {
            std::vector<EntityID> const &cell = cells[_x][_y];
            for (uint32_t i = 0; i < cell.size(); ++i) {
                EntityID::id_type id = cell[i].id;
                if (id >= ENTITY_CAP || seen_stamp[id] == query_gen) continue;
                Entity &ent = simulation->get_ent(cell[i]);
                if (ent.get_x() + ent.get_radius() < x - w) continue;
                if (ent.get_x() - ent.get_radius() > x + w) continue;
                if (ent.get_y() + ent.get_radius() < y - h) continue;
                if (ent.get_y() - ent.get_radius() > y + h) continue;
                seen_stamp[id] = query_gen;
                cb(simulation, ent);
            }
        }
    }
}