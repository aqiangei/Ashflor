#include <Server/EntityFunctions.hh>

#include <Server/Client.hh>
#include <Server/DevCheats.hh>
#include <Server/Loot.hh>
#include <Server/Server.hh>
#include <Server/Spawn.hh>
#include <Server/TrainingDummy.hh>
#include <Shared/Entity.hh>
#include <Shared/Simulation.hh>
#include <Shared/SpawnRules.hh>
#include <Shared/StaticData.hh>
#include <Helpers/Math.hh>

#include <cmath>
#include <cstring>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static bool _yggdrasil_revival_clause(Simulation *sim, Entity &player) {
    for (uint32_t i = 0; i < player.get_loadout_count(); ++i) {
        if (!player.loadout[i].already_spawned) continue;
        if (player.loadout[i].get_petal_id() != PetalID::kYggdrasil) continue;
        player.set_loadout_ids(i, PetalID::kNone);
        return true;
    }
    return false;
}

void inflict_damage(Simulation *sim, EntityID const atk_id, EntityID const def_id, float amt, uint8_t type) {
    if (!is_finite(amt) || amt <= 0) return;
    if (!sim->ent_alive(def_id)) return;
    Entity &defender = sim->get_ent(def_id);
    if (!defender.has_component(kHealth)) return;
    DEBUG_ONLY(assert(!defender.pending_delete);)
    DEBUG_ONLY(assert(defender.has_component(kHealth));)
    if (defender.immunity_ticks > 0) return;
    bool const training = TrainingDummy::is_dummy(defender);
    auto *training_stats = training ? TrainingDummy::personal_stats(sim, defender, atk_id) : nullptr;
    // Local QA: "dev" account godmode (does not use immunity_ticks — portals still work).
    if (defender.has_component(kFlower)) {
        Client *owner = Server::client_by_camera(sim, defender.get_parent());
        if (DevCheats::active(owner)) return;
    }
    // Explore guide spirits / treasure NPCs: scripted props, never die to petals or mobs.
    // Hatch summons share kNoDrops — never treat player-faction pets as immortal props.
    // Training dummies also match the 0-damage/NoDrops pattern but must take hits.
    if (defender.has_component(kMob)
        && !TrainingDummy::is_dummy(defender)
        && !is_player_faction(sim, defender)
        && defender.damage <= 0.f && defender.score_reward == 0
        && BitMath::at(defender.flags, EntityFlags::kNoDrops)
        && defender.get_nametag_visible())
        return;
    // No PvP: players, CPU bots, petals, and flower summons cannot hurt each other
    if (sim->ent_alive(atk_id)) {
        Entity &attacker = sim->get_ent(atk_id);
        if (is_player_faction(sim, attacker) && is_player_faction(sim, defender))
            return;
    }
    // Flower defensive wiki attrs (Cotton absorb / Talisman evasion / Nazar)
    if (defender.has_component(kFlower) && !defender.has_component(kMob)
        && type != DamageType::kReflect) {
        float evade = 0.f;
        float absorb = 0.f;
        float nazar = 0.f;
        for (uint32_t si = 0; si < defender.get_loadout_count(); ++si) {
            PetalID::T pid = defender.get_loadout_ids(si);
            if (pid == PetalID::kNone || pid >= PetalID::kNumPetals) continue;
            PetalAttributes const &B = PETAL_DATA[pid].attributes;
            if (B.flower_evasion > evade) evade = B.flower_evasion;
            if (B.damage_absorb > 0.f) absorb += B.damage_absorb;
            if (B.nazar_charges > 0 && B.nazar_mitigation > nazar)
                nazar = B.nazar_mitigation;
        }
        if (evade > 0.f && frand() < evade)
            return;
        if (absorb > 0.f) amt = fmaxf(0.f, amt - absorb);
        if (nazar > 0.f) amt *= fmaxf(0.05f, 1.f - nazar);
    }
    // Petal contact modifiers (read live from PETAL_DATA).
    if (type == DamageType::kContact && sim->ent_alive(atk_id)) {
        Entity &atk = sim->get_ent(atk_id);
        if (atk.has_component(kPetal)) {
            // Glass: phase through — petal takes no contact durability loss as defender
            if (defender.has_component(kPetal)) {
                PetalAttributes const &Pd = PETAL_DATA[defender.get_petal_id()].attributes;
                if (Pd.phase_through)
                    return;
            }
            PetalAttributes const &A = PETAL_DATA[atk.get_petal_id()].attributes;
            // Wiki: petal evasion (Poker Chip) — chance to take no durability hit as defender petal
            if (defender.has_component(kPetal)) {
                PetalAttributes const &D = PETAL_DATA[defender.get_petal_id()].attributes;
                if (D.petal_evasion > 0.f && frand() < D.petal_evasion)
                    return;
                if (A.vs_projectile_factor > 1.f)
                    amt *= A.vs_projectile_factor;
            }
            // Wiki: Glass hit interval — skip if recently hit by this petal instance
            if (A.hit_interval_seconds > 0.f) {
                game_tick_t need = (game_tick_t)(A.hit_interval_seconds * TPS);
                if (need < 1) need = 1;
                if (atk.secondary_reload < need) return;
                atk.secondary_reload = 0;
            }
            if (A.bonus_vs_poisoned > 0 && (training_stats ? training_stats->poison_ticks : defender.poison_ticks) > 0)
                amt *= (1.f + A.bonus_vs_poisoned);
            if (A.bonus_vs_slowed > 0 && (training_stats ? training_stats->slow_ticks : defender.slow_ticks) > 0)
                amt *= (1.f + A.bonus_vs_slowed);
            // Wiki: Dice crit (base chance; luck from flower buffs applied as flat add via petal luck attr sum later)
            float crit_p = A.crit_chance;
            if (crit_p > 0.f && sim->ent_alive(atk.get_parent())) {
                Entity &flower = sim->get_ent(atk.get_parent());
                if (flower.has_component(kFlower)) {
                    for (uint32_t si = 0; si < flower.get_loadout_count(); ++si) {
                        PetalID::T pid = flower.get_loadout_ids(si);
                        if (pid == PetalID::kNone || pid >= PetalID::kNumPetals) continue;
                        crit_p += PETAL_DATA[pid].attributes.luck * 0.04f;
                    }
                }
            }
            if (crit_p > 0.f && frand() < crit_p)
                amt *= (A.crit_factor > 1.f ? A.crit_factor : 35.f);
            // Wiki: Bean solo bonus
            if (A.solo_damage_factor > 1.f && sim->ent_alive(atk.get_parent())) {
                Entity &flower = sim->get_ent(atk.get_parent());
                uint8_t ally = 0;
                float const fx = flower.get_x(), fy = flower.get_y();
                sim->spatial_hash.query(fx, fy, 400.f, 400.f, [&](Simulation *s, Entity &ent) {
                    if (ally) return;
                    if (!ent.has_component(kFlower) || !ent.has_component(kHealth)) return;
                    if (ent.id == flower.id) return;
                    if (ent.get_team() != flower.get_team()) return;
                    ally = 1;
                });
                if (!ally) amt *= A.solo_damage_factor;
            }
            if (A.hit_cycle > 1) {
                ++atk.contact_hit_count;
                if (atk.contact_hit_count >= A.hit_cycle) {
                    atk.contact_hit_count = 0;
                    amt *= (A.hit_cycle_factor > 0.f ? A.hit_cycle_factor : 2.f);
                }
            }
            // Claw wiki: bonus when target ABOVE 80% HP (invert execute gate)
            if (A.execute_threshold >= 0.79f && A.execute_threshold <= 0.81f
                && defender.max_health > 0.f
                && defender.health / defender.max_health > A.execute_threshold)
                amt *= (A.execute_factor > 0.f ? A.execute_factor : 1.5f);
            else if (A.execute_threshold > 0.f && A.execute_threshold < 0.79f
                && defender.max_health > 0.f
                && defender.health / defender.max_health <= A.execute_threshold)
                amt *= (A.execute_factor > 0.f ? A.execute_factor : 1.5f);
            float armor_mit = defender.armor;
            if (A.armor_shred > 0.f)
                armor_mit = fmaxf(0.f, armor_mit - A.armor_shred);
            if (A.armor_penetration > 0.f) {
                float pen = A.armor_penetration;
                if (pen > 1.f) pen = 1.f;
                armor_mit *= (1.f - pen);
            }
            amt -= armor_mit;
            // Lightning contact burst (Lightning / Laser / Blueberries)
            if (A.lightning_damage > 0.f && !A.battery_charges) {
                float flash = A.lightning_damage;
                if (flash > amt) amt = flash;
            }
        } else {
            amt -= defender.armor;
        }
    } else if (type == DamageType::kContact) {
        amt -= defender.armor;
    } else if (type == DamageType::kPoison) {
        amt -= defender.poison_armor;
    }
    // Shield Sentinel: frontal damage reduction (~60% when hit from facing arc)
    if (type == DamageType::kContact
        && defender.has_component(kMob)
        && defender.get_mob_id() == MobID::kShieldSentinel
        && sim->ent_alive(atk_id)) {
        Entity &atk = sim->get_ent(atk_id);
        if (atk.has_component(kPhysics) && defender.has_component(kPhysics)) {
            float dx = atk.get_x() - defender.get_x();
            float dy = atk.get_y() - defender.get_y();
            float hit_ang = std::atan2(dy, dx);
            float d = hit_ang - defender.get_angle();
            while (d > (float)M_PI) d -= (float)(2.0 * M_PI);
            while (d < -(float)M_PI) d += (float)(2.0 * M_PI);
            if (std::fabs(d) < (float)M_PI * 0.55f)
                amt *= 0.4f;
        }
    }
    if (!is_finite(amt) || amt <= 0) return;
    // Petal vs mob contact: hits = ceil(petal_hp / hit_damage).
    if (type == DamageType::kContact
        && defender.has_component(kPetal)
        && defender.max_health > 0.f
        && amt > 0.f) {
        float hits = ceilf(defender.max_health / amt);
        if (hits < 1.f) hits = 1.f;
        amt = defender.max_health / hits;
    }
    //if (amt <= defender.armor) return;
    float old_health = defender.health;
    defender.set_damaged(1);
    float damage_dealt;
    if (training) {
        // Count the complete hit, including overkill. Never enter death/loot paths.
        TrainingDummy::record_damage(sim, defender, atk_id, amt);
        defender.health = defender.max_health;
        damage_dealt = amt;
    } else {
        defender.health = fclamp(defender.health - amt, 0, defender.health);
        damage_dealt = old_health - defender.health;
    }
    // Wiki Bone: each hit halves remaining petal armor (after mitigation).
    if (type == DamageType::kContact
        && damage_dealt > 0.f
        && defender.has_component(kPetal)
        && defender.armor > 0.f) {
        PetalID::T const pid = defender.get_petal_id();
        if (pid < PetalID::kNumPetals
            && PETAL_DATA[pid].name
            && std::strcmp(PETAL_DATA[pid].name, "Bone") == 0)
            defender.armor *= 0.5f;
    }
    // Record before reflection can kill the source. All player-owned damage,
    // including reflection and summons, contributes to the personal claim.
    if (defender.has_component(kMob) && !training && damage_dealt > 0.f) {
        EntityID const claim = Loot::owner_camera(sim, atk_id);
        if (!claim.null()) {
            uint8_t i = 0;
            for (; i < defender.loot_claim_n && i < 16; ++i)
                if (defender.loot_claim_teams[i] == claim) break;
            if (i < 16) {
                if (i == defender.loot_claim_n) {
                    ++defender.loot_claim_n;
                    defender.loot_claim_teams[i] = claim;
                }
                defender.loot_claim_damage[i] += damage_dealt;
            }
        }
        // Termite psionic connection: nearby kin share a portion of the hit.
        static thread_local int share_depth = 0;
        MobID::T const mid = defender.get_mob_id();
        if (share_depth == 0
            && mid < MobID::kNumMobs
            && MOB_DATA[mid].attributes.share_damage
            && type != DamageType::kReflect
            && type != DamageType::kPoison
            && type != DamageType::kBurn) {
            float const R = 380.f;
            float const px = defender.get_x();
            float const py = defender.get_y();
            float const share = damage_dealt * 0.35f;
            EntityID skip = def_id;
            EntityID src = atk_id;
            ++share_depth;
            sim->spatial_hash.query(px, py, R, R, [&](Simulation *s, Entity &ent) {
                if (ent.id == skip) return;
                if (!ent.has_component(kMob) || !ent.has_component(kHealth)) return;
                MobID::T oid = ent.get_mob_id();
                if (oid >= MobID::kNumMobs || !MOB_DATA[oid].attributes.share_damage) return;
                float dx = ent.get_x() - px;
                float dy = ent.get_y() - py;
                if (dx * dx + dy * dy > R * R) return;
                inflict_damage(s, src, ent.id, share, DamageType::kSplash);
            });
            --share_depth;
        }
    }
    //ant hole spawns
    //floor start, ceil end
    if (defender.has_component(kMob) && defender.get_mob_id() == MobID::kAntHole) {
        uint32_t const num_waves = ANTHOLE_SPAWNS.size() - 1;
        uint32_t start = ceilf((defender.max_health - old_health) / defender.max_health * num_waves);
        uint32_t end = ceilf((defender.max_health - defender.health) / defender.max_health * num_waves);
        if (defender.health <= 0) end = num_waves + 1;
        for (uint32_t i = start; i < end; ++i) {
            for (MobID::T mob_id : ANTHOLE_SPAWNS[i]) {
                Entity &child = alloc_mob(
                    sim, mob_id, 
                    defender.get_x(), defender.get_y(), 
                    defender.get_team(), [](Entity &mob) {
                    mob.score_reward = MOB_DATA[mob.get_mob_id()].xp;
                    BitMath::set(mob.flags, EntityFlags::kHasCulling);
                });
                child.set_parent(defender.id);
                child.target = defender.target;
            }
        }
    }
    /* yggdrasil revive clause
    if (defender.health == 0 && defender.has_component(kFlower)) {
        if (_yggdrasil_revival_clause(sim, defender)) {
            defender.health = defender.max_health * 0.25;
            defender.immunity_ticks = 1.0 * TPS;
        }
    }
    */
    if (!sim->ent_exists(atk_id)) return;
    Entity &attacker = sim->get_ent(atk_id);

    if (type != DamageType::kReflect && defender.damage_reflection > 0)
        inflict_damage(sim, def_id, attacker.base_entity, damage_dealt * defender.damage_reflection, DamageType::kReflect);
    
    if (!sim->ent_alive(atk_id)) return;

    if (!training) {
        if (defender.slow_ticks < attacker.slow_inflict)
            defender.slow_ticks = attacker.slow_inflict;

        if (attacker.has_component(kPetal)) {
            PetalID::T pid = attacker.get_petal_id();
            if (pid < PetalID::kNumPetals) {
                float dandy_s = PETAL_DATA[pid].attributes.dandelion_inflict_seconds;
                if (dandy_s > 0.f) {
                    game_tick_t ticks = (game_tick_t)(dandy_s * TPS);
                    if (defender.dandy_ticks < ticks)
                        defender.dandy_ticks = ticks;
                }
            }
        }

        if (attacker.has_component(kPetal)) {
            if (!sim->ent_alive(defender.target))
                defender.target = attacker.base_entity;

        } else {
            if (!sim->ent_alive(defender.target))
                defender.target = atk_id;
        }
        defender.last_damaged_by = attacker.base_entity;

        if (type == DamageType::kContact && defender.poison_ticks < attacker.poison_damage.time * TPS) {
            defender.poison_ticks = attacker.poison_damage.time * TPS;
            defender.poison_inflicted = attacker.poison_damage.damage / TPS;
            defender.poison_dealer = defender.last_damaged_by;
        }
        if (type == DamageType::kContact && defender.burn_ticks < attacker.burn_damage.time * TPS) {
            defender.burn_ticks = attacker.burn_damage.time * TPS;
            defender.burn_inflicted = attacker.burn_damage.damage / TPS;
            defender.burn_dealer = defender.last_damaged_by;
        }
    } else if (training_stats) {
        TrainingDummy::inflict_status(*training_stats, attacker, type);
    }

    if (type == DamageType::kContact && attacker.has_component(kPetal) && damage_dealt > 0.f) {
        PetalAttributes const &A = PETAL_DATA[attacker.get_petal_id()].attributes;
        if (A.snare_seconds > 0.f) {
            game_tick_t sn = (game_tick_t)(A.snare_seconds * TPS);
            if (training_stats) training_stats->slow_ticks = std::max(training_stats->slow_ticks, sn);
            else if (defender.snare_ticks < sn) defender.snare_ticks = sn;
        }
        if (A.on_hit_heal > 0.f && sim->ent_alive(attacker.get_parent())) {
            Entity &flower = sim->get_ent(attacker.get_parent());
            if (flower.has_component(kFlower) && flower.has_component(kHealth))
                inflict_heal(sim, flower, A.on_hit_heal);
        }
        if (A.on_hit_heal_ratio > 0.f && sim->ent_alive(attacker.get_parent())) {
            Entity &flower = sim->get_ent(attacker.get_parent());
            if (flower.has_component(kFlower) && flower.has_component(kHealth))
                inflict_heal(sim, flower, damage_dealt * A.on_hit_heal_ratio);
        }
        // Lightning chain to nearby enemies
        if (A.lightning_damage > 0.f && A.lightning_chains > 1) {
            float const R = 180.f;
            float const px = defender.get_x();
            float const py = defender.get_y();
            float const chain_dmg = A.lightning_damage * 0.65f;
            uint8_t left = A.lightning_chains - 1;
            EntityID atk_team = attacker.get_team();
            EntityID skip = def_id;
            EntityID src = atk_id;
            sim->spatial_hash.query(px, py, R, R, [&](Simulation *s, Entity &ent) {
                if (left == 0) return;
                if (ent.id == skip) return;
                if (!ent.has_component(kHealth)) return;
                if (ent.get_team() == atk_team) return;
                if (ent.has_component(kPetal) || ent.has_component(kDrop)) return;
                float dx = ent.get_x() - px;
                float dy = ent.get_y() - py;
                if (dx * dx + dy * dy > R * R) return;
                --left;
                inflict_damage(s, src, ent.id, chain_dmg, DamageType::kSplash);
            });
        }
        if (A.taunt_radius > 0.f && sim->ent_alive(attacker.base_entity)) {
            float const R = A.taunt_radius;
            float const px = defender.get_x();
            float const py = defender.get_y();
            EntityID flower_id = attacker.base_entity;
            EntityID atk_team = attacker.get_team();
            sim->spatial_hash.query(px, py, R, R, [&](Simulation *s, Entity &ent) {
                if (!ent.has_component(kMob)) return;
                if (ent.get_team() == atk_team) return;
                float dx = ent.get_x() - px;
                float dy = ent.get_y() - py;
                if (dx * dx + dy * dy > R * R) return;
                ent.target = flower_id;
            });
        }
        if (A.splash_damage > 0.f && A.splash_radius > 0.f) {
            float const R = A.splash_radius;
            float const px = defender.get_x();
            float const py = defender.get_y();
            float const splash = A.splash_damage;
            EntityID atk_team = attacker.get_team();
            EntityID skip = def_id;
            EntityID src = atk_id;
            sim->spatial_hash.query(px, py, R, R, [&](Simulation *s, Entity &ent) {
                if (ent.id == skip) return;
                if (!ent.has_component(kHealth)) return;
                if (ent.get_team() == atk_team) return;
                if (ent.has_component(kPetal) || ent.has_component(kDrop)) return;
                float dx = ent.get_x() - px;
                float dy = ent.get_y() - py;
                if (dx * dx + dy * dy > R * R) return;
                inflict_damage(s, src, ent.id, splash, DamageType::kSplash);
            });
        }
    }
}

void inflict_heal(Simulation *sim, Entity &ent, float amt) {
    DEBUG_ONLY(assert(ent.has_component(kHealth));)
    if (ent.pending_delete || ent.health <= 0) return;
    if (ent.dandy_ticks > 0) return;
    // Basil: amplify healing received
    if (ent.has_component(kFlower) && !ent.has_component(kMob) && amt > 0.f) {
        float amp = 0.f;
        for (uint32_t si = 0; si < ent.get_loadout_count(); ++si) {
            PetalID::T pid = ent.get_loadout_ids(si);
            if (pid == PetalID::kNone || pid >= PetalID::kNumPetals) continue;
            float a = PETAL_DATA[pid].attributes.heal_amp;
            if (a > amp) amp = a;
        }
        if (amp > 0.f) amt *= (1.f + amp);
    }
    ent.health = fclamp(ent.health + amt, 0, ent.max_health);
}
