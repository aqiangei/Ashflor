// Included at the end of Shared/Simulation.hh after class Simulation is
// complete. Template body calls Simulation methods; defining it while
// Simulation was still incomplete made em++ reject lambda instantiations
// ("type does not have linkage") on clean WASM builds.
#pragma once

template <class F>
void SpatialHash::query(float x, float y, float w, float h, F &&cb) {
    static uint32_t seen_stamp[SPATIAL_QUERY_ENTITY_CAP] = {};
    static uint32_t query_gen = 0;
    if (++query_gen == 0) {
        std::memset(seen_stamp, 0, sizeof(seen_stamp));
        query_gen = 1;
    }
    float const aw = (float)ARENA_WIDTH;
    float const ah = (float)ARENA_HEIGHT;
    if (!(is_finite(x) && is_finite(y)
            && x >= -aw && x <= aw * 2.f
            && y >= -ah && y <= ah * 2.f))
        return;
    if (!is_finite(w) || !is_finite(h) || w < 0.f || h < 0.f) return;
    if (w > SPATIAL_QUERY_MAX_HALF) w = SPATIAL_QUERY_MAX_HALF;
    if (h > SPATIAL_QUERY_MAX_HALF) h = SPATIAL_QUERY_MAX_HALF;
    // Center-insert: pad by one cell so entities whose edge enters the box
    // but center sits just outside are still candidates (rrolf does the same).
    float const pad = (float)GRID_SIZE;
    uint32_t sx = (uint32_t) (fclamp(x - w - pad, 0, ARENA_WIDTH - 1) / GRID_SIZE);
    uint32_t sy = (uint32_t) (fclamp(y - h - pad, 0, ARENA_HEIGHT - 1) / GRID_SIZE);
    uint32_t ex = (uint32_t) (fclamp(x + w + pad, 0, ARENA_WIDTH - 1) / GRID_SIZE);
    uint32_t ey = (uint32_t) (fclamp(y + h + pad, 0, ARENA_HEIGHT - 1) / GRID_SIZE);
    if (sx >= MAX_GRID_X) sx = MAX_GRID_X - 1;
    if (sy >= MAX_GRID_Y) sy = MAX_GRID_Y - 1;
    if (ex >= MAX_GRID_X) ex = MAX_GRID_X - 1;
    if (ey >= MAX_GRID_Y) ey = MAX_GRID_Y - 1;
    if (ex < sx) ex = sx;
    if (ey < sy) ey = sy;
    for (uint32_t _x = sx; _x <= ex; ++_x) {
        for (uint32_t _y = sy; _y <= ey; ++_y) {
            SpatialHashCell const &cell = cells[_x][_y];
            for (uint32_t i = 0; i < cell.count; ++i) {
                EntityID::id_type id = cell.entities[i].id;
                if (id >= SPATIAL_QUERY_ENTITY_CAP || seen_stamp[id] == query_gen) continue;
                if (!simulation->ent_exists(cell.entities[i])) continue;
                Entity &ent = simulation->get_ent(cell.entities[i]);
                if (!ent.has_component(kPhysics)) continue;
                float er = ent.get_radius();
                if (!is_finite(er) || er < 0.f) er = 1.f;
                if (er > SPATIAL_QUERY_MAX_RADIUS) er = SPATIAL_QUERY_MAX_RADIUS;
                float ex_ = ent.get_x();
                float ey_ = ent.get_y();
                if (!(is_finite(ex_) && is_finite(ey_)
                        && ex_ >= -aw && ex_ <= aw * 2.f
                        && ey_ >= -ah && ey_ <= ah * 2.f))
                    continue;
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
