#include <Server/Process.hh>

#include <Server/Client.hh>
#include <Server/EntityFunctions.hh>
#include <Server/Server.hh>
#include <Server/Spawn.hh>
#include <Shared/Craft.hh>
#include <Shared/Entity.hh>
#include <Shared/Map.hh>
#include <Shared/Simulation.hh>
#include <Shared/StaticData.hh>

#include <Helpers/Math.hh>
#include <Helpers/Vector.hh>

#include <cmath>

// Unique level bonus only for petals from the first primary slot holding this ID.
static float owner_petal_level_mult(Simulation *sim, Entity const &player, Entity const &petal) {
    PetalID::T id = petal.get_petal_id();
    if (player.has_component(kMob) || id == PetalID::kNone || id >= PetalID::kNumPetals)
        return 1.f;
    Client *owner = Server::client_by_camera(sim, player.get_parent());
    if (owner == nullptr) return 1.f;
    uint32_t n = player.get_loadout_count();
    for (uint32_t i = 0; i < n; ++i) {
        LoadoutSlot const &slot = player.loadout[i];
        auto slot_id = slot.get_petal_id();
        if (slot_id != id && slot.behavior_id != id) continue;
        for (uint32_t j = 0; j < slot.size(); ++j) {
            if (slot.petals[j].ent_id != petal.id) continue;
            for (uint32_t earlier = 0; earlier < i; ++earlier)
                if (player.loadout[earlier].get_petal_id() == slot_id) return 1.f;
            return Craft::level_stat_mult(
                Craft::petal_level_get(owner->petal_levels, slot_id));
        }
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
    uint8_t const detached = BitMath::at(petal.flags, EntityFlags::kIsDetached)
        || BitMath::at(petal.flags, EntityFlags::kIsDespawning);
    // PassiveRot must not run while detached — Honey was curving from spin (looked like orbit).
    if (petal_data.attributes.rotation_style == PetalAttributes::kPassiveRot && !detached) {
        //simulate on clientside
        float rot_amt = petal_name_is(petal.get_petal_id(), "Wing") ? 10.0 : 1.0;
        if (petal.id.id % 2) petal.set_angle(petal.get_angle() + rot_amt / TPS);
        else petal.set_angle(petal.get_angle() - rot_amt / TPS);
    } else if (petal_data.attributes.rotation_style == PetalAttributes::kFollowRot && !detached) {
        Vector delta(petal.get_x() - player.get_x(), petal.get_y() - player.get_y());
        petal.set_angle(delta.angle());
    }
    if (detached) {
        PetalID::T pid = petal.get_petal_id();
        if (petal_name_is(pid, "Electric Web")) {
            petal.acceleration.set(0, 0);
            if (++petal.secondary_reload >= std::fmax(1.f, petal_data.attributes.secondary_reload * TPS)) {
                petal.secondary_reload = 0;
                float radius = petal_data.attributes.aura_radius;
                sim->spatial_hash.query(petal.get_x(), petal.get_y(), radius, radius,
                    [&](Simulation *s, Entity &target) {
                        if (!target.has_component(kHealth) || !target.has_component(kMob)
                            || target.get_team() == petal.get_team()) return;
                        Vector delta(target.get_x() - petal.get_x(), target.get_y() - petal.get_y());
                        if (delta.magnitude() > radius) return;
                        target.slow_ticks = std::max(target.slow_ticks, (game_tick_t)(TPS / 2));
                        inflict_damage(s, petal.id, target.id, petal_data.attributes.lightning_damage,
                            DamageType::kSplash);
                    });
            }
            return;
        }
        if (petal_name_is(pid, "Missile") || petal_name_is(pid, "Magic Missile")
            || petal_name_is(pid, "Carrot") || petal_name_is(pid, "Sand Leaf Dart")) {
            petal.acceleration.unit_normal(petal.get_angle()).set_magnitude(2.0 * PLAYER_ACCELERATION);
        } else if (petal_is_honey(pid)) {
            // Straight bait toss — slower than Missile so mobs can catch it.
            petal.acceleration.unit_normal(petal.get_angle()).set_magnitude(0.7f * PLAYER_ACCELERATION);
        } else if (petal_name_is(pid, "Peas") || petal_name_is(pid, "Glass Storm")
            || petal_name_is(pid, "Grapes") || petal_name_is(pid, "Blueberries")) {
            // Sustained push so peas keep range after the initial spray burst.
            petal.acceleration.unit_normal(petal.heading_angle).set_magnitude(2.6f * PLAYER_ACCELERATION);
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
        // Grounded aura: heal allies, slow enemies, and/or radiate poison (Uranium).
        if (petal_data.attributes.aura_radius > 0
            && (petal_data.attributes.aura_heal > 0
                || petal_data.attributes.aura_slow_seconds > 0
                || petal_data.attributes.poison_damage.damage > 0)) {
            float const R = petal_data.attributes.aura_radius;
            float const lv_mult = owner_petal_level_mult(sim, player, petal);
            float const heal = petal_data.attributes.aura_heal * lv_mult
                * heal_tier_catchup(petal_data.rarity) / TPS;
            float const px = petal.get_x();
            float const py = petal.get_y();
            EntityID const team = petal.get_team();
            game_tick_t const slow_ticks =
                (game_tick_t)(petal_data.attributes.aura_slow_seconds * TPS);
            PoisonDamage const rad = petal_data.attributes.poison_damage;
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
                if (rad.damage > 0.f && ent.has_component(kHealth)
                    && !(ent.get_team() == team)
                    && !ent.has_component(kPetal) && !ent.has_component(kDrop)) {
                    game_tick_t want = (game_tick_t)(rad.time * TPS);
                    if (ent.poison_ticks < want) {
                        ent.poison_ticks = want;
                        ent.poison_inflicted = rad.damage * lv_mult / TPS;
                        ent.poison_dealer = player.id;
                    }
                }
            });
        }
        return;
    }
    // Orbiting radiation (Uranium): no plant / secondary_reload required.
    if (petal_data.attributes.aura_radius > 0
        && petal_data.attributes.poison_damage.damage > 0
        && petal_data.attributes.aura_heal <= 0
        && petal_data.attributes.aura_slow_seconds <= 0) {
        float const R = petal_data.attributes.aura_radius;
        float const lv_mult = owner_petal_level_mult(sim, player, petal);
        float const px = petal.get_x();
        float const py = petal.get_y();
        EntityID const team = petal.get_team();
        PoisonDamage const rad = petal_data.attributes.poison_damage;
        sim->spatial_hash.query(px, py, R, R, [&](Simulation *s, Entity &ent) {
            if (!ent.has_component(kHealth)) return;
            if (ent.get_team() == team) return;
            if (ent.has_component(kPetal) || ent.has_component(kDrop)) return;
            float dx = ent.get_x() - px;
            float dy = ent.get_y() - py;
            if (dx * dx + dy * dy > R * R) return;
            game_tick_t want = (game_tick_t)(rad.time * TPS);
            if (ent.poison_ticks < want) {
                ent.poison_ticks = want;
                ent.poison_inflicted = rad.damage * lv_mult / TPS;
                ent.poison_dealer = player.id;
            }
        });
    }
    if (petal_data.attributes.ankh_mark
        && BitMath::at(player.input, InputFlags::kDefending)
        && petal.secondary_reload >= petal_data.attributes.secondary_reload * TPS) {
        BitMath::set(petal.flags, EntityFlags::kIsDetached);
        petal.velocity.set(0, 0);
        petal.acceleration.set(0, 0);
        petal.friction = 1.f;
        return;
    }
    // secondary_reload only gates pulse / burst-heal. Shootables (Peas/Grapes/
    // Missile/Carrot/Bubble…) must still run when secondary_reload is 0 —
    // otherwise Grapes never spray (wiki import left the field unset).
    uint8_t sec_ready = 1;
    if (petal_data.attributes.secondary_reload > 0) {
        if (petal.secondary_reload <= petal_data.attributes.secondary_reload * TPS) {
            ++petal.secondary_reload;
            sec_ready = 0;
        }
    }
    float const lv_mult = owner_petal_level_mult(sim, player, petal);
    // Radial pulse: damage enemies around the flower when attacking.
    if (sec_ready
        && petal_data.attributes.radial_pulse_damage > 0
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
    if (sec_ready && petal_data.attributes.burst_heal > 0
        && player.dandy_ticks == 0) {
        uint8_t const is_shell = petal_name_is(petal.get_petal_id(), "Shell");
        uint8_t const want_heal = is_shell || player.health < player.max_health;
        if (want_heal) {
            Vector delta(player.get_x() - petal.get_x(), player.get_y() - petal.get_y());
            // Touch the flower body (not only the tiny core). Old check used petal
            // radius alone (~10) so a stationary player rarely absorbed the rose.
            float const reach = player.get_radius() + petal.get_radius();
            if (delta.magnitude() < reach) {
                float amt = petal_data.attributes.burst_heal * lv_mult
                    * heal_tier_catchup(petal_data.rarity);
                if (is_shell) {
                    player.flower_shield += amt;
                    float cap = player.max_health * 0.75f;
                    if (cap < amt) cap = amt;
                    if (player.flower_shield > cap) player.flower_shield = cap;
                } else {
                    inflict_heal(sim, player, amt);
                }
                sim->request_delete(petal.id);
                return;
            }
            delta.set_magnitude(PLAYER_ACCELERATION * 4);
            petal.acceleration = delta;
        }
    }
    // Magic Eye: spend half mana → lightning burst while attacking
    if (sec_ready && petal_data.attributes.mana_lightning_per > 0.f
        && BitMath::at(player.input, InputFlags::kAttacking)
        && player.mana > 1.f) {
        float spend = player.mana * 0.5f;
        float flash = spend * petal_data.attributes.mana_lightning_per;
        uint8_t chains = petal_data.attributes.lightning_chains;
        bool fired = false;
        if (chains == 0) chains = 1;
        float const R = 220.f;
        float const px = player.get_x(), py = player.get_y();
        EntityID team = player.get_team();
        EntityID src = petal.id;
        sim->spatial_hash.query(px, py, R, R, [&](Simulation *s, Entity &ent) {
            if (chains == 0) return;
            if (!ent.has_component(kHealth)) return;
            if (ent.get_team() == team) return;
            if (ent.has_component(kPetal) || ent.has_component(kDrop)) return;
            float dx = ent.get_x() - px, dy = ent.get_y() - py;
            if (dx * dx + dy * dy > R * R) return;
            if (!fired) { player.mana -= spend; fired = true; }
            --chains;
            inflict_damage(s, src, ent.id, flash, DamageType::kSplash);
            flash *= 0.7f;
        });
        if (fired) petal.secondary_reload = 0;
        return;
    }
    PetalID::T pid = petal.get_petal_id();
    if (petal_name_is(pid, "Electric Web") && sec_ready
        && BitMath::at(player.input, InputFlags::kAttacking)) {
        BitMath::set(petal.flags, EntityFlags::kIsDetached);
        petal.velocity.set(0, 0); petal.acceleration.set(0, 0);
        petal.friction = 1.f;
        entity_set_despawn_tick(petal, (game_tick_t)(petal_data.attributes.aura_duration * TPS));
        petal.secondary_reload = 0;
        return;
    }
    if (petal_name_is(pid, "Pollen")) {
        if (BitMath::at(player.input, InputFlags::kAttacking)
            || BitMath::at(player.input, InputFlags::kDefending)) {
            petal.friction = DEFAULT_FRICTION;
            entity_set_despawn_tick(petal, 4 * TPS);
        }
    } else if (petal_name_is(pid, "Missile") || petal_name_is(pid, "Magic Missile")
        || petal_name_is(pid, "Sand Leaf Dart")) {
            if (BitMath::at(player.input, InputFlags::kAttacking)) {
                petal.velocity.unit_normal(petal.get_angle()).set_magnitude(15 * PLAYER_ACCELERATION);
                petal.friction = DEFAULT_FRICTION;
                entity_set_despawn_tick(petal, 2.5 * TPS);
            }
    } else if (petal_name_is(pid, "Carrot")) {
            if (BitMath::at(player.input, InputFlags::kAttacking)
                && !BitMath::at(petal.flags, EntityFlags::kIsDespawning)) {
                float ang = petal.get_angle();
                petal.velocity.unit_normal(ang).set_magnitude(22.f * PLAYER_ACCELERATION);
                petal.friction = DEFAULT_FRICTION;
                petal.heading_angle = ang;
                BitMath::unset(petal.flags, EntityFlags::kTerrainBounced);
                entity_set_despawn_tick(petal, 3.0 * TPS);
            }
    } else if (petal_is_honey(pid)) {
            if (BitMath::at(player.input, InputFlags::kAttacking)) {
                // Straight outward bait — lock angle away from flower (no PassiveRot curve).
                BitMath::set(petal.flags, EntityFlags::kIsDetached);
                Vector out(petal.get_x() - player.get_x(), petal.get_y() - player.get_y());
                float ang = out.magnitude() > 1.f ? out.angle() : player.get_angle();
                petal.set_angle(ang);
                petal.velocity.unit_normal(ang).set_magnitude(4.5f * PLAYER_ACCELERATION);
                petal.friction = DEFAULT_FRICTION;
                entity_set_despawn_tick(petal, 8.0 * TPS);
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
                float plant_r = BASE_FLOWER_RADIUS * std::sqrt(16.f);
                petal.set_radius(plant_r);
                // Heavy enough that Unique-scale wildlife can't shove it away.
                // Attr mass is a rarity knob; floor keeps Common usable.
                float m = petal_data.attributes.mass;
                if (m < 1.f) m = 1.f;
                petal.mass = 48.f + m * 80.f;
                // Don't plant inside berms / walls ("陨石卡到地形里").
                {
                    float px = petal.get_x(), py = petal.get_y();
                    if (Map::circle_hits_impassable(sim->map_id, px, py, plant_r)) {
                        Map::eject_from_impassable(sim->map_id, px, py, plant_r);
                        px = fclamp(px, plant_r, (float)ARENA_WIDTH - plant_r);
                        py = fclamp(py, plant_r, (float)ARENA_HEIGHT - plant_r);
                        petal.set_x(px);
                        petal.set_y(py);
                    }
                }
                // Contact damage is ignored while planted (see Damage.cc). Keep a
                // high HP floor anyway for splash / non-contact edge cases.
                float plant_hp = petal.max_health * 40.f;
                if (plant_hp < 500.f) plant_hp = 500.f;
                if (plant_hp > petal.max_health) {
                    petal.max_health = plant_hp;
                    petal.health = plant_hp;
                }
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
    } else if (petal_name_is(pid, "Bubble") || petal_name_is(pid, "Magic Bubble")) {
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
    } else if (petal_name_is(pid, "Peas") || petal_name_is(pid, "Glass Storm")
        || petal_name_is(pid, "Grapes") || petal_name_is(pid, "Blueberries")) {
        // Full rarity ladders — exact PetalID cases missed every rung but the seed.
        // One-shot spray: already-flying shots must not split again every attack tick.
        // Peas use secondary_reload as a short ready gate; Grapes have none.
        uint8_t const can_spray = petal_data.attributes.secondary_reload <= 0 || sec_ready;
        if (can_spray
            && BitMath::at(player.input, InputFlags::kAttacking)
            && !BitMath::at(petal.flags, EntityFlags::kIsDespawning)) {
            float spray_angle = frand() * 2 * M_PI;
            // Farther spray: higher launch speed + longer life; Motion grants 1 terrain bounce.
            float const kPeaSpeed = 28.f * PLAYER_ACCELERATION;
            game_tick_t const kPeaLife = (game_tick_t)(2.25f * TPS);
            petal.friction = DEFAULT_FRICTION;
            petal.velocity.unit_normal(spray_angle).set_magnitude(kPeaSpeed);
            petal.set_split_projectile(0);
            petal.heading_angle = spray_angle;
            BitMath::unset(petal.flags, EntityFlags::kTerrainBounced);
            entity_set_despawn_tick(petal, kPeaLife);
            for (uint32_t i = 1; i < petal_data.count; ++i) {
                Entity &new_petal = alloc_petal(sim, petal.get_petal_id(), player);
                // Inherit leveled / buffed combat stats from the orbiting seed.
                // alloc_petal only copies PETAL_DATA base — left 15k vs seed 27k on dummies.
                new_petal.damage = petal.damage;
                new_petal.poison_damage = petal.poison_damage;
                new_petal.burn_damage = petal.burn_damage;
                new_petal.armor = petal.armor;
                new_petal.max_health = petal.max_health;
                new_petal.health = petal.health;
                new_petal.friction = DEFAULT_FRICTION;
                new_petal.set_x(petal.get_x());
                new_petal.set_y(petal.get_y());
                new_petal.set_split_projectile(0);
                new_petal.heading_angle = spray_angle + (2 * M_PI * i) / petal_data.count;
                new_petal.velocity
                    .unit_normal(new_petal.heading_angle)
                    .set_magnitude(kPeaSpeed);
                BitMath::unset(new_petal.flags, EntityFlags::kTerrainBounced);
                entity_set_despawn_tick(new_petal, kPeaLife);
            }
        }
    } else switch (pid) {
        case PetalID::kPollen:
            if (BitMath::at(player.input, InputFlags::kAttacking) || BitMath::at(player.input, InputFlags::kDefending)) {
                petal.friction = DEFAULT_FRICTION;
                entity_set_despawn_tick(petal, 4.0 * TPS);
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
            if (petal_data.attributes.aura_radius > 0.f
                && (petal_data.attributes.aura_heal > 0.f || petal_data.attributes.aura_slow_seconds > 0.f)
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
