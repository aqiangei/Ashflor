#include <Server/EntityFunctions.hh>
#include <Server/PetalMechanics.hh>

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
    if (!is_finite(amt) || amt < 0) return;
    // Lightning petals ship with body damage 0; flash is applied in contact
    // modifiers from PETAL_DATA / live petal.damage. Do not early-out on 0.
    if (amt <= 0) {
        if (type != DamageType::kContact || !sim->ent_alive(atk_id)) return;
        Entity const &atk0 = sim->get_ent(atk_id);
        if (!atk0.has_component(kPetal)) return;
        PetalID::T pid0 = atk0.get_petal_id();
        if (pid0 == PetalID::kNone || pid0 >= PetalID::kNumPetals) return;
        PetalAttributes const &A0 = PETAL_DATA[pid0].attributes;
        if (!(A0.lightning_damage > 0.f && A0.battery_charges == 0)) return;
        amt = 0.f;
    }
    if (!sim->ent_alive(def_id)) return;
    Entity &defender = sim->get_ent(def_id);
    if (!defender.has_component(kHealth)) return;
    DEBUG_ONLY(assert(!defender.pending_delete);)
    DEBUG_ONLY(assert(defender.has_component(kHealth));)
    if (defender.immunity_ticks > 0 && type != DamageType::kDeferred) return;
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
    // No PvP unless Corruption is active on either side.
    if (sim->ent_alive(atk_id)) {
        Entity &attacker = sim->get_ent(atk_id);
        if (is_player_faction(sim, attacker) && is_player_faction(sim, defender)
            && !player_faction_pvp(sim, attacker, defender))
            return;
    }
    // Flower defensive wiki attrs (Cotton absorb / Talisman evasion / Nazar / Wax / Shell / Sponge / Mana)
    if (defender.has_component(kFlower) && !defender.has_component(kMob)
        && type != DamageType::kReflect && type != DamageType::kDeferred) {
        float evade = 0.f;
        float absorb = 0.f;
        float nazar = 0.f;
        float mana_abs = 0.f;
        for (uint32_t si = 0; si < defender.get_loadout_count(); ++si) {
            PetalID::T pid = effective_petal_id(defender, si);
            if (pid == PetalID::kNone || pid >= PetalID::kNumPetals) continue;
            PetalAttributes const &B = PETAL_DATA[pid].attributes;
            if (B.flower_evasion > evade) evade = B.flower_evasion;
            if (B.damage_absorb > 0.f) absorb += B.damage_absorb;
            auto &slot = defender.loadout[si];
            if (B.nazar_charges && slot.effect_charges && B.nazar_mitigation > nazar)
                nazar = B.nazar_mitigation;
            if (B.mana_absorb_per > mana_abs) mana_abs = B.mana_absorb_per;
        }
        if (evade > 0.f && frand() < evade)
            return;
        // Wax consumes a slot charge and destroys the intercepted projectile.
        if (type == DamageType::kContact && sim->ent_alive(atk_id)
            && petal_projectile(sim->get_ent(atk_id))) {
            for (uint32_t si = 0; si < defender.get_loadout_count(); ++si) {
                auto pid = effective_petal_id(defender, si);
                auto &slot = defender.loadout[si];
                if (pid != PetalID::kNone && PETAL_DATA[pid].attributes.wax_block
                    && slot.effect_charges) {
                    --slot.effect_charges;
                    sim->request_delete(atk_id);
                    return;
                }
            }
        }
        // Shell / Amulet shield pool
        if (defender.flower_shield > 0.f && amt > 0.f) {
            float take = fminf(defender.flower_shield, amt);
            defender.flower_shield -= take;
            amt -= take;
            if (amt <= 0.f) return;
        }
        // Magic Cotton: spend mana to cancel damage (mana_absorb_per = dmg per mana)
        if (mana_abs > 0.f && defender.mana > 0.f && amt > 0.f) {
            float can = defender.mana * mana_abs;
            float take = fminf(can, amt);
            defender.mana -= take / mana_abs;
            if (defender.mana < 0.f) defender.mana = 0.f;
            amt -= take;
            if (amt <= 0.f) return;
        }
        if (absorb > 0.f) amt = fmaxf(0.f, amt - absorb);
        if (nazar > 0.f) {
            amt *= fmaxf(0.02f, 1.f - nazar);
            for (uint32_t si = 0; si < defender.get_loadout_count(); ++si) {
                auto pid = effective_petal_id(defender, si);
                auto &slot = defender.loadout[si];
                if (pid != PetalID::kNone && slot.effect_charges
                    && PETAL_DATA[pid].attributes.nazar_mitigation == nazar) {
                    --slot.effect_charges;
                    break;
                }
            }
        }
    }
    if (type == DamageType::kContact && defender.has_component(kPetal)) {
        // Planted Meteor crater: solid obstacle, not a disposable hit sponge.
        // Orbit HP ×40 still melts vs Unique-zone body damage on first touch.
        if (petal_is_meteor(defender.get_petal_id())
            && defender.get_split_projectile())
            return;
        auto const &a = PETAL_DATA[defender.get_petal_id()].attributes;
        if (a.phase_through || (a.petal_evasion > 0.f && frand() < a.petal_evasion)) return;
    }
    // Petal contact modifiers (read live from PETAL_DATA).
    if (type == DamageType::kContact && sim->ent_alive(atk_id)) {
        Entity &atk = sim->get_ent(atk_id);
        if (atk.has_component(kPetal)) {
            PetalAttributes const &A = PETAL_DATA[atk.get_petal_id()].attributes;
            if (A.vs_projectile_factor > 1.f && petal_projectile(defender))
                amt *= A.vs_projectile_factor;
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
                        PetalID::T pid = effective_petal_id(flower, si);
                        if (pid == PetalID::kNone || pid >= PetalID::kNumPetals) continue;
                        crit_p += PETAL_DATA[pid].attributes.luck * 0.04f;
                    }
                }
            }
            if (crit_p > 0.f && frand() < crit_p)
                amt *= (A.crit_factor > 1.f ? A.crit_factor : 35.f);
            // Domino: roll damage between current and domino_max
            if (A.domino_max_damage > 0.f) {
                float lo = atk.damage;
                float hi = A.domino_max_damage;
                if (hi < lo) { float t = lo; lo = hi; hi = t; }
                amt = lo + frand() * (hi - lo);
            }
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
            // Dark Mark: petal applies a brief vulnerability mark
            if (A.dark_mark) {
                game_tick_t mark = (game_tick_t)(4 * TPS);
                if (defender.dark_marked_ticks < mark)
                    defender.dark_marked_ticks = mark;
            }
            float armor_mit = defender.armor;
            if (A.armor_shred > 0.f) {
                armor_mit = fmaxf(0.f, armor_mit - A.armor_shred);
                // Track only actual removed armor and restore it on expiry.
                if (A.armor_shred_seconds > 0.f) {
                    float removed = defender.armor - armor_mit;
                    defender.armor_shredded += removed;
                    defender.armor = armor_mit;
                    defender.armor_shred_ticks = (game_tick_t)(A.armor_shred_seconds * TPS);
                }
            }
            if (A.armor_penetration > 0.f) {
                float pen = A.armor_penetration;
                if (pen > 1.f) pen = 1.f;
                armor_mit *= (1.f - pen);
            }
            amt -= armor_mit;
            // Lightning contact burst (Lightning / Laser / Blueberries).
            // Prefer live petal.damage — Flower.cc bakes Unique Lv + amp into it
            // when base body damage is 0 (Lightning ladder).
            if (A.lightning_damage > 0.f && !A.battery_charges) {
                float flash = A.lightning_damage;
                if (atk.damage > flash) flash = atk.damage;
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
    if (defender.dark_marked_ticks && type != DamageType::kDeferred) amt *= 1.25f;
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
    // Wiki Bone: each hit halves armor — including fully blocked hits (amt≤0).
    if (type == DamageType::kContact
        && defender.has_component(kPetal)
        && defender.armor > 0.f
        && petal_is_bone(defender.get_petal_id()))
        defender.armor *= 0.5f;
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
    if (defender.has_component(kFlower) && !defender.has_component(kMob)
        && (type == DamageType::kContact || type == DamageType::kSplash)) {
        float delay = 0.f;
        for (uint32_t si = 0; si < defender.get_loadout_count(); ++si) {
            auto pid = effective_petal_id(defender, si);
            if (pid != PetalID::kNone) delay = fmaxf(delay, PETAL_DATA[pid].attributes.damage_delay_seconds);
        }
        if (delay > 0.f) {
            if (!defender.pending_damage_ticks)
                defender.pending_damage_ticks = (game_tick_t)fmaxf(1.f, delay * TPS);
            defender.pending_damage += amt;
            defender.pending_damage_dealer = atk_id;
            defender.powder_suppressed_ticks = 3 * TPS;
            return;
        }
    }
    //if (amt <= defender.armor) return;
    double old_health = defender.health;
    defender.set_damaged(1);
    float damage_dealt;
    if (training) {
        // Count the complete hit, including overkill. Never enter death/loot paths.
        TrainingDummy::record_damage(sim, defender, atk_id, amt);
        defender.health = defender.max_health;
        damage_dealt = amt;
    } else {
        // High-tier mobs exceed float's unit precision (Unique Hornet ~1.6B
        // HP). Keep subtraction in double so low-damage hits still accumulate.
        defender.health = std::fmax(0.0, defender.health - (double)amt);
        damage_dealt = old_health - defender.health;
    }
    // Bandage / Ankh: intercept lethal hits on flowers
    if (!training && defender.health <= 0.f
        && defender.has_component(kFlower) && !defender.has_component(kMob)) {
        float death_delay_s = 0.f;
        for (uint32_t si = 0; si < defender.get_loadout_count(); ++si) {
            PetalID::T pid = effective_petal_id(defender, si);
            if (pid == PetalID::kNone || pid >= PetalID::kNumPetals) continue;
            PetalAttributes const &B = PETAL_DATA[pid].attributes;
            if (B.death_delay_seconds > death_delay_s)
                death_delay_s = B.death_delay_seconds;
        }
        if (death_delay_s > 0.f && !defender.death_delay_used && defender.death_delay_ticks == 0) {
            defender.death_delay_used = 1;
            defender.health = 1.f;
            defender.death_delay_ticks = (game_tick_t)(death_delay_s * TPS);
            if (defender.death_delay_ticks < 1) defender.death_delay_ticks = 1;
            defender.immunity_ticks = defender.death_delay_ticks;
            damage_dealt = old_health - 1.f;
            if (damage_dealt < 0.f) damage_dealt = 0.f;
        }
    }
    if (damage_dealt > 0.f && defender.has_component(kFlower))
        defender.powder_suppressed_ticks = 3 * TPS;
    defender.set_hit_damage(defender.get_hit_damage() + damage_dealt);
    // Battery: discharge lightning when the flower is hit.
    if (damage_dealt > 0.f
        && type != DamageType::kReflect
        && defender.has_component(kFlower) && !defender.has_component(kMob)) {
        for (uint32_t si = 0; si < defender.get_loadout_count(); ++si) {
            PetalID::T pid = effective_petal_id(defender, si);
            if (pid == PetalID::kNone || pid >= PetalID::kNumPetals) continue;
            PetalAttributes const &B = PETAL_DATA[pid].attributes;
            auto &slot = defender.loadout[si];
            if (B.battery_charges == 0 || B.lightning_damage <= 0.f || !slot.effect_charges) continue;
            --slot.effect_charges;
            float const R = 200.f;
            float const px = defender.get_x();
            float const py = defender.get_y();
            float flash = B.lightning_damage;
            uint8_t left = B.lightning_chains > 0 ? B.lightning_chains : 1;
            EntityID team = defender.get_team();
            EntityID src = defender.id;
            sim->spatial_hash.query(px, py, R, R, [&](Simulation *s, Entity &ent) {
                if (left == 0) return;
                if (!ent.has_component(kHealth)) return;
                if (ent.get_team() == team) return;
                if (ent.has_component(kPetal) || ent.has_component(kDrop)) return;
                float dx = ent.get_x() - px;
                float dy = ent.get_y() - py;
                if (dx * dx + dy * dy > R * R) return;
                --left;
                inflict_damage(s, src, ent.id, flash, DamageType::kSplash);
                flash *= 0.65f;
            });
            break; // one battery discharge per hit
        }
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
            // Termite psionic link — keep tight so packs don't feel map-wide shared HP.
            float const R = 180.f;
            float const px = defender.get_x();
            float const py = defender.get_y();
            float const share = damage_dealt * 0.22f;
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
        uint8_t hole_r = defender.get_rarity();
        for (uint32_t i = start; i < end; ++i) {
            for (MobID::T mob_id : ANTHOLE_SPAWNS[i]) {
                Entity &child = alloc_mob(
                    sim, mob_id, 
                    defender.get_x(), defender.get_y(), 
                    defender.get_team(), [hole_r](Entity &mob) {
                    mob.score_reward = MOB_DATA[mob.get_mob_id()].xp;
                    BitMath::set(mob.flags, EntityFlags::kHasCulling);
                    apply_mob_live_rarity(mob, hole_r);
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

    // Resolve the committed contact hit before reflection or source teardown.
    // Fang has only one durability at low tiers: breaking on this contact must
    // not cancel the heal earned by the very same hit.
    if (type == DamageType::kContact && attacker.has_component(kPetal) && damage_dealt > 0.f
        && sim->ent_alive(attacker.get_parent())) {
        auto const &a = PETAL_DATA[attacker.get_petal_id()].attributes;
        auto &owner = sim->get_ent(attacker.get_parent());
        if (owner.has_component(kFlower) && owner.has_component(kHealth)) {
            if (a.on_hit_heal > 0.f) inflict_heal(sim, owner, a.on_hit_heal);
            if (a.on_hit_heal_ratio > 0.f)
                inflict_heal(sim, owner, damage_dealt * a.on_hit_heal_ratio);
        }
    }

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
            float pdot = attacker.poison_damage.damage / TPS;
            // Privet Berry: amp poison DPS while the attacker flower equips it.
            if (sim->ent_alive(attacker.base_entity)) {
                Entity &own = sim->get_ent(attacker.base_entity);
                if (own.has_component(kFlower) && !own.has_component(kMob)) {
                    float amp = 0.f;
                    for (uint32_t si = 0; si < own.get_loadout_count(); ++si) {
                        PetalID::T pid = effective_petal_id(own, si);
                        if (pid == PetalID::kNone || pid >= PetalID::kNumPetals) continue;
                        float a = PETAL_DATA[pid].attributes.privet_extra_poison;
                        if (a > amp) amp = a;
                    }
                    if (amp > 0.f) pdot *= (1.f + amp);
                }
            }
            defender.poison_inflicted = pdot;
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
        // Lightning chain: jump nearest→nearest (not flat AoE in one query).
        if (A.lightning_damage > 0.f && A.lightning_chains > 1) {
            float const R = 220.f;
            float const R2 = R * R;
            float chain_dmg = (attacker.damage > A.lightning_damage
                ? attacker.damage : A.lightning_damage) * 0.65f;
            uint8_t left = A.lightning_chains - 1;
            EntityID atk_team = attacker.get_team();
            EntityID src = atk_id;
            EntityID hit[16];
            uint8_t nhit = 1;
            hit[0] = def_id;
            float cx = defender.get_x();
            float cy = defender.get_y();
            while (left > 0 && nhit < 16) {
                EntityID best = NULL_ENTITY;
                float best_d2 = R2;
                float const qx = cx, qy = cy;
                sim->spatial_hash.query(qx, qy, R, R, [&](Simulation *, Entity &ent) {
                    if (!ent.has_component(kHealth)) return;
                    if (ent.get_team() == atk_team) return;
                    if (ent.has_component(kPetal) || ent.has_component(kDrop)) return;
                    for (uint8_t i = 0; i < nhit; ++i)
                        if (ent.id == hit[i]) return;
                    float dx = ent.get_x() - qx;
                    float dy = ent.get_y() - qy;
                    float d2 = dx * dx + dy * dy;
                    if (d2 > R2 || d2 >= best_d2) return;
                    best_d2 = d2;
                    best = ent.id;
                });
                if (best == NULL_ENTITY) break;
                hit[nhit++] = best;
                --left;
                inflict_damage(sim, src, best, chain_dmg, DamageType::kSplash);
                if (!sim->ent_alive(best)) break;
                Entity &nxt = sim->get_ent(best);
                cx = nxt.get_x();
                cy = nxt.get_y();
                chain_dmg *= 0.65f;
            }
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
    if (!is_finite(amt) || amt <= 0.f || ent.pending_delete || ent.health <= 0) return;
    if (ent.dandy_ticks > 0) return;
    // Basil: amplify healing received
    if (ent.has_component(kFlower) && !ent.has_component(kMob) && amt > 0.f) {
        float amp = 0.f;
        for (uint32_t si = 0; si < ent.get_loadout_count(); ++si) {
            PetalID::T pid = effective_petal_id(ent, si);
            if (pid == PetalID::kNone || pid >= PetalID::kNumPetals) continue;
            float a = PETAL_DATA[pid].attributes.heal_amp;
            if (a > amp) amp = a;
        }
        if (amp > 0.f) amt *= (1.f + amp);
    }
    float room = ent.max_health - ent.health;
    if (room < 0.f) room = 0.f;
    float applied = fminf(amt, room);
    ent.health = std::fmin(ent.max_health, std::fmax(0.0, ent.health + applied));
    // Amulet: overheal → flower_shield
    float over = amt - applied;
    if (over > 0.f && ent.has_component(kFlower) && !ent.has_component(kMob)
        && ent.overheal_shield_ratio > 0.f) {
        ent.flower_shield += over * ent.overheal_shield_ratio;
        float cap = ent.max_health;
        if (ent.flower_shield > cap) ent.flower_shield = cap;
    }
}
