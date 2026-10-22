#include <Server/SpatialHash.hh>

#include <Shared/Simulation.hh>
#include <Shared/Entity.hh>
#include <Helpers/Math.hh>

#include <algorithm>
#include <cstring>

// florr_clone enemyGrid: NaN/huge radius → whole-grid loops / 100% CPU hang.
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

SpatialHash::SpatialHash(Simulation *sim) : simulation(sim), width(1), height(1) {}

void SpatialHash::refresh(uint32_t _width, uint32_t _height) {
    DEBUG_ONLY(assert(_width <= ARENA_WIDTH && _height <= ARENA_HEIGHT));
    width = div_round_up(_width, GRID_SIZE);
    height = div_round_up(_height, GRID_SIZE);
    for (uint32_t x = 0; x < MAX_GRID_X; ++x)
        for (uint32_t y = 0; y < MAX_GRID_Y; ++y)
            cells[x][y].clear();
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
                cells[x][y].push_back(ent.id);
        return;
    }
    uint32_t cx, cy;
    _center_cell(ent.get_x(), ent.get_y(), cx, cy);
    cells[cx][cy].push_back(ent.id);
}

static void _pair(Simulation *sim,
    std::function<void(Simulation *, Entity &, Entity &)> const &on_collide,
    EntityID a_id, EntityID b_id)
{
    if (a_id.id == b_id.id) return;
    if (!sim->ent_exists(a_id) || !sim->ent_exists(b_id)) return;
    Entity &a = sim->get_ent(a_id);
    Entity &b = sim->get_ent(b_id);
    if (!a.has_component(kPhysics) || !b.has_component(kPhysics)) return;
    on_collide(sim, a, b);
}

// Pair cell[i] with every small entity in `other` (big ones handled below).
static void _pair_cell(Simulation *sim,
    std::function<void(Simulation *, Entity &, Entity &)> const &on_collide,
    EntityID a_id, std::vector<EntityID> const &other)
{
    for (uint32_t j = 0; j < other.size(); ++j)
        if (!_is_big(other[j])) _pair(sim, on_collide, a_id, other[j]);
}

void SpatialHash::collide(std::function<void(Simulation *, Entity &, Entity &)> on_collide) {
    // Small × small — rrolf half-neighbourhood: same cell + (x-1,y) + (x-1,y-1)
    // + (x,y-1) + (x+1,y-1). Small entities live in exactly one cell, so each
    // unordered pair fires exactly once. Fat-inserted big entries are skipped:
    // a duplicate on_collide means duplicate inflict_damage (×N boss damage).
    for (uint32_t x = 0; x < MAX_GRID_X; ++x) {
        for (uint32_t y = 0; y < MAX_GRID_Y; ++y) {
            std::vector<EntityID> const &cell = cells[x][y];
            uint32_t const n = (uint32_t) cell.size();
            for (uint32_t i = 0; i < n; ++i) {
                if (_is_big(cell[i])) continue;
                for (uint32_t j = i + 1; j < n; ++j)
                    if (!_is_big(cell[j])) _pair(simulation, on_collide, cell[i], cell[j]);
                if (x > 0) {
                    _pair_cell(simulation, on_collide, cell[i], cells[x - 1][y]);
                    if (y > 0)
                        _pair_cell(simulation, on_collide, cell[i], cells[x - 1][y - 1]);
                }
                if (y > 0) {
                    _pair_cell(simulation, on_collide, cell[i], cells[x][y - 1]);
                    if (x + 1 < MAX_GRID_X)
                        _pair_cell(simulation, on_collide, cell[i], cells[x + 1][y - 1]);
                }
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
        // Big × big — few of these (bosses / high-rarity giants), brute force.
        for (uint32_t j = k + 1; j < big.size(); ++j)
            _pair(simulation, on_collide, big_id, big[j]);
    }
}

void SpatialHash::query(float x, float y, float w, float h, std::function<void(Simulation *, Entity &)> cb) {
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
    // Center-insert: pad by one cell so entities whose edge enters the box
    // but center sits just outside are still candidates (rrolf does the same).
    float const pad = (float)GRID_SIZE;
    uint32_t sx = (uint32_t) (fclamp(x - w - pad, 0, ARENA_WIDTH - 1) / GRID_SIZE);
    uint32_t sy = (uint32_t) (fclamp(y - h - pad, 0, ARENA_HEIGHT - 1) / GRID_SIZE);
    uint32_t ex = (uint32_t) (fclamp(x + w + pad, 0, ARENA_WIDTH - 1) / GRID_SIZE);
    uint32_t ey = (uint32_t) (fclamp(y + h + pad, 0, ARENA_HEIGHT - 1) / GRID_SIZE);
    _clamp_cell(sx, sy);
    _clamp_cell(ex, ey);
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
