#include <Shared/Simulation.hh>

#include <Client/Game.hh>
#include <Client/Particle.hh>

#include <Client/Ui/Extern.hh>

#include <Shared/RarityScale.hh>
#include <Shared/StaticData.hh>

#include <cmath>
#include <iostream>

namespace {
float estimate_max_hp(Entity const &ent) {
    if (ent.has_component(kMob)) {
        MobID::T mid = ent.get_mob_id();
        if (mid >= MobID::kNumMobs) return 100.f;
        float base = MOB_DATA[mid].health.get_single(0.5f);
        uint8_t br = MOB_DATA[mid].rarity;
        uint8_t sr = ent.get_rarity();
        if (sr >= RarityID::kNumRarities) sr = br;
        float hp = base * mob_spawn_threat_mult(br, sr);
        if (hp < 1.f) hp = 1.f;
        return hp;
    }
    if (ent.has_component(kFlower)) {
        uint32_t lv = 1;
        if (ent.has_component(kScore))
            lv = score_to_level(ent.get_score());
        float hp = hp_at_level(lv);
        if (hp < 1.f) hp = 1.f;
        return hp;
    }
    return 100.f;
}
} // namespace

void Entity::tick_lerp(float amt) {
    // rrolf-style: snappier local flower + camera (less input lag); keep the
    // global curve for everyone else so remote entities don't jitter.
    float phys_amt = amt;
    float cam_amt = amt;
    if (Game::alive()) {
        if (id == Game::player_id && has_component(kFlower) && !has_component(kMob))
            phys_amt = 1.f - powf(1.f - 0.62f, Ui::dt * 60.f / 1000.f);
        if (id == Game::camera_id)
            cam_amt = 1.f - powf(1.f - 0.62f, Ui::dt * 60.f / 1000.f);
    }
    if (has_component(kPhysics)) {
        float prev_x = x;
        float prev_y = y;
        if (!pending_delete) {
            x.step(phys_amt);
            y.step(phys_amt);
        }
        if (has_component(kDrop) || has_component(kWeb)) {
            if (lifetime < TPS)
                animation = lerp(animation, 1, amt * 0.75);
            else animation = 1;
        } else {
            Vector vel(x - prev_x, y - prev_y);
            animation += (1 + 0.75 * vel.magnitude()) * 0.075;
        }
        radius.step(amt);
        angle.step_angle(phys_amt);
        if (pending_delete)
            deletion_animation = fclamp(deletion_animation + Ui::dt / 150, 0, 1);
    }
    if (has_component(kCamera)) {
        camera_x.step(cam_amt);
        camera_y.step(cam_amt);
        fov.step(cam_amt);
        Game::respawn_level = respawn_level;
    }
    if (has_component(kHealth)) {
        float const target = health_ratio.anchor();
        if (!health_ratio_seen) {
            prev_health_ratio = target;
            health_ratio_seen = 1;
        } else {
            float const delta = target - prev_health_ratio;
            // Mob damage pops (petal / contact / poison — any HP drop with damaged flag).
            if (damaged && delta < -0.0004f
                && has_component(kMob) && !has_component(kPetal)
                && !pending_delete) {
                float amt = -delta * estimate_max_hp(*this);
                // Cap Y offset so huge Unique+ radii do not bury floats inside the sprite
                // or push them off-screen above the mob.
                float rad = (float) get_radius();
                float y_off = rad * 0.35f + 18.f;
                if (y_off > 72.f) y_off = 72.f;
                Particle::add_damage_number(
                    id, (float) get_x(), (float) get_y() - y_off, amt);
            }
            // Local player heal pops (pink).
            if (delta > 0.0004f
                && has_component(kFlower) && !has_component(kMob)
                && id == Game::player_id && !pending_delete) {
                float amt = delta * estimate_max_hp(*this);
                Particle::add_heal_number(
                    (float) get_x(), (float) get_y() - (float) get_radius() * 0.85f, amt);
            }
            if (std::fabs(delta) > 1e-6f)
                prev_health_ratio = target;
        }
        if (pending_delete)
            Particle::clear_combat_accum(id);

        health_ratio.step(amt);
        if (damaged == 1 && damage_flash < 0.1 && !pending_delete)
            damage_flash = 1;
        else
            damage_flash = lerp(damage_flash, 0, amt);
        if (damaged)
            last_damaged_time = Game::timestamp;
        damaged.clear();
        if ((float) health_ratio > 0.999)
            healthbar_opacity = lerp(healthbar_opacity, 0, amt);
        else
            healthbar_opacity = 1;
        if (healthbar_lag < health_ratio)
            healthbar_lag = health_ratio;
        else if (Game::timestamp - last_damaged_time > 250)
            healthbar_lag = lerp(healthbar_lag, health_ratio, amt / 3);
    }
    if (has_component(kFlower)) {
        float eye_angle = angle.anchor();
        eye_x = lerp(eye_x, cosf(eye_angle) * 2, amt);
        eye_y = lerp(eye_y, sinf(eye_angle) * 4, amt);
        if (BitMath::at(face_flags, FaceFlags::kAttacking)
            || BitMath::at(face_flags, FaceFlags::kPoisoned) 
            || BitMath::at(face_flags, FaceFlags::kDandelioned)
            || pending_delete) mouth = lerp(mouth, 5, amt);
        else if (BitMath::at(face_flags, FaceFlags::kDefending)) mouth = lerp(mouth, 8, amt);
        else mouth = lerp(mouth, 15, amt);
    }
}

void Simulation::on_tick() {
    for_each_entity([](Simulation *sim, Entity &ent) {
        ent.tick_lerp(Ui::lerp_amount);
    });

    // Leaderboard is persisted top scores, not "online only". Zeroing past
    // player_count wiped historical entries whenever few people were in-map.
    for (uint32_t i = 0; i < LEADERBOARD_SIZE; ++i) {
        if (arena_info.names[i].empty())
            arena_info.scores[i] = 0;
        else
            arena_info.scores[i].step(Ui::lerp_amount);
    }
}

void Simulation::post_tick() {
    for_each_entity([](Simulation *sim, Entity &ent) {
        ent.reset_protocol();
    });
}