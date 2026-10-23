#include <Server/SpatialHash.hh>

#include <Shared/Simulation.hh>
#include <Shared/Entity.hh>
#include <Helpers/Math.hh>

#include <cstring>

// florr_clone enemyGrid: NaN/huge radius → whole-grid loops / 100% CPU hang.
static float const kMaxMobRadius = 4096.f;

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

static void _clamp_cell(uint32_t &cx, uint32_t &cy) {
    if (cx >= MAX_GRID_X) cx = MAX_GRID_X - 1;
    if (cy >= MAX_GRID_Y) cy = MAX_GRID_Y - 1;
}

static void _center_cell(float x, float y, uint32_t &cx, uint32_t &cy) {
    cx = (uint32_t) (fclamp(x, 0, ARENA_WIDTH - 1) / GRID_SIZE);
    cy = (uint32_t) (fclamp(y, 0, ARENA_HEIGHT - 1) / GRID_SIZE);
    _clamp_cell(cx, cy);
}

static void _fat_range(float x, float y, float r, uint32_t &sx, uint32_t &sy, uint32_t &ex, uint32_t &ey) {
    r = _safe_radius(r);
    float x0 = fclamp(x - r, 0, ARENA_WIDTH - 1);
    float y0 = fclamp(y - r, 0, ARENA_HEIGHT - 1);
    float x1 = fclamp(x + r, 0, ARENA_WIDTH - 1);
    float y1 = fclamp(y + r, 0, ARENA_HEIGHT - 1);
    sx = (uint32_t) (x0 / GRID_SIZE);
    sy = (uint32_t) (y0 / GRID_SIZE);
    ex = (uint32_t) (x1 / GRID_SIZE);
    ey = (uint32_t) (y1 / GRID_SIZE);
    _clamp_cell(sx, sy);
    _clamp_cell(ex, ey);
    if (ex < sx) ex = sx;
    if (ey < sy) ey = sy;
}

// Center-inserted entities must satisfy r1 + r2 <= GRID_SIZE so any touching
// pair sits in the same or an adjacent cell. Anything larger goes to `big`.
static float const kSmallMaxRadius = (float)GRID_SIZE * 0.5f;

// big_stamp[id] == big_gen  <=>  entity is in `big` this tick.
static uint32_t big_stamp[ENTITY_CAP] = {};
static uint32_t big_gen = 1;

static uint8_t _is_big(EntityID const &id) {
    return id.id < ENTITY_CAP && big_stamp[id.id] == big_gen;
}

SpatialHash::SpatialHash(Simulation *sim) : simulation(sim), width(1), height(1) {
    active.reserve(256);
    big.reserve(32);
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
    // O(active): rrolf count=0 / florr_clone epoch — not full-grid vector.clear.
    for (uint32_t i = 0; i < active.size(); ++i) {
        uint32_t const key = active[i];
        uint32_t const cx = key >> 16;
        uint32_t const cy = key & 0xffffu;
        cells[cx][cy].count = 0;
    }
    active.clear();
    big.clear();
    if (++big_gen == 0) {
        std::memset(big_stamp, 0, sizeof(big_stamp));
        big_gen = 1;
    }
}

void SpatialHash::insert(Entity const &ent) {
    DEBUG_ONLY(assert(ent.has_component(kPhysics));)
    if (!_safe_pos(ent.get_x(), ent.get_y())) return;
    float r = _safe_radius(ent.get_radius());
    // rrolf: normal entities occupy ONE cell (center). Big ones fat-insert
    // (so query finds them without extra padding) and collide via `big`.
    if (r > kSmallMaxRadius) {
        if (ent.id.id < ENTITY_CAP) big_stamp[ent.id.id] = big_gen;
        big.push_back(ent.id);
        uint32_t sx, sy, ex, ey;
        _fat_range(ent.get_x(), ent.get_y(), r, sx, sy, ex, ey);
        for (uint32_t x = sx; x <= ex; ++x)
            for (uint32_t y = sy; y <= ey; ++y)
                _push_cell(x, y, ent.id);
        return;
    }
    uint32_t cx, cy;
    _center_cell(ent.get_x(), ent.get_y(), cx, cy);
    _push_cell(cx, cy, ent.id);
}

static void _pair(Simulation *sim, SpatialHashCollideFn on_collide,
    EntityID a_id, EntityID b_id)
{
    if (a_id.id == b_id.id) return;
    if (!sim->ent_exists(a_id) || !sim->ent_exists(b_id)) return;
    Entity &a = sim->get_ent(a_id);
    Entity &b = sim->get_ent(b_id);
    if (!a.has_component(kPhysics) || !b.has_component(kPhysics)) return;
    on_collide(sim, a, b);
}

static void _pair_cell(Simulation *sim, SpatialHashCollideFn on_collide,
    EntityID a_id, SpatialHashCell const &other)
{
    for (uint32_t j = 0; j < other.count; ++j)
        if (!_is_big(other.entities[j])) _pair(sim, on_collide, a_id, other.entities[j]);
}

void SpatialHash::collide(SpatialHashCollideFn on_collide) {
    // Small × small — only cells that received inserts this tick.
    for (uint32_t ai = 0; ai < active.size(); ++ai) {
        uint32_t const key = active[ai];
        uint32_t const x = key >> 16;
        uint32_t const y = key & 0xffffu;
        SpatialHashCell const &cell = cells[x][y];
        uint32_t const n = cell.count;
        if (n == 0) continue;
        for (uint32_t i = 0; i < n; ++i) {
            if (_is_big(cell.entities[i])) continue;
            for (uint32_t j = i + 1; j < n; ++j)
                if (!_is_big(cell.entities[j]))
                    _pair(simulation, on_collide, cell.entities[i], cell.entities[j]);
            // Half-neighbourhood: same cell already done; adjacent may be empty
            // (count==0) — safe to read without being on active list.
            if (x > 0) {
                _pair_cell(simulation, on_collide, cell.entities[i], cells[x - 1][y]);
                if (y > 0)
                    _pair_cell(simulation, on_collide, cell.entities[i], cells[x - 1][y - 1]);
            }
            if (y > 0) {
                _pair_cell(simulation, on_collide, cell.entities[i], cells[x][y - 1]);
                if (x + 1 < MAX_GRID_X)
                    _pair_cell(simulation, on_collide, cell.entities[i], cells[x + 1][y - 1]);
            }
        }
    }
    // Big × small — scan cells covering big AABB padded by the max small
    // radius; each small entity is in one cell, so no duplicates.
    for (uint32_t k = 0; k < big.size(); ++k) {
        EntityID const big_id = big[k];
        if (!simulation->ent_exists(big_id)) continue;
        Entity const &b = simulation->get_ent(big_id);
        uint32_t sx, sy, ex, ey;
        _fat_range(b.get_x(), b.get_y(), _safe_radius(b.get_radius()) + kSmallMaxRadius,
            sx, sy, ex, ey);
        for (uint32_t x = sx; x <= ex; ++x)
            for (uint32_t y = sy; y <= ey; ++y)
                _pair_cell(simulation, on_collide, big_id, cells[x][y]);
        for (uint32_t j = k + 1; j < big.size(); ++j)
            _pair(simulation, on_collide, big_id, big[j]);
    }
}
