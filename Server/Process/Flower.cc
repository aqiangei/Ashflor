#include <Server/Process.hh>

#include <Server/Client.hh>
#include <Server/EntityFunctions.hh>
#include <Server/Server.hh>
#include <Server/Spawn.hh>
#include <Shared/Entity.hh>
#include <Shared/Map.hh>
#include <Shared/Simulation.hh>
#include <Shared/StaticData.hh>
#include <Shared/Talent.hh>

#include <Helpers/Bits.hh>

#include <cmath>

struct PlayerBuffs {
    float extra_rot = 0;
    float extra_range = 0;
    float heal = 0;
    float vision_factor = 1;
    float move_speed_factor = 1;
    float magnet_radius = 0;
    float extra_health = 0;
    float extra_damage = 0;
    float damage_factor = 1;
    float reload_factor = 1;
    uint8_t yinyang_count = 0;
    uint8_t is_poisonous = 0;
    uint8_t equip_flags = 0;
};

struct RotationCenter {
    float x;
    float y;
    float r;
};

static struct PlayerBuffs _get_petal_passive_buffs(Simulation *sim, Entity &player) {
    struct PlayerBuffs buffs = {0};
    if (player.has_component(kMob)) return buffs;
    player.set_equip_flags(0);
    player.damage_reflection = 0;
    player.poison_armor = 0;
    for (uint32_t i = 0; i < player.get_loadout_count(); ++i) {
        LoadoutSlot const &slot = player.loadout[i];
        PetalID::T slot_petal_id = slot.get_petal_id();
        struct PetalData const &petal_data = PETAL_DATA[slot_petal_id];
        struct PetalAttributes const &attrs = petal_data.attributes;
        if (attrs.equipment != EquipmentFlags::kNone)
            player.set_equip_flags(player.get_equip_flags() | (1 << attrs.equipment));
        buffs.vision_factor = std::min(buffs.vision_factor, attrs.vision_factor);
        buffs.move_speed_factor *= attrs.move_speed_factor;
        buffs.magnet_radius = std::fmax(buffs.magnet_radius, attrs.magnet_radius);
        buffs.extra_range = std::fmax(attrs.extra_range, buffs.extra_range);
        buffs.extra_damage = std::fmax(buffs.extra_damage, attrs.extra_body_damage);
        buffs.damage_factor *= attrs.extra_damage_factor;
        buffs.reload_factor *= attrs.extra_reload_factor;
        if (slot_petal_id == PetalID::kYinYang)
            ++buffs.yinyang_count;
        if (!player.loadout[i].already_spawned) continue;
        if (slot_petal_id == PetalID::kLeaf) 
            buffs.heal += attrs.constant_heal / TPS;
        else if (slot_petal_id == PetalID::kYucca && BitMath::at(player.input, InputFlags::kDefending) && !BitMath::at(player.input, InputFlags::kAttacking)) 
            buffs.heal += attrs.constant_heal / TPS;
        buffs.extra_rot += attrs.extra_rotation_speed;
        buffs.extra_health += attrs.extra_health;
        player.damage_reflection = std::fmax(player.damage_reflection, attrs.damage_reflection);
        player.poison_armor = std::fmax(player.poison_armor, attrs.poison_armor / TPS);
        if (slot_petal_id == PetalID::kPoisonCactus)
            buffs.is_poisonous = 1;
    }
    return buffs;
}

static uint32_t _get_petal_rotation_count(Simulation *sim, Entity &player) {
    uint32_t count = 0;
    for (uint8_t i = 0; i < player.get_loadout_count(); ++i) {
        LoadoutSlot const &slot = player.loadout[i];
        struct PetalData const &petal_data = PETAL_DATA[slot.get_petal_id()];
        if (petal_data.attributes.clump_radius > 0)
            ++count;
        else {
            for (uint32_t j = 0; j < slot.size(); ++j) {
                if (!sim->ent_alive(slot.petals[j].ent_id))
                    ++count;
                else if (!BitMath::at(sim->get_ent(slot.petals[j].ent_id).flags, EntityFlags::kIsDetached))
                    ++count;
            }
        }
    }
    return count;
}

static RotationCenter const _get_petal_rotation_center(Simulation *sim, Entity const &player) {
    RotationCenter rotation_center = { 
        .x = player.get_x(), 
        .y = player.get_y(), 
        .r = player.get_radius() 
    };
    for (uint32_t i = 0; i < player.get_loadout_count(); ++i) {
        LoadoutSlot const &slot = player.loadout[i];
        if (slot.get_petal_id() != PetalID::kMoon) continue;
        for (uint32_t j = 0; j < slot.size(); ++j) {
            LoadoutPetal const &petal_slot = slot.petals[j];
            if (!sim->ent_alive(petal_slot.ent_id)) continue;
            Entity &moon = sim->get_ent(petal_slot.ent_id);
            rotation_center.x = moon.get_x();
            rotation_center.y = moon.get_y();
            rotation_center.r = moon.get_radius();
            return rotation_center;
        }
    }
    return rotation_center;
}

void tick_player_behavior(Simulation *sim, Entity &player) {
    if (player.pending_delete) return;
    DEBUG_ONLY(assert(player.max_health > 0);)
    // Fast channels: walkable but no attacking (leaf/sand/coral corridors)
    if (Map::in_no_attack_zone(sim->map_id, player.get_x(), player.get_y()))
        BitMath::unset(player.input, InputFlags::kAttacking);
    PlayerBuffs const buffs = _get_petal_passive_buffs(sim, player);
    float talent_hp = 0;
    float talent_rot = 0;
    float rot_mult = Talent::rot_setting_mult(Talent::kRotSettingDefault);
    if (!player.has_component(kMob)) {
        if (Client *owner = Server::client_by_camera(sim, player.get_parent())) {
            talent_hp = Talent::health_bonus(owner->talent_hp);
            talent_rot = Talent::rot_bonus(owner->talent_rot);
            rot_mult = Talent::rot_setting_mult(owner->rot_speed_setting);
        }
    }
    float health_ratio = player.health / player.max_health;
    if (!player.has_component(kMob)) {
        player.max_health = hp_at_level(score_to_level(player.get_score())) + buffs.extra_health + talent_hp;
        player.damage = BASE_BODY_DAMAGE + buffs.extra_damage;
    }
    player.health = health_ratio * player.max_health;
    if (buffs.heal > 0)
        inflict_heal(sim, player, buffs.heal);
    if (buffs.is_poisonous)
        player.poison_damage = {10.0, 2};
    else
        player.poison_damage = {0, 0};
    
    float rot_pos = 0;
    uint32_t rotation_count = _get_petal_rotation_count(sim, player);
    RotationCenter const rotation_center = _get_petal_rotation_center(sim, player);
    //maybe use delta mode for face flags?
    player.set_face_flags(0);

    if (sim->ent_alive(player.get_parent())) {
        Entity &camera = sim->get_ent(player.get_parent());
        camera.set_fov(BASE_FOV * buffs.vision_factor);
    }
    if (buffs.move_speed_factor != 1.f &&
        (player.acceleration.x != 0.f || player.acceleration.y != 0.f))
        player.acceleration *= buffs.move_speed_factor;
    if (buffs.magnet_radius > 0.f) {
        float r2 = buffs.magnet_radius * buffs.magnet_radius;
        float pickup_r = player.get_radius() + 25.f + 8.f;
        float pickup_r2 = pickup_r * pickup_r;
        sim->for_each<kDrop>([&](Simulation *, Entity &drop) {
            if (drop.pending_delete) return;
            if (drop.immunity_ticks > 0 && !(drop.get_team() == player.get_team())) return;
            float dx = player.get_x() - drop.get_x();
            float dy = player.get_y() - drop.get_y();
            float d2 = dx * dx + dy * dy;
            if (d2 > r2) return;
            // Already overlapping flower — pick up immediately (avoids magnet orbit)
            if (d2 <= pickup_r2) {
                drop.velocity.set(0, 0);
                drop.acceleration.set(0, 0);
                drop.collision_velocity.set(0, 0);
                on_collide(sim, player, drop);
                return;
            }
            if (d2 <= 1.f) return;
            float dist = std::sqrt(d2);
            // Pull harder at range, ease off near the flower so drops don't slingshot
            float t = dist / buffs.magnet_radius;
            float pull = PLAYER_ACCELERATION * (3.f + 5.f * t);
            drop.acceleration.set(dx / dist * pull, dy / dist * pull);
            // Kill excess tangential speed so loot doesn't orbit
            float vx = drop.velocity.x;
            float vy = drop.velocity.y;
            float radial = (vx * dx + vy * dy) / dist;
            float tx = vx - dx / dist * radial;
            float ty = vy - dy / dist * radial;
            drop.velocity.x -= tx * 0.35f;
            drop.velocity.y -= ty * 0.35f;
        });
    }

    DEBUG_ONLY(assert(player.get_loadout_count() <= MAX_SLOT_COUNT);)
    for (uint32_t i = 0; i < player.get_loadout_count(); ++i) {
        LoadoutSlot &slot = player.loadout[i];
        //player.set_loadout_ids(i, slot.id);
        //other way around. loadout_ids should dictate loadout
        if (slot.get_petal_id() != player.get_loadout_ids(i) || player.get_overlevel_timer() >= PETAL_DISABLE_DELAY * TPS)
            slot.update_id(sim, player.get_loadout_ids(i));
        PetalID::T slot_petal_id = slot.get_petal_id();
        struct PetalData const &petal_data = PETAL_DATA[slot_petal_id];
        DEBUG_ONLY(assert(petal_data.count <= MAX_PETALS_IN_CLUMP);)

        if (slot_petal_id == PetalID::kNone || petal_data.count == 0)
            continue;
        //if overleveled timer too large
        if (player.get_overlevel_timer() >= PETAL_DISABLE_DELAY * TPS) {
            player.set_loadout_reloads(i, 0);
            continue;
        }
        float min_reload = 1;
        for (uint32_t j = 0; j < slot.size(); ++j) {
            LoadoutPetal &petal_slot = slot.petals[j];
            if (!sim->ent_alive(petal_slot.ent_id)) {
                petal_slot.ent_id = NULL_ENTITY;
                game_tick_t reload_time = (petal_data.reload * TPS) * buffs.reload_factor;
                if (!slot.already_spawned) reload_time += TPS;
                float this_reload = reload_time == 0 ? 1 : (float) petal_slot.reload / reload_time;
                min_reload = std::min(min_reload, this_reload);
                if (petal_slot.reload >= reload_time) {
                    petal_slot.ent_id = alloc_petal(sim, slot_petal_id, player).id;
                    sim->get_ent(petal_slot.ent_id).damage *= buffs.damage_factor;
                    sim->get_ent(petal_slot.ent_id).set_x(rotation_center.x);
                    sim->get_ent(petal_slot.ent_id).set_y(rotation_center.y);
                    petal_slot.reload = 0;
                    slot.already_spawned = 1;
                } 
                else
                    ++petal_slot.reload;
            }
            if (sim->ent_alive(petal_slot.ent_id)) {
                Entity &petal = sim->get_ent(petal_slot.ent_id);
                //only do this if petal not despawning
                if (petal.has_component(kPetal) && 
                    !BitMath::at(petal.flags, EntityFlags::kIsDespawning) &&
                    !BitMath::at(petal.flags, EntityFlags::kIsDetached)) {
                    //petal rotation behavior
                    Vector wanting;
                    Vector delta(rotation_center.x - petal.get_x(), rotation_center.y - petal.get_y());
                    if (rotation_count > 0)
                        wanting.unit_normal(2 * M_PI * rot_pos / rotation_count + player.heading_angle);

                    float range = rotation_center.r + 40;
                    if (BitMath::at(player.input, InputFlags::kAttacking)) { 
                        if (petal_data.attributes.defend_only == 0) 
                            range = rotation_center.r + 100 + buffs.extra_range; 
                        if (petal.get_petal_id() == PetalID::kWing) {
                            float wave = sinf((float) petal.lifetime / (0.4 * TPS));
                            wave = wave * wave;
                            range += wave * 120;
                        }
                    }
                    else if (BitMath::at(player.input, InputFlags::kDefending)) 
                        range = rotation_center.r + 15;
                    wanting *= range;
                    if (petal_data.attributes.clump_radius > 0) {
                        Vector secondary;
                        secondary.unit_normal(2 * M_PI * j / petal_data.count + player.heading_angle * 0.2)
                        .set_magnitude(petal_data.attributes.clump_radius);
                        wanting += secondary;
                    }
                    wanting += delta;
                    wanting *= 0.5f;
                    petal.acceleration = wanting;
                    game_tick_t sec_reload_ticks = petal_data.attributes.secondary_reload * TPS;
                    if (petal_data.attributes.spawns != MobID::kNumMobs &&
                        petal.secondary_reload > sec_reload_ticks) {
                        uint8_t spawn_id = petal_data.attributes.spawns;
                        Entity &mob = alloc_mob(sim, spawn_id, petal.get_x(), petal.get_y(), petal.get_team(), [&](Entity &mob){
                            mob.set_parent(player.id);
                            mob.set_color(player.get_color());
                            mob.base_entity = player.id;
                            BitMath::set(mob.flags, EntityFlags::kDieOnParentDeath);
                            BitMath::set(mob.flags, EntityFlags::kNoDrops);
                            BitMath::set(mob.flags, EntityFlags::kIsDetached);
                        });
                    
                        if (petal_data.attributes.spawn_count == 0) {
                            petal_slot.ent_id = mob.id;
                            sim->request_delete(petal.id);
                            break;
                        } else {
                            entity_set_despawn_tick(mob, sec_reload_ticks * petal_data.attributes.spawn_count);
                            petal.secondary_reload = 0;
                            //needed
                            mob.set_parent(petal.id);
                            mob.base_entity = player.id;
                        }
                    }
                } else {
                    //if petal is a mob, or detached (IsDespawning)
                    if (BitMath::at(petal.flags, EntityFlags::kIsDespawning))
                        petal_slot.ent_id = NULL_ENTITY;
                    if (BitMath::at(petal.flags, EntityFlags::kIsDetached))
                        --rot_pos;
                }
            }
            //spread out
            if (petal_data.attributes.clump_radius == 0) ++rot_pos;
        }
        //clump
        if (petal_data.attributes.clump_radius > 0) ++rot_pos;
        player.set_loadout_reloads(i, min_reload * 255);
    };
    if (BitMath::at(player.input, InputFlags::kAttacking)) 
        player.set_face_flags(player.get_face_flags() | (1 << FaceFlags::kAttacking));
    else if (BitMath::at(player.input, InputFlags::kDefending))
        player.set_face_flags(player.get_face_flags() | (1 << FaceFlags::kDefending));
    if (player.poison_ticks > 0)
        player.set_face_flags(player.get_face_flags() | (1 << FaceFlags::kPoisoned));
    if (player.dandy_ticks > 0)
        player.set_face_flags(player.get_face_flags() | (1 << FaceFlags::kDandelioned));
    float rot_speed = (BASE_PETAL_ROTATION_SPEED + buffs.extra_rot + talent_rot) * rot_mult;
    if (buffs.yinyang_count != MAX_SLOT_COUNT) {
        switch (buffs.yinyang_count % 3) {
            case 0:
                player.heading_angle += rot_speed / TPS;
                break;
            case 1:
                player.heading_angle -= rot_speed / TPS;
                break;
            default:
                break;
        }
    } else
        player.heading_angle += 10 * rot_speed / TPS;
}