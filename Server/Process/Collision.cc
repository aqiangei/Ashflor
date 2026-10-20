#include <Server/EntityFunctions.hh>
#include <Server/AccountStore.hh>
#include <Server/Client.hh>
#include <Server/Game.hh>
#include <Server/Server.hh>

#include <Shared/Simulation.hh>
#include <Shared/Entity.hh>
#include <Shared/Recipe.hh>
#include <Shared/StaticData.hh>
#include <Shared/Binary.hh>

#include <Helpers/Math.hh>

#include <cmath>
#include <iostream>

static bool _should_interact(Entity const &ent1, Entity const &ent2) {
    //if (ent1.has_component(kFlower) || ent2.has_component(kFlower)) return false;
    //if (ent1.has_component(kPetal) || ent2.has_component(kPetal)) return false;
    if (ent1.pending_delete || ent2.pending_delete) return false;
    // Drops must collide with flowers so pickup can run (team/immunity checked in _pickup_drop)
    if ((ent1.has_component(kDrop) && ent2.has_component(kFlower))
        || (ent2.has_component(kDrop) && ent1.has_component(kFlower)))
        return true;
    if (!(ent1.get_team() == ent2.get_team())) return true;
    if (BitMath::at((ent1.flags | ent2.flags), EntityFlags::kNoFriendlyCollision)) return false;
    //if (ent1.has_component(kPetal) || ent2.has_component(kPetal)) return false;
    if (ent1.has_component(kMob) && ent2.has_component(kMob)) return true;
    return false;
}

// Overflow into backpack stacks when inventory slots are full.
static void _add_to_backpack_stack(Client *client, PetalID::T id) {
    if (client == nullptr || id == PetalID::kNone || id >= PetalID::kNumPetals) return;
    if (client->craft_bank[id] < 1000000)
        ++client->craft_bank[id];
    AccountStore::capture_nosave(client);
    client_sync_craft(client);
}

static void _pickup_drop(Simulation *sim, Entity &player, Entity &drop) {
    if (!sim->ent_alive(player.get_parent())) return;
    // Private loot: team-tagged drops are forever exclusive to that solo/party.
    // Unowned drops (NULL team) are locked briefly, then free-for-all.
    if (drop.get_team() != NULL_ENTITY) {
        if (!(drop.get_team() == player.get_team())) return;
    } else if (drop.immunity_ticks > 0) {
        return;
    }

    PetalID::T drop_id = drop.get_drop_id();
    uint16_t qty = drop.get_drop_count();
    if (qty < 1) qty = 1;
    Client *client = Server::client_by_camera(sim, player.get_parent());
    auto place_one = [&](uint32_t begin, uint32_t end) -> bool {
        for (uint32_t i = begin; i < end; ++i) {
            if (player.get_loadout_ids(i) != PetalID::kNone) continue;
            player.set_loadout_ids(i, drop_id);
            return true;
        }
        return false;
    };
    uint32_t const n = player.get_loadout_count();
    for (uint16_t q = 0; q < qty; ++q) {
        uint8_t placed = 0;
        if (Recipe::is_bag_only(drop_id)) {
            placed = place_one(n, n + MAX_SLOT_COUNT);
        } else {
            placed = place_one(n, n + MAX_SLOT_COUNT) || place_one(0, n);
        }
        if (!placed) {
            if (client != nullptr) {
                if (client->craft_bank[drop_id] < 1000000 - (qty - q))
                    client->craft_bank[drop_id] += (uint32_t) (qty - q);
                else
                    client->craft_bank[drop_id] = 1000000;
                for (uint16_t c = q; c < qty; ++c)
                    client_credit_petal_pick(client, drop_id);
            }
            break;
        }
        if (client != nullptr) client_credit_petal_pick(client, drop_id);
    }
    if (client != nullptr) {
        AccountStore::capture_nosave(client);
        if (Recipe::is_bag_only(drop_id))
            client_sync_craft(client);
    }
    drop.set_x(player.get_x());
    drop.set_y(player.get_y());
    BitMath::unset(drop.flags, EntityFlags::kIsDespawning);
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
    float dist = min_dist - separation.magnitude();
    if (dist < 0) return;
    if (NO(kDrop) && NO(kWeb)) {
        if (separation.x == 0 && separation.y == 0)
            separation.unit_normal(frand() * 2 * M_PI);
        else
            separation.normalize();
        float ratio = ent2.mass / (ent1.mass + ent2.mass);
        if (!(ent1.get_team() == ent2.get_team())) {
            if (ent1.has_component(kFlower) && !ent2.has_component(kPetal))
                _cancel_movement(ent1, separation, ent2.velocity - ent1.velocity);
            else
                _deal_knockback(ent1, separation, ratio);
            if (ent2.has_component(kFlower) && !ent1.has_component(kPetal))
                _cancel_movement(ent2, separation*-1, ent1.velocity - ent2.velocity);
            else
                _deal_knockback(ent2, separation*-1, 1 - ratio);
        }
        _deal_push(ent1, separation, ratio, dist);
        _deal_push(ent2, separation*-1, 1 - ratio, dist);
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
            if (proj.get_petal_id() != PetalID::kMissile
                && proj.get_petal_id() != PetalID::kPeas
                && proj.get_petal_id() != PetalID::kPoisonPeas)
                return false;
            inflict_damage(sim, proj.id, guard.id, proj.damage * 0.35f, DamageType::kContact);
            sim->request_delete(proj.id);
            if (guard.health <= 0) sim->request_delete(guard.id);
            return true;
        };
        if (try_iron_guard(ent1, ent2) || try_iron_guard(ent2, ent1))
            return;
        if (ent1.health > 0 && ent2.health > 0) {
            inflict_damage(sim, ent1.id, ent2.id, ent1.damage, DamageType::kContact);
            inflict_damage(sim, ent2.id, ent1.id, ent2.damage, DamageType::kContact);
        }
        if (ent1.health == 0) sim->request_delete(ent1.id);
        if (ent2.health == 0) sim->request_delete(ent2.id);
    }

    if (ent1.has_component(kDrop) && ent2.has_component(kFlower)) 
        _pickup_drop(sim, ent2, ent1);
    if (ent2.has_component(kDrop) && ent1.has_component(kFlower))
        _pickup_drop(sim, ent1, ent2);

    if (ent1.has_component(kWeb) && !ent2.has_component(kPetal) && !ent2.has_component(kDrop))
        ent2.speed_ratio = 0.5;
    if (ent2.has_component(kWeb) && !ent1.has_component(kPetal) && !ent1.has_component(kDrop))
        ent1.speed_ratio = 0.5;
}
