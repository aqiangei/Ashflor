#include <Server/Process.hh>

#include <Server/EntityFunctions.hh>
#include <Server/Spawn.hh>
#include <Shared/Entity.hh>
#include <Shared/Map.hh>
#include <Shared/Simulation.hh>
#include <Shared/StaticData.hh>

#include <Helpers/Math.hh>

#include <cmath>

static void _focus_lose_clause(Entity &ent, Vector const &v) {
    if (v.magnitude() > 1.35f * ent.detection_radius) ent.target = NULL_ENTITY;
}

static void default_tick_idle(Simulation *sim, Entity &ent) {
    if (ent.ai_tick >= 1 * TPS) {
        ent.ai_tick = 0;
        ent.set_angle(frand() * 2 * M_PI);
        ent.ai_state = AIState::kIdleMoving;
    }
}

static void default_tick_idle_moving(Simulation *sim, Entity &ent) {
    if (ent.ai_tick > 2.5 * TPS) {
        ent.ai_tick = 0;
        ent.ai_state = AIState::kIdle;
        return;
    }
    if (ent.ai_tick < 0.5 * TPS) return;
    float r = (ent.ai_tick - 0.5 * TPS) / (2 * TPS);
    ent.acceleration
        .unit_normal(ent.get_angle())
        .set_magnitude(2 * PLAYER_ACCELERATION * (r - r * r));
}

static void default_tick_returning(Simulation *sim, Entity &ent, float speed = 1.0) {
    if (!sim->ent_alive(ent.get_parent())) {
        // Zone-spawned wilds: walk back toward spawn home (not zone AABB center —
        // outer difficulty rings cover the whole arena, so AABB center == hub).
        if (BitMath::at(ent.flags, EntityFlags::kSpawnedFromZone)) {
            float cx = ent.home_x;
            float cy = ent.home_y;
            Vector delta(cx - ent.get_x(), cy - ent.get_y());
            float dist = delta.magnitude();
            if (dist < 160.f || ent.ai_tick > 5 * TPS) {
                ent.ai_tick = 0;
                ent.ai_state = AIState::kIdle;
                return;
            }
            delta.set_magnitude(PLAYER_ACCELERATION * speed);
            ent.acceleration = delta;
            ent.set_angle(delta.angle());
            return;
        }
        ent.ai_tick = 0;
        ent.ai_state = AIState::kIdle;
        return;
    }
    Entity &parent = sim->get_ent(ent.get_parent());
    Vector delta(parent.get_x() - ent.get_x(), parent.get_y() - ent.get_y());
    if (delta.magnitude() > 300) {
        ent.ai_tick = 0;
    } else if (ent.ai_tick > 2 * TPS || delta.magnitude() < 100) {
        ent.ai_tick = 0;
        ent.ai_state = AIState::kIdle;
        return;
    } 
    delta.set_magnitude(PLAYER_ACCELERATION * speed);
    ent.acceleration = delta;
    ent.set_angle(delta.angle());
}

static void _maybe_start_wild_leash(Simulation *sim, Entity &ent) {
    if (!BitMath::at(ent.flags, EntityFlags::kSpawnedFromZone)) return;
    if (sim->ent_alive(ent.get_parent())) return;
    float leash = WILD_LEASH_MULT * (ent.detection_radius > 1.f ? ent.detection_radius : 500.f);
    float dx = ent.get_x() - ent.home_x;
    float dy = ent.get_y() - ent.home_y;
    if (dx * dx + dy * dy > leash * leash) {
        ent.target = NULL_ENTITY;
        ent.ai_state = AIState::kReturning;
        ent.ai_tick = 0;
    }
}


static void tick_default_passive(Simulation *sim, Entity &ent) {
    switch(ent.ai_state) {
        case AIState::kIdle: {
            default_tick_idle(sim, ent);
            break;
        }
        case AIState::kIdleMoving: {
            default_tick_idle_moving(sim, ent);
            break;
        }
        case AIState::kReturning: {
            default_tick_returning(sim, ent);
            break;
        }
        default:
            ent.ai_state = AIState::kIdle;
            break;
    }
}

static void tick_default_neutral(Simulation *sim, Entity &ent) {
    if (sim->ent_alive(ent.target)) {
        Entity &target = sim->get_ent(ent.target);
        Vector v(target.get_x() - ent.get_x(), target.get_y() - ent.get_y());
        v.set_magnitude(PLAYER_ACCELERATION * 0.975);
        ent.acceleration = v;
        ent.set_angle(v.angle());
        return;
    } else {
        if (!(ent.target == NULL_ENTITY)) {
            ent.target = NULL_ENTITY;
            ent.ai_state = AIState::kIdle;
            ent.ai_tick = 0;
        }
        tick_default_passive(sim, ent);
    }
}

static void tick_default_aggro(Simulation *sim, Entity &ent, float speed) {
    if (sim->ent_alive(ent.target)) {
        Entity &target = sim->get_ent(ent.target);
        Vector v(target.get_x() - ent.get_x(), target.get_y() - ent.get_y());
        _focus_lose_clause(ent, v);
        if (!sim->ent_alive(ent.target)) {
            _maybe_start_wild_leash(sim, ent);
            tick_default_passive(sim, ent);
            return;
        }
        v.set_magnitude(PLAYER_ACCELERATION * speed);
        ent.acceleration = v;
        ent.set_angle(v.angle());
        return;
    } else {
        if (!(ent.target == NULL_ENTITY)) {
            ent.ai_state = AIState::kIdle;
            ent.ai_tick = 0;
            _maybe_start_wild_leash(sim, ent);
        }
        if (ent.ai_state == AIState::kReturning) {
            tick_default_passive(sim, ent);
            return;
        }
        ent.target = find_nearest_enemy(sim, ent, ent.detection_radius + ent.get_radius());
        if (!sim->ent_alive(ent.target))
            _maybe_start_wild_leash(sim, ent);
        tick_default_passive(sim, ent);
    }
}

static void tick_flee(Simulation *sim, Entity &ent, float speed) {
    if (sim->ent_alive(ent.target)) {
        Entity &target = sim->get_ent(ent.target);
        Vector v(ent.get_x() - target.get_x(), ent.get_y() - target.get_y());
        _focus_lose_clause(ent, v);
        v.set_magnitude(PLAYER_ACCELERATION * speed);
        ent.acceleration = v;
        ent.set_angle(v.angle());
        return;
    } else {
        if (!(ent.target == NULL_ENTITY)) {
            ent.ai_state = AIState::kIdle;
            ent.ai_tick = 0;
        }
        ent.target = find_nearest_enemy(sim, ent, ent.detection_radius + ent.get_radius());
        tick_default_passive(sim, ent);
    }
}

static void tick_mimic_wake(Simulation *sim, Entity &ent, float speed) {
    if (sim->ent_alive(ent.target)) {
        // Short lunge then recover
        if (ent.ai_state == 2) {
            if (ent.ai_tick > (game_tick_t)(0.35f * TPS)) {
                ent.ai_state = 3;
                ent.ai_tick = 0;
            }
            return;
        }
        if (ent.ai_state == 3) {
            ent.acceleration.set(0, 0);
            if (ent.ai_tick > (game_tick_t)(1.2f * TPS)) {
                ent.ai_state = AIState::kBasicAggro;
                ent.ai_tick = 0;
            }
            return;
        }
        Entity &target = sim->get_ent(ent.target);
        Vector v(target.get_x() - ent.get_x(), target.get_y() - ent.get_y());
        float dist = v.magnitude();
        if (dist < 220.f && ent.ai_tick > (game_tick_t)(1.0f * TPS)) {
            v.set_magnitude(PLAYER_ACCELERATION * 2.4f);
            ent.acceleration = v;
            ent.velocity = v * 0.6f;
            ent.set_angle(v.angle());
            ent.ai_state = 2;
            ent.ai_tick = 0;
            return;
        }
        tick_default_aggro(sim, ent, speed);
        return;
    }
    if (!(ent.target == NULL_ENTITY)) {
        ent.ai_state = AIState::kIdle;
        ent.ai_tick = 0;
    }
    EntityID enemy = find_nearest_enemy(sim, ent, ent.detection_radius);
    if (sim->ent_alive(enemy)) {
        ent.target = enemy;
        ent.ai_state = AIState::kBasicAggro;
        tick_default_aggro(sim, ent, speed);
    } else {
        ent.target = NULL_ENTITY;
        // Stay still while disguised
        ent.acceleration.set(0, 0);
    }
}

static void _fire_missile_fan(Simulation *sim, Entity &ent, float base_ang,
                              uint32_t count, float spread, float dmg, float life_s) {
    for (uint32_t i = 0; i < count; ++i) {
        float ang = base_ang;
        if (count > 1)
            ang += -spread * 0.5f + spread * ((float)i / (float)(count - 1));
        Entity &missile = alloc_petal(sim, PetalID::kMissile, ent);
        missile.damage = dmg;
        missile.health = missile.max_health = 8.f;
        missile.friction = DEFAULT_FRICTION;
        BitMath::set(missile.flags, EntityFlags::kIsDetached);
        entity_set_despawn_tick(missile, (game_tick_t)(life_s * TPS));
        missile.set_angle(ang);
        missile.velocity.unit_normal(ang).set_magnitude(14.f * PLAYER_ACCELERATION);
    }
}

static uint32_t _count_children(Simulation *sim, EntityID parent) {
    uint32_t n = 0;
    sim->for_each<kMob>([&](Simulation *s, Entity &m) {
        if (m.get_parent() == parent && s->ent_alive(m.id)) ++n;
    });
    return n;
}

static void _clamp_to_disk(Entity &ent, float cx, float cy, float max_r) {
    Vector d(ent.get_x() - cx, ent.get_y() - cy);
    float m = d.magnitude();
    if (m > max_r) {
        d.set_magnitude(max_r);
        ent.set_x(cx + d.x);
        ent.set_y(cy + d.y);
        ent.velocity *= 0.3f;
    }
}

// ai_state: 0 chase, 1 windup, 2 dash
static void tick_charge_mob(Simulation *sim, Entity &ent, float chase_spd,
                            float dash_spd, float windup_s, float dash_s, float cd_s) {
    if (!sim->ent_alive(ent.target)) {
        if (!(ent.target == NULL_ENTITY)) {
            ent.ai_state = AIState::kIdle;
            ent.ai_tick = 0;
            ent.target = NULL_ENTITY;
        }
        ent.target = find_nearest_enemy(sim, ent, ent.detection_radius + ent.get_radius());
        tick_default_passive(sim, ent);
        return;
    }
    Entity &target = sim->get_ent(ent.target);
    Vector v(target.get_x() - ent.get_x(), target.get_y() - ent.get_y());
    _focus_lose_clause(ent, v);
    if (!sim->ent_alive(ent.target)) return;
    float dist = v.magnitude();
    ent.set_angle(v.angle());
    if (ent.ai_state == 1) {
        ent.acceleration.set(0, 0);
        if (ent.ai_tick >= (game_tick_t)(windup_s * TPS)) {
            ent.ai_state = 2;
            ent.ai_tick = 0;
            ent.heading_angle = v.angle();
        }
        return;
    }
    if (ent.ai_state == 2) {
        Vector dash;
        dash.unit_normal(ent.heading_angle).set_magnitude(PLAYER_ACCELERATION * dash_spd);
        ent.acceleration = dash;
        if (ent.ai_tick >= (game_tick_t)(dash_s * TPS)) {
            ent.ai_state = 0;
            ent.ai_tick = 0;
            ent.secondary_reload = (game_tick_t)(cd_s * TPS);
        }
        return;
    }
    if (ent.secondary_reload > 0) --ent.secondary_reload;
    if (ent.secondary_reload == 0 && dist < 700.f && dist > 80.f) {
        ent.ai_state = 1;
        ent.ai_tick = 0;
        ent.acceleration.set(0, 0);
        return;
    }
    v.set_magnitude(PLAYER_ACCELERATION * chase_spd);
    ent.acceleration = v;
}

static void tick_thorn_bug(Simulation *sim, Entity &ent) {
    if (sim->ent_alive(ent.target)) {
        Entity &target = sim->get_ent(ent.target);
        Vector v(target.get_x() - ent.get_x(), target.get_y() - ent.get_y());
        _focus_lose_clause(ent, v);
        float dist = v.magnitude();
        if (dist > 280.f) {
            v.set_magnitude(PLAYER_ACCELERATION * 0.7f);
            ent.acceleration = v;
        } else {
            ent.acceleration.set(0, 0);
        }
        ent.set_angle(v.angle());
        if (ent.ai_tick >= (game_tick_t)(2.2f * TPS) && dist < 900.f) {
            ent.ai_tick = 0;
            _fire_missile_fan(sim, ent, ent.get_angle(), 6, (float)(2.0 * M_PI), 9.f, 2.2f);
        }
        return;
    }
    if (!(ent.target == NULL_ENTITY)) {
        ent.ai_state = AIState::kIdle;
        ent.ai_tick = 0;
        ent.target = NULL_ENTITY;
    }
    ent.target = find_nearest_enemy(sim, ent, ent.detection_radius);
    tick_default_passive(sim, ent);
}

static void tick_phantom_orb(Simulation *sim, Entity &ent) {
    if (sim->ent_alive(ent.target)) {
        Entity &target = sim->get_ent(ent.target);
        Vector v(target.get_x() - ent.get_x(), target.get_y() - ent.get_y());
        _focus_lose_clause(ent, v);
        float dist = v.magnitude();
        // Blink every ~2.8s to a nearby clear point toward/around the target
        if (ent.ai_tick >= (game_tick_t)(2.8f * TPS)) {
            ent.ai_tick = 0;
            float ang = v.angle() + (frand() - 0.5f) * 1.8f;
            float leap = 280.f + frand() * 220.f;
            float nx = ent.get_x() + cosf(ang) * leap;
            float ny = ent.get_y() + sinf(ang) * leap;
            nx = fclamp(nx, ent.get_radius() + 40.f, ARENA_WIDTH - ent.get_radius() - 40.f);
            ny = fclamp(ny, ent.get_radius() + 40.f, ARENA_HEIGHT - ent.get_radius() - 40.f);
            if (!Map::circle_hits_impassable(sim->map_id, nx, ny, ent.get_radius() + 8.f)) {
                ent.set_x(nx);
                ent.set_y(ny);
                ent.velocity.set(0, 0);
            }
        }
        v.set_magnitude(PLAYER_ACCELERATION * (dist < 200.f ? 1.35f : 1.1f));
        ent.acceleration = v;
        ent.set_angle(v.angle());
        return;
    }
    if (!(ent.target == NULL_ENTITY)) {
        ent.ai_state = AIState::kIdle;
        ent.ai_tick = 0;
        ent.target = NULL_ENTITY;
    }
    ent.target = find_nearest_enemy(sim, ent, ent.detection_radius + ent.get_radius());
    tick_default_passive(sim, ent);
}

// Palace boss arena center (must match gen_royal palace coords)
static float const kKingArenaX = ARENA_WIDTH * 0.5f + 4200.f;
static float const kKingArenaY = ARENA_HEIGHT * 0.5f - 1800.f;
static float const kKingArenaR = 1900.f;

// Jungle Heart boss — must match MapGen::gen_jungle boss anchor (tree-heart valley)
static float const kTreantArenaX = ARENA_WIDTH * 0.80f;
static float const kTreantArenaY = ARENA_HEIGHT * 0.22f;
static float const kTreantArenaR = 1600.f;

static void tick_ancient_treant(Simulation *sim, Entity &ent) {
    _clamp_to_disk(ent, kTreantArenaX, kTreantArenaY, kTreantArenaR);
    float hp = ent.max_health > 0.f ? ent.health / ent.max_health : 0.f;
    uint8_t phase = hp > 0.65f ? 1 : (hp > 0.30f ? 2 : 3);
    float chase = phase == 1 ? 0.55f : (phase == 2 ? 0.70f : 0.85f);
    float skill_cd = phase == 1 ? 2.8f : (phase == 2 ? 2.2f : 1.7f);

    if (!sim->ent_alive(ent.target)) {
        if (!(ent.target == NULL_ENTITY)) {
            ent.ai_state = 0;
            ent.ai_tick = 0;
            ent.target = NULL_ENTITY;
        }
        ent.target = find_nearest_enemy(sim, ent, ent.detection_radius + ent.get_radius());
        if (!sim->ent_alive(ent.target)) {
            tick_default_passive(sim, ent);
            return;
        }
    }
    Entity &target = sim->get_ent(ent.target);
    Vector v(target.get_x() - ent.get_x(), target.get_y() - ent.get_y());
    _focus_lose_clause(ent, v);
    if (!sim->ent_alive(ent.target)) return;
    float dist = v.magnitude();
    ent.set_angle(v.angle());

    // ai_state 1 = root windup (still), 2 = slam recover
    if (ent.ai_state == 1) {
        ent.acceleration.set(0, 0);
        if (ent.ai_tick >= (game_tick_t)(0.65f * TPS)) {
            ent.ai_state = 2;
            ent.ai_tick = 0;
            uint32_t n = phase == 1 ? 4 : (phase == 2 ? 7 : 10);
            _fire_missile_fan(sim, ent, v.angle(), n, (float)(1.4 + phase * 0.35), 10.f + phase * 3.f, 2.8f);
        }
        return;
    }
    if (ent.ai_state == 2) {
        ent.acceleration.set(0, 0);
        if (ent.ai_tick >= (game_tick_t)(0.55f * TPS)) {
            ent.ai_state = 0;
            ent.ai_tick = 0;
        }
        return;
    }

    if (ent.ai_tick >= (game_tick_t)(skill_cd * TPS)) {
        ent.ai_tick = 0;
        float roll = frand();
        if (roll < 0.45f && dist < 900.f) {
            ent.ai_state = 1;
            return;
        }
        if (roll < 0.80f || phase < 2) {
            uint32_t n = phase + 2;
            _fire_missile_fan(sim, ent, v.angle(), n, 0.7f + phase * 0.2f, 11.f + phase * 2.f, 2.4f);
        } else {
            uint32_t kids = _count_children(sim, ent.id);
            uint32_t cap = phase >= 3 ? 4 : 3;
            if (kids < cap) {
                MobID::T sid = phase >= 3 ? MobID::kVineBug : MobID::kLeafBeetle;
                float ox = ent.get_x() + (frand() - 0.5f) * 240.f;
                float oy = ent.get_y() + (frand() - 0.5f) * 240.f;
                Entity &spawned = alloc_mob(sim, sid, ox, oy, ent.get_team(), [](Entity &m) {
                    m.score_reward = MOB_DATA[m.get_mob_id()].xp / 2;
                    BitMath::set(m.flags, EntityFlags::kHasCulling);
                    BitMath::set(m.flags, EntityFlags::kDieOnParentDeath);
                    BitMath::set(m.flags, EntityFlags::kNoDrops);
                });
                spawned.set_parent(ent.id);
                spawned.target = ent.target;
                entity_set_despawn_tick(spawned, 22 * TPS);
            }
        }
    }

    if (dist > 140.f) {
        v.set_magnitude(PLAYER_ACCELERATION * chase);
        ent.acceleration = v;
    } else {
        ent.acceleration.set(0, 0);
    }
    _clamp_to_disk(ent, kTreantArenaX, kTreantArenaY, kTreantArenaR);
}

static void tick_fallen_king(Simulation *sim, Entity &ent) {
    _clamp_to_disk(ent, kKingArenaX, kKingArenaY, kKingArenaR);
    float hp = ent.max_health > 0.f ? ent.health / ent.max_health : 0.f;
    uint8_t phase = hp > 0.70f ? 1 : (hp > 0.35f ? 2 : 3);
    float chase = phase == 1 ? 0.75f : (phase == 2 ? 0.85f : 1.0f);
    float skill_cd = phase == 1 ? 2.4f : (phase == 2 ? 1.9f : 1.45f);

    if (!sim->ent_alive(ent.target)) {
        if (!(ent.target == NULL_ENTITY)) {
            ent.ai_state = 0;
            ent.ai_tick = 0;
            ent.target = NULL_ENTITY;
        }
        ent.target = find_nearest_enemy(sim, ent, ent.detection_radius + ent.get_radius());
        if (!sim->ent_alive(ent.target)) {
            tick_default_passive(sim, ent);
            return;
        }
    }
    Entity &target = sim->get_ent(ent.target);
    Vector v(target.get_x() - ent.get_x(), target.get_y() - ent.get_y());
    _focus_lose_clause(ent, v);
    if (!sim->ent_alive(ent.target)) return;
    float dist = v.magnitude();
    ent.set_angle(v.angle());

    // ai_state 1 = charge windup, 2 = charging
    if (ent.ai_state == 1) {
        ent.acceleration.set(0, 0);
        if (ent.ai_tick >= (game_tick_t)(0.55f * TPS)) {
            ent.ai_state = 2;
            ent.ai_tick = 0;
            ent.heading_angle = v.angle();
            // Second charge in phase 3 queued via secondary_reload flag bit
            if (phase >= 3) ent.secondary_reload = 1;
        }
        return;
    }
    if (ent.ai_state == 2) {
        Vector dash;
        dash.unit_normal(ent.heading_angle).set_magnitude(PLAYER_ACCELERATION * 2.6f);
        ent.acceleration = dash;
        if (ent.ai_tick >= (game_tick_t)(0.45f * TPS)) {
            if (phase >= 3 && ent.secondary_reload == 1) {
                // Immediate second charge toward current target
                ent.secondary_reload = 0;
                ent.ai_state = 1;
                ent.ai_tick = 0;
                return;
            }
            ent.ai_state = 0;
            ent.ai_tick = 0;
        }
        return;
    }

    // Periodic skills
    if (ent.ai_tick >= (game_tick_t)(skill_cd * TPS)) {
        ent.ai_tick = 0;
        float roll = frand();
        if (roll < 0.40f && dist < 1100.f) {
            // Charge (telegraph via windup stillness)
            ent.ai_state = 1;
            return;
        }
        if (roll < 0.75f) {
            uint32_t n = phase == 1 ? 3 : (phase == 2 ? 8 : 10);
            float spread = phase == 1 ? 0.9f : (float)(2.0 * M_PI);
            _fire_missile_fan(sim, ent, v.angle(), n, spread, 12.f + phase * 2.f, 2.5f);
        } else if (phase >= 2) {
            uint32_t kids = _count_children(sim, ent.id);
            uint32_t cap = phase >= 3 ? 4 : 3;
            if (kids < cap) {
                MobID::T sid = phase >= 3 ? MobID::kSpearSentinel : MobID::kShieldSentinel;
                float ox = ent.get_x() + (frand() - 0.5f) * 200.f;
                float oy = ent.get_y() + (frand() - 0.5f) * 200.f;
                Entity &spawned = alloc_mob(sim, sid, ox, oy, ent.get_team(), [](Entity &m) {
                    m.score_reward = MOB_DATA[m.get_mob_id()].xp / 2;
                    BitMath::set(m.flags, EntityFlags::kHasCulling);
                    BitMath::set(m.flags, EntityFlags::kDieOnParentDeath);
                    BitMath::set(m.flags, EntityFlags::kNoDrops);
                });
                spawned.set_parent(ent.id);
                spawned.target = ent.target;
                entity_set_despawn_tick(spawned, 25 * TPS);
            }
        }
    }

    if (dist > 120.f) {
        v.set_magnitude(PLAYER_ACCELERATION * chase);
        ent.acceleration = v;
    } else {
        ent.acceleration.set(0, 0);
    }
    _clamp_to_disk(ent, kKingArenaX, kKingArenaY, kKingArenaR);
}

static void tick_bee_passive(Simulation *sim, Entity &ent) {
    switch(ent.ai_state) {
        case AIState::kIdle: {
            if (ent.ai_tick >= 5 * TPS) {
                ent.ai_tick = 0;
                ent.set_angle(frand() * 2 * M_PI);
                ent.ai_state = AIState::kIdle;
            }
            ent.set_angle(ent.get_angle() + 1.5 * sinf(((float) ent.lifetime) / (TPS / 2)) / TPS);
            Vector v(cosf(ent.get_angle()), sinf(ent.get_angle()));
            v *= 1.5;
            if (ent.lifetime % (TPS * 3 / 2) < TPS / 2)
                v *= 0.5;
            ent.acceleration = v;
            break;
        }
        case AIState::kIdleMoving: {
            break;
        }
        case AIState::kReturning: {
            default_tick_returning(sim, ent);
            break;
        }
        default:
            ent.ai_state = AIState::kIdle;
            break;
    }
}

// ai_state: 0 chase/ready, 2 windup, 3 post-shot cooldown
static void tick_hornet_aggro(Simulation *sim, Entity &ent) {
    if (sim->ent_alive(ent.target)) {
        Entity &target = sim->get_ent(ent.target);
        Vector v(target.get_x() - ent.get_x(), target.get_y() - ent.get_y());
        _focus_lose_clause(ent, v);
        if (!sim->ent_alive(ent.target)) {
            ent.ai_state = AIState::kIdle;
            ent.ai_tick = 0;
            return;
        }
        float dist = v.magnitude();
        ent.set_angle(v.angle());

        if (ent.ai_state == 2) {
            // Windup: slight retreat, then fire
            if (dist > 1.f) {
                Vector back = v;
                back.set_magnitude(-0.15f * PLAYER_ACCELERATION);
                ent.acceleration = back;
            } else {
                ent.acceleration.set(0, 0);
            }
            if (ent.ai_tick >= (game_tick_t)(0.45f * TPS)) {
                ent.ai_tick = 0;
                ent.ai_state = 3;
                Entity &missile = alloc_petal(sim, PetalID::kMissile, ent);
                missile.damage = 14;
                missile.health = missile.max_health = 12;
                missile.friction = DEFAULT_FRICTION;
                entity_set_despawn_tick(missile, 3 * TPS);
                missile.set_angle(ent.get_angle());
                missile.velocity.unit_normal(ent.get_angle()).set_magnitude(15 * PLAYER_ACCELERATION);
                Vector kb;
                kb.unit_normal(ent.get_angle() - M_PI).set_magnitude(2.5f * PLAYER_ACCELERATION);
                ent.velocity += kb;
            }
            return;
        }

        if (ent.ai_state == 3) {
            if (dist > 300) {
                v.set_magnitude(PLAYER_ACCELERATION * 0.975f);
                ent.acceleration = v;
            } else {
                ent.acceleration.set(0, 0);
            }
            if (ent.ai_tick >= (game_tick_t)(1.2f * TPS)) {
                ent.ai_state = 0;
                ent.ai_tick = 0;
            }
            return;
        }

        // Chase / ready
        if (dist > 300) {
            v.set_magnitude(PLAYER_ACCELERATION * 0.975f);
            ent.acceleration = v;
        } else {
            ent.acceleration.set(0, 0);
        }
        if (dist < 800 && ent.ai_tick >= (game_tick_t)(0.35f * TPS)) {
            ent.ai_state = 2;
            ent.ai_tick = 0;
        }
        return;
    } else {
        if (!(ent.target == NULL_ENTITY)) {
            ent.ai_state = AIState::kIdle;
            ent.ai_tick = 0;
            ent.target = NULL_ENTITY;
        }
        ent.target = find_nearest_enemy(sim, ent, ent.detection_radius);
        tick_bee_passive(sim, ent);
    }
}

static void tick_centipede_passive(Simulation *sim, Entity &ent) {
    switch(ent.ai_state) {
        case AIState::kIdle: {
            ent.set_angle(ent.get_angle() + 0.25 / TPS);
            if (frand() < 1 / (5.0 * TPS)) ent.ai_state = AIState::kIdleMoving;
            break;
        }
        case AIState::kIdleMoving: {
            ent.set_angle(ent.get_angle() - 0.25 / TPS);
            if (frand() < 1 / (5.0 * TPS)) ent.ai_state = AIState::kIdle;
            break;
        }
        case AIState::kReturning: {
            default_tick_returning(sim, ent);
            break;
        }
    }
    ent.acceleration.unit_normal(ent.get_angle()).set_magnitude(PLAYER_ACCELERATION / 10);
}

static void tick_centipede_neutral(Simulation *sim, Entity &ent, float speed) {
    if (sim->ent_alive(ent.target)) {
        Entity &target = sim->get_ent(ent.target);
        Vector v(target.get_x() - ent.get_x(), target.get_y() - ent.get_y());
        v.set_magnitude(PLAYER_ACCELERATION * speed);
        ent.acceleration = v;
        ent.set_angle(v.angle());
        return;
    } else {
        if (!(ent.target == NULL_ENTITY)) {
            ent.ai_state = AIState::kIdle;
            ent.ai_tick = 0;
        }
        switch(ent.ai_state) {
            case AIState::kIdle: {
                ent.set_angle(ent.get_angle() + 0.25 / TPS);
                if (frand() < 1 / (5.0 * TPS)) ent.ai_state = AIState::kIdleMoving;
                ent.acceleration.unit_normal(ent.get_angle()).set_magnitude(PLAYER_ACCELERATION * speed);
                break;
            }
            case AIState::kIdleMoving: {
                ent.set_angle(ent.get_angle() - 0.25 / TPS);
                if (frand() < 1 / (5.0 * TPS)) ent.ai_state = AIState::kIdle;
                ent.acceleration.unit_normal(ent.get_angle()).set_magnitude(PLAYER_ACCELERATION * speed);
                break;
            }
            case AIState::kReturning: {
                default_tick_returning(sim, ent);
                break;
            }
        }
    }
}

static void tick_centipede_aggro(Simulation *sim, Entity &ent) {
    if (sim->ent_alive(ent.target)) {
        Entity &target = sim->get_ent(ent.target);
        Vector v(target.get_x() - ent.get_x(), target.get_y() - ent.get_y());
        _focus_lose_clause(ent, v);
        v.set_magnitude(PLAYER_ACCELERATION * 0.95);
        ent.acceleration = v;
        ent.set_angle(v.angle());
        return;
    } else {
        if (!(ent.target == NULL_ENTITY)) {
            ent.ai_state = AIState::kIdle;
            ent.ai_tick = 0;
        }
        ent.target = find_nearest_enemy(sim, ent, ent.detection_radius + ent.get_radius());
        switch(ent.ai_state) {
            case AIState::kIdle: {
                ent.set_angle(ent.get_angle() + 0.25 / TPS);
                if (frand() < 1 / (5.0 * TPS)) ent.ai_state = AIState::kIdleMoving;
                ent.acceleration.unit_normal(ent.get_angle()).set_magnitude(PLAYER_ACCELERATION / 10);
                break;
            }
            case AIState::kIdleMoving: {
                ent.set_angle(ent.get_angle() - 0.25 / TPS);
                if (frand() < 1 / (5.0 * TPS)) ent.ai_state = AIState::kIdle;
                ent.acceleration.unit_normal(ent.get_angle()).set_magnitude(PLAYER_ACCELERATION / 10);
                break;
            }
            case AIState::kReturning: {
                default_tick_returning(sim, ent);
                break;
            }
        }
    }
}

static void tick_sandstorm(Simulation *sim, Entity &ent) {
    switch(ent.ai_state) {
        case AIState::kIdle: {
            if (frand() > 1.0f / TPS) {
                ent.ai_tick = 0;
                ent.heading_angle = frand() * 2 * M_PI;
                ent.ai_state = AIState::kIdleMoving;
            }
            Vector rand = Vector::rand(PLAYER_ACCELERATION * 0.5);
            ent.acceleration.set(rand.x, rand.y);
            break;
        }
        case AIState::kIdleMoving: {
            if (ent.ai_tick >= 2.5 * TPS) {
                ent.ai_tick = 0;
                ent.ai_state = AIState::kIdle;
            }
            if (frand() > 2.5f / TPS)
                ent.heading_angle += frand() * M_PI - M_PI / 2;
            Vector head;
            head.unit_normal(ent.heading_angle);
            head.set_magnitude(PLAYER_ACCELERATION);
            Vector rand;
            rand.unit_normal(ent.heading_angle + frand() * M_PI - M_PI / 2);
            rand.set_magnitude(PLAYER_ACCELERATION * 0.5);
            head += rand;
            ent.acceleration.set(head.x, head.y);
            break;
        }
        case AIState::kReturning: {
            default_tick_returning(sim, ent, 1.5);
            break;
        }
        default:
            ent.ai_state = AIState::kIdle;
            break;
    }
    if (sim->ent_alive(ent.get_parent())) {
        Entity &parent = sim->get_ent(ent.get_parent());
        ent.acceleration = (ent.acceleration + parent.acceleration) * 0.75;
    }
}

// ---- Ocean mobs ----

// Passive drifter: wanders slowly, only fights back after being hit.
static void tick_sea_fry(Simulation *sim, Entity &ent) {
    if (sim->ent_alive(ent.target)) {
        Entity &target = sim->get_ent(ent.target);
        Vector v(target.get_x() - ent.get_x(), target.get_y() - ent.get_y());
        v.set_magnitude(PLAYER_ACCELERATION * 0.65f);
        ent.acceleration = v;
        ent.set_angle(v.angle());
        return;
    }
    tick_default_passive(sim, ent);
}

// Lays a slowing web patch on a timer (IceSpider / DuneSpider style).
static void tick_web_layer(Simulation *sim, Entity &ent, float speed, float web_radius) {
    if (ent.lifetime % (8 * TPS) == 0)
        alloc_web(sim, web_radius, ent);
    tick_default_aggro(sim, ent, speed);
}

// Ambush predator: stays disguised until a player comes close, then lunges.
static void tick_ink_cuttle(Simulation *sim, Entity &ent) {
    if (sim->ent_alive(ent.target)) {
        Entity &target = sim->get_ent(ent.target);
        Vector v(target.get_x() - ent.get_x(), target.get_y() - ent.get_y());
        float dist = v.magnitude();
        ent.set_angle(v.angle());
        if (ent.ai_state == 2) {
            // Lunge burst
            v.set_magnitude(PLAYER_ACCELERATION * 1.3f);
            ent.acceleration = v;
            if (ent.ai_tick >= (game_tick_t)(1.2f * TPS)) {
                ent.ai_state = 0;
                ent.ai_tick = 0;
            }
            return;
        }
        if (dist < 180.f && ent.ai_tick >= (game_tick_t)(1.0f * TPS)) {
            ent.ai_state = 2;
            ent.ai_tick = 0;
            ent.acceleration.set(0, 0);
            return;
        }
        // Slow drift toward the target while disguised
        v.set_magnitude(PLAYER_ACCELERATION * 0.5f);
        ent.acceleration = v;
        return;
    }
    if (!(ent.target == NULL_ENTITY)) {
        ent.ai_state = 0;
        ent.ai_tick = 0;
        ent.target = NULL_ENTITY;
    }
    ent.target = find_nearest_enemy(sim, ent, ent.detection_radius);
    if (sim->ent_alive(ent.target)) {
        // Hold still until the player is in striking range
        ent.acceleration.set(0, 0);
    }
}

// Abyss boss: phase 1 chase, phase 2 charge, phase 3 chase + summon fry.
static void tick_abyss_maw(Simulation *sim, Entity &ent) {
    float hp = ent.max_health > 0.f ? ent.health / ent.max_health : 0.f;
    uint8_t phase = hp > 0.70f ? 1 : (hp > 0.35f ? 2 : 3);
    float chase = phase == 1 ? 0.70f : (phase == 2 ? 0.85f : 1.0f);

    if (!sim->ent_alive(ent.target)) {
        if (!(ent.target == NULL_ENTITY)) {
            ent.ai_state = 0;
            ent.ai_tick = 0;
            ent.target = NULL_ENTITY;
        }
        ent.target = find_nearest_enemy(sim, ent, ent.detection_radius + ent.get_radius());
        if (!sim->ent_alive(ent.target)) {
            tick_default_passive(sim, ent);
            return;
        }
    }
    Entity &target = sim->get_ent(ent.target);
    Vector v(target.get_x() - ent.get_x(), target.get_y() - ent.get_y());
    _focus_lose_clause(ent, v);
    if (!sim->ent_alive(ent.target)) return;
    float dist = v.magnitude();
    ent.set_angle(v.angle());

    // ai_state 1 = charge windup, 2 = charging
    if (ent.ai_state == 1) {
        ent.acceleration.set(0, 0);
        if (ent.ai_tick >= (game_tick_t)(0.8f * TPS)) {
            ent.ai_state = 2;
            ent.ai_tick = 0;
            ent.heading_angle = v.angle();
        }
        return;
    }
    if (ent.ai_state == 2) {
        Vector dash;
        dash.unit_normal(ent.heading_angle).set_magnitude(PLAYER_ACCELERATION * 2.4f);
        ent.acceleration = dash;
        if (ent.ai_tick >= (game_tick_t)(0.45f * TPS)) {
            ent.ai_state = 0;
            ent.ai_tick = 0;
            ent.secondary_reload = (game_tick_t)(7.0f * TPS);
        }
        return;
    }

    if (ent.secondary_reload > 0) --ent.secondary_reload;
    if (ent.secondary_reload == 0 && dist < 700.f && dist > 120.f && phase >= 2) {
        ent.ai_state = 1;
        ent.ai_tick = 0;
        ent.acceleration.set(0, 0);
        return;
    }

    // Phase 3 periodically summons a Sea Fry behind it
    if (phase == 3 && ent.lifetime % (8 * TPS) == 0) {
        uint32_t kids = _count_children(sim, ent.id);
        if (kids < 3) {
            Vector behind;
            behind.unit_normal(ent.get_angle() + M_PI);
            behind *= ent.get_radius();
            Entity &spawned = alloc_mob(sim, MobID::kSeaFry,
                ent.get_x() + behind.x, ent.get_y() + behind.y, ent.get_team(),
                [](Entity &m) {
                    m.score_reward = MOB_DATA[m.get_mob_id()].xp;
                    BitMath::set(m.flags, EntityFlags::kHasCulling);
                    BitMath::set(m.flags, EntityFlags::kDieOnParentDeath);
                    BitMath::set(m.flags, EntityFlags::kNoDrops);
                });
            spawned.set_parent(ent.id);
            entity_set_despawn_tick(spawned, 10 * TPS);
        }
    }

    if (dist > 130.f) {
        v.set_magnitude(PLAYER_ACCELERATION * chase);
        ent.acceleration = v;
    } else {
        ent.acceleration.set(0, 0);
    }
}

// Winter boss: phase 1 chase, phase 2 charge, phase 3 chase + summon mites.
static void tick_winter_sovereign(Simulation *sim, Entity &ent) {
    float hp = ent.max_health > 0.f ? ent.health / ent.max_health : 0.f;
    uint8_t phase = hp > 0.70f ? 1 : (hp > 0.35f ? 2 : 3);
    float chase = phase == 1 ? 0.70f : (phase == 2 ? 0.85f : 1.0f);

    if (!sim->ent_alive(ent.target)) {
        if (!(ent.target == NULL_ENTITY)) {
            ent.ai_state = 0;
            ent.ai_tick = 0;
            ent.target = NULL_ENTITY;
        }
        ent.target = find_nearest_enemy(sim, ent, ent.detection_radius + ent.get_radius());
        if (!sim->ent_alive(ent.target)) {
            tick_default_passive(sim, ent);
            return;
        }
    }
    Entity &target = sim->get_ent(ent.target);
    Vector v(target.get_x() - ent.get_x(), target.get_y() - ent.get_y());
    _focus_lose_clause(ent, v);
    if (!sim->ent_alive(ent.target)) return;
    float dist = v.magnitude();
    ent.set_angle(v.angle());

    // ai_state 1 = charge windup, 2 = charging
    if (ent.ai_state == 1) {
        ent.acceleration.set(0, 0);
        if (ent.ai_tick >= (game_tick_t)(0.9f * TPS)) {
            ent.ai_state = 2;
            ent.ai_tick = 0;
            ent.heading_angle = v.angle();
        }
        return;
    }
    if (ent.ai_state == 2) {
        Vector dash;
        dash.unit_normal(ent.heading_angle).set_magnitude(PLAYER_ACCELERATION * 2.3f);
        ent.acceleration = dash;
        if (ent.ai_tick >= (game_tick_t)(0.45f * TPS)) {
            ent.ai_state = 0;
            ent.ai_tick = 0;
            ent.secondary_reload = (game_tick_t)(7.0f * TPS);
        }
        return;
    }

    if (ent.secondary_reload > 0) --ent.secondary_reload;
    if (ent.secondary_reload == 0 && dist < 700.f && dist > 120.f && phase >= 2) {
        ent.ai_state = 1;
        ent.ai_tick = 0;
        ent.acceleration.set(0, 0);
        return;
    }

    // Phase 3 periodically summons a Snow Mite behind it
    if (phase == 3 && ent.lifetime % (10 * TPS) == 0) {
        uint32_t kids = _count_children(sim, ent.id);
        if (kids < 3) {
            Vector behind;
            behind.unit_normal(ent.get_angle() + M_PI);
            behind *= ent.get_radius();
            Entity &spawned = alloc_mob(sim, MobID::kSnowMite,
                ent.get_x() + behind.x, ent.get_y() + behind.y, ent.get_team(),
                [](Entity &m) {
                    m.score_reward = MOB_DATA[m.get_mob_id()].xp;
                    BitMath::set(m.flags, EntityFlags::kHasCulling);
                    BitMath::set(m.flags, EntityFlags::kDieOnParentDeath);
                    BitMath::set(m.flags, EntityFlags::kNoDrops);
                });
            spawned.set_parent(ent.id);
            entity_set_despawn_tick(spawned, 10 * TPS);
        }
    }

    if (dist > 130.f) {
        v.set_magnitude(PLAYER_ACCELERATION * chase);
        ent.acceleration = v;
    } else {
        ent.acceleration.set(0, 0);
    }
}

// Molten boss: phase 1 chase, phase 2 charge, phase 3 chase + summon mites.
static void tick_molten_overlord(Simulation *sim, Entity &ent) {
    float hp = ent.max_health > 0.f ? ent.health / ent.max_health : 0.f;
    uint8_t phase = hp > 0.70f ? 1 : (hp > 0.35f ? 2 : 3);
    float chase = phase == 1 ? 0.70f : (phase == 2 ? 0.85f : 1.0f);

    if (!sim->ent_alive(ent.target)) {
        if (!(ent.target == NULL_ENTITY)) {
            ent.ai_state = 0;
            ent.ai_tick = 0;
            ent.target = NULL_ENTITY;
        }
        ent.target = find_nearest_enemy(sim, ent, ent.detection_radius + ent.get_radius());
        if (!sim->ent_alive(ent.target)) {
            tick_default_passive(sim, ent);
            return;
        }
    }
    Entity &target = sim->get_ent(ent.target);
    Vector v(target.get_x() - ent.get_x(), target.get_y() - ent.get_y());
    _focus_lose_clause(ent, v);
    if (!sim->ent_alive(ent.target)) return;
    float dist = v.magnitude();
    ent.set_angle(v.angle());

    // ai_state 1 = charge windup, 2 = charging
    if (ent.ai_state == 1) {
        ent.acceleration.set(0, 0);
        if (ent.ai_tick >= (game_tick_t)(0.9f * TPS)) {
            ent.ai_state = 2;
            ent.ai_tick = 0;
            ent.heading_angle = v.angle();
        }
        return;
    }
    if (ent.ai_state == 2) {
        Vector dash;
        dash.unit_normal(ent.heading_angle).set_magnitude(PLAYER_ACCELERATION * 2.3f);
        ent.acceleration = dash;
        if (ent.ai_tick >= (game_tick_t)(0.45f * TPS)) {
            ent.ai_state = 0;
            ent.ai_tick = 0;
            ent.secondary_reload = (game_tick_t)(7.0f * TPS);
        }
        return;
    }

    if (ent.secondary_reload > 0) --ent.secondary_reload;
    if (ent.secondary_reload == 0 && dist < 700.f && dist > 120.f && phase >= 2) {
        ent.ai_state = 1;
        ent.ai_tick = 0;
        ent.acceleration.set(0, 0);
        return;
    }

    // Phase 3 periodically summons an Ash Mite behind it
    if (phase == 3 && ent.lifetime % (9 * TPS) == 0) {
        uint32_t kids = _count_children(sim, ent.id);
        if (kids < 3) {
            Vector behind;
            behind.unit_normal(ent.get_angle() + M_PI);
            behind *= ent.get_radius();
            Entity &spawned = alloc_mob(sim, MobID::kAshMite,
                ent.get_x() + behind.x, ent.get_y() + behind.y, ent.get_team(),
                [](Entity &m) {
                    m.score_reward = MOB_DATA[m.get_mob_id()].xp;
                    BitMath::set(m.flags, EntityFlags::kHasCulling);
                    BitMath::set(m.flags, EntityFlags::kDieOnParentDeath);
                    BitMath::set(m.flags, EntityFlags::kNoDrops);
                });
            spawned.set_parent(ent.id);
            entity_set_despawn_tick(spawned, 10 * TPS);
        }
    }

    if (dist > 130.f) {
        v.set_magnitude(PLAYER_ACCELERATION * chase);
        ent.acceleration = v;
    } else {
        ent.acceleration.set(0, 0);
    }
}

// Sandworm boss: 10-segment monarch. Phase 1 chase, phase 2 charge,
// phase 3 chase + summon sand mites.
static void tick_sandworm_monarch(Simulation *sim, Entity &ent) {
    float hp = ent.max_health > 0.f ? ent.health / ent.max_health : 0.f;
    uint8_t phase = hp > 0.70f ? 1 : (hp > 0.35f ? 2 : 3);
    float chase = phase == 1 ? 0.70f : (phase == 2 ? 0.85f : 1.0f);

    if (!sim->ent_alive(ent.target)) {
        if (!(ent.target == NULL_ENTITY)) {
            ent.ai_state = 0;
            ent.ai_tick = 0;
            ent.target = NULL_ENTITY;
        }
        ent.target = find_nearest_enemy(sim, ent, ent.detection_radius + ent.get_radius());
        if (!sim->ent_alive(ent.target)) {
            tick_default_passive(sim, ent);
            return;
        }
    }
    Entity &target = sim->get_ent(ent.target);
    Vector v(target.get_x() - ent.get_x(), target.get_y() - ent.get_y());
    _focus_lose_clause(ent, v);
    if (!sim->ent_alive(ent.target)) return;
    float dist = v.magnitude();
    ent.set_angle(v.angle());

    // ai_state 1 = charge windup, 2 = charging
    if (ent.ai_state == 1) {
        ent.acceleration.set(0, 0);
        if (ent.ai_tick >= (game_tick_t)(0.9f * TPS)) {
            ent.ai_state = 2;
            ent.ai_tick = 0;
            ent.heading_angle = v.angle();
        }
        return;
    }
    if (ent.ai_state == 2) {
        Vector dash;
        dash.unit_normal(ent.heading_angle).set_magnitude(PLAYER_ACCELERATION * 2.3f);
        ent.acceleration = dash;
        if (ent.ai_tick >= (game_tick_t)(0.45f * TPS)) {
            ent.ai_state = 0;
            ent.ai_tick = 0;
            ent.secondary_reload = (game_tick_t)(7.0f * TPS);
        }
        return;
    }

    if (ent.secondary_reload > 0) --ent.secondary_reload;
    if (ent.secondary_reload == 0 && dist < 700.f && dist > 120.f && phase >= 2) {
        ent.ai_state = 1;
        ent.ai_tick = 0;
        ent.acceleration.set(0, 0);
        return;
    }

    // Phase 3 periodically summons a Sand Mite behind it
    if (phase == 3 && ent.lifetime % (10 * TPS) == 0) {
        uint32_t kids = _count_children(sim, ent.id);
        if (kids < 3) {
            Vector behind;
            behind.unit_normal(ent.get_angle() + M_PI);
            behind *= ent.get_radius();
            Entity &spawned = alloc_mob(sim, MobID::kSandMite,
                ent.get_x() + behind.x, ent.get_y() + behind.y, ent.get_team(),
                [](Entity &m) {
                    m.score_reward = MOB_DATA[m.get_mob_id()].xp;
                    BitMath::set(m.flags, EntityFlags::kHasCulling);
                    BitMath::set(m.flags, EntityFlags::kDieOnParentDeath);
                    BitMath::set(m.flags, EntityFlags::kNoDrops);
                });
            spawned.set_parent(ent.id);
            entity_set_despawn_tick(spawned, 10 * TPS);
        }
    }

    if (dist > 130.f) {
        v.set_magnitude(PLAYER_ACCELERATION * chase);
        ent.acceleration = v;
    } else {
        ent.acceleration.set(0, 0);
    }
}

static void tick_digger(Simulation *sim, Entity &ent) {
    ent.input = 0;
    if (sim->ent_alive(ent.target)) {
        Entity &target = sim->get_ent(ent.target);
        Vector v(target.get_x() - ent.get_x(), target.get_y() - ent.get_y());
        _focus_lose_clause(ent, v);
        v.set_magnitude(PLAYER_ACCELERATION * 0.95);
        if (ent.health / ent.max_health > 0.1) {
            BitMath::set(ent.input, InputFlags::kAttacking);
        } else {
            BitMath::set(ent.input, InputFlags::kDefending);
            v *= -1;
        }
        ent.acceleration = v;
        ent.set_angle(v.angle());
        return;
    } else {
        if (!(ent.target == NULL_ENTITY)) {
            ent.ai_state = AIState::kIdle;
            ent.ai_tick = 0;
            ent.target = NULL_ENTITY;
        }
        ent.target = find_nearest_enemy(sim, ent, ent.detection_radius + ent.get_radius());
        switch(ent.ai_state) {
            case AIState::kIdle: {
                ent.set_angle(frand() * M_PI * 2);
                ent.ai_state = AIState::kIdleMoving;
                ent.ai_tick = 0;
                break;
            }
            case AIState::kIdleMoving: {
                if (ent.ai_tick > 5 * TPS)
                    ent.ai_state = AIState::kIdle;
                ent.acceleration.unit_normal(ent.get_angle()).set_magnitude(PLAYER_ACCELERATION);
                break;
            }
            case AIState::kReturning: {
                default_tick_returning(sim, ent);
                break;
            }
        }
    }
}

void tick_ai_behavior(Simulation *sim, Entity &ent) {
    if (ent.pending_delete) return;
    if (sim->ent_alive(ent.seg_head)) return;
    ent.acceleration.set(0,0);
    if (!sim->ent_alive(ent.target) && sim->ent_alive(ent.last_damaged_by)) {
        // Hub sanctuary only: hits from a player inside the respawn pad do not start a chase.
        Entity &attacker = sim->get_ent(ent.last_damaged_by);
        if (!(is_player_faction(sim, attacker)
              && Map::in_spawn_sanctuary(attacker.get_x(), attacker.get_y()))) {
            ent.target = ent.last_damaged_by;
            // Break leash-return so hits in the deep ring actually start a chase.
            if (ent.ai_state == AIState::kReturning) {
                ent.ai_state = AIState::kIdle;
                ent.ai_tick = 0;
            }
        }
    }
    if (!(ent.get_parent() == NULL_ENTITY)) {
        if (!sim->ent_alive(ent.get_parent())) {
            if (BitMath::at(ent.flags, EntityFlags::kDieOnParentDeath))
                sim->request_delete(ent.id);
            ent.set_parent(NULL_ENTITY);
        } else {
            Entity const &parent = sim->get_ent(ent.get_parent());
            Vector delta(parent.get_x() - ent.get_x(), parent.get_y() - ent.get_y());
            if (delta.magnitude() > SUMMON_RETREAT_RADIUS) {
                ent.target = NULL_ENTITY;
                ent.ai_state = AIState::kReturning;
            }
            if (sim->ent_alive(ent.target)) {
                Entity const &target = sim->get_ent(ent.target);
                delta = Vector(parent.get_x() - target.get_x(), parent.get_y() - target.get_y());
                if (delta.magnitude() > SUMMON_RETREAT_RADIUS)
                    ent.target = NULL_ENTITY;
            }
        }
    }
    if (BitMath::at(ent.flags, EntityFlags::kIsCulled)) {
        ent.target = NULL_ENTITY;
        ent.ai_tick = 0;
        return;
    }
    // Hub respawn pad = sole no-aggro zone: drop chase if prey is inside.
    // Player attacks are unrestricted everywhere (including this pad).
    if (sim->ent_alive(ent.target) && !is_player_faction(sim, ent)) {
        Entity &prey = sim->get_ent(ent.target);
        if (is_player_faction(sim, prey)
            && Map::in_spawn_sanctuary(prey.get_x(), prey.get_y()))
            ent.target = NULL_ENTITY;
    }
    switch(ent.get_mob_id()) {
        case MobID::kBabyAnt:            
        case MobID::kLadybug:
        case MobID::kMassiveLadybug:
            tick_default_passive(sim, ent);
            break;
        case MobID::kBee:
            tick_bee_passive(sim, ent);
            break;
        case MobID::kCentipede:
            tick_centipede_passive(sim, ent);
            break;
        case MobID::kEvilCentipede:
            tick_centipede_aggro(sim, ent);
            break;
        case MobID::kDesertCentipede:
            tick_centipede_neutral(sim, ent, 1.33);
            break;
        case MobID::kWorkerAnt:
        case MobID::kDarkLadybug:
        case MobID::kShinyLadybug:
            tick_default_neutral(sim, ent);
            break;
        case MobID::kSoldierAnt:
        case MobID::kBeetle:
        case MobID::kMassiveBeetle:
            tick_default_aggro(sim, ent, 0.95);
            break;
        case MobID::kFlowerGuardian:
            // Garden Boss — tank chase with windup charge bursts
            tick_charge_mob(sim, ent, 0.85f, 2.1f, 0.7f, 0.45f, 6.0f);
            break;
        case MobID::kScorpion:
            tick_default_aggro(sim, ent, 1.20);
            break;
        case MobID::kSpider:
            if (ent.lifetime % (TPS) == 0) 
                alloc_web(sim, 25, ent);
            tick_default_aggro(sim, ent, 1.20);
            break;
        case MobID::kQueenAnt:
            if (ent.lifetime % (2 * TPS) == 0) {
                Vector behind;
                behind.unit_normal(ent.get_angle() + M_PI);
                behind *= ent.get_radius();
                Entity &spawned = alloc_mob(
                    sim, MobID::kSoldierAnt, ent.get_x() + behind.x, ent.get_y() + behind.y, 
                    ent.get_team(), [](Entity &mob) {
                    mob.score_reward = MOB_DATA[mob.get_mob_id()].xp;
                    BitMath::set(mob.flags, EntityFlags::kHasCulling);
                });
                entity_set_despawn_tick(spawned, 10 * TPS);
                spawned.set_parent(ent.get_parent());
            }
            tick_default_aggro(sim, ent, 0.95);
            break;
        case MobID::kHornet:
            tick_hornet_aggro(sim, ent);
            break;
        case MobID::kBoulder:
        case MobID::kRock:
        case MobID::kCactus:
        case MobID::kSquare:
        case MobID::kPushCrate:
        case MobID::kYahahaPad:
        case MobID::kTreasureNpc:
        case MobID::kLootChest:
            break;
        case MobID::kSandstorm:
            tick_sandstorm(sim, ent);
            break;
        case MobID::kDigger:
            tick_digger(sim, ent);
            break;
        case MobID::kWanderingGuard:
            tick_default_aggro(sim, ent, 1.05f);
            break;
        case MobID::kStoneBeetle:
            tick_default_aggro(sim, ent, 0.75f);
            break;
        case MobID::kRustMite:
            tick_default_aggro(sim, ent, 1.25f);
            break;
        case MobID::kMimicChest:
            tick_mimic_wake(sim, ent, 1.0f);
            break;
        case MobID::kCoinBeetle:
            tick_flee(sim, ent, 1.3f);
            break;
        case MobID::kRoyalBee:
            tick_charge_mob(sim, ent, 1.1f, 2.2f, 0.35f, 0.4f, 2.5f);
            break;
        case MobID::kThornBug:
            tick_thorn_bug(sim, ent);
            break;
        case MobID::kShieldSentinel:
            tick_default_aggro(sim, ent, 0.8f);
            break;
        case MobID::kSpearSentinel:
            tick_charge_mob(sim, ent, 0.95f, 2.8f, 0.5f, 0.35f, 2.8f);
            break;
        case MobID::kPhantomOrb:
            tick_phantom_orb(sim, ent);
            break;
        case MobID::kFallenKnight:
            tick_charge_mob(sim, ent, 0.9f, 2.5f, 0.45f, 0.4f, 2.2f);
            break;
        case MobID::kFallenKing:
            tick_fallen_king(sim, ent);
            break;
        case MobID::kLeafBeetle:
            tick_default_aggro(sim, ent, 1.0f);
            break;
        case MobID::kJungleMite:
            tick_default_aggro(sim, ent, 1.3f);
            break;
        case MobID::kVineBug:
            tick_default_aggro(sim, ent, 0.75f);
            break;
        case MobID::kJungleSpider:
            tick_default_aggro(sim, ent, 1.2f);
            break;
        case MobID::kEmeraldMantis:
            tick_default_aggro(sim, ent, 0.85f);
            break;
        case MobID::kPoisonToad:
            tick_default_aggro(sim, ent, 0.7f);
            break;
        case MobID::kSporeMushroom:
            tick_default_aggro(sim, ent, 0.65f);
            break;
        case MobID::kCarnivorousPlant:
            tick_mimic_wake(sim, ent, 0.9f);
            break;
        case MobID::kRiverCrab:
            tick_default_aggro(sim, ent, 0.75f);
            break;
        case MobID::kStoneGuardian:
            tick_default_aggro(sim, ent, 0.65f);
            break;
        case MobID::kJungleWasp:
            // Ranged windup template (same kit as Hornet)
            tick_hornet_aggro(sim, ent);
            break;
        case MobID::kAncientTreant:
            tick_ancient_treant(sim, ent);
            break;
        // ---- Ocean ----
        case MobID::kReefStone:
            // Stationary reef boulder — no AI at all.
            break;
        case MobID::kSeaFry:
            tick_sea_fry(sim, ent);
            break;
        case MobID::kMoonJelly:
            tick_default_aggro(sim, ent, 0.75f);
            break;
        case MobID::kCoralStar:
            // Neutral: only retaliates after being hit.
            tick_default_neutral(sim, ent);
            break;
        case MobID::kReefCrab:
            tick_default_aggro(sim, ent, 0.85f);
            break;
        case MobID::kSpinePuffer:
            // Passive until hit, then fights back.
            tick_default_neutral(sim, ent);
            break;
        case MobID::kSeaLeech:
            // Segmented: the head drives, the links follow.
            if (ent.seg_head == NULL_ENTITY)
                tick_default_aggro(sim, ent, 0.95f);
            break;
        case MobID::kMorayEel:
            // Straight-line charge with a windup.
            tick_charge_mob(sim, ent, 0.9f, 2.2f, 0.6f, 0.4f, 5.0f);
            break;
        case MobID::kInkCuttle:
            tick_ink_cuttle(sim, ent);
            break;
        case MobID::kAbyssShark:
            tick_default_aggro(sim, ent, 1.2f);
            break;
        case MobID::kAbyssMaw:
            tick_abyss_maw(sim, ent);
            break;
        // ---- Snow ----
        case MobID::kFrostStone:
            // Stationary ice boulder — no AI at all.
            break;
        case MobID::kSnowMite:
            // Passive until hit, then fights back.
            tick_default_neutral(sim, ent);
            break;
        case MobID::kSnowHare:
            tick_flee(sim, ent, 1.3f);
            break;
        case MobID::kIceSpider:
            tick_web_layer(sim, ent, 1.0f, 55.f);
            break;
        case MobID::kFrostHornet:
            // Ranged windup charge (same kit as the Hornet).
            tick_hornet_aggro(sim, ent);
            break;
        case MobID::kRimeCentipede:
            // Segmented: the head drives, the links follow.
            if (ent.seg_head == NULL_ENTITY)
                tick_default_aggro(sim, ent, 0.9f);
            break;
        case MobID::kBlizzardWisp:
            // Sandstorm-style erratic rush.
            tick_sandstorm(sim, ent);
            break;
        case MobID::kGlacierBeast:
            tick_default_aggro(sim, ent, 0.75f);
            break;
        case MobID::kFrostYeti:
            tick_default_aggro(sim, ent, 0.85f);
            break;
        case MobID::kIceGolem:
            tick_default_aggro(sim, ent, 0.65f);
            break;
        case MobID::kWinterSovereign:
            tick_winter_sovereign(sim, ent);
            break;
        // ---- Volcano ----
        case MobID::kEmberStone:
            // Stationary volcanic rock — no AI at all.
            break;
        case MobID::kAshMite:
            // Passive until hit, then fights back.
            tick_default_neutral(sim, ent);
            break;
        case MobID::kCinderMoth:
            tick_flee(sim, ent, 1.3f);
            break;
        case MobID::kLavaBeetle:
            tick_default_aggro(sim, ent, 0.85f);
            break;
        case MobID::kEmberWasp:
            // Ranged windup charge (same kit as the Hornet).
            tick_hornet_aggro(sim, ent);
            break;
        case MobID::kMagmaCrab:
            // Passive until hit, then fights back.
            tick_default_neutral(sim, ent);
            break;
        case MobID::kFireSpider:
            // Poison comes from MOB_DATA; the AI is a plain chase.
            tick_default_aggro(sim, ent, 1.0f);
            break;
        case MobID::kMagmaCentipede:
            // Segmented: the head drives, the links follow.
            if (ent.seg_head == NULL_ENTITY)
                tick_default_aggro(sim, ent, 0.95f);
            break;
        case MobID::kObsidianGolem:
            tick_default_aggro(sim, ent, 0.65f);
            break;
        case MobID::kLavaColossus:
            // Straight-line charge with a windup.
            tick_charge_mob(sim, ent, 0.9f, 2.0f, 0.9f, 0.4f, 7.0f);
            break;
        case MobID::kMoltenOverlord:
            tick_molten_overlord(sim, ent);
            break;
        // ---- Desert ----
        case MobID::kDuneStone:
            // Stationary sandstone — no AI at all.
            break;
        case MobID::kSandMite:
            // Passive until hit, then fights back.
            tick_default_neutral(sim, ent);
            break;
        case MobID::kDuneBeetle:
            tick_default_aggro(sim, ent, 0.85f);
            break;
        case MobID::kDuneSpider:
            tick_web_layer(sim, ent, 1.0f, 55.f);
            break;
        case MobID::kDuneHornet:
            // Ranged windup charge (same kit as the Hornet).
            tick_hornet_aggro(sim, ent);
            break;
        case MobID::kDuneTitan:
            tick_default_aggro(sim, ent, 0.65f);
            break;
        case MobID::kSandwormMonarch:
            tick_sandworm_monarch(sim, ent);
            break;
        // ---- Ant Hell (new) ----
        case MobID::kAcidAnt:
            // Ranged acid spit — same windup kit as Hornet.
            tick_hornet_aggro(sim, ent);
            break;
        case MobID::kScoutAnt:
            tick_default_aggro(sim, ent, 1.35f);
            break;
        case MobID::kArmoredAnt:
            tick_default_aggro(sim, ent, 0.55f);
            break;
        case MobID::kBroodCarrier:
            if (ent.lifetime % (3 * TPS) == 0) {
                Vector behind;
                behind.unit_normal(ent.get_angle() + M_PI);
                behind *= ent.get_radius();
                Entity &spawned = alloc_mob(
                    sim, MobID::kBabyAnt, ent.get_x() + behind.x, ent.get_y() + behind.y,
                    ent.get_team(), [](Entity &mob) {
                    mob.score_reward = MOB_DATA[mob.get_mob_id()].xp;
                    BitMath::set(mob.flags, EntityFlags::kHasCulling);
                });
                entity_set_despawn_tick(spawned, 8 * TPS);
                spawned.set_parent(ent.id);
            }
            tick_default_aggro(sim, ent, 0.8f);
            break;
        case MobID::kWingedAnt:
            tick_default_aggro(sim, ent, 1.15f);
            break;
        case MobID::kTunnelGuard:
            tick_charge_mob(sim, ent, 0.7f, 2.0f, 0.55f, 0.4f, 5.5f);
            break;
        case MobID::kSporeAnt:
            tick_default_aggro(sim, ent, 0.9f);
            break;
        case MobID::kFangAnt:
            tick_charge_mob(sim, ent, 0.85f, 2.3f, 0.5f, 0.35f, 4.5f);
            break;
        case MobID::kRoyalSentinel:
            tick_charge_mob(sim, ent, 0.8f, 1.9f, 0.6f, 0.4f, 5.0f);
            break;
        // ---- Spawn-point fillers ----
        case MobID::kJungleStone:
        case MobID::kRoyalBoulder:
            // Stationary stones — no AI at all.
            break;
        case MobID::kJungleCrawler:
        case MobID::kRoyalPebble:
            // Passive until hit, then fights back.
            tick_default_neutral(sim, ent);
            break;
        default:
            break;
    }
    //wall avoidance
    if (!sim->ent_alive(ent.target)) {
        if (ent.get_x() - ent.get_radius() <= 0 && angle_within(ent.get_angle(), M_PI, M_PI / 2))
            ent.set_angle(M_PI - ent.get_angle());
        if (ent.get_x() + ent.get_radius() >= ARENA_WIDTH && angle_within(ent.get_angle(), 0, M_PI / 2))
            ent.set_angle(M_PI - ent.get_angle());
        if (ent.get_y() - ent.get_radius() <= 0 && angle_within(ent.get_angle(), 3 * M_PI / 2, M_PI / 2))
            ent.set_angle(0 - ent.get_angle());
        if (ent.get_y() + ent.get_radius() >= ARENA_HEIGHT && angle_within(ent.get_angle(), M_PI / 2, M_PI / 2))
            ent.set_angle(0 - ent.get_angle());
    }
    ++ent.ai_tick;
}