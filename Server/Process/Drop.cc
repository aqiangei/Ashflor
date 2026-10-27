#include <Server/Process.hh>
#include <Server/Spawn.hh>

#include <Shared/Entity.hh>
#include <Shared/Map.hh>
#include <Shared/Simulation.hh>
#include <Shared/StaticData.hh>

#include <Helpers/Math.hh>

#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Keep loot reachable: eject from impassable terrain.
// Tagged drops stay private (no FFA unlock) — only the owner may see or pick.
void tick_drop_behavior(Simulation *sim, Entity &drop) {
    if (drop.pending_delete) return;
    if (!drop.has_component(kDrop) || !drop.has_component(kPhysics)) return;

    float x = drop.get_x();
    float y = drop.get_y();
    float r = drop.get_radius();
    if (!is_finite(x) || !is_finite(y) || !is_finite(r) || r <= 0.f) return;

    // Staggered coin coalesce — mop up piles that missed the spawn-time merge.
    if (drop.get_drop_id() == PetalID::kCoin
        && ((drop.id.id + sim->tick_index) & 7u) == 0u) {
        if (try_merge_coin_drop(sim, drop)) return;
    }

    // Push out of hills / water so players can walk onto the pile.
    if (Map::circle_hits_impassable(sim->map_id, x, y, r + 4.f)) {
        Map::resolve_circle_terrain(sim->map_id, x, y, r + 8.f);
        // If still blocked (tight pockets), try a few random nearby points.
        if (Map::circle_hits_impassable(sim->map_id, x, y, r + 4.f)) {
            float ox = drop.get_x(), oy = drop.get_y();
            for (uint32_t k = 0; k < 8; ++k) {
                float ang = frand() * 2.f * (float) M_PI;
                float dist = 40.f + frand() * 120.f;
                float tx = ox + std::cos(ang) * dist;
                float ty = oy + std::sin(ang) * dist;
                tx = fclamp(tx, r + 8.f, ARENA_WIDTH - r - 8.f);
                ty = fclamp(ty, r + 8.f, ARENA_HEIGHT - r - 8.f);
                if (!Map::circle_hits_impassable(sim->map_id, tx, ty, r + 4.f)) {
                    x = tx;
                    y = ty;
                    break;
                }
            }
        }
        x = fclamp(x, r, ARENA_WIDTH - r);
        y = fclamp(y, r, ARENA_HEIGHT - r);
        drop.set_x(x);
        drop.set_y(y);
        // Kill velocity into walls so magnet doesn't re-wedge the pile.
        drop.velocity *= 0.3f;
    }
}
