#include <Server/Spawn.hh>

#include <Server/Client.hh>
#include <Server/EntityFunctions.hh>
#include <Server/PetalTracker.hh>
#include <Server/Server.hh>

#include <Shared/Map.hh>
#include <Shared/RarityScale.hh>
#include <Shared/Simulation.hh>
#include <Shared/StaticData.hh>
#include <Shared/StaticDefinitions.hh>
#include <Shared/Talent.hh>

#include <Helpers/Bits.hh>
#include <Helpers/Math.hh>

#include <cmath>

Entity &alloc_drop(Simulation *sim, PetalID::T drop_id, EntityID const team, uint16_t count) {
    DEBUG_ONLY(assert(drop_id < PetalID::kNumPetals);)
    if (count < 1) count = 1;
    // Alloc first — add_petal before alloc leaked tracker counts when the pool
    // was full and reclaim recycled a different drop.
    Entity &drop = sim->alloc_ent();
    PetalTracker::add_petal(sim, drop_id);
    drop.add_component(kPhysics);
    // Bigger pile for stacked drops (coins etc.)
    float rad = 25.f;
    if (count > 1) rad = fminf(42.f, 22.f + 2.5f * sqrtf((float) count));
    drop.set_radius(rad);
    drop.set_angle(frand() * 0.2 - 0.1);
    drop.friction = 0.25;

    drop.add_component(kRelations);
    drop.set_team(team);

    drop.add_component(kDrop);
    drop.set_drop_id(drop_id);
    drop.set_drop_count(count);
    entity_set_despawn_tick(drop, 10 * (2 + PETAL_DATA[drop_id].rarity) * TPS);
    // Tagged drops stay private to their personal camera for their whole life.
    // Unowned (manual discard) gets a brief lock so the dropper doesn't re-grab, then FFA.
    if (team == NULL_ENTITY)
        drop.immunity_ticks = TPS / 3;
    else
        drop.immunity_ticks = 0; // team gate alone is enough; never opens to strangers
    return drop;
}

Entity &alloc_corpse(
    Simulation *sim, EntityID const team, float x, float y, std::string const &name,
    uint8_t color, uint8_t equip_flags
) {
    Entity &corpse = sim->alloc_ent();
    corpse.add_component(kPhysics);
    // Slightly smaller + tipped — reads as fallen, not standing AFK.
    corpse.set_radius(BASE_FLOWER_RADIUS * 0.92f);
    corpse.set_angle(-0.55f);
    corpse.set_x(x);
    corpse.set_y(y);
    corpse.mass = 10000;
    corpse.friction = 1;
    corpse.add_component(kRelations);
    corpse.set_team(team);
    corpse.set_color(color);
    BitMath::set(corpse.flags, EntityFlags::kNoFriendlyCollision);
    // Visible dead flower (no kHealth → skipped by tick_player_behavior / AI).
    corpse.add_component(kFlower);
    corpse.set_face_flags(1 << FaceFlags::kDeadEyes);
    corpse.set_equip_flags(equip_flags);
    corpse.set_loadout_count(0);
    corpse.add_component(kName);
    corpse.set_name(name + " [DOWN]");
    corpse.set_nametag_visible(1);
    return corpse;
}

static Entity &__alloc_mob(
    Simulation *sim, MobID::T mob_id, float x, float y, 
    EntityID const team, std::function<void(Entity &)> on_spawn
) {
    DEBUG_ONLY(assert(mob_id < MobID::kNumMobs);)
    struct MobData const &data = MOB_DATA[mob_id];
    float seed = frand();
    Entity &mob = sim->alloc_ent();

    mob.add_component(kPhysics);
    mob.set_radius(data.radius.get_single(seed) * MOB_RADIUS_MULTIPLIER);
    mob.set_angle(frand() * 2 * M_PI);
    mob.set_x(x);
    mob.set_y(y);
    mob.friction = DEFAULT_FRICTION;
    mob.mass = (1 + mob.get_radius() / BASE_FLOWER_RADIUS) * (data.attributes.stationary ? 10000 : 1);
    if (mob_id == MobID::kAntHole)
        BitMath::set(mob.flags, EntityFlags::kNoFriendlyCollision);
        
    mob.add_component(kRelations);
    mob.set_team(team);

    mob.add_component(kMob);
    mob.set_mob_id(mob_id);
    mob.set_rarity(data.rarity);

    mob.add_component(kHealth);
    mob.health = mob.max_health = data.health.get_single(seed);
    mob.damage = data.damage;
    mob.poison_damage = data.attributes.poison_damage;
    mob.set_health_ratio(1);

    mob.detection_radius = data.attributes.aggro_radius;
    // Default XP; zone spawn / AI callbacks may override.
    mob.score_reward = data.xp;

    mob.add_component(kName);
    mob.set_name(data.name);

    mob.base_entity = mob.id;
    if (mob_id == MobID::kDigger) {
        mob.add_component(kFlower);
        mob.set_angle(0);
        mob.set_color(ColorID::kGray);
    }
    if (on_spawn)
        on_spawn(mob);
    return mob;
}

void apply_mob_live_rarity(Entity &mob, uint8_t spawn_rarity) {
    MobID::T mid = mob.get_mob_id();
    if (mid >= MobID::kNumMobs) return;
    uint8_t base_r = MOB_DATA[mid].rarity;
    uint8_t spawn_r = spawn_rarity;
    if (spawn_r >= RarityID::kNumRarities) spawn_r = base_r;
    if (spawn_r < base_r) spawn_r = base_r;
    mob.set_rarity(spawn_r);
    float th = mob_spawn_threat_mult(base_r, spawn_r);
    mob.max_health *= th;
    mob.health = mob.max_health;
    mob.damage *= mob_spawn_damage_mult(base_r, spawn_r);
    float rs = mob_spawn_radius_mult(base_r, spawn_r);
    if (rs != 1.f) {
        float nr = mob.get_radius() * rs;
        mob.set_radius(nr);
        mob.mass = (1 + nr / BASE_FLOWER_RADIUS)
            * (MOB_DATA[mid].attributes.stationary ? 10000.f : 1.f);
    }
}

Entity &alloc_mob(
    Simulation *sim, MobID::T mob_id, float x, float y, 
    EntityID const team, std::function<void(Entity &)> on_spawn
) {
    struct MobData const &data = MOB_DATA[mob_id];
    if (data.attributes.segments <= 1) {
        Entity &ent = __alloc_mob(sim, mob_id, x, y, team, on_spawn);
        if (mob_id == MobID::kAntHole) {
            std::vector<MobID::T> const spawns = { 
                MobID::kBabyAnt, MobID::kBabyAnt, MobID::kBabyAnt, 
                MobID::kWorkerAnt, MobID::kWorkerAnt, MobID::kSoldierAnt
            };
            for (MobID::T mob_id : spawns) {
                Vector rand = Vector::rand(ent.get_radius() * 2);
                Entity &ant = __alloc_mob(sim, mob_id, x + rand.x, y + rand.y, team, on_spawn);
                ant.set_parent(ent.id);
            }
        }
        return ent;
    }
    else {
        Entity &head = __alloc_mob(sim, mob_id, x, y, team, on_spawn);
        // Segmented packs share NULL team with wildlife — without this, spawn
        // overlap + Unique radius shove the whole FOV ("多节怪一刷新挤压乱飞").
        BitMath::set(head.flags, EntityFlags::kNoFriendlyCollision);
        Entity *curr = &head;
        for (uint32_t i = 1; i < data.attributes.segments; ++i) {
            Entity &seg = __alloc_mob(sim, mob_id, x, y, team, on_spawn);
            seg.add_component(kSegmented);
            BitMath::set(seg.flags, EntityFlags::kNoFriendlyCollision);
            seg.seg_head = curr->id;
            seg.set_angle(curr->get_angle() + frand() * 0.1 - 0.05);
            seg.set_x(curr->get_x() - (curr->get_radius() + seg.get_radius()) * cosf(seg.get_angle()));
            seg.set_y(curr->get_y() - (curr->get_radius() + seg.get_radius()) * sinf(seg.get_angle()));
            curr = &seg;
        }
        return head;
    }
}

Entity &alloc_player(Simulation *sim, EntityID const team) {
    Entity &player = sim->alloc_ent();

    player.add_component(kPhysics);
    player.set_radius(BASE_FLOWER_RADIUS);
    player.friction = DEFAULT_FRICTION;
    player.mass = 1;

    player.add_component(kFlower);

    player.add_component(kRelations);
    player.set_team(team);

    player.add_component(kHealth);
    player.health = player.max_health = BASE_HEALTH;
    player.set_health_ratio(1);
    player.damage = BASE_BODY_DAMAGE;
    player.immunity_ticks = 1.0 * TPS;

    player.add_component(kScore);

    player.add_component(kName);
    player.set_nametag_visible(1);

    player.base_entity = player.id;
    return player;
}

Entity &alloc_petal(Simulation *sim, PetalID::T petal_id, Entity const &parent) {
    DEBUG_ONLY(assert(petal_id < PetalID::kNumPetals);)
    struct PetalData const &petal_data = PETAL_DATA[petal_id];
    Entity &petal = sim->alloc_ent();
    petal.add_component(kPhysics);
    petal.set_x(parent.get_x());
    petal.set_y(parent.get_y());
    petal.set_radius(petal_data.radius);
    if (petal_data.attributes.rotation_style == PetalAttributes::kPassiveRot)
        petal.set_angle(frand() * 2 * M_PI);
    // gardn-master: Moon leaves the petal ring (other petals orbit it instead).
    if (petal_is_moon(petal_id))
        BitMath::set(petal.flags, EntityFlags::kIsDetached);
    petal.mass = petal_data.attributes.mass;
    // Moon uses the same friction as other petals in gardn (no *1.5 drag) so it
    // can seat on its detached path instead of sticking in the flower ring.
    petal.friction = petal_is_moon(petal_id) ? DEFAULT_FRICTION : DEFAULT_FRICTION * 1.5;
    petal.add_component(kRelations);
    petal.set_parent(parent.id);
    petal.set_team(parent.get_team());
    petal.set_color(parent.get_color());
    petal.add_component(kPetal);
    petal.set_petal_id(petal_id);
    petal.set_split_projectile(petal_data.attributes.split_projectile);
    petal.add_component(kHealth);
    petal.health = petal.max_health = petal_data.health;
    petal.damage = petal_data.damage;
    petal.set_health_ratio(1);
    petal.poison_damage = petal_data.attributes.poison_damage;
    petal.burn_damage = petal_data.attributes.burn_damage;
    // Ultra+ wiki armor used √3 while combat stayed ×3 — catch up like heals.
    petal.armor = petal_data.attributes.armor
        * armor_tier_catchup(petal_data.rarity);
    petal.slow_inflict = TPS * petal_data.attributes.slow_inflict_seconds;

    if (parent.id == NULL_ENTITY) petal.base_entity = petal.id;
    else petal.base_entity = parent.id;
    return petal;
}

Entity &alloc_web(Simulation *sim, float radius, Entity const &parent) {
    Entity &web = sim->alloc_ent();
    web.add_component(kPhysics);
    web.set_x(parent.get_x());
    web.set_y(parent.get_y());
    web.set_angle(frand() * 2 * M_PI);
    web.set_radius(radius);
    web.mass = 1.0;
    web.friction = 1.0;
    web.add_component(kRelations);
    web.set_team(parent.get_team());
    web.set_parent(parent.id);
    web.set_color(parent.get_color());
    web.add_component(kWeb);
    entity_set_despawn_tick(web, 10 * TPS);
    return web;
}

Entity &alloc_cpu_camera(Simulation *sim, EntityID const team) {
    Entity &ent = sim->alloc_ent();
    ent.add_component(kCamera);
    ent.add_component(kRelations);

    ent.set_fov(BASE_FOV);
    ent.set_respawn_level(1 + (uint32_t) (frand() * 25));
    // Own team so mobs aggro (NULL_ENTITY would be friendly with wild mobs)
    ent.set_team(team == NULL_ENTITY ? ent.id : team);
    BitMath::set(ent.flags, EntityFlags::kCPUControlled);
    ent.set_color(ColorID::kGray);
    
    //need to auto add to petaltracker
    std::vector<PetalID::T> const inventory = {
        PetalID::kRose, PetalID::kBasic, PetalID::kBasic, 
        PetalID::kRose, PetalID::kBasic, PetalID::kBasic,
        PetalID::kRose, PetalID::kBasic, PetalID::kBasic
    };

    for (uint32_t i = 0; i < loadout_slots_at_level(ent.get_respawn_level()); ++i)
        ent.set_inventory(i, inventory[i]);
    
    if (frand() < 0.001 && PetalTracker::get_count(sim, PetalID::kUniqueBasic) == 0)
        ent.set_inventory(0, PetalID::kUniqueBasic);
    for (uint32_t i = 0; i < loadout_slots_at_level(ent.get_respawn_level()); ++i)
        PetalTracker::add_petal(sim, ent.get_inventory(i));
    return ent;
}

static void _finish_player_spawn(Simulation *sim, Entity &camera, Entity &player, float spawn_x, float spawn_y) {
    camera.set_player(player.id);
    player.set_parent(camera.id);
    player.set_color(camera.get_color());
    camera.set_camera_x(spawn_x);
    camera.set_camera_y(spawn_y);
    player.set_x(spawn_x);
    player.set_y(spawn_y);
    {
        // Keep mid-level XP across death — never drop below the level floor.
        uint32_t floor = level_to_score(camera.get_respawn_level());
        uint32_t score = floor;
        Client *owner = Server::client_by_camera(sim, camera.id);
        if (owner != nullptr && owner->pending_score > score)
            score = owner->pending_score;
        if (owner != nullptr) owner->pending_score = score;
        player.set_score(score);
    }
    {
        uint32_t slots = loadout_slots_at_level(camera.get_respawn_level());
        if (Client *owner = Server::client_by_camera(sim, camera.id))
            slots = Talent::slot_count_from_ranks(owner->talent_ranks);
        player.set_loadout_count(slots);
    }
    player.health = player.max_health = hp_at_level(camera.get_respawn_level());
    player.damage = body_damage_at_level(camera.get_respawn_level());
    // Give the player time to regain control after the spawn packet arrives.
    player.immunity_ticks = 4 * TPS;
    for (uint32_t i = 0; i < player.get_loadout_count(); ++i) {
        PetalID::T id = camera.get_inventory(i);
        LoadoutSlot &slot = player.loadout[i];
        player.set_loadout_ids(i, id);
        slot.update_id(sim, id);
        slot.force_reload();
    }

    for (uint32_t i = player.get_loadout_count(); i < player.get_loadout_count() + MAX_SLOT_COUNT; ++i)
        player.set_loadout_ids(i, camera.get_inventory(i));
    // Clear unused tail so a later slot unlock does not reveal stale IDs.
    for (uint32_t i = player.get_loadout_count() + MAX_SLOT_COUNT; i < 2 * MAX_SLOT_COUNT; ++i)
        player.set_loadout_ids(i, PetalID::kNone);

    //peaceful transfer, no petal tracking needed
    for (uint32_t i = 0; i < MAX_SLOT_COUNT * 2; ++i)
        camera.set_inventory(i, PetalID::kNone);
}

void player_spawn(Simulation *sim, Entity &camera, Entity &player) {
    // Hub spawn: first map entry / Continue without a death site.
    // Death Continue uses player_spawn_at near death (Client.cc); party revive / portals too.
    float spawn_x = MAP_SPAWN_X + (frand() - 0.5f) * 240.f;
    float spawn_y = MAP_SPAWN_Y + (frand() - 0.5f) * 240.f;
    Map::resolve_circle_terrain(sim->map_id, spawn_x, spawn_y, BASE_FLOWER_RADIUS + 8);
    spawn_x = fclamp(spawn_x, BASE_FLOWER_RADIUS, ARENA_WIDTH - BASE_FLOWER_RADIUS);
    spawn_y = fclamp(spawn_y, BASE_FLOWER_RADIUS, ARENA_HEIGHT - BASE_FLOWER_RADIUS);
    _finish_player_spawn(sim, camera, player, spawn_x, spawn_y);
}

void player_spawn_at(Simulation *sim, Entity &camera, Entity &player, float x, float y) {
    _finish_player_spawn(sim, camera, player, x, y);
}
