// Production drop generation, damage claims, AOI, magnet, pickup and cleanup.
// Only transport, account disk writes and notification side effects are stubbed.
#include <Server/Game.cc>
#include <Server/Chat.hh>
#include <Server/DevCheats.hh>
#include <Server/Loot.hh>
#include <Server/Process.hh>
#include <Server/Treasure.hh>
#include <Server/TrainingDummy.hh>
#include <Shared/RarityScale.hh>

#include <cassert>
#include <cstdio>
#include <memory>

static GameInstance *fixture_game;
static uint32_t credited;
static uint32_t killed;
static std::unordered_map<Client *, std::vector<std::vector<uint8_t>>> personal_packets;
uint8_t Server::OUTGOING_PACKET[MAX_PACKET_LEN];
GameInstance *Server::game_for(Simulation *sim) {
    return fixture_game && &fixture_game->simulation == sim ? fixture_game : nullptr;
}
Client *Server::client_by_camera(Simulation *sim, EntityID const &id) {
    auto *game = game_for(sim);
    return game ? game->client_by_camera(id) : nullptr;
}
Client::Client() : game(nullptr) {}
uint8_t Client::alive() {
    return game && game->simulation.ent_alive(camera)
        && game->simulation.ent_alive(game->simulation.get_ent(camera).get_player());
}
uint8_t Client::can_accept_update() { return 1; }
void Client::send_packet(uint8_t const *data, size_t size) {
    personal_packets[this].emplace_back(data, data + size);
}
void AccountStore::capture_nosave(Client *) {}
void AccountStore::note_score(Client *, uint32_t) {}
void client_sync_craft(Client *) {}
void client_credit_petal_pick(Client *, PetalID::T) { ++credited; }
void client_credit_mob_kill(Client *, MobID::T) { ++killed; }
void client_send_life_summary(Client *) {}
uint8_t DevCheats::active(Client const *) { return 0; }
uint8_t Party::same_party(Client *a, Client *b) {
    return a && b && a->party_id && a->party_id == b->party_id;
}
void Party::clear_corpse(Client *) {}
void Party::on_death(Client *, float, float) {}
void Treasure::on_loot_chest_killed(Simulation *, Entity const &) {}
void Chat::broadcast_server(std::string const &, std::string const &, std::string const &,
                            Chat::LiveBcKind, std::string const &, uint8_t) {}

struct Fixture {
    std::unique_ptr<GameInstance> game = std::make_unique<GameInstance>();
    Client alice, bob, stranger;
    Fixture() {
        personal_packets.clear();
        fixture_game = game.get();
        for (Client *c : {&alice, &bob, &stranger}) {
            c->game = game.get(); c->verified = 1;
            auto &cam = sim().alloc_ent();
            cam.add_component(kCamera); cam.add_component(kRelations);
            cam.set_team(cam.id); cam.set_fov(BASE_FOV);
            cam.set_camera_x(5000); cam.set_camera_y(5000);
            c->camera = cam.id;
            auto &p = alloc_player(&sim(), cam.id);
            p.set_parent(cam.id); p.set_loadout_count(5);
            p.set_x(5000); p.set_y(5000); p.immunity_ticks = 0;
            cam.set_player(p.id);
            game->clients.insert(c);
        }
        alice.party_id = bob.party_id = 7; alice.party_leader = 1;
        sim().get_ent(bob.camera).set_team(alice.camera);
        flower(bob).set_team(alice.camera);
    }
    Simulation &sim() { return game->simulation; }
    Entity &flower(Client &c) { return sim().get_ent(sim().get_ent(c.camera).get_player()); }
    Entity &mob() {
        auto &m = alloc_mob(&sim(), MobID::kLadybug, 5000, 5000, NULL_ENTITY);
        m.health = m.max_health = 100; m.armor = 0; m.immunity_ticks = 0;
        return m;
    }
    auto drops() {
        std::vector<EntityID> out;
        sim().refresh_active_entities();
        sim().for_each<kDrop>([&](Simulation *, Entity &e) { out.push_back(e.id); });
        return out;
    }
    void hash() {
        sim().refresh_active_entities(); sim().spatial_hash.refresh(ARENA_WIDTH, ARENA_HEIGHT);
        sim().for_each<kPhysics>([&](Simulation *, Entity &e) { sim().spatial_hash.insert(e); });
    }
};

static void party_copies_and_visibility() {
    Fixture f;
    auto &m = f.mob();
    inflict_damage(&f.sim(), f.flower(f.alice).id, m.id, 3, DamageType::kSplash);
    inflict_damage(&f.sim(), f.flower(f.bob).id, m.id, 3, DamageType::kSplash);
    inflict_damage(&f.sim(), f.flower(f.stranger).id, m.id, 1, DamageType::kSplash);
    m.health = 0; entity_on_death(&f.sim(), m);
    auto drops = f.drops();
    assert(!drops.empty());
    EntityID alice_drop, bob_drop;
    for (auto id : drops) {
        auto &d = f.sim().get_ent(id);
        assert(d.get_team() == f.alice.camera || d.get_team() == f.bob.camera);
        if (d.get_team() == f.alice.camera) alice_drop = id;
        if (d.get_team() == f.bob.camera) bob_drop = id;
    }
    assert(!alice_drop.null() && !bob_drop.null());
    f.hash();
    for (Client *c : {&f.alice, &f.bob, &f.stranger}) {
        _update_client(&f.sim(), c);
        for (auto id : drops)
            assert(c->in_view.contains(id) == (f.sim().get_ent(id).get_team() == c->camera));
    }
    auto &a = f.sim().get_ent(alice_drop); auto &b = f.sim().get_ent(bob_drop);
    auto const bank = f.bob.craft_bank;
    pickup_drop(&f.sim(), f.flower(f.bob), a);
    assert(!a.pending_delete && bank == f.bob.craft_bank);
    pickup_drop(&f.sim(), f.flower(f.bob), b);
    assert(b.pending_delete && !a.pending_delete);

    // A third player must never inherit piles after their owner leaves the map.
    EntityID owner = f.alice.camera;
    f.sim()._delete_ent(owner);
    auto &replacement = f.sim().alloc_ent();
    replacement.add_component(kCamera);
    assert(replacement.id.id == owner.id && !(replacement.id == owner));
    assert(!Loot::can_access_drop(a, replacement.id));
    entity_clear_references(&f.sim(), a);
    assert(a.pending_delete && a.get_team() == owner);
    f.bob.in_view.insert(a.id); // simulate an older snapshot with a leaked pile
    _update_client(&f.sim(), &f.bob);
    assert(!f.bob.in_view.contains(a.id));
    pickup_drop(&f.sim(), f.flower(f.bob), a);
    assert(a.get_team() == owner);
}

static void magnet_is_personal() {
    Fixture f;
    f.bob.talent_ranks[TalentID::kMagnet] = 9;
    auto &a = alloc_drop(&f.sim(), PetalID::kRose, f.alice.camera);
    auto &b = alloc_drop(&f.sim(), PetalID::kRose, f.bob.camera);
    a.set_x(5300); a.set_y(5000); b.set_x(5300); b.set_y(5000);
    f.hash();
    tick_player_behavior(&f.sim(), f.flower(f.bob));
    assert(a.acceleration.x == 0 && a.acceleration.y == 0 && !a.pending_delete);
    assert(b.acceleration.x < 0 && !b.pending_delete);
    f.bob.party_id = 0;
    assert(!Loot::can_access_drop(a, f.bob.camera) && Loot::can_access_drop(b, f.bob.camera));
}

static void downed_mates_and_no_public_fallback() {
    Fixture f;
    f.sim().get_ent(f.bob.camera).set_player(NULL_ENTITY);
    auto &m = f.mob();
    inflict_damage(&f.sim(), f.flower(f.alice).id, m.id, 100, DamageType::kSplash);
    entity_on_death(&f.sim(), m);
    bool has_bob = false;
    for (auto id : f.drops()) {
        auto const owner = f.sim().get_ent(id).get_team();
        assert(!owner.null()); has_bob |= owner == f.bob.camera;
    }
    assert(has_bob);
    auto before = f.drops().size();
    auto &unclaimed = f.mob(); unclaimed.health = 0;
    entity_on_death(&f.sim(), unclaimed);
    assert(f.drops().size() == before);
}

static void summon_and_reflect_claims() {
    Fixture f; auto &m = f.mob(); auto &p = f.flower(f.bob);
    auto &petal = alloc_petal(&f.sim(), PetalID::kBasic, p);
    auto &summon = alloc_mob(&f.sim(), MobID::kBabyAnt, 5000, 5000, f.alice.camera);
    summon.set_parent(p.id);
    auto &projectile = alloc_petal(&f.sim(), PetalID::kBasic, summon);
    for (auto source : {petal.id, summon.id, projectile.id})
        inflict_damage(&f.sim(), source, m.id, 10, DamageType::kSplash);
    inflict_damage(&f.sim(), p.id, m.id, 10, DamageType::kReflect);
    assert(m.loot_claim_n == 1 && m.loot_claim_teams[0] == f.bob.camera);
    assert(m.loot_claim_damage[0] == 40);
}

static void full_inventory_keeps_remainder() {
    Fixture f; auto &p = f.flower(f.alice);
    for (uint8_t i = 0; i < 15; ++i) {
        p.set_loadout_ids(i, PetalID::kBasic);
        PetalTracker::add_petal(&f.sim(), PetalID::kBasic);
    }
    auto &d = alloc_drop(&f.sim(), PetalID::kRose, f.alice.camera, 5);
    uint32_t cap = Recipe::bank_stack_cap(PetalID::kRose);
    f.alice.craft_bank[PetalID::kRose] = cap - 2;
    credited = 0;
    pickup_drop(&f.sim(), p, d);
    assert(!d.pending_delete && d.get_drop_count() == 3);
    assert(f.alice.craft_bank[PetalID::kRose] == cap && credited == 2);
    pickup_drop(&f.sim(), p, d);
    assert(!d.pending_delete && d.get_drop_count() == 3 && credited == 2);
    p.set_loadout_ids(0, PetalID::kNone);
    PetalTracker::remove_petal(&f.sim(), PetalID::kBasic);
    p.set_loadout_ids(5, PetalID::kNone);
    PetalTracker::remove_petal(&f.sim(), PetalID::kBasic);
    pickup_drop(&f.sim(), p, d);
    assert(!d.pending_delete && d.get_drop_count() == 3 && credited == 2);
    assert(p.get_loadout_ids(0) == PetalID::kNone && p.get_loadout_ids(5) == PetalID::kNone);
    f.alice.craft_bank[PetalID::kRose] -= 3;
    pickup_drop(&f.sim(), p, d);
    assert(d.pending_delete && credited == 5);
    assert(PetalTracker::get_count(&f.sim(), PetalID::kRose) == 0);
    pickup_drop(&f.sim(), p, d);
    assert(credited == 5); // duplicate callbacks cannot collect twice
}

static void empty_slots_stay_empty_on_pickup() {
    Fixture f; auto &p = f.flower(f.alice);
    std::array<PetalID::T, 2 * MAX_SLOT_COUNT> original;
    for (uint8_t i = 0; i < original.size(); ++i) original[i] = p.get_loadout_ids(i);
    credited = 0;
    for (auto id : {PetalID::kRose, PetalID::kLight, PetalID::kCoin}) {
        auto &drop = alloc_drop(&f.sim(), id, f.alice.camera, 7);
        uint32_t before = f.alice.craft_bank[id];
        pickup_drop(&f.sim(), p, drop);
        assert(drop.pending_delete && f.alice.craft_bank[id] == before + 7);
        for (uint8_t i = 0; i < original.size(); ++i) assert(p.get_loadout_ids(i) == original[i]);
        assert(PetalTracker::get_count(&f.sim(), id) == 0);
    }
    assert(credited == 21);
}

static void training_dummies_are_personal() {
    Fixture f;
    f.sim().map_id = MapID::kDesert;
    TrainingDummy::ensure(&f.sim());
    for (auto id : f.sim().training_dummies) assert(id.null());
    f.sim().map_id = MapID::kGarden;
    TrainingDummy::ensure(&f.sim());
    auto const ids = f.sim().training_dummies;
    TrainingDummy::ensure(&f.sim());
    assert(ids == f.sim().training_dummies);
    float previous_radius = 0;
    for (uint8_t rarity = 0; rarity < RarityID::kNumRarities; ++rarity) {
        Entity &dummy = f.sim().get_ent(ids[rarity]);
        assert(TrainingDummy::is_dummy(dummy) && dummy.get_rarity() == rarity);
        assert(dummy.get_radius() > previous_radius);
        assert(std::fabs(dummy.get_radius() - 44.f * mob_rarity_radius_mult(rarity)) < .01f);
        previous_radius = dummy.get_radius();
        assert(Map::in_spawn_sanctuary(dummy.get_x(), dummy.get_y()));
        float dx = (dummy.get_x() - MAP_SPAWN_X) / 960.f;
        float dy = (dummy.get_y() - MAP_SPAWN_Y) / 720.f;
        assert(std::fabs(dx * dx + dy * dy - 1.f) < .001f);
        assert(!Map::circle_hits_impassable(MapID::kGarden, dummy.get_x(), dummy.get_y(), dummy.get_radius()));
        assert(dummy.damage == 0 && dummy.score_reward == 0);
        assert(BitMath::at(dummy.flags, EntityFlags::kNoDrops));
        dummy.velocity.set(1000, -1000); dummy.acceleration.set(10, 10);
        dummy.collision_velocity.set(50, 50);
        tick_entity_motion(&f.sim(), dummy);
        assert(dummy.get_x() == dummy.home_x && dummy.get_y() == dummy.home_y);
        assert(dummy.velocity.x == 0 && dummy.acceleration.y == 0);
    }
    Entity &dummy = f.sim().get_ent(ids[0]);
    auto &a = f.flower(f.alice); auto &b = f.flower(f.bob);
    inflict_damage(&f.sim(), a.id, dummy.id, 100, DamageType::kSplash);
    inflict_damage(&f.sim(), b.id, dummy.id, 200, DamageType::kSplash);
    assert(f.alice.training_stats[0].damage == 100 && f.bob.training_stats[0].damage == 200);
    assert(f.stranger.training_stats[0].damage == 0);
    assert(dummy.get_name() == "Training Dummy" && dummy.health == dummy.max_health);
    assert(dummy.loot_claim_n == 0 && dummy.poison_ticks == 0);
    TrainingDummy::tick_client(&f.sim(), &f.alice);
    assert(personal_packets[&f.bob].empty() && personal_packets[&f.stranger].empty());
    auto const &packet = personal_packets[&f.alice].back();
    Reader r(packet.data()); assert(r.read<uint8_t>() == Clientbound::kTrainingStats);
    assert(r.read<std::string>() == "100.0");
    assert(r.read<std::string>() == "100.0");
    assert(r.read<std::string>() == "100.0");

    // Poison and burn must not share duration, damage, or ownership.
    a.poison_damage = {80, 1}; a.burn_damage = {120, 1};
    b.poison_damage = {200, 1};
    inflict_damage(&f.sim(), a.id, dummy.id, 20, DamageType::kContact);
    inflict_damage(&f.sim(), b.id, dummy.id, 50, DamageType::kContact);
    for (uint32_t tick = 1; tick <= TPS; ++tick) {
        f.sim().tick_index = tick;
        TrainingDummy::tick_client(&f.sim(), &f.alice);
        TrainingDummy::tick_client(&f.sim(), &f.bob);
    }
    assert(std::fabs(f.alice.training_stats[0].damage - 320) < .01);
    assert(std::fabs(f.bob.training_stats[0].damage - 450) < .01);
    assert(dummy.poison_ticks == 0 && dummy.burn_ticks == 0);
    // An equipment edit resets only the editor's round, including pending DOTs.
    a.set_loadout_ids(0, PetalID::kLight);
    TrainingDummy::tick_client(&f.sim(), &f.alice);
    assert(f.alice.training_stats[0].damage == 0 && f.bob.training_stats[0].damage == 450);
    TrainingDummy::reset(&f.bob);
    assert(f.bob.training_stats[0].damage == 0);
    // Full overkill and summon/reflection damage count without killing the target.
    auto &petal = alloc_petal(&f.sim(), PetalID::kBasic, a);
    inflict_damage(&f.sim(), petal.id, dummy.id, 1000000000.f, DamageType::kSplash);
    assert(f.alice.training_stats[0].damage == 1000000000.0 && !dummy.pending_delete);
    assert(dummy.health == dummy.max_health && dummy.get_health_ratio() == 1);
    f.sim().tick_index += 3 * TPS;
    TrainingDummy::tick_client(&f.sim(), &f.alice);
    assert(f.alice.training_stats[0].damage == 0);
    killed = 0; credited = 0;
    entity_on_death(&f.sim(), dummy);
    assert(killed == 0 && credited == 0 && f.drops().empty());
    f.sim()._delete_ent(ids[0]);
    TrainingDummy::ensure(&f.sim());
    assert(f.sim().ent_alive(f.sim().training_dummies[0]));
    assert(!(ids[0] == f.sim().training_dummies[0]));
    std::puts("PASS: 9 growing dummy tiers, anchors, personal damage/DPS packets and DOTs, overkill, equip/manual/idle resets, no rewards, replacement");
}

int main() {
    party_copies_and_visibility();
    magnet_is_personal();
    downed_mates_and_no_public_fallback();
    summon_and_reflect_claims();
    full_inventory_keeps_remainder();
    empty_slots_stay_empty_on_pickup();
    training_dummies_are_personal();
    std::puts("PASS: independent party loot, private AOI/magnet/pickup, orphan cleanup, "
              "downed mates, summon/reflection claims, no auto-equip, full stacks and duplicate pickup");
}
