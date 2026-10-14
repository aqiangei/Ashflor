#include <Server/Process.hh>

#include <Server/Spawn.hh>

#include <Shared/Entity.hh>
#include <Shared/Map.hh>
#include <Shared/Simulation.hh>
#include <Shared/StaticData.hh>
#include <Shared/StaticDefinitions.hh>

#include <Helpers/Bits.hh>
#include <Helpers/Math.hh>
#include <Helpers/Vector.hh>

#include <array>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Lightweight bot fill (zorr-inspired): CPU cameras wander / engage nearest mob.

static constexpr float BOT_VISION = 520.f;
static constexpr float BOT_FIGHT = 70.f;
static uint16_t const BOT_RESPAWN_TICKS = (uint16_t) (2 * TPS);

static std::array<uint16_t, ENTITY_CAP> bot_respawn_cd = {};
static uint32_t bot_phase_tick = 0;

void tick_player_ai_behavior(Simulation *sim, Entity &cam) {
    if (!BitMath::at(cam.flags, EntityFlags::kCPUControlled)) return;
    ++bot_phase_tick;

    if (!sim->ent_alive(cam.get_player())) {
        uint16_t &cd = bot_respawn_cd[cam.id.id];
        if (cd == 0) cd = BOT_RESPAWN_TICKS;
        if (cd > 1) {
            --cd;
            return;
        }
        cd = 0;
        Entity &player = alloc_player(sim, cam.get_team());
        player_spawn(sim, cam, player);
        static char const *names[] = {
            "Bot-Alpha", "Bot-Beta", "Bot-Gamma", "Bot-Delta",
            "Bot-Echo", "Bot-Foxtrot", "Bot-Golf", "Bot-Hotel"
        };
        player.set_name(names[cam.id.id % 8]);
        return;
    }

    Entity &player = sim->get_ent(cam.get_player());
    Entity *target = nullptr;
    float best = BOT_VISION * BOT_VISION;
    sim->for_each<kMob>([&](Simulation *, Entity &mob) {
        if (mob.pending_delete) return;
        if (mob.get_team() == player.get_team()) return;
        float dx = mob.get_x() - player.get_x();
        float dy = mob.get_y() - player.get_y();
        float d2 = dx * dx + dy * dy;
        if (d2 < best) {
            best = d2;
            target = &mob;
        }
    });

    player.input = 0;
    if (target == nullptr) {
        float a = frand() * 2.f * (float) M_PI;
        player.acceleration.set(
            std::cos(a) * PLAYER_ACCELERATION * 0.55f,
            std::sin(a) * PLAYER_ACCELERATION * 0.55f
        );
        return;
    }

    float dx = target->get_x() - player.get_x();
    float dy = target->get_y() - player.get_y();
    float dist = std::sqrt(best);
    Vector dir(dx, dy);
    if (dist > 1.f) dir.set_magnitude(PLAYER_ACCELERATION);
    else dir.set(0, 0);

    if (dist > BOT_FIGHT) {
        player.acceleration = dir;
        BitMath::set(player.input, InputFlags::kAttacking);
    } else {
        Vector tangent(-dir.y, dir.x);
        float phase = std::sin((float) cam.id.id * 0.7f + (float) (bot_phase_tick % 10000) * 0.05f);
        if (!is_finite(phase)) phase = 0;
        player.acceleration = dir * 0.2f + tangent * (0.6f * phase);
        if (player.acceleration.magnitude() > 1e-3f)
            player.acceleration.set_magnitude(PLAYER_ACCELERATION);
        BitMath::set(player.input, InputFlags::kAttacking);
    }
}
