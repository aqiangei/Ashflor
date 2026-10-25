#include <Shared/Simulation.hh>

#ifdef DEBUG
#include <iostream>

std::ostream &operator<<(std::ostream &out, EntityID const &id) { 
    return out << '<' << (uint32_t) id.hash << ',' << (uint32_t) id.id << '>';
}
#endif

#ifdef SERVERSIDE
#include <Shared/Map.hh>
#include <Server/PetalTracker.hh>
#include <iostream>
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
// Cap pressure reclaim. Prefer finished corpses (safe hard-free). Soft-schedule
// live wildlife for later ticks. Desperate path hard-frees drops/webs/orphans
// and culled zone wildlife WITHOUT mid-tick entity_on_death (no loot) so
// alloc_ent can obtain a slot without aborting the process (ABRT → client 1006).
//
// Never touch cameras, flowers, bosses, dummies, live-parent petals, or summons
// still attached to an alive flower.
uint32_t Simulation::emergency_reclaim_entities(uint32_t budget) {
    uint32_t freed = 0;
    uint32_t soft = 0;
    auto is_zone_wildlife = [&](Entity &ent) -> bool {
        if (!ent_exists(ent.id)) return false;
        if (ent.has_component(kCamera)) return false;
        if (TrainingDummy::is_dummy(ent)) return false;
        if (ent.has_component(kFlower) && ent.has_component(kHealth) && !ent.has_component(kMob))
            return false;
        if (!ent.has_component(kMob)) return false;
        if (ent.zone == 0xFF) return false;
        if (!BitMath::at(ent.flags, EntityFlags::kSpawnedFromZone)) return false;
        return true;
    };

    auto scan = [&](auto pred, auto action) {
        for (EntityID::id_type i = 1; i < ENTITY_CAP && freed < budget; ++i) {
            if (!BitMath::at_arr(entity_tracker.data(), i)) continue;
            Entity &ent = entities[i];
            if (!pred(ent)) continue;
            action(ent);
        }
    };

    // 1) Finished corpses of any type — death already ran; free the slot now.
    scan(
        [&](Entity &e) { return e.pending_delete && e.deletion_tick > 0; },
        [&](Entity &e) { _delete_ent(e.id); ++freed; }
    );

    // 2) Soft-mark culled, then any zone wildlife (next post_tick frees).
    auto soft_wildlife = [&](Entity &ent) {
        if (freed >= budget) return;
        if (!is_zone_wildlife(ent)) return;
        if (ent.pending_delete) return;
        BitMath::set(ent.flags, EntityFlags::kNoDrops);
        request_delete(ent.id);
        ++soft;
    };
    scan(
        [&](Entity &e) {
            return !e.pending_delete && is_zone_wildlife(e)
                && BitMath::at(e.flags, EntityFlags::kIsCulled);
        },
        soft_wildlife
    );
    scan(
        [&](Entity &e) { return !e.pending_delete && is_zone_wildlife(e); },
        soft_wildlife
    );

    // 3) Desperate hard-free when soft cannot open a slot for THIS alloc.
    if (freed < budget) {
        // Drops: fix petal tracker, skip death path.
        scan(
            [&](Entity &e) {
                return e.has_component(kDrop) && !e.has_component(kCamera);
            },
            [&](Entity &e) {
                if (freed >= budget) return;
                PetalTracker::remove_petal(this, e.get_drop_id());
                _delete_ent(e.id);
                ++freed;
            }
        );
        // Webs / transient.
        scan(
            [&](Entity &e) { return e.has_component(kWeb); },
            [&](Entity &e) {
                if (freed >= budget) return;
                _delete_ent(e.id);
                ++freed;
            }
        );
        // Orphan petals (parent gone).
        scan(
            [&](Entity &e) {
                return e.has_component(kPetal) && !ent_alive(e.get_parent());
            },
            [&](Entity &e) {
                if (freed >= budget) return;
                _delete_ent(e.id);
                ++freed;
            }
        );
        // Culled zone wildlife (incl. soft-scheduled): free slot now, no loot.
        scan(
            [&](Entity &e) {
                return is_zone_wildlife(e)
                    && BitMath::at(e.flags, EntityFlags::kIsCulled);
            },
            [&](Entity &e) {
                if (freed >= budget) return;
                if (e.pending_delete && e.deletion_tick > 0) {
                    _delete_ent(e.id);
                } else {
                    BitMath::set(e.flags, EntityFlags::kNoDrops);
                    Map::remove_mob(this, e);
                    _delete_ent(e.id);
                }
                ++freed;
            }
        );
    }

    if (soft > 0 || freed > 0) {
        std::cerr << "[sim] reclaim soft-scheduled wildlife=" << soft
                  << " hard_freed=" << freed << std::endl;
    }
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
    // Never ABRT the process on a full pool — that is mass client 1006.
    uint32_t const before = entity_count();
    uint32_t freed = emergency_reclaim_entities(128);
    std::cerr << "[sim] entity cap hit count=" << before
              << " reclaimed=" << freed << std::endl;
    if (Entity *e = try_claim()) return *e;

    freed = emergency_reclaim_entities(768);
    std::cerr << "[sim] entity cap retry reclaimed=" << freed
              << " count=" << entity_count() << std::endl;
    if (Entity *e = try_claim()) return *e;

    // Last resort: hard-evict any remaining zone wildlife (even in FOV).
    {
        uint32_t stole = 0;
        for (EntityID::id_type i = 1; i < ENTITY_CAP && stole < 64; ++i) {
            if (!BitMath::at_arr(entity_tracker.data(), i)) continue;
            Entity &ent = entities[i];
            if (!ent.has_component(kMob)) continue;
            if (ent.zone == 0xFF) continue;
            if (!BitMath::at(ent.flags, EntityFlags::kSpawnedFromZone)) continue;
            if (TrainingDummy::is_dummy(ent)) continue;
            // Soft-scheduled (pending, deletion_tick==0): death has NOT run yet —
            // must remove_mob here or zone counts leak. Skip only if somehow
            // already past death (deletion_tick>0) without being freed above.
            if (ent.pending_delete && ent.deletion_tick > 0) {
                _delete_ent(ent.id);
            } else {
                BitMath::set(ent.flags, EntityFlags::kNoDrops);
                Map::remove_mob(this, ent);
                _delete_ent(ent.id);
            }
            ++stole;
            if (Entity *e = try_claim()) {
                std::cerr << "[sim] entity cap last-resort stole=" << stole
                          << " count=" << entity_count() << std::endl;
                return *e;
            }
        }
    }

    std::cerr << "[sim] entity cap exhausted (no abort) count="
              << entity_count() << " — recycling slot 1" << std::endl;
    // Absolute fallback: never abort. Clobber the lowest id that is not a
    // camera/flower. Prefer a drop/web/mob; if somehow impossible, slot 1.
    for (EntityID::id_type i = 1; i < ENTITY_CAP; ++i) {
        if (!BitMath::at_arr(entity_tracker.data(), i)) continue;
        Entity &ent = entities[i];
        if (ent.has_component(kCamera)) continue;
        if (ent.has_component(kFlower) && !ent.has_component(kMob)) continue;
        if (ent.has_component(kDrop))
            PetalTracker::remove_petal(this, ent.get_drop_id());
        else if (ent.has_component(kMob)
            && BitMath::at(ent.flags, EntityFlags::kSpawnedFromZone)
            && ent.zone != 0xFF
            && !(ent.pending_delete && ent.deletion_tick > 0)) {
            Map::remove_mob(this, ent);
        }
        _delete_ent(ent.id);
        if (Entity *e = try_claim()) return *e;
    }
#endif
    assert(!"Entity cap reached");
#ifdef SERVERSIDE
    // Still full of cameras/flowers only — extremely unlikely. Reuse slot 1.
    EntityID::id_type i = 1;
    if (BitMath::at_arr(entity_tracker.data(), i))
        _delete_ent(EntityID(i, hash_tracker[i]));
    BitMath::set_arr(entity_tracker.data(), i);
    entities[i].init();
    entities[i].id = EntityID(i, hash_tracker[i]);
    return entities[i];
#else
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
