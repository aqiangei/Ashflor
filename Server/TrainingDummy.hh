#pragma once

#include <Shared/Entity.hh>

class Simulation;
class Client;

namespace TrainingDummy {
    // These counters belong to a connection, never to the shared target entity.
    struct Stats {
        double damage = 0;
        float last_hit = 0;
        uint32_t first_tick = 0;
        uint32_t last_tick = 0;
        bool hit = false;
        game_tick_t poison_ticks = 0, burn_ticks = 0, slow_ticks = 0;
        float poison_per_tick = 0, burn_per_tick = 0;
    };

    inline bool is_dummy(Entity const &ent) {
        return ent.has_component(kMob) && ent.get_mob_id() == MobID::kTrainingDummy;
    }

    void ensure(Simulation *);
    Stats *personal_stats(Simulation *, Entity const &, EntityID source);
    void record_damage(Simulation *, Entity &, EntityID source, float);
    void inflict_status(Stats &, Entity const &, uint8_t damage_type);
    void tick_client(Simulation *, Client *);
    void reset(Client *);
}
