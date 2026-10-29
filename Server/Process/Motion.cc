#include <Server/Process.hh>
#include <Server/EntityFunctions.hh>
#include <Server/TrainingDummy.hh>

#include <Shared/Entity.hh>
#include <Shared/Map.hh>
#include <Shared/Simulation.hh>
#include <Shared/StaticData.hh>
#include <Shared/StaticDefinitions.hh>

#include <Helpers/Bits.hh>
#include <Helpers/Math.hh>
#include <Helpers/Vector.hh>

#include <cmath>

constexpr float BASE_TPS = 20;

void tick_entity_motion(Simulation *sim, Entity &ent) {
    // Keep hostile wildlife out of respawn, including old/admin-spawned packs.
    // Player summons and the training bay are unaffected.
    if (ent.has_component(kMob) && ent.damage > 0.f
        && !is_player_faction(sim, ent)
        && Map::in_spawn_plaza(sim->map_id, ent.get_x(), ent.get_y())) {
        if (!Map::in_spawn_plaza(sim->map_id, ent.home_x, ent.home_y)
            && ent.home_x > 0.f && ent.home_y > 0.f) {
            ent.set_x(ent.home_x); ent.set_y(ent.home_y);
            ent.target = NULL_ENTITY;
            ent.velocity.set(0, 0); ent.acceleration.set(0, 0);
            ent.collision_velocity.set(0, 0);
        } else {
            BitMath::set(ent.flags, EntityFlags::kNoDrops);
            sim->request_delete(ent.id);
        }
        return;
    }
    if (ent.pending_delete) return;
    if (ent.has_component(kAnimation)) return;
    if (TrainingDummy::is_dummy(ent)) {
        ent.set_x(ent.home_x);
        ent.set_y(ent.home_y);
        ent.velocity.set(0, 0);
        ent.acceleration.set(0, 0);
        ent.collision_velocity.set(0, 0);
        ent.speed_ratio = 1;
        return;
    }
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
        Vector const add = term_vel * dt + v * ((::powf(f, dt) - 1) / ::logf(f));
        ent.set_x(ent.get_x() + add.x + ent.collision_velocity.x);
        ent.set_y(ent.get_y() + add.y + ent.collision_velocity.y);
        ent.velocity = term_vel + v * (::powf(f, dt));
    }
    ent.velocity += ent.collision_velocity * 0.5;
    // Soft cap flower velocity so bubble / knockback stacks can't yeet across the map.
    if (ent.has_component(kFlower)) {
        float const kFlowerVelCap = PLAYER_ACCELERATION * 26.f;
        float vm = ent.velocity.magnitude();
        if (vm > kFlowerVelCap && is_finite(vm))
            ent.velocity.set_magnitude(kFlowerVelCap);
    }
    uint8_t const is_pea_shot = ent.has_component(kPetal)
        && BitMath::at(ent.flags, EntityFlags::kIsDespawning)
        && (petal_name_is(ent.get_petal_id(), "Peas")
            || petal_name_is(ent.get_petal_id(), "Glass Storm")
            || petal_name_is(ent.get_petal_id(), "Grapes")
            || petal_name_is(ent.get_petal_id(), "Blueberries")
            || petal_name_is(ent.get_petal_id(), "Carrot"));
    // Planted Meteor crater must stay out of berms (orbit petals skip terrain).
    uint8_t const is_planted_meteor = ent.has_component(kPetal)
        && petal_is_meteor(ent.get_petal_id())
        && ent.get_split_projectile();
    if ((!ent.has_component(kPetal) && !ent.has_component(kWeb))
        || is_pea_shot || is_planted_meteor) {
        float x = ent.get_x();
        float y = ent.get_y();
        float r = ent.get_radius();
        if (!is_finite(x) || !is_finite(y) || !is_finite(r)) {
            x = r = BASE_FLOWER_RADIUS;
            y = BASE_FLOWER_RADIUS;
        }
        if (is_pea_shot) {
            // Peas fly through open air; one bounce off berms / arena edge.
            auto blocked = [&](float px, float py) -> uint8_t {
                if (px < r || py < r || px > (float)ARENA_WIDTH - r
                    || py > (float)ARENA_HEIGHT - r)
                    return 1;
                return Map::circle_hits_impassable(sim->map_id, px, py, r);
            };
            if (blocked(x, y)) {
                if (!BitMath::at(ent.flags, EntityFlags::kTerrainBounced)) {
                    uint8_t wall_x = blocked(x, oy);
                    uint8_t wall_y = blocked(ox, y);
                    float nx = x, ny = y;
                    if (wall_x) {
                        ent.velocity.x = -ent.velocity.x;
                        nx = ox;
                    }
                    if (wall_y) {
                        ent.velocity.y = -ent.velocity.y;
                        ny = oy;
                    }
                    if (!wall_x && !wall_y) {
                        // Corner / diagonal into a berm — reverse both axes.
                        ent.velocity.x = -ent.velocity.x;
                        ent.velocity.y = -ent.velocity.y;
                        nx = ox;
                        ny = oy;
                    }
                    BitMath::set(ent.flags, EntityFlags::kTerrainBounced);
                    if (ent.velocity.magnitude() > 1e-3f) {
                        ent.heading_angle = ent.velocity.angle();
                        // Carrot thrust reads heading; keep visual angle in sync.
                        ent.set_angle(ent.heading_angle);
                    }
                    x = nx;
                    y = ny;
                    if (blocked(x, y)) {
                        Map::eject_from_impassable(sim->map_id, x, y, r);
                        x = fclamp(x, r, (float)ARENA_WIDTH - r);
                        y = fclamp(y, r, (float)ARENA_HEIGHT - r);
                    }
                } else {
                    // Bounce already spent — slide/stop at the wall.
                    Map::ccd_move_circle(sim->map_id, x, y, ox, oy, r);
                    x = fclamp(x, r, (float)ARENA_WIDTH - r);
                    y = fclamp(y, r, (float)ARENA_HEIGHT - r);
                    if (Map::circle_hits_impassable(sim->map_id, x, y, r))
                        Map::eject_from_impassable(sim->map_id, x, y, r);
                    x = fclamp(x, r, (float)ARENA_WIDTH - r);
                    y = fclamp(y, r, (float)ARENA_HEIGHT - r);
                }
            }
        } else if (ent.has_component(kDrop)
            && BitMath::at(ent.flags, EntityFlags::kMagnetPull)) {
            // Magnet suck ignores terrain — drops phase through walls toward flower.
            BitMath::unset(ent.flags, EntityFlags::kMagnetPull);
            x = fclamp(x, r, ARENA_WIDTH - r);
            y = fclamp(y, r, ARENA_HEIGHT - r);
        } else {
            // Terrain CCD slides along walls (never freezes input). Arena bounds are
            // hard walls via clamp; if clamp pins us inside an edge berm, eject inward.
            Map::ccd_move_circle(sim->map_id, x, y, ox, oy, r);
            x = fclamp(x, r, ARENA_WIDTH - r);
            y = fclamp(y, r, ARENA_HEIGHT - r);
            if (Map::circle_hits_impassable(sim->map_id, x, y, r))
                Map::eject_from_impassable(sim->map_id, x, y, r);
            x = fclamp(x, r, ARENA_WIDTH - r);
            y = fclamp(y, r, ARENA_HEIGHT - r);
        }
        ent.set_x(x);
        ent.set_y(y);
    }
    //ent.acceleration.set(0,0);
    ent.collision_velocity.set(0,0);
    ent.speed_ratio = 1;
}
