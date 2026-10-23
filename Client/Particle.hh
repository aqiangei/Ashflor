#pragma once

#include <Client/Render/Renderer.hh>

#include <Shared/EntityDef.hh>
#include <Shared/StaticData.hh>

#include <cstddef>

namespace Particle {
    class TitleParticleEntity {
    public:
        float x;
        float y;
        float x_velocity;
        float angle;
        float sin_offset;
        float radius;
        PetalID::T id;
    };

    class GameParticleEntity {
    public:
        float x;
        float y;
        float x_velocity;
        float y_velocity;
        float radius;
        float opacity;
        uint32_t fill = 0x80ffffff;
        // 1 = Unique petal FX (ash→gold by level); drives halo in tick_game.
        uint8_t unique_fx = 0;
    };

    void tick_title(Renderer &, double);
    void tick_game(Renderer &, double);
    // Unique sparkles: Lv.0 charcoal grit → higher levels increasingly gold.
    void add_unique_particle(float x, float y, uint32_t level = 0);
    // Escort spirit trail: spawn behind the spirit, drift back and fade.
    void add_spirit_trail(float x, float y, float back_dx, float back_dy);
    // P3: budget helper for low-end throttling.
    size_t game_particle_count();
    // Design §7: Korok-style guide sparks toward nearby treasure sites (client-only).
    void tick_guide_sprite(double dt);
    // Design G04 / Phase5: ambient biome motes (pollen, dust…) — separate from Unique petal FX.
    void tick_ambient_env(double dt);

    // World-space floating combat numbers (throttled / accumulated per entity).
    void add_damage_number(EntityID const &, float x, float y, float amount);
    void add_heal_number(float x, float y, float amount);
    void clear_combat_accum(EntityID const &);
}
