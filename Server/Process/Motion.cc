#include <Server/Process.hh>

#include <Shared/Entity.hh>
#include <Shared/Map.hh>
#include <Shared/Simulation.hh>
#include <Shared/StaticData.hh>

#include <Helpers/Math.hh>

#include <cmath>

constexpr float BASE_TPS = 20;

void tick_entity_motion(Simulation *sim, Entity &ent) {
    if (ent.pending_delete) return;
    // Self-heal NaN/Inf physics (from florr_incremental): one poisoned entity
    // otherwise freezes the arena / breaks spatial hash after respawns.
    if (!is_finite(ent.get_x()) || !is_finite(ent.get_y())) {
        ent.set_x(ent.get_radius());
        ent.set_y(ent.get_radius());
    }
    if (!is_finite(ent.velocity.x) || !is_finite(ent.velocity.y))
        ent.velocity.set(0, 0);
    if (!is_finite(ent.acceleration.x) || !is_finite(ent.acceleration.y))
        ent.acceleration.set(0, 0);
    if (!is_finite(ent.collision_velocity.x) || !is_finite(ent.collision_velocity.y))
        ent.collision_velocity.set(0, 0);
    if (!is_finite(ent.friction)) ent.friction = DEFAULT_FRICTION;
    if (!is_finite(ent.speed_ratio)) ent.speed_ratio = 1;

    float const ox = ent.get_x();
    float const oy = ent.get_y();

    if (ent.snare_ticks > 0) {
        ent.speed_ratio *= 0.05f;
        --ent.snare_ticks;
    } else if (ent.slow_ticks > 0) {
        ent.speed_ratio *= 0.5;
        --ent.slow_ticks;
    }
    float const dt = (BASE_TPS / TPS);
    if (ent.friction <= 0) {
        Vector const add = ent.velocity * dt + ent.acceleration * (0.5 * dt * dt);
        ent.velocity += ent.acceleration * dt;
        ent.set_x(ent.get_x() + add.x + ent.collision_velocity.x);
        ent.set_y(ent.get_y() + add.y + ent.collision_velocity.y);
    }
    else if (ent.friction >= 1) {
        ent.velocity.set(0,0);
        ent.set_x(ent.get_x() + ent.acceleration.x * dt + ent.collision_velocity.x);
        ent.set_y(ent.get_y() + ent.acceleration.y * dt + ent.collision_velocity.y);
    }
    else {
        float const f = 1 - ent.friction;
        Vector const term_vel = ent.acceleration * (ent.speed_ratio / ent.friction);
        Vector const v = ent.velocity - term_vel;
        Vector const add = term_vel * dt + v * ((std::powf(f, dt) - 1) / std::logf(f));
        ent.set_x(ent.get_x() + add.x + ent.collision_velocity.x);
        ent.set_y(ent.get_y() + add.y + ent.collision_velocity.y);
        ent.velocity = term_vel + v * (std::powf(f, dt));
    }
    ent.velocity += ent.collision_velocity * 0.5;
    // Soft cap flower velocity so bubble / knockback stacks can't yeet across the map.
    if (ent.has_component(kFlower)) {
        float const kFlowerVelCap = PLAYER_ACCELERATION * 36.f;
        float vm = ent.velocity.magnitude();
        if (vm > kFlowerVelCap && is_finite(vm))
            ent.velocity.set_magnitude(kFlowerVelCap);
    }
    if (!ent.has_component(kPetal) && !ent.has_component(kWeb)) {
        float x = ent.get_x();
        float y = ent.get_y();
        float r = ent.get_radius();
        if (!is_finite(x) || !is_finite(y) || !is_finite(r)) {
            x = r = BASE_FLOWER_RADIUS;
            y = BASE_FLOWER_RADIUS;
        }
        // Terrain CCD slides along walls (never freezes input). Arena bounds are
        // hard walls via clamp; if clamp pins us inside an edge berm, eject inward.
        Map::ccd_move_circle(sim->map_id, x, y, ox, oy, r);
        x = fclamp(x, r, ARENA_WIDTH - r);
        y = fclamp(y, r, ARENA_HEIGHT - r);
        if (Map::circle_hits_impassable(sim->map_id, x, y, r))
            Map::eject_from_impassable(sim->map_id, x, y, r);
        x = fclamp(x, r, ARENA_WIDTH - r);
        y = fclamp(y, r, ARENA_HEIGHT - r);
        ent.set_x(x);
        ent.set_y(y);
    }
    //ent.acceleration.set(0,0);
    ent.collision_velocity.set(0,0);
    ent.speed_ratio = 1;
}