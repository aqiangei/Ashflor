// Reuse the real simulation fixture and transport-only stubs from the loot suite.
#define main private_loot_regression_main
#include "test_private_loot.cc"
#undef main
#include <Server/PetalMechanics.hh>
#include <cstring>

static PetalID::T petal(char const *name, uint8_t rarity = RarityID::kCommon) {
    for (PetalID::T id = 1; id < PetalID::kNumPetals; ++id)
        if (!std::strcmp(PETAL_DATA[id].name, name) && PETAL_DATA[id].rarity == rarity) return id;
    return PetalID::kNone;
}
static void equip(Fixture &f, std::initializer_list<PetalID::T> ids) {
    auto &p = f.flower(f.alice);
    for (uint32_t i = 0; i < MAX_SLOT_COUNT; ++i) {
        p.loadout[i].update_id(&f.sim(), PetalID::kNone);
        p.set_loadout_ids(i, PetalID::kNone);
    }
    p.set_loadout_count((uint8_t)ids.size());
    uint32_t i = 0;
    for (auto id : ids) p.set_loadout_ids(i++, id);
    tick_player_behavior(&f.sim(), p);
}
static void steps(Fixture &f, uint32_t n) {
    auto &p = f.flower(f.alice);
    while (n--) {
        ++f.sim().tick_index;
        if (p.immunity_ticks) --p.immunity_ticks;
        tick_player_behavior(&f.sim(), p);
    }
}

static void hornet_precision_and_collision() {
    Fixture f; auto &p = f.flower(f.alice);
    for (uint8_t r = 0; r < RarityID::kNumRarities; ++r) {
        auto &m = alloc_mob(&f.sim(), MobID::kHornet, 5000, 5000, NULL_ENTITY);
        m.max_health *= mob_spawn_threat_mult(RarityID::kCommon, r);
        m.health = m.max_health; m.set_rarity(r);
        m.damage *= rarity_combat_mult(r);
        auto &b = alloc_petal(&f.sim(), petal("Basic"), p);
        b.set_x(5000); b.set_y(5000);
        double before = m.health;
        on_collide(&f.sim(), m, b);
        assert(std::fabs((before - m.health) - b.damage) < .001);
        // Fractional DoT also accumulates on billion-HP mobs.
        before = m.health;
        for (int hit = 0; hit < 100; ++hit)
            inflict_damage(&f.sim(), p.id, m.id, .01f, DamageType::kPoison);
        assert(std::fabs(before - m.health - 1.) < .001);
    }
    std::puts("PASS hornet_precision: contact and fractional DOT damage across all 9 tiers");
}

static void spawn_caps_and_sanctuary() {
    Fixture f;
    for (uint8_t map = 0; map < MapID::kNumMaps; ++map) {
        f.sim().map_id = map;
        auto const &zones = map_zones(map);
        for (uint32_t z = 0; z < zones.size(); ++z) {
            auto before = f.sim().entity_count();
            Map::_spawn_mob_at(&f.sim(), z, MobID::kHornet, MAP_SPAWN_X, MAP_SPAWN_Y, RarityID::kUnique);
            assert(f.sim().entity_count() == before);
            Map::_spawn_mob_at(&f.sim(), z, MobID::kSquare, 4000.f, 4000.f, RarityID::kUnique);
            f.sim().refresh_active_entities();
            f.sim().for_each<kMob>([&](Simulation *, Entity &m) {
                if (m.zone == z && m.get_mob_id() == MobID::kSquare && !m.pending_delete) {
                    assert(m.get_rarity() <= zone_spawn_rarity_cap(zones[z].difficulty));
                    f.sim().request_delete(m.id);
                }
            });
        }
    }
    f.sim().map_id = MapID::kGarden;
    auto &m = alloc_mob(&f.sim(), MobID::kHornet, MAP_SPAWN_X, MAP_SPAWN_Y, NULL_ENTITY);
    m.home_x = MAP_SPAWN_X + 3000; m.home_y = MAP_SPAWN_Y;
    tick_entity_motion(&f.sim(), m);
    assert(!Map::in_spawn_plaza(m.get_x(), m.get_y()));
    for (int n = 0; n < 1000; ++n) {
        assert(pick_zone_spawn_rarity(0) == RarityID::kCommon);
        assert(pick_zone_spawn_rarity(1) <= RarityID::kUnusual);
    }
    std::puts("PASS spawn_safety: every zone in all 9 maps, direct-spawn caps, hub and wildlife exclusion");
}

static void mimic_formations_and_passives() {
    Fixture f; auto &p = f.flower(f.alice);
    auto mimic = petal("Mimic", RarityID::kRare);
    equip(f, {petal("Sand", RarityID::kRare), mimic});
    assert(effective_petal_id(p, 1) == petal("Sand", RarityID::kRare));
    assert(p.loadout[1].size() == p.loadout[0].size());
    steps(f, 200);
    auto old = p.loadout[1].petals[0].ent_id;
    assert(f.sim().ent_alive(old));
    p.set_loadout_ids(0, petal("Rose", RarityID::kRare));
    tick_player_behavior(&f.sim(), p);
    assert(!f.sim().ent_alive(old));
    equip(f, {petal("Magnet", RarityID::kRare), mimic});
    assert(effective_petal_id(p, 1) == petal("Magnet", RarityID::kRare));
    assert(p.get_equip_flags() & (1 << EquipmentFlags::kMagnet));
    equip(f, {petal("Mimic"), petal("Mimic")});
    assert(effective_petal_id(p, 0) == PetalID::kNone);
    equip(f, {petal("Corruption"), petal("Mimic")});
    assert(effective_petal_id(p, 1) == PetalID::kNone);
    // Partial family: use a lower existing rung when the requested one is absent.
    bool checked = false;
    for (PetalID::T id = 1; id < PetalID::kNumPetals && !checked; ++id) {
        if (Recipe::is_bag_only(id) || PETAL_DATA[id].rarity >= RarityID::kUnique) continue;
        for (uint8_t r = PETAL_DATA[id].rarity + 1; r < RarityID::kNumRarities; ++r) {
            if (Craft::petal_at_rarity(id, r) != PetalID::kNone) continue;
            equip(f, {id, petal("Mimic", r)});
            assert(effective_petal_id(p, 1) != PetalID::kNone);
            assert(PETAL_DATA[effective_petal_id(p, 1)].rarity <= r);
            checked = true; break;
        }
    }
    assert(checked);
    std::puts("PASS mimic: formations, equipment, target edits, exclusions and missing-rung fallback");
}

static void defensive_lifetimes() {
    Fixture f; auto &p = f.flower(f.alice); auto &m = f.mob();
    equip(f, {petal("Root")});
    steps(f, 3 * TPS); assert(p.armor > 0);
    equip(f, {petal("Basic")}); assert(p.armor == 0);
    equip(f, {petal("Shovel")});
    p.input = 1 << InputFlags::kDefending;
    steps(f, 1);
    assert(p.get_effect_flags() & FlowerEffects::kBurrow);
    assert(p.get_ability_cooldown() > 0);
    steps(f, TPS); assert(p.immunity_ticks == 0 && p.burrow_ticks == 0);
    steps(f, 3 * TPS); assert(p.immunity_ticks == 0);
    p.input = 0; steps(f, 1);
    equip(f, {petal("Wax")}); steps(f, 31 * TPS);
    auto &shot = alloc_petal(&f.sim(), petal("Missile"), m);
    entity_set_despawn_tick(shot, TPS);
    auto hp = p.health;
    inflict_damage(&f.sim(), shot.id, p.id, 20, DamageType::kContact);
    assert(p.health == hp && shot.pending_delete && p.loadout[0].effect_charges == 0);
    steps(f, 1); assert(p.loadout[0].effect_charges == 0);
    equip(f, {petal("Nazar Amulet")}); steps(f, 61 * TPS);
    for (int i = 0; i < 3; ++i) inflict_damage(&f.sim(), m.id, p.id, 1, DamageType::kSplash);
    assert(p.loadout[0].effect_charges == 0);
    hp = p.health;
    inflict_damage(&f.sim(), m.id, p.id, 1, DamageType::kSplash);
    assert(std::fabs(hp - p.health - 1.) < .001);
    equip(f, {petal("Sponge"), petal("Bandage")});
    p.health = p.max_health;
    inflict_damage(&f.sim(), m.id, p.id, 1000, DamageType::kSplash);
    auto deadline = p.pending_damage_ticks;
    steps(f, 1);
    inflict_damage(&f.sim(), m.id, p.id, 10, DamageType::kSplash);
    assert(p.pending_damage_ticks == deadline - 1);
    steps(f, deadline - 1);
    assert(p.health == 1 && p.death_delay_ticks > 0 && p.pending_damage == 0);
    steps(f, 3 * TPS); assert(p.health == 0 && p.death_delay_used);
    std::puts("PASS defense: Root removal, finite Shovel/Wax/Nazar, fixed Sponge deadline and Bandage settlement");
}

static void armor_phase_ankh_mana() {
    Fixture f; auto &p = f.flower(f.alice); auto &m = f.mob();
    auto &bur = alloc_petal(&f.sim(), petal("Bur"), p);
    m.armor = 10; m.health = m.max_health = 10000;
    inflict_damage(&f.sim(), bur.id, m.id, 100, DamageType::kContact);
    assert(m.armor < 10 && m.armor_shred_ticks > 0);
    for (uint32_t i = 0; i < 4 * TPS; ++i) tick_health_behavior(&f.sim(), m);
    assert(std::fabs(m.armor - 10) < .001);
    auto &glass = alloc_petal(&f.sim(), petal("Glass"), p);
    auto hp = glass.health;
    inflict_damage(&f.sim(), m.id, glass.id, 100, DamageType::kContact);
    assert(glass.health == hp);
    auto &ankh = alloc_petal(&f.sim(), petal("Ankh"), p);
    ankh.set_x(5200); ankh.set_y(5000);
    ankh.secondary_reload = TPS; p.input = 1 << InputFlags::kDefending;
    tick_petal_behavior(&f.sim(), ankh);
    assert(BitMath::at(ankh.flags, EntityFlags::kIsDetached));
    p.set_x(6000); entity_on_death(&f.sim(), ankh);
    assert(std::fabs(p.get_x() - 5200) < 1);
    equip(f, {petal("Mana Orb"), petal("Magic Eye")});
    steps(f, 2 * TPS); assert(p.mana > 0);
    auto &eye = alloc_petal(&f.sim(), petal("Magic Eye"), p);
    p.input = 1 << InputFlags::kAttacking;
    auto mana = p.mana; tick_petal_behavior(&f.sim(), eye);
    assert(p.mana == mana); // no target: no mana spent
    p.set_x(5000); p.set_y(5000); f.hash();
    auto before = m.health;
    tick_petal_behavior(&f.sim(), eye);
    assert(p.mana < mana && m.health < before);
    std::puts("PASS combat_effects: temporary armor strip, Glass vs mob, planted Ankh and targeted mana spending");
}

static void aura_projectiles_and_summons() {
    Fixture f; auto &p = f.flower(f.alice);
    auto &enemy = f.mob(); enemy.health = enemy.max_health = 1e9;
    p.input = 1 << InputFlags::kAttacking;
    for (char const *name : {"Pollen Sandstorm", "World Tree Seed", "Chrono Vortex"}) {
        auto id = petal(name, RarityID::kUnique); assert(id != PetalID::kNone);
        auto &aura = alloc_petal(&f.sim(), id, p);
        aura.set_x(5000); aura.set_y(5000); aura.secondary_reload = 65000;
        tick_petal_behavior(&f.sim(), aura);
        assert(BitMath::at(aura.flags, EntityFlags::kIsDespawning));
        p.health = p.max_health * .5; auto before = p.health;
        f.hash(); tick_petal_behavior(&f.sim(), aura);
        if (PETAL_DATA[id].attributes.aura_heal > 0) assert(p.health > before);
        if (PETAL_DATA[id].attributes.aura_slow_seconds > 0) assert(enemy.slow_ticks > 0);
    }
    auto &web = alloc_petal(&f.sim(), petal("Electric Web"), p);
    web.set_x(5000); web.set_y(5000); web.secondary_reload = 65000;
    tick_petal_behavior(&f.sim(), web);
    assert(BitMath::at(web.flags, EntityFlags::kIsDetached));
    web.secondary_reload = 65000; auto hp = enemy.health;
    f.hash(); tick_petal_behavior(&f.sim(), web);
    assert(enemy.health < hp && enemy.slow_ticks > 0);
    for (char const *name : {"Sand Leaf Dart", "Pollen", "Grapes", "Peas"}) {
        auto id = petal(name, RarityID::kUnique);
        auto &shot = alloc_petal(&f.sim(), id, p);
        shot.secondary_reload = 65000; tick_petal_behavior(&f.sim(), shot);
        assert(BitMath::at(shot.flags, EntityFlags::kIsDespawning));
    }
    equip(f, {petal("Soldier Egg")});
    p.loadout[0].force_reload(); steps(f, TPS + 1);
    auto egg_id = p.loadout[0].petals[0].ent_id;
    assert(f.sim().ent_alive(egg_id));
    f.sim().get_ent(egg_id).secondary_reload = 65000;
    tick_player_behavior(&f.sim(), p);
    auto &summon = f.sim().get_ent(p.loadout[0].petals[0].ent_id);
    assert(summon.has_component(kMob) && summon.get_parent() == p.id);
    assert(summon.get_team() == p.get_team() && BitMath::at(summon.flags, EntityFlags::kNoDrops));
    std::puts("PASS active_skills: Unique planted auras, Electric Web damage+slow, ladder projectiles and owned egg hatching");
}

static void fang_and_poo_gameplay() {
    Fixture f; auto &p = f.flower(f.alice);
    auto &m = alloc_mob(&f.sim(), MobID::kHornet, 5000, 5000, NULL_ENTITY);
    m.health = m.max_health = 1e9; m.damage = 50;
    for (uint8_t r = 0; r < RarityID::kNumRarities; ++r) {
        auto id = petal("Fang", r); assert(id != PetalID::kNone);
        p.max_health = p.health = 1e7; p.health *= .5;
        auto &fang = alloc_petal(&f.sim(), id, p);
        fang.set_x(5000); fang.set_y(5000);
        double hp = p.health;
        on_collide(&f.sim(), m, fang); // mob-first, including the fragile Fang breaking
        double expected = fang.damage * PETAL_DATA[id].attributes.on_hit_heal_ratio;
        assert(p.health > hp);
        assert(std::fabs(p.health - hp - expected) < .1);
    }
    // Test detection and the level-60 forced-aggro path with exactly one target.
    f.flower(f.bob).set_x(9000); f.flower(f.stranger).set_x(9000);
    p.set_x(5000); p.set_y(5000); p.set_score(level_to_score(100));
    m.set_x(5700); m.set_y(5000); m.detection_radius = 500;
    equip(f, {petal("Basic")}); f.hash();
    m.target = NULL_ENTITY; tick_curse_behavior(&f.sim());
    assert(m.target == p.id);
    equip(f, {petal("Poo", RarityID::kUnique)});
    assert(flower_aggro_range_mult(p) < .1f);
    m.target = NULL_ENTITY; tick_curse_behavior(&f.sim());
    assert(m.target == NULL_ENTITY);
    // Regular acquisition: 100 units is outside Unique Poo's 37.5-unit radius.
    m.set_x(5100); m.lifetime = m.id.id; f.hash();
    assert(find_nearest_enemy(&f.sim(), m, 500) == NULL_ENTITY);
    equip(f, {petal("Basic")}); f.hash();
    assert(find_nearest_enemy(&f.sim(), m, 500) == p.id);
    std::puts("PASS fang_poo: all 9 Fang tiers heal on real collision, Poo suppresses detection and level-60 curse");
}

static void registry_dispatch_smoke() {
    Fixture f; auto &p = f.flower(f.alice);
    uint32_t checked = 0;
    for (PetalID::T id = 1; id < PetalID::kNumPetals; ++id) {
        if (Recipe::is_bag_only(id) || PETAL_DATA[id].count == 0) continue;
        p.input = 0; p.set_x(5000); p.set_y(5000);
        equip(f, {id, petal("Mana Orb")});
        p.mana = 1e6;
        p.loadout[0].already_spawned = 1;
        for (uint32_t i = 0; i < MAX_PETALS_IN_CLUMP; ++i)
            p.loadout[0].petals[i].reload = 65000;
        tick_player_behavior(&f.sim(), p);
        auto effective = effective_petal_id(p, 0);
        assert(effective != PetalID::kNone);
        auto const &a = PETAL_DATA[effective].attributes;
        if (a.equipment == EquipmentFlags::kNone) {
            assert(f.sim().ent_alive(p.loadout[0].petals[0].ent_id));
            auto &live = f.sim().get_ent(p.loadout[0].petals[0].ent_id);
            assert(live.get_petal_id() == effective);
            live.secondary_reload = 65000;
            live.set_x(5080); live.set_y(5000);
            p.input = 1 << InputFlags::kAttacking;
            tick_petal_behavior(&f.sim(), live);
            if (!live.pending_delete) {
                p.input = 1 << InputFlags::kDefending;
                tick_petal_behavior(&f.sim(), live);
            }
        }
        assert(std::isfinite(p.health) && std::isfinite(p.max_health));
        ++checked;
        // Discard case-owned entities, preserving the connected players/cameras.
        f.sim().refresh_active_entities();
        std::vector<EntityID> cleanup;
        f.sim().for_each<kPetal>([&](Simulation *, Entity &e){ cleanup.push_back(e.id); });
        f.sim().for_each<kMob>([&](Simulation *, Entity &e){ cleanup.push_back(e.id); });
        for (auto eid : cleanup) if (f.sim().ent_exists(eid)) f.sim()._delete_ent(eid);
    }
    std::printf("PASS registry_smoke: %u combat petal IDs exercise equip, spawn, attack and defend dispatch\n", checked);
}

static void health_status_wire_precision() {
    Fixture f;
    auto &e = f.sim().alloc_ent();
    e.add_component(kHealth);
    e.set_health_ratio(.9999f); e.set_hit_damage(.35f); e.set_damaged(1);
    uint8_t packet[128] = {};
    Writer writer(packet); e.write(&writer, 1);
    uint8_t *at = packet;
    auto number = [&]() {
        uint64_t value = 0; int shift = 0;
        while (true) {
            assert(at < writer.at);
            auto byte = *at++; value |= (uint64_t)(byte & 127) << shift;
            if (!(byte & 128)) return value;
            shift += 7; assert(shift < 64);
        }
    };
    assert(number() == (1u << kHealth)); assert(number() == 0);
    double ratio = number() / 65536.;
    double damage = number() / 65536.;
    assert(std::fabs(ratio - .9999) < .00002 && ratio < 1.);
    assert(std::fabs(damage - .35) < .00002);
    assert(number() == 1 && at == writer.at);
    std::puts("PASS status_wire: sub-percent health changes and fractional damage survive serialization");
}

int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    hornet_precision_and_collision();
    health_status_wire_precision();
    fang_and_poo_gameplay();
    spawn_caps_and_sanctuary();
    mimic_formations_and_passives();
    defensive_lifetimes();
    armor_phase_ankh_mana();
    aura_projectiles_and_summons();
    registry_dispatch_smoke();
    private_loot_regression_main();
    std::puts("PASS mechanics_suite");
}
