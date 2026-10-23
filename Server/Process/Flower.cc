#include <Server/Process.hh>

#include <Server/AccountStore.hh>
#include <Server/Client.hh>
#include <Server/EntityFunctions.hh>
#include <Server/Party.hh>
#include <Server/Server.hh>
#include <Server/Spawn.hh>
#include <Shared/Craft.hh>
#include <Shared/Entity.hh>
#include <Shared/Map.hh>
#include <Shared/Simulation.hh>
#include <Shared/StaticData.hh>
#include <Shared/Talent.hh>

#include <Helpers/Bits.hh>

#include <array>
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

static float _owner_level_mult(std::array<uint32_t, PetalID::kNumPetals> const *levels,
                               PetalID::T id) {
    if (levels == nullptr || id == PetalID::kNone || id >= PetalID::kNumPetals)
        return 1.f;
    return Craft::level_stat_mult((*levels)[id]);
}

static struct PlayerBuffs _get_petal_passive_buffs(
    Simulation *sim, Entity &player,
    std::array<uint32_t, PetalID::kNumPetals> const *petal_levels) {
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
        float lv_mult = _owner_level_mult(petal_levels, slot_petal_id);
        if (attrs.equipment != EquipmentFlags::kNone)
            player.set_equip_flags(player.get_equip_flags() | (1 << attrs.equipment));
        buffs.vision_factor = std::min(buffs.vision_factor, attrs.vision_factor);
        // Speed boosts: strongest single factor wins. Multiplying several
        // Accelerators (1.3^n) turned map travel into an accidental rocket.
        if (attrs.move_speed_factor > 1.f)
            buffs.move_speed_factor = std::fmax(buffs.move_speed_factor, attrs.move_speed_factor);
        else if (attrs.move_speed_factor > 0.f && attrs.move_speed_factor < 1.f)
            buffs.move_speed_factor *= attrs.move_speed_factor;
        buffs.magnet_radius = std::fmax(buffs.magnet_radius, attrs.magnet_radius);
        buffs.extra_range = std::fmax(attrs.extra_range, buffs.extra_range);
        buffs.extra_damage = std::fmax(buffs.extra_damage, attrs.extra_body_damage * lv_mult);
        buffs.damage_factor *= attrs.extra_damage_factor;
        buffs.reload_factor *= attrs.extra_reload_factor;
        if (slot_petal_id == PetalID::kYinYang)
            ++buffs.yinyang_count;
        if (!player.loadout[i].already_spawned) continue;
        // constant_heal is data-driven (HP/s). Yucca keeps classic defend-only gating;
        // all other petals with the attr heal while the slot is spawned.
        if (attrs.constant_heal > 0.f) {
            if (slot_petal_id == PetalID::kYucca) {
                if (BitMath::at(player.input, InputFlags::kDefending)
                    && !BitMath::at(player.input, InputFlags::kAttacking))
                    buffs.heal += attrs.constant_heal * lv_mult / TPS;
            } else {
                buffs.heal += attrs.constant_heal * lv_mult / TPS;
            }
        }
        buffs.extra_rot += attrs.extra_rotation_speed;
        buffs.extra_health += attrs.extra_health * lv_mult;
        player.damage_reflection = std::fmax(player.damage_reflection, attrs.damage_reflection);
        player.poison_armor = std::fmax(player.poison_armor, attrs.poison_armor * lv_mult / TPS);
        if (slot_petal_id == PetalID::kPoisonCactus)
            buffs.is_poisonous = 1;
        // King's Oath: low-HP damage surge (on top of petal extra_damage_factor)
        if (slot_petal_id == PetalID::kKingsOath
            && player.max_health > 0.f
            && player.health / player.max_health < 0.35f)
            buffs.damage_factor *= 1.35f;
        // Royal Banner / Phantom Eye spin & damage come only from PETAL_DATA attrs
        // (no extra hardcoded stacks — keeps tooltip values honest).
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
    float talent_hp = 0;
    float talent_rot = 0;
    float talent_reload = 1.f;
    float talent_petal_hp = 1.f;
    float rot_mult = Talent::rot_setting_mult(Talent::kRotSettingDefault);
    std::array<uint32_t, PetalID::kNumPetals> const *petal_levels = nullptr;
    if (!player.has_component(kMob)) {
        if (Client *owner = Server::client_by_camera(sim, player.get_parent())) {
            petal_levels = &owner->petal_levels;
            uint8_t const *r = owner->talent_ranks;
            talent_hp = Talent::health_bonus(r[TalentID::kHealth]);
            talent_rot = Talent::rot_bonus(r[TalentID::kRotSpeed]);
            talent_reload = Talent::reload_mult(r[TalentID::kCooldown]);
            talent_petal_hp = Talent::petal_health_mult(r[TalentID::kPetalArmor]);
            rot_mult = Talent::rot_setting_mult(owner->rot_speed_setting);
        }
    }
    PlayerBuffs buffs = _get_petal_passive_buffs(sim, player, petal_levels);
    if (!player.has_component(kMob)) {
        if (Client *owner = Server::client_by_camera(sim, player.get_parent())) {
            uint8_t const *r = owner->talent_ranks;
            buffs.extra_range += Talent::range_bonus(r[TalentID::kAttackRange]);
            buffs.magnet_radius += Talent::magnet_bonus(r[TalentID::kMagnet]);
        }
    }
    buffs.reload_factor *= talent_reload;
    float health_ratio = player.health / player.max_health;
    if (!player.has_component(kMob)) {
        uint32_t const sc = player.get_score();
        uint32_t level = score_to_level(sc);
        player.max_health = hp_at_level(level) + buffs.extra_health + talent_hp;
        player.damage = body_damage_at_level(level) + buffs.extra_damage;
        // Keep account/pending XP high-water every tick so refresh cannot miss
        // mid-level progress if disconnect capture races the flower teardown.
        if (Client *owner = Server::client_by_camera(sim, player.get_parent()))
            AccountStore::note_score(owner, sc);
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
    // Proximity pickup always (spatial-hash collide can miss edge cases).
    // Magnet optionally pulls from farther away. Query local cells only —
    // same pull/pickup math as the former full-drop scan (gardn-style AOI).
    {
        float pickup_r = player.get_radius() + 30.f + 12.f;
        float pickup_r2 = pickup_r * pickup_r;
        float magnet_r = buffs.magnet_radius;
        float magnet_r2 = magnet_r > 0.f ? magnet_r * magnet_r : 0.f;
        float query_r = magnet_r > pickup_r ? magnet_r : pickup_r;
        Client *owner = Server::client_by_camera(sim, player.get_parent());
        sim->spatial_hash.query(
            player.get_x(), player.get_y(), query_r, query_r,
            [&](Simulation *, Entity &drop) {
            if (!drop.has_component(kDrop) || drop.pending_delete) return;
            // Team piles: owner/party only. Unowned: brief immunity then FFA.
            if (drop.get_team() != NULL_ENTITY) {
                if (!(drop.get_team() == player.get_team())
                    && !Party::can_take_drop(owner, sim, drop.get_team()))
                    return;
            } else if (drop.immunity_ticks > 0) {
                return;
            }
            float dx = player.get_x() - drop.get_x();
            float dy = player.get_y() - drop.get_y();
            float d2 = dx * dx + dy * dy;
            if (d2 <= pickup_r2) {
                drop.velocity.set(0, 0);
                drop.acceleration.set(0, 0);
                drop.collision_velocity.set(0, 0);
                on_collide(sim, player, drop);
                return;
            }
            if (magnet_r2 <= 0.f || d2 > magnet_r2) return;
            if (d2 <= 1.f) return;
            float dist = std::sqrt(d2);
            float t = dist / magnet_r;
            float pull = PLAYER_ACCELERATION * (3.f + 5.f * t);
            drop.acceleration.set(dx / dist * pull, dy / dist * pull);
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
        if (slot.get_petal_id() != player.get_loadout_ids(i))
            slot.update_id(sim, player.get_loadout_ids(i));
        PetalID::T slot_petal_id = slot.get_petal_id();
        struct PetalData const &petal_data = PETAL_DATA[slot_petal_id];
        DEBUG_ONLY(assert(petal_data.count <= MAX_PETALS_IN_CLUMP);)

        if (slot_petal_id == PetalID::kNone || petal_data.count == 0)
            continue;
        float min_reload = 1;
        float health_sum = 0.f;
        uint32_t health_n = 0;
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
                    Entity &spawned = sim->get_ent(petal_slot.ent_id);
                    spawned.damage *= buffs.damage_factor;
                    float hp_mult = talent_petal_hp;
                    // Infinite craft level (Unique): scale contact + DoT + armor.
                    float lv_mult = _owner_level_mult(petal_levels, slot_petal_id);
                    if (lv_mult != 1.f) {
                        spawned.damage *= lv_mult;
                        spawned.poison_damage.damage *= lv_mult;
                        spawned.burn_damage.damage *= lv_mult;
                        spawned.armor *= lv_mult;
                        hp_mult *= lv_mult;
                    }
                    if (hp_mult != 1.f) {
                        spawned.max_health *= hp_mult;
                        spawned.health = spawned.max_health;
                    }
                    spawned.set_x(rotation_center.x);
                    spawned.set_y(rotation_center.y);
                    petal_slot.reload = 0;
                    slot.already_spawned = 1;
                } 
                else
                    ++petal_slot.reload;
            }
            if (sim->ent_alive(petal_slot.ent_id)) {
                Entity &petal = sim->get_ent(petal_slot.ent_id);
                if (petal.max_health > 0.f) {
                    float hr = petal.health / petal.max_health;
                    if (hr < 0.f) hr = 0.f;
                    if (hr > 1.f) hr = 1.f;
                    health_sum += hr;
                    ++health_n;
                }
                //only do this if petal not despawning
                if (petal.has_component(kPetal) && 
                    !BitMath::at(petal.flags, EntityFlags::kIsDespawning) &&
                    !BitMath::at(petal.flags, EntityFlags::kIsDetached)) {
                    //petal rotation behavior
                    Vector wanting;
                    Vector delta(rotation_center.x - petal.get_x(), rotation_center.y - petal.get_y());
                    if (rotation_count > 0)
                        wanting.unit_normal(2 * M_PI * rot_pos / rotation_count + player.heading_angle);

                    float range = rotation_center.r + PETAL_ORBIT_IDLE;
                    if (BitMath::at(player.input, InputFlags::kAttacking)) { 
                        if (petal_data.attributes.defend_only == 0) 
                            range = rotation_center.r + PETAL_ORBIT_ATTACK + buffs.extra_range; 
                        if (petal_name_is(petal.get_petal_id(), "Wing")) {
                            float wave = sinf((float) petal.lifetime / (0.4 * TPS));
                            wave = wave * wave;
                            range += wave * PETAL_WING_WAVE_EXTRA;
                        }
                    }
                    else if (BitMath::at(player.input, InputFlags::kDefending)) 
                        range = rotation_center.r + PETAL_ORBIT_DEFEND;
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
        // Alive petals only — while reloading (none out) keep full so CD pie owns the cue.
        float slot_hp = health_n > 0 ? health_sum / (float) health_n : 1.f;
        if (slot_hp < 0.f) slot_hp = 0.f;
        if (slot_hp > 1.f) slot_hp = 1.f;
        player.set_loadout_healths(i, (uint8_t) (slot_hp * 255.f + 0.5f));
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
