#include <Shared/Simulation.hh>

#include <Helpers/Bits.hh>

#ifdef SERVERSIDE
#include <Shared/Map.hh>
#include <Shared/StaticDefinitions.hh>
#include <Server/PetalTracker.hh>
#include <Server/TrainingDummy.hh>
#include <iostream>
#endif

#include <cstdlib>

#ifdef DEBUG
#include <iostream>

std::ostream &operator<<(std::ostream &out, EntityID const &id) { 
    return out << '<' << (uint32_t) id.hash << ',' << (uint32_t) id.id << '>';
}
#endif

Simulation::Simulation() SERVER_ONLY(: spatial_hash(this)) {
    reset();
}

void Simulation::reset() {
    active_entities.clear();
    hash_tracker = {0};
    entity_tracker = {0};
    
    for (EntityID::id_type i = 0; i < ENTITY_CAP; ++i)
        entities[i].init();

    arena_info.init();
    #ifdef SERVERSIDE
    spatial_hash.refresh(ARENA_WIDTH, ARENA_HEIGHT);
    petal_count_tracker = {0};
    zone_mob_counts = {0};
    plaza_mob_count = 0;
    boss_respawn_ticks = 0;
    boss_ever_spawned = 0;
    boss_ent = NULL_ENTITY;
    training_dummies = {};
    tick_index = 0;
    #endif
}

uint32_t Simulation::entity_count() const {
    uint32_t n = 0;
    for (EntityID::id_type i = 1; i < ENTITY_CAP; ++i)
        if (BitMath::at_arr(entity_tracker.data(), i)) ++n;
    return n;
}

#ifdef SERVERSIDE
uint32_t Simulation::emergency_reclaim_entities(uint32_t budget) {
    uint32_t freed = 0;
    auto force_free = [&](Entity &ent) {
        if (freed >= budget) return;
        if (!ent_exists(ent.id)) return;
        if (ent.has_component(kCamera)) return;
        if (TrainingDummy::is_dummy(ent)) return;
        // Living player flower
        if (ent.has_component(kFlower) && ent.has_component(kHealth) && !ent.has_component(kMob))
            return;
        // Map-anchor boss
        if (ent.has_component(kMob) && ent.zone == 0xFF) return;
        if (ent.has_component(kDrop))
            PetalTracker::remove_petal(this, ent.get_drop_id());
        if (ent.has_component(kMob) && BitMath::at(ent.flags, EntityFlags::kSpawnedFromZone))
            Map::remove_mob(this, ent);
        // No entity_on_death — avoid loot alloc recursion under cap pressure.
        _delete_ent(ent.id);
        ++freed;
    };

    auto scan = [&](auto pred) {
        for (EntityID::id_type i = 1; i < ENTITY_CAP && freed < budget; ++i) {
            if (!BitMath::at_arr(entity_tracker.data(), i)) continue;
            Entity &ent = entities[i];
            if (!pred(ent)) continue;
            force_free(ent);
        }
    };

    scan([&](Entity &e) { return e.pending_delete; });
    scan([&](Entity &e) {
        return !e.pending_delete && (e.has_component(kDrop) || e.has_component(kWeb));
    });
    scan([&](Entity &e) {
        return e.has_component(kMob)
            && BitMath::at(e.flags, EntityFlags::kSpawnedFromZone)
            && BitMath::at(e.flags, EntityFlags::kIsCulled);
    });
    scan([&](Entity &e) {
        return e.has_component(kMob)
            && BitMath::at(e.flags, EntityFlags::kSpawnedFromZone);
    });
    return freed;
}
#endif

Entity &Simulation::alloc_ent() {
    auto try_claim = [this]() -> Entity * {
        for (EntityID::id_type i = 1; i < ENTITY_CAP; ++i) {
            if (BitMath::at_arr(entity_tracker.data(), i)) continue;
            BitMath::set_arr(entity_tracker.data(), i);
            entities[i].init();
            DEBUG_ONLY(std::cout << "ent_create " << EntityID(i, hash_tracker[i]) << "\n";)
            entities[i].id = EntityID(i, hash_tracker[i]);
            return &entities[i];
        }
        return nullptr;
    };

    if (Entity *e = try_claim()) return *e;

#ifdef SERVERSIDE
    // Release builds strip assert(); the old abort() caused silent ABRT
    // (2026-10-24 14:45) when the pool filled under spawn / admin pressure.
    uint32_t const before = entity_count();
    uint32_t freed = emergency_reclaim_entities(96);
    std::cerr << "[sim] entity cap hit count=" << before
              << " reclaimed=" << freed << std::endl;
    if (Entity *e = try_claim()) return *e;

    freed = emergency_reclaim_entities(512);
    std::cerr << "[sim] entity cap retry reclaimed=" << freed
              << " count=" << entity_count() << std::endl;
    if (Entity *e = try_claim()) return *e;

    // Last resort: any mob/drop/web/petal except players/cameras/bosses/dummies.
    for (EntityID::id_type i = 1; i < ENTITY_CAP; ++i) {
        if (!BitMath::at_arr(entity_tracker.data(), i)) continue;
        Entity &ent = entities[i];
        if (ent.has_component(kCamera)) continue;
        if (ent.has_component(kFlower) && ent.has_component(kHealth) && !ent.has_component(kMob))
            continue;
        if (ent.has_component(kMob) && ent.zone == 0xFF) continue;
        if (TrainingDummy::is_dummy(ent)) continue;
        if (!(ent.has_component(kMob) || ent.has_component(kDrop)
              || ent.has_component(kWeb) || ent.has_component(kPetal)))
            continue;
        if (ent.has_component(kDrop))
            PetalTracker::remove_petal(this, ent.get_drop_id());
        if (ent.has_component(kMob) && BitMath::at(ent.flags, EntityFlags::kSpawnedFromZone))
            Map::remove_mob(this, ent);
        _delete_ent(ent.id);
        if (Entity *e = try_claim()) {
            std::cerr << "[sim] entity cap last-resort freed slot=" << (uint32_t)i << std::endl;
            return *e;
        }
    }

    std::cerr << "[sim] FATAL entity pool exhausted after reclaim count="
              << entity_count() << " — recycling slot 1 (no abort)\n";
    if (BitMath::at_arr(entity_tracker.data(), 1))
        _delete_ent(EntityID(1, hash_tracker[1]));
    BitMath::set_arr(entity_tracker.data(), 1);
    entities[1].init();
    entities[1].id = EntityID(1, hash_tracker[1]);
    return entities[1];
#else
    assert(!"Entity cap reached");
    abort();
#endif
}

Entity &Simulation::get_ent(EntityID const &id) {
    DEBUG_ONLY(assert(ent_exists(id));)
    return entities[id.id];
}

void Simulation::force_alloc_ent(EntityID const &id) {
    assert(id.id < ENTITY_CAP);
    DEBUG_ONLY(std::cout << "ent_create " << id << "\n";)
    // Client may still hold a stale slot after map transfer; replace in place.
    // Do not bump hash_tracker — the server owns the new generation.
    if (BitMath::at_arr(entity_tracker.data(), id.id))
        BitMath::unset_arr(entity_tracker.data(), id.id);
    entities[id.id].init();
    BitMath::set_arr(entity_tracker.data(), id.id);
    hash_tracker[id.id] = id.hash;
    entities[id.id].id = id;
}

uint8_t Simulation::ent_exists(EntityID const &id) const {
    DEBUG_ONLY(assert(id.id < ENTITY_CAP);)
    return BitMath::at_arr(entity_tracker.data(), id.id) && hash_tracker[id.id] == id.hash;
}

uint8_t Simulation::ent_alive(EntityID const &id) const {
    return ent_exists(id) && !entities[id.id].pending_delete
    SERVER_ONLY(&& entities[id.id].deletion_tick == 0);
}

void Simulation::request_delete(EntityID const &id) {
    DEBUG_ONLY(assert(ent_exists(id)));
    entities[id.id].pending_delete = 1;
}

void Simulation::_delete_ent(EntityID const &id) {
    DEBUG_ONLY(std::cout << "ent_delete " << id << "\n";)
    DEBUG_ONLY(assert(ent_exists(id)));
    BitMath::unset_arr(entity_tracker.data(), id.id);
    hash_tracker[id.id]++;
}

void Simulation::tick() {
    refresh_active_entities();
    on_tick();
}

void Simulation::refresh_active_entities() {
    active_entities.clear();
    for (EntityID::id_type i = 1; i < ENTITY_CAP; ++i) {
        if (!BitMath::at_arr(entity_tracker.data(), i)) continue;
        active_entities.push(entities[i].id.id);
    }
}

void Simulation::for_each_entity(std::function<void(Simulation *, Entity &)> cb) { \
    for (EntityID::id_type i = 0; i < active_entities.size(); ++i) { \
        if (!BitMath::at_arr(entity_tracker.data(), active_entities[i])) continue; \
        Entity &ent = entities[active_entities[i]]; \
        cb(this, ent); \
    } \
}

#define COMPONENT(name) \
template<> \
void Simulation::for_each<k##name>(std::function<void(Simulation *, Entity &)> cb) { \
    for (EntityID::id_type i = 0; i < active_entities.size(); ++i) { \
        if (!BitMath::at_arr(entity_tracker.data(), active_entities[i])) continue; \
        Entity &ent = entities[active_entities[i]]; \
        SERVER_ONLY(if (ent.pending_delete) continue;) \
        if (ent.has_component(k##name)) cb(this, ent); \
    } \
}
PERCOMPONENT
#undef COMPONENT
