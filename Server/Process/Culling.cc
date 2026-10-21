#include <Server/Process.hh>

#include <Shared/Entity.hh>
#include <Shared/Map.hh>
#include <Shared/Simulation.hh>
#include <Shared/StaticData.hh>

// AI AOI stays near base FOV so Antenna/Observer wide views don't burn CPU on
// far mobs. Network visibility (Game.cc) expands separately so they still render.
constexpr float CULL_EXTRA_RADIUS = 220;

void tick_culling_behavior(Simulation *sim, Entity &ent) {
    // Floor FOV used for AI: never thinner than ~72% of base (limits AOI growth).
    float fov = fclamp(ent.get_fov(), BASE_FOV * 0.72f, BASE_FOV);
    sim->spatial_hash.query(ent.get_camera_x(), ent.get_camera_y(),
        960 / fov + CULL_EXTRA_RADIUS, 540 / fov + CULL_EXTRA_RADIUS,
        [](Simulation *, Entity &ent) {
            BitMath::unset(ent.flags, EntityFlags::kIsCulled);
        });
}
