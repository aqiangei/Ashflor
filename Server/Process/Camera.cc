#include <Server/Process.hh>

#include <Server/Client.hh>
#include <Server/Server.hh>

#include <Shared/Entity.hh>
#include <Shared/Map.hh>
#include <Shared/Simulation.hh>
#include <Shared/StaticData.hh>

void tick_camera_behavior(Simulation *sim, Entity &ent) {
    if (sim->ent_exists(ent.get_player())) {
        Entity &player = sim->get_ent(ent.get_player());
        ent.set_camera_x(player.get_x());
        ent.set_camera_y(player.get_y());
        player.set_loadout_count(loadout_slots_at_level(score_to_level(player.get_score())));
        if (player.acceleration.x != 0 || player.acceleration.y != 0)
            player.set_angle(player.acceleration.angle());

        ent.last_damaged_by = player.last_damaged_by;
        uint32_t zid = Map::get_zone_from_pos(sim->map_id, player.get_x(), player.get_y());
        struct ZoneDefinition const &zone = map_zones(sim->map_id)[zid];
        if (zone.difficulty < Map::difficulty_at_level(score_to_level(player.get_score()))) {
            if (player.get_overlevel_timer() < PETAL_DISABLE_DELAY * TPS)
                player.set_overlevel_timer(player.get_overlevel_timer() + 1);
            else player.set_overlevel_timer(PETAL_DISABLE_DELAY * TPS);
        } else {
            if (player.get_overlevel_timer() > 0)
                player.set_overlevel_timer(player.get_overlevel_timer() - 0.1);
            else player.set_overlevel_timer(0);
        }
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
        }
        if (sim->ent_exists(ent.last_damaged_by)){
            Entity &spectating = sim->get_ent(ent.last_damaged_by);
            ent.set_camera_x(spectating.get_x());
            ent.set_camera_y(spectating.get_y());
        }
    }
}
