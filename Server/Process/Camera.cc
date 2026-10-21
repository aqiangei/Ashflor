#include <Server/Process.hh>

#include <Server/Client.hh>
#include <Server/Server.hh>

#include <Shared/Entity.hh>
#include <Shared/Simulation.hh>
#include <Shared/StaticData.hh>
#include <Shared/Talent.hh>

void tick_camera_behavior(Simulation *sim, Entity &ent) {
    if (sim->ent_exists(ent.get_player())) {
        Entity &player = sim->get_ent(ent.get_player());
        ent.set_camera_x(player.get_x());
        ent.set_camera_y(player.get_y());
        uint32_t slots = loadout_slots_at_level(score_to_level(player.get_score()));
        if (Client *owner = Server::client_by_camera(sim, ent.id))
            slots = Talent::slot_count_from_ranks(owner->talent_ranks);
        player.set_loadout_count(slots);
        if (player.acceleration.x != 0 || player.acceleration.y != 0)
            player.set_angle(player.acceleration.angle());

        ent.last_damaged_by = player.last_damaged_by;
    } else {
        ent.set_player(NULL_ENTITY);
        ent.set_fov(BASE_FOV * 0.9);
        // CPU bots: keep camera alive; FlowerAi respawns the flower
        if (BitMath::at(ent.flags, EntityFlags::kCPUControlled))
            return;
        if (Client *owner = Server::client_by_camera(sim, ent.id)) {
            if (owner->awaiting_revive) {
                ent.set_camera_x(owner->corpse_x);
                ent.set_camera_y(owner->corpse_y);
                return;
            }
            if (owner->has_death_pos) {
                ent.set_camera_x(owner->death_x);
                ent.set_camera_y(owner->death_y);
                return;
            }
        }
        if (sim->ent_exists(ent.last_damaged_by)){
            Entity &spectating = sim->get_ent(ent.last_damaged_by);
            ent.set_camera_x(spectating.get_x());
            ent.set_camera_y(spectating.get_y());
        }
    }
}
