#include <Server/Process.hh>

#include <Server/Client.hh>
#include <Server/EntityFunctions.hh>
#include <Server/Server.hh>
#include <Server/Spawn.hh>
#include <Shared/Craft.hh>
#include <Shared/Entity.hh>
#include <Shared/Simulation.hh>
#include <Shared/StaticData.hh>

#include <Helpers/Vector.hh>

#include <cmath>

// Unique level bonus only for petals from the first primary slot holding this ID.
static float owner_petal_level_mult(Simulation *sim, Entity const &player, Entity const &petal) {
    PetalID::T id = petal.get_petal_id();
    if (player.has_component(kMob) || id == PetalID::kNone || id >= PetalID::kNumPetals)
        return 1.f;
    Client *owner = Server::client_by_camera(sim, player.get_parent());
    if (owner == nullptr || owner->petal_levels[id] == 0) return 1.f;
    uint8_t seen_earlier_slot = 0;
    uint32_t n = player.get_loadout_count();
    for (uint32_t i = 0; i < n; ++i) {
        LoadoutSlot const &slot = player.loadout[i];
        if (slot.get_petal_id() != id) continue;
        for (uint32_t j = 0; j < slot.size(); ++j) {
            if (slot.petals[j].ent_id != petal.id) continue;
            return seen_earlier_slot ? 1.f
                : Craft::level_stat_mult(owner->petal_levels[id]);
        }
        seen_earlier_slot = 1;
    }
    return 1.f;
}

void tick_petal_behavior(Simulation *sim, Entity &petal) {
    if (petal.pending_delete) return;
    if (!sim->ent_alive(petal.get_parent())) {
        sim->request_delete(petal.id);
        return;
    }
    Entity &player = sim->get_ent(petal.get_parent());
    struct PetalData const &petal_data = PETAL_DATA[petal.get_petal_id()];
    if (petal_data.attributes.rotation_style == PetalAttributes::kPassiveRot) {
        //simulate on clientside
        float rot_amt = petal_name_is(petal.get_petal_id(), "Wing") ? 10.0 : 1.0;
        if (petal.id.id % 2) petal.set_angle(petal.get_angle() + rot_amt / TPS);
        else petal.set_angle(petal.get_angle() - rot_amt / TPS);
    } else if (petal_data.attributes.rotation_style == PetalAttributes::kFollowRot && !(BitMath::at(petal.flags, EntityFlags::kIsDespawning))) {
        Vector delta(petal.get_x() - player.get_x(), petal.get_y() - player.get_y());
        petal.set_angle(delta.angle());
    }
    if (BitMath::at(petal.flags, EntityFlags::kIsDespawning) ||
        BitMath::at(petal.flags, EntityFlags::kIsDetached)) {
        PetalID::T pid = petal.get_petal_id();
        if (petal_name_is(pid, "Missile") || pid == PetalID::kSandLeafDart) {
            petal.acceleration.unit_normal(petal.get_angle()).set_magnitude(2.0 * PLAYER_ACCELERATION);
        } else if (petal_is_honey(pid)) {
            // Very slow crawl — keep honey near the flower / magnet AFK bubble.
            petal.acceleration.unit_normal(petal.get_angle()).set_magnitude(0.30f * PLAYER_ACCELERATION);
        } else if (pid == PetalID::kPoisonPeas || pid == PetalID::kPeas
                   || pid == PetalID::kGlassStorm) {
            petal.acceleration.unit_normal(petal.heading_angle).set_magnitude(2.0 * PLAYER_ACCELERATION);
        } else if (petal_is_moon(pid)) {
            // Orbit the flower: advance angle every tick (all Moon ladder IDs).
            float dir = (petal.id.id % 2) ? 1.f : -1.f;
            petal.set_angle(petal.get_angle() + dir * 2.5f / TPS);
            float wanting_angle = petal.get_angle();
            float wanting_radius = 100.f + player.get_radius();
            float wanting_x = player.get_x() + wanting_radius * (float) std::cos(wanting_angle);
            float wanting_y = player.get_y() + wanting_radius * (float) std::sin(wanting_angle);
            Vector delta(wanting_x - petal.get_x(), wanting_y - petal.get_y());
            petal.acceleration.set(delta.x * 0.5f, delta.y * 0.5f);
        } else {
            petal.acceleration.set(0,0);
        }
        // Grounded aura: heal allies and/or slow enemies inside radius.
        if (petal_data.attributes.aura_radius > 0
            && (petal_data.attributes.aura_heal > 0 || petal_data.attributes.aura_slow_seconds > 0)) {
            float const R = petal_data.attributes.aura_radius;
            float const lv_mult = owner_petal_level_mult(sim, player, petal);
            float const heal = petal_data.attributes.aura_heal * lv_mult / TPS;
            float const px = petal.get_x();
            float const py = petal.get_y();
            EntityID const team = petal.get_team();
            game_tick_t const slow_ticks =
                (game_tick_t)(petal_data.attributes.aura_slow_seconds * TPS);
            sim->spatial_hash.query(px, py, R, R, [&](Simulation *s, Entity &ent) {
                float dx = ent.get_x() - px;
                float dy = ent.get_y() - py;
                if (dx * dx + dy * dy > R * R) return;
                if (petal_data.attributes.aura_heal > 0
                    && ent.has_component(kFlower) && ent.has_component(kHealth)
                    && ent.get_team() == team) {
                    inflict_heal(s, ent, heal);
                }
                if (slow_ticks > 0 && ent.has_component(kMob) && !(ent.get_team() == team)) {
                    if (ent.slow_ticks < slow_ticks) ent.slow_ticks = slow_ticks;
                }
            });
        }
        return;
    }
    if (petal_data.attributes.secondary_reload == 0) return;
    if (petal.secondary_reload <= petal_data.attributes.secondary_reload * TPS) {
        ++petal.secondary_reload;
        return;
    }
    float const lv_mult = owner_petal_level_mult(sim, player, petal);
    // Radial pulse: damage enemies around the flower when attacking.
    if (petal_data.attributes.radial_pulse_damage > 0
        && petal_data.attributes.radial_pulse_radius > 0
        && BitMath::at(player.input, InputFlags::kAttacking)) {
        float const R = petal_data.attributes.radial_pulse_radius;
        float const dmg = petal_data.attributes.radial_pulse_damage * lv_mult;
        float const px = player.get_x();
        float const py = player.get_y();
        EntityID const team = player.get_team();
        EntityID const src = petal.id;
        sim->spatial_hash.query(px, py, R, R, [&](Simulation *s, Entity &ent) {
            if (!ent.has_component(kHealth) || !ent.has_component(kMob)) return;
            if (ent.get_team() == team) return;
            float dx = ent.get_x() - px;
            float dy = ent.get_y() - py;
            if (dx * dx + dy * dy > R * R) return;
            inflict_damage(s, src, ent.id, dmg, DamageType::kSplash);
        });
        petal.secondary_reload = 0;
        return;
    }
    if (petal_data.attributes.burst_heal > 0 && player.health < player.max_health && player.dandy_ticks == 0) {
        Vector delta(player.get_x() - petal.get_x(), player.get_y() - petal.get_y());
        if (delta.magnitude() < petal.get_radius()) {
            inflict_heal(sim, player, petal_data.attributes.burst_heal * lv_mult);
            sim->request_delete(petal.id);
            return;
        }
        delta.set_magnitude(PLAYER_ACCELERATION * 4);
        petal.acceleration = delta;
    }
    PetalID::T pid = petal.get_petal_id();
    if (petal_name_is(pid, "Missile") || pid == PetalID::kSandLeafDart) {
            if (BitMath::at(player.input, InputFlags::kAttacking)) {
                petal.velocity.unit_normal(petal.get_angle()).set_magnitude(15 * PLAYER_ACCELERATION);
                petal.friction = DEFAULT_FRICTION;
                entity_set_despawn_tick(petal, 2.5 * TPS);
            }
    } else if (petal_is_honey(pid)) {
            if (BitMath::at(player.input, InputFlags::kAttacking)) {
                // Short soft toss; 4s life, stays close for AFK magnet farming.
                petal.velocity.unit_normal(petal.get_angle()).set_magnitude(2.0f * PLAYER_ACCELERATION);
                petal.friction = DEFAULT_FRICTION;
                entity_set_despawn_tick(petal, 4.0 * TPS);
            }
    } else if (petal_is_meteor(pid)) {
            if (BitMath::at(player.input, InputFlags::kAttacking)
                || BitMath::at(player.input, InputFlags::kDefending)) {
                // Plant under the petal's current orbit point — no toss.
                petal.velocity.set(0, 0);
                petal.acceleration.set(0, 0);
                petal.friction = 1.f;
                petal.set_split_projectile(1); // planted / fixed visual
                // Footprint ≈ 16 flower areas (was 8, then ×2): R = BASE_FLOWER_RADIUS * √16
                petal.set_radius(BASE_FLOWER_RADIUS * std::sqrt(16.f));
                petal.mass = 12.f; // heavy obstacle; still overlaps other meteors (same team)
                float dur = petal_data.attributes.aura_duration > 0
                    ? petal_data.attributes.aura_duration : 18.f;
                entity_set_despawn_tick(petal, (game_tick_t)(dur * TPS));
            }
    } else if (petal_name_is(pid, "Web")) {
            if (BitMath::at(player.input, InputFlags::kAttacking)) {
                Vector delta(petal.get_x() - player.get_x(), petal.get_y() - player.get_y());
                petal.friction = DEFAULT_FRICTION;
                float angle = delta.angle();
                if (PETAL_DATA[pid].count >= 3) angle += frand() - 0.5;
                petal.acceleration.unit_normal(angle).set_magnitude(25 * PLAYER_ACCELERATION);
                entity_set_despawn_tick(petal, 0.6 * TPS);
            } else if (BitMath::at(player.input, InputFlags::kDefending))
                entity_set_despawn_tick(petal, 0.6 * TPS);
    } else if (petal_name_is(pid, "Bubble")) {
            if (BitMath::at(player.input, InputFlags::kDefending)) {
                // All ready bubbles still pop, but impulse is capped to ~1 pop.
                // Without a cap, 3–5 bubbles + defend (combat) stacked into a
                // silent long-range rocket with no telegraph.
                float const kBubbleImpulse = PLAYER_ACCELERATION * 20.f;
                float const kBubbleVelCap = PLAYER_ACCELERATION * 24.f;
                Vector v(player.get_x() - petal.get_x(), player.get_y() - petal.get_y());
                float sep = v.magnitude();
                if (sep > 1.f && std::isfinite(sep)
                    && player.velocity.magnitude() < kBubbleVelCap) {
                    v.set_magnitude(kBubbleImpulse);
                    player.velocity += v;
                    float nm = player.velocity.magnitude();
                    if (nm > kBubbleVelCap && std::isfinite(nm))
                        player.velocity.set_magnitude(kBubbleVelCap);
                }
                sim->request_delete(petal.id);
            }
    } else switch (pid) {
        case PetalID::kPollen:
            if (BitMath::at(player.input, InputFlags::kAttacking) || BitMath::at(player.input, InputFlags::kDefending)) {
                petal.friction = DEFAULT_FRICTION;
                entity_set_despawn_tick(petal, 4.0 * TPS);
            }
            break;
        case PetalID::kPeas:
        case PetalID::kPoisonPeas:
        case PetalID::kGlassStorm:
            if (BitMath::at(player.input, InputFlags::kAttacking)) {
                float spray_angle = frand() * 2 * M_PI;
                petal.friction = DEFAULT_FRICTION;
                petal.velocity.unit_normal(spray_angle).set_magnitude(20 * PLAYER_ACCELERATION);
                petal.set_split_projectile(0);
                petal.heading_angle = spray_angle;
                entity_set_despawn_tick(petal, 1.0 * TPS);
                for (uint32_t i = 1; i < petal_data.count; ++i) {
                    Entity &new_petal = alloc_petal(sim, petal.get_petal_id(), player);
                    new_petal.friction = DEFAULT_FRICTION;
                    new_petal.set_x(petal.get_x());
                    new_petal.set_y(petal.get_y());
                    new_petal.set_split_projectile(0);
                    new_petal.heading_angle = spray_angle + (2 * M_PI * i) / petal_data.count;
                    new_petal.velocity
                        .unit_normal(new_petal.heading_angle)
                        .set_magnitude(20 * PLAYER_ACCELERATION);
                    entity_set_despawn_tick(new_petal, 1.0 * TPS);
                }
            }
            break;
        case PetalID::kMoon: {
            /*
            if (BitMath::at(player.input, InputFlags::kAttacking)) {
                Vector delta(petal.get_x() - player.get_x(), petal.get_y() - player.get_y());
                petal.friction = 0;
                petal.acceleration.unit_normal(delta.angle() + M_PI / 3).set_magnitude(3 * PLAYER_ACCELERATION);
                entity_set_despawn_tick(petal, 10 * TPS);
            }
            */
            break;
        }
        case PetalID::kMonstera:
        case PetalID::kMonsteraEpic:
        case PetalID::kMonsteraLegendary:
        case PetalID::kMonsteraMythic:
        case PetalID::kMonsteraUnique:
        case PetalID::kMonsteraSuper:
        case PetalID::kMonsteraUltra:
        case PetalID::kPollenSandstorm:
        case PetalID::kWorldTreeSeed:
        case PetalID::kChronoVortex:
            // fallthrough shared plant path below
            if (BitMath::at(player.input, InputFlags::kAttacking)
                || BitMath::at(player.input, InputFlags::kDefending)) {
                Vector delta(petal.get_x() - player.get_x(), petal.get_y() - player.get_y());
                petal.friction = DEFAULT_FRICTION;
                float angle = delta.angle();
                petal.acceleration.unit_normal(angle).set_magnitude(18 * PLAYER_ACCELERATION);
                // Reuse split_projectile as client "rooted" visual flag.
                petal.set_split_projectile(1);
                float dur = petal_data.attributes.aura_duration > 0
                    ? petal_data.attributes.aura_duration : 18.f;
                entity_set_despawn_tick(petal, (game_tick_t)(dur * TPS));
            }
            break;
        default:
            // Heartwood ladder (any rarity): same grounded heal-aura plant as Monstera.
            if (petal_name_is(pid, "Heartwood")
                && (BitMath::at(player.input, InputFlags::kAttacking)
                    || BitMath::at(player.input, InputFlags::kDefending))) {
                Vector delta(petal.get_x() - player.get_x(), petal.get_y() - player.get_y());
                petal.friction = DEFAULT_FRICTION;
                petal.acceleration.unit_normal(delta.angle()).set_magnitude(18 * PLAYER_ACCELERATION);
                petal.set_split_projectile(1);
                float dur = petal_data.attributes.aura_duration > 0
                    ? petal_data.attributes.aura_duration : 16.f;
                entity_set_despawn_tick(petal, (game_tick_t)(dur * TPS));
            }
            break;
    }
}