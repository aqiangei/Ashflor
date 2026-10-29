#include <Shared/Simulation.hh>

#include <Client/Game.hh>
#include <Client/Particle.hh>

#include <Client/Ui/Extern.hh>

#include <Shared/RarityScale.hh>
#include <Shared/StaticData.hh>

#include <cmath>
#include <algorithm>
#include <iostream>

namespace {
double motion_last_snapshot = 0, motion_lateness = 0;
double motion_delay = 75, motion_delay_frame = -1;
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

void Entity::record_motion_sample(double now) {
    // Short mob-only playback history absorbs snapshot jitter. Player/camera
    // stay on their responsive path; no speculative movement through walls.
    if (!has_component(kMob) || has_component(kFlower) || !has_component(kPhysics)) return;
    if (now != motion_last_snapshot) {
        double gap = now-motion_last_snapshot;
        if (!motion_last_snapshot || gap>500) motion_lateness=0;
        else motion_lateness=std::max(motion_lateness*.99,std::max(0.0,gap-50.0));
        motion_last_snapshot=now;
    }
    float const nx = x.anchor(), ny = y.anchor();
    if (motion_sample_count) {
        auto const &last = motion_samples[motion_sample_count - 1];
        float dx = nx-last.x, dy = ny-last.y;
        if (dx*dx + dy*dy > 600.f*600.f || now-last.at > 500.0) {
            motion_sample_count = 0; // teleport / tab resume: start at authority
            x.set_render(nx); y.set_render(ny);
        }
    }
    if (motion_sample_count == 10) {
        for (int i=0; i<9; ++i) motion_samples[i] = motion_samples[i+1];
        motion_sample_count = 9;
    }
    motion_samples[motion_sample_count++] = {nx,ny,now};
}

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
            if (motion_sample_count >= 2 && has_component(kMob) && !has_component(kFlower)) {
                // Slew the shared playback delay so changing the jitter buffer
                // cannot reverse all mobs or jump their positions in one frame.
                if (motion_delay_frame != Game::timestamp) {
                    motion_delay_frame=Game::timestamp;
                    double target=std::clamp(65.0+motion_lateness,75.0,200.0);
                    double dt=std::clamp((double)Ui::dt,0.0,50.0);
                    motion_delay+=std::clamp(target-motion_delay,-dt*.03,dt*.15);
                }
                double const playback = Game::timestamp - motion_delay;
                MotionSample a = motion_samples[0], b = a;
                for (uint8_t i=1; i<motion_sample_count; ++i) {
                    b = motion_samples[i];
                    if (b.at >= playback) break;
                    a = b;
                }
                float const t = b.at > a.at
                    ? fclamp((float)((playback-a.at)/(b.at-a.at)),0.f,1.f) : 1.f;
                x.set_render(a.x+(b.x-a.x)*t);
                y.set_render(a.y+(b.y-a.y)*t);
            } else {
                x.step(phys_amt);
                y.step(phys_amt);
            }
        }
        if (has_component(kDrop) || has_component(kWeb) || has_component(kAnimation)) {
            if (lifetime < TPS)
                animation = lerp(animation, 1, amt * 0.75);
            else animation = 1;
        } else {
            Vector vel(x - prev_x, y - prev_y);
            animation += (1 + 0.75 * vel.magnitude()) * 0.075;
        }
        radius.step(amt);
        angle.step_angle(phys_amt);
        if (pending_delete) {
            if (has_component(kAnimation)) {
                // Short bolt: snap toward fade so one FOV packet still reads.
                if (deletion_animation == 0.f)
                    deletion_animation = 1.f - powf(0.9f, 60.f * (float)lifetime / (float)TPS);
                else
                    deletion_animation = lerp(deletion_animation, 1.f, amt * 0.55f);
            } else {
                // Match server deletion_tick (~TPS/5 ≈ 200ms): finish inflate before hard delete.
                deletion_animation = fclamp(deletion_animation + Ui::dt / 150.f, 0, 1);
            }
        }
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
            if (damaged && get_state_hit_damage() && hit_damage.anchor() > 0.f
                && has_component(kMob) && !has_component(kPetal)
                // Practice targets show exact server readouts, including lethal
                // hits; an HP-ratio estimate would undercount their resets.
                && get_mob_id() != MobID::kTrainingDummy
                && !pending_delete) {
                float amt = hit_damage.anchor();
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
        // Flash even on the lethal packet (pending_delete same tick) so one-shots
        // still read as a hit before the inflate-fade, not a silent vanish.
        if (damaged == 1 && damage_flash < 0.1)
            damage_flash = 1;
        else
            damage_flash = lerp(damage_flash, 0, amt);
        if (damaged)
            last_damaged_time = Game::timestamp;
        damaged.clear();
        if ((float) health_ratio > 0.999
            && (last_damaged_time <= 0 || Game::timestamp - last_damaged_time > 1500))
            healthbar_opacity = lerp(healthbar_opacity, 0, amt);
        else
            healthbar_opacity = 1;
        if (healthbar_lag < health_ratio)
            healthbar_lag = health_ratio;
        else if (Game::timestamp - last_damaged_time > 250)
            healthbar_lag = lerp(healthbar_lag, health_ratio, amt / 3);
    }
    if (has_component(kFlower)) {
        mana_ratio.step(amt);
        shield_ratio.step(amt);
        sponge_pending.step(amt);
        float eye_angle = angle.anchor();
        eye_x = lerp(eye_x, cosf(eye_angle) * 2, amt);
        eye_y = lerp(eye_y, sinf(eye_angle) * 4, amt);
        if (BitMath::at(face_flags, FaceFlags::kAttacking)
            || BitMath::at(face_flags, FaceFlags::kPoisoned) 
            || BitMath::at(face_flags, FaceFlags::kDandelioned)
            || BitMath::at(face_flags, FaceFlags::kDeadEyes)
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
        // Keep the slot until the server deletes it. The 150ms visual fade can
        // finish before the server's 200ms retirement; deltas still target it.
        ent.reset_protocol();
    });
}
