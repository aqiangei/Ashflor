#include <Server/Process.hh>

#include <Shared/Entity.hh>
#include <Shared/Map.hh>
#include <Shared/Simulation.hh>
#include <Shared/StaticData.hh>

// AOI margin — keep modest; every extra entity burns AI/collision on the hot path
constexpr float CULL_EXTRA_RADIUS = 180;

void tick_culling_behavior(Simulation *sim, Entity &ent) {
    float fov = fclamp(ent.get_fov(), BASE_FOV * 0.1, BASE_FOV);
    sim->spatial_hash.query(ent.get_camera_x(), ent.get_camera_y(), 960 / fov + CULL_EXTRA_RADIUS, 540 / fov + CULL_EXTRA_RADIUS, [](Simulation *, Entity &ent) {
        BitMath::unset(ent.flags, EntityFlags::kIsCulled);
    });
}
