#include <Server/SpatialHash.hh>

#include <Shared/Simulation.hh>
#include <Shared/Entity.hh>
#include <Helpers/Math.hh>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <iostream>

// Ported from florr_clone-web enemyGrid.ts: degenerate radius / non-finite coords
// turn cell-range loops into whole-grid scans (100% CPU hang) or OOB. Clamp hard.
static float const kMaxMobRadius = 4096.f;
static float const kMaxQueryHalf = 8192.f;

static float _safe_radius(float r) {
    if (!is_finite(r) || r < 0.f) return 1.f;
    if (r > kMaxMobRadius) return kMaxMobRadius;
    return r;
}

static uint8_t _safe_pos(float x, float y) {
    float const aw = (float)ARENA_WIDTH;
    float const ah = (float)ARENA_HEIGHT;
    return is_finite(x) && is_finite(y)
        && x >= -aw && x <= aw * 2.f
        && y >= -ah && y <= ah * 2.f;
}

static void _cell_range(float x, float y, float r, uint32_t &sx, uint32_t &sy, uint32_t &ex, uint32_t &ey) {
    r = _safe_radius(r);
    if (!_safe_pos(x, y)) {
        sx = sy = ex = ey = 0;
        return;
    }
    float x0 = fclamp(x - r, 0, ARENA_WIDTH - 1);
    float y0 = fclamp(y - r, 0, ARENA_HEIGHT - 1);
    float x1 = fclamp(x + r, 0, ARENA_WIDTH - 1);
    float y1 = fclamp(y + r, 0, ARENA_HEIGHT - 1);
    sx = (uint32_t) (x0 / GRID_SIZE);
    sy = (uint32_t) (y0 / GRID_SIZE);
    ex = (uint32_t) (x1 / GRID_SIZE);
    ey = (uint32_t) (y1 / GRID_SIZE);
    if (sx >= MAX_GRID_X) sx = MAX_GRID_X - 1;
    if (sy >= MAX_GRID_Y) sy = MAX_GRID_Y - 1;
    if (ex >= MAX_GRID_X) ex = MAX_GRID_X - 1;
    if (ey >= MAX_GRID_Y) ey = MAX_GRID_Y - 1;
    if (ex < sx) ex = sx;
    if (ey < sy) ey = sy;
}

static void _cell_range_ent(Entity const &ent, uint32_t &sx, uint32_t &sy, uint32_t &ex, uint32_t &ey) {
    _cell_range(ent.get_x(), ent.get_y(), ent.get_radius(), sx, sy, ex, ey);
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
    if (!_safe_pos(ent.get_x(), ent.get_y())) return;
    uint32_t sx, sy, ex, ey;
    _cell_range_ent(ent, sx, sy, ex, ey);
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
                if (!simulation->ent_exists(cell[i])) continue;
                Entity &a = simulation->get_ent(cell[i]);
                if (!a.has_component(kPhysics)) continue;
                uint32_t asx, asy, aex, aey;
                _cell_range_ent(a, asx, asy, aex, aey);
                for (uint32_t j = i + 1; j < cell.size(); ++j) {
                    if (!simulation->ent_exists(cell[j])) continue;
                    Entity &b = simulation->get_ent(cell[j]);
                    if (!b.has_component(kPhysics)) continue;
                    uint32_t bsx, bsy, bex, bey;
                    _cell_range_ent(b, bsx, bsy, bex, bey);
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
    if (!_safe_pos(x, y)) return;
    if (!is_finite(w) || !is_finite(h) || w < 0.f || h < 0.f) return;
    if (w > kMaxQueryHalf) w = kMaxQueryHalf;
    if (h > kMaxQueryHalf) h = kMaxQueryHalf;
    uint32_t sx = (uint32_t) (fclamp(x - w, 0, ARENA_WIDTH - 1) / GRID_SIZE);
    uint32_t sy = (uint32_t) (fclamp(y - h, 0, ARENA_HEIGHT - 1) / GRID_SIZE);
    uint32_t ex = (uint32_t) (fclamp(x + w, 0, ARENA_WIDTH - 1) / GRID_SIZE);
    uint32_t ey = (uint32_t) (fclamp(y + h, 0, ARENA_HEIGHT - 1) / GRID_SIZE);
    if (sx >= MAX_GRID_X) sx = MAX_GRID_X - 1;
    if (sy >= MAX_GRID_Y) sy = MAX_GRID_Y - 1;
    if (ex >= MAX_GRID_X) ex = MAX_GRID_X - 1;
    if (ey >= MAX_GRID_Y) ey = MAX_GRID_Y - 1;
    if (ex < sx) ex = sx;
    if (ey < sy) ey = sy;
    for (uint32_t _x = sx; _x <= ex; ++_x) {
        for (uint32_t _y = sy; _y <= ey; ++_y) {
            std::vector<EntityID> const &cell = cells[_x][_y];
            for (uint32_t i = 0; i < cell.size(); ++i) {
                EntityID::id_type id = cell[i].id;
                if (id >= ENTITY_CAP || seen_stamp[id] == query_gen) continue;
                if (!simulation->ent_exists(cell[i])) continue;
                Entity &ent = simulation->get_ent(cell[i]);
                if (!ent.has_component(kPhysics)) continue;
                float er = _safe_radius(ent.get_radius());
                float ex_ = ent.get_x();
                float ey_ = ent.get_y();
                if (!_safe_pos(ex_, ey_)) continue;
                if (ex_ + er < x - w) continue;
                if (ex_ - er > x + w) continue;
                if (ey_ + er < y - h) continue;
                if (ey_ - er > y + h) continue;
                seen_stamp[id] = query_gen;
                cb(simulation, ent);
            }
        }
    }
}
