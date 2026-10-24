#include <Server/EntityFunctions.hh>
#include <Server/AccountStore.hh>
#include <Server/Client.hh>
#include <Server/Game.hh>
#include <Server/Loot.hh>
#include <Server/PetalTracker.hh>
#include <Server/Server.hh>
#include <Server/TrainingDummy.hh>

#include <Shared/Simulation.hh>
#include <Shared/Entity.hh>
#include <Shared/Recipe.hh>
#include <Shared/StaticData.hh>
#include <Shared/Binary.hh>

#include <Helpers/Math.hh>

#include <algorithm>
#include <cmath>
#include <iostream>

static bool _should_interact(Entity const &ent1, Entity const &ent2) {
    //if (ent1.has_component(kFlower) || ent2.has_component(kFlower)) return false;
    //if (ent1.has_component(kPetal) || ent2.has_component(kPetal)) return false;
    if (ent1.pending_delete || ent2.pending_delete) return false;
    // Drops must collide with living flowers so pickup can run (not revive corpses).
    if ((ent1.has_component(kDrop) && ent2.has_component(kFlower))
        || (ent2.has_component(kDrop) && ent1.has_component(kFlower))) {
        Entity const &flower = ent1.has_component(kFlower) ? ent1 : ent2;
        if (!flower.has_component(kHealth)) return false;
        return true;
    }
    // Loot has no push/damage/web interaction. Discard these pairs before
    // distance normalization when a battle leaves many private piles behind.
    if (ent1.has_component(kDrop) || ent2.has_component(kDrop)) return false;
    // Planted meteors may overlap each other (AFK stacking).
    if (ent1.has_component(kPetal) && ent2.has_component(kPetal)
        && petal_is_meteor(ent1.get_petal_id())
        && petal_is_meteor(ent2.get_petal_id()))
        return false;
    // Players never collide with Meteor (own or enemy) — mobs still blocked.
    if (ent1.has_component(kFlower) && ent2.has_component(kPetal)
        && petal_is_meteor(ent2.get_petal_id()))
        return false;
    if (ent2.has_component(kFlower) && ent1.has_component(kPetal)
        && petal_is_meteor(ent1.get_petal_id()))
        return false;
    if (!(ent1.get_team() == ent2.get_team())) return true;
    if (BitMath::at((ent1.flags | ent2.flags), EntityFlags::kNoFriendlyCollision)) return false;
    //if (ent1.has_component(kPetal) || ent2.has_component(kPetal)) return false;
    if (ent1.has_component(kMob) && ent2.has_component(kMob)) return true;
    // Same-team flowers still soft-collide (party / auto-party shared team).
    if (ent1.has_component(kFlower) && ent2.has_component(kFlower)) return true;
    return false;
}

void pickup_drop(Simulation *sim, Entity &player, Entity &drop) {
    if (player.pending_delete || drop.pending_delete) return;
    if (!sim->ent_alive(player.get_parent())) return;
    // Private loot: tagged piles belong to one camera only (not party-shared).
    // Unowned piles: brief immunity then anyone. Party still shares mob XP.
    Client *client = Server::client_by_camera(sim, player.get_parent());
    if (client == nullptr || !Loot::can_access_drop(drop, player.get_parent())
        || drop.immunity_ticks > 0) return;

    PetalID::T drop_id = drop.get_drop_id();
    if (drop_id == PetalID::kNone || drop_id >= PetalID::kNumPetals) return;
    uint16_t qty = drop.get_drop_count();
    if (qty < 1) qty = 1;
    // Picking up loot never changes either equipment row, even when empty.
    uint32_t const cap = Recipe::bank_stack_cap(drop_id);
    uint32_t const held = client->craft_bank[drop_id];
    uint16_t const collected = (uint16_t) std::min<uint32_t>(qty, held < cap ? cap - held : 0);
    if (collected == 0) return;
    client->craft_bank[drop_id] += collected;
    for (uint16_t i = 0; i < collected; ++i) client_credit_petal_pick(client, drop_id);
    AccountStore::capture_nosave(client);
    client_sync_craft(client);
    if (collected < qty) {
        drop.set_drop_count(qty - collected);
        return;
    }
    // Do not teleport the pile onto the flower before delete — the client
    // lerps that position update, so a moving player looks like drops slowly
    // chase them. Instant absorb = delete in place.
    BitMath::unset(drop.flags, EntityFlags::kIsDespawning);
    PetalTracker::remove_petal(sim, drop_id);
    sim->request_delete(drop.id);
}

#define NO(component) (!ent1.has_component(component) && !ent2.has_component(component))
#define BOTH(component) (ent1.has_component(component) && ent2.has_component(component))
#define EITHER(component) (ent1.has_component(component) || ent2.has_component(component))

static void _deal_push(Entity &ent, Vector knockback, float mass_ratio, float scale) {
    if (fabsf(mass_ratio) < 0.01) return;
    if (!is_finite(knockback.x) || !is_finite(knockback.y)) return;
    if (!is_finite(mass_ratio) || !is_finite(scale)) return;
    knockback *= scale * mass_ratio;
    ent.collision_velocity += knockback;
}

static void _deal_knockback(Entity &ent, Vector knockback, float mass_ratio) {
    if (fabsf(mass_ratio) < 0.01) return;
    if (!is_finite(knockback.x) || !is_finite(knockback.y)) return;
    if (!is_finite(mass_ratio)) return;
    float scale = PLAYER_ACCELERATION * 2;
    knockback *= scale * mass_ratio;
    ent.collision_velocity += knockback;
    ent.velocity += knockback * 2;
}

static void _cancel_movement(Entity &ent, Vector dir, Vector add) {
    Vector push = dir;
    push.normalize();
    float dot = fclamp(push.x * add.x + push.y * add.y, PLAYER_ACCELERATION * 0.5, PLAYER_ACCELERATION * 25);
    ent.velocity += push * (PLAYER_ACCELERATION + dot * 2);
    ent.collision_velocity += push * (0.5 * PLAYER_ACCELERATION);
}

void on_collide(Simulation *sim, Entity &ent1, Entity &ent2) {
    //do a distance dependent check first (it's faster)
    float min_dist = ent1.get_radius() + ent2.get_radius();
    if (fabs(ent1.get_x() - ent2.get_x()) > min_dist || fabs(ent1.get_y() - ent2.get_y()) > min_dist) return;
    //check if collide (distance independent)
    if (!_should_interact(ent1, ent2)) return;
    //finer distance check
    Vector separation(ent1.get_x() - ent2.get_x(), ent1.get_y() - ent2.get_y());
    float distance_squared = separation.x * separation.x + separation.y * separation.y;
    if (distance_squared > min_dist * min_dist) return;
    float distance = std::sqrt(distance_squared);
    float dist = min_dist - distance;
    if (NO(kDrop) && NO(kWeb)) {
        if (separation.x == 0 && separation.y == 0)
            separation.unit_normal(frand() * 2 * M_PI);
        else if (!is_finite(distance))
            separation.set(0.f, 0.f);
        else if (distance > 0.f)
            separation *= 1.f / distance;
        float ratio = ent2.mass / (ent1.mass + ent2.mass);
        // Players soft-block each other (overlap resolve + cancel into each
        // other) but never get attack knockback.
        uint8_t const both_players =
            is_player_faction(sim, ent1) && is_player_faction(sim, ent2);
        if (both_players) {
            if (ent1.has_component(kFlower) && ent2.has_component(kFlower)) {
                _cancel_movement(ent1, separation, ent2.velocity - ent1.velocity);
                _cancel_movement(ent2, separation*-1, ent1.velocity - ent2.velocity);
            }
        } else if (!(ent1.get_team() == ent2.get_team())) {
            if (ent1.has_component(kFlower) && !ent2.has_component(kPetal))
                _cancel_movement(ent1, separation, ent2.velocity - ent1.velocity);
            else
                _deal_knockback(ent1, separation, ratio);
            if (ent2.has_component(kFlower) && !ent1.has_component(kPetal))
                _cancel_movement(ent2, separation*-1, ent1.velocity - ent2.velocity);
            else
                _deal_knockback(ent2, separation*-1, 1 - ratio);
        }
        // Player soft-block: shallow overlaps still need a separation floor or
        // bodies slowly sink through each other (looks like pass-through).
        float push_dist = dist;
        if (both_players)
            push_dist = fmaxf(dist, min_dist * 0.08f);
        _deal_push(ent1, separation, ratio, push_dist);
        _deal_push(ent2, separation*-1, 1 - ratio, push_dist);
    }

    if (BOTH(kHealth) && !(ent1.get_team() == ent2.get_team())) {
        // No PvP between players / CPU bots / their petals & summons
        if (is_player_faction(sim, ent1) && is_player_faction(sim, ent2))
            return;
        // Iron Guard intercepts enemy detached / despawning projectiles
        auto try_iron_guard = [&](Entity &guard, Entity &proj) {
            if (!guard.has_component(kPetal) || !proj.has_component(kPetal)) return false;
            if (guard.get_petal_id() != PetalID::kIronGuard) return false;
            if (!(BitMath::at(proj.flags, EntityFlags::kIsDetached)
                  || BitMath::at(proj.flags, EntityFlags::kIsDespawning)))
                return false;
            PetalID::T pp = proj.get_petal_id();
            if (pp != PetalID::kMissile
                && !petal_name_is(pp, "Peas")
                && !petal_name_is(pp, "Glass Storm"))
                return false;
            inflict_damage(sim, proj.id, guard.id, proj.damage * 0.35f, DamageType::kContact);
            sim->request_delete(proj.id);
            if (guard.health <= 0) sim->request_delete(guard.id);
            return true;
        };
        if (try_iron_guard(ent1, ent2) || try_iron_guard(ent2, ent1))
            return;
        if (ent1.health > 0 && ent2.health > 0) {
            // Dummies keep damage==0 so flower bodies are safe in the plaza;
            // petals still take rarity-scaled return hits for durability practice.
            float d1 = ent1.damage;
            float d2 = ent2.damage;
            if (TrainingDummy::is_dummy(ent1) && ent2.has_component(kPetal))
                d1 = TrainingDummy::petal_contact_damage(ent1);
            if (TrainingDummy::is_dummy(ent2) && ent1.has_component(kPetal))
                d2 = TrainingDummy::petal_contact_damage(ent2);
            inflict_damage(sim, ent1.id, ent2.id, d1, DamageType::kContact);
            inflict_damage(sim, ent2.id, ent1.id, d2, DamageType::kContact);
        }
        if (ent1.health == 0) sim->request_delete(ent1.id);
        if (ent2.health == 0) sim->request_delete(ent2.id);
    }

    if (ent1.has_component(kDrop) && ent2.has_component(kFlower)) 
        pickup_drop(sim, ent2, ent1);
    if (ent2.has_component(kDrop) && ent1.has_component(kFlower))
        pickup_drop(sim, ent1, ent2);

    if (ent1.has_component(kWeb) && !ent2.has_component(kPetal) && !ent2.has_component(kDrop))
        ent2.speed_ratio = 0.5;
    if (ent2.has_component(kWeb) && !ent1.has_component(kPetal) && !ent1.has_component(kDrop))
        ent1.speed_ratio = 0.5;
}
