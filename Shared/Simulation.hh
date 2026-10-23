#pragma once

#include <Shared/Arena.hh>
#include <Shared/Entity.hh>

#ifdef SERVERSIDE
#include <Server/SpatialHash.hh>
#endif

#include <functional>
#include <string>

inline uint32_t const ENTITY_CAP = 8192;

class Simulation {
    std::array<uint8_t, div_round_up(ENTITY_CAP, 8)> entity_tracker;
    std::array<EntityID::hash_type, ENTITY_CAP> hash_tracker;
    std::array<Entity, ENTITY_CAP> entities;
    StaticArray<EntityID::id_type, ENTITY_CAP> active_entities;
public:
    SERVER_ONLY(std::array<uint32_t, PetalID::kNumPetals> petal_count_tracker;)
    SERVER_ONLY(std::array<uint32_t, MAX_ZONES_PER_MAP> zone_mob_counts;)
    // 出生广场内的怪数量（与 zone 计数分开，广场外不占用名额）
    SERVER_ONLY(uint32_t plaza_mob_count;)
    // Boss 生命周期：唯一实例 + 死亡后重生计时
    SERVER_ONLY(int32_t boss_respawn_ticks;)
    SERVER_ONLY(uint8_t boss_ever_spawned;)
    SERVER_ONLY(EntityID boss_ent;)
    SERVER_ONLY(SpatialHash spatial_hash;)
    // Tick counter for throttling non-critical work (leaderboard fill_top, etc.)
    SERVER_ONLY(uint32_t tick_index;)
    Arena arena_info;
    uint8_t map_id = MapID::kGarden;
    Simulation();
    void reset();
    Entity &alloc_ent();
    void _delete_ent(EntityID const &); //DANGEROUS
    void force_alloc_ent(EntityID const &);
    void request_delete(EntityID const &);
    Entity &get_ent(EntityID const &);
    uint8_t ent_exists(EntityID const &) const;
    uint8_t ent_alive(EntityID const &) const;
    void tick();
    void on_tick();
    void post_tick();
    // Rebuild the active-entity list without advancing the simulation.
    // Needed by for_each/for_each_entity before the first tick() call.
    void refresh_active_entities();

    //will only consider active entities from the start of the tick() call
    void for_each_entity(std::function<void (Simulation *, Entity &)>);
    template <uint8_t>
    void for_each(std::function<void (Simulation *, Entity &)>);
};

#ifdef SERVERSIDE
#include <Server/SpatialHash.inl>
#endif