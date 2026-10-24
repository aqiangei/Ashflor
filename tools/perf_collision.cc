// Focused collision behavior regression using the production collision handler.
// Account/network side effects and damage delivery are isolated below; entity,
// geometry, loot gating, simulation allocation and deletion are real code.
// From repo root:
// clang++ -std=c++20 -O3 -DSERVERSIDE=1 -DWASM_SERVER=1 -Wno-narrowing \
//   -Wno-reorder-init-list -ffunction-sections -fdata-sections -I. \
//   tools/perf_collision.cc Shared/Entity.cc Shared/EntityDef.cc \
//   Shared/Simulation.cc Shared/Arena.cc Shared/Recipe.cc Shared/StaticData.cc \
//   Server/SpatialHashCanonical.cc Server/PetalTracker.cc Helpers/Vector.cc Helpers/Math.cc \
//   -Wl,--gc-sections -o /tmp/perf-collision && /tmp/perf-collision
// Repeat with -ffast-math for the production release floating-point settings.
#include <Server/Process/Collision.cc>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <memory>
#include <random>

static unsigned damage_calls = 0;
static unsigned checks = 0;
static bool players = false;
bool is_player_faction(Simulation *, Entity const &) { return players; }
void inflict_damage(Simulation *, EntityID const, EntityID const, float, uint8_t) {
    ++damage_calls;
}
static Client *pickup_client;
Client::Client() : game(nullptr) {}
Client *Server::client_by_camera(Simulation *, EntityID const &id) {
    return pickup_client && pickup_client->camera == id ? pickup_client : nullptr;
}
void AccountStore::capture_nosave(Client *) {}
void client_sync_craft(Client *) {}
void client_credit_petal_pick(Client *, PetalID::T) {}

static void check(bool result, char const *message) {
    ++checks;
    if (result) return;
    std::fprintf(stderr, "FAIL: %s\n", message);
    std::exit(1);
}
static void reset(Entity &e, Components component, float x, float y, EntityID team) {
    e.init();
    e.add_component(kPhysics);
    e.add_component(kRelations);
    e.add_component(component);
    e.set_x(x); e.set_y(y); e.set_radius(10.f); e.set_team(team);
    e.health = 100;
    e.damage = 1;
}
static bool finite_motion(Entity const &e) {
    return is_finite(e.velocity.x) && is_finite(e.velocity.y)
        && is_finite(e.collision_velocity.x) && is_finite(e.collision_velocity.y);
}
static bool no_motion(Entity const &e) {
    return e.velocity.x == 0.f && e.velocity.y == 0.f
        && e.collision_velocity.x == 0.f && e.collision_velocity.y == 0.f;
}

int main() {
    auto sim = std::make_unique<Simulation>();
    Entity &camera = sim->alloc_ent();
    camera.add_component(kCamera);
    Client collector;
    collector.camera = camera.id;
    pickup_client = &collector;
    Entity &a = sim->alloc_ent();
    Entity &b = sim->alloc_ent();
    EntityID team(500, 1), enemy(501, 1);

    // The broad-phase may report diagonal circles that do not touch.
    reset(a, kMob, 0, 0, team); reset(b, kMob, 19, 19, enemy);
    a.add_component(kHealth); b.add_component(kHealth);
    on_collide(sim.get(), a, b);
    check(no_motion(a) && no_motion(b) && damage_calls == 0, "diagonal non-contact rejected");

    reset(a, kMob, 0, 0, team); reset(b, kMob, 20, 0, enemy);
    a.add_component(kHealth); b.add_component(kHealth);
    on_collide(sim.get(), a, b);
    check(damage_calls == 2, "touching circles deliver contact damage");
    reset(a, kMob, 0, 0, team); reset(b, kMob, 15, 0, team);
    on_collide(sim.get(), a, b);
    check(a.collision_velocity.x == -2.5f && b.collision_velocity.x == 2.5f,
          "friendly mob overlap resolves equally");

    // Web slow and loot pickup must survive the early pair filter.
    reset(a, kWeb, 0, 0, team); reset(b, kMob, 5, 0, enemy);
    on_collide(sim.get(), a, b);
    check(b.speed_ratio == .5f && no_motion(a) && no_motion(b), "web slows mobs without push");
    reset(b, kPetal, 5, 0, enemy);
    on_collide(sim.get(), a, b);
    check(b.speed_ratio == 1.f, "web does not slow petals");
    for (Components component : {kMob, kPetal, kDrop, kWeb}) {
        reset(a, kDrop, 0, 0, team); reset(b, component, 0, 0, enemy);
        check(!_should_interact(a, b) && !_should_interact(b, a), "non-flower drop pair rejected");
        on_collide(sim.get(), a, b);
        check(no_motion(a) && no_motion(b) && a.speed_ratio == 1.f && b.speed_ratio == 1.f,
              "non-flower drop collision is inert");
    }
    reset(a, kFlower, 0, 0, team); reset(b, kDrop, 5, 0, camera.id);
    a.add_component(kHealth); // living flower — corpses (no Health) must not pick up
    a.set_parent(camera.id); a.set_loadout_count(1);
    b.set_drop_id(PetalID::kBasic); b.set_drop_count(1);
    check(_should_interact(a, b) && _should_interact(b, a), "drop-flower pair accepted both orders");
    on_collide(sim.get(), b, a);
    check(b.pending_delete && collector.craft_bank[PetalID::kBasic] == 1
          && a.get_loadout_ids(0) == PetalID::kNone && a.get_loadout_ids(1) == PetalID::kNone,
          "owned loot enters stacks without equipping empty slots");
    reset(a, kFlower, 0, 0, team); reset(b, kDrop, 5, 0, camera.id);
    a.set_parent(camera.id);
    check(!_should_interact(a, b), "corpse flower without Health rejects drops");
    reset(a, kFlower, 0, 0, team); reset(b, kDrop, 5, 0, enemy);
    a.add_component(kHealth);
    a.set_parent(camera.id); a.set_loadout_count(1);
    b.set_drop_id(PetalID::kBasic); b.set_drop_count(1);
    on_collide(sim.get(), a, b);
    check(!b.pending_delete && a.get_loadout_ids(1) == PetalID::kNone, "private loot remains private");
    b.set_team(NULL_ENTITY); b.immunity_ticks = 1;
    on_collide(sim.get(), a, b);
    check(!b.pending_delete, "unowned loot immunity respected");
    b.immunity_ticks = 0;
    on_collide(sim.get(), a, b);
    check(b.pending_delete, "unowned loot pickable after immunity");

    reset(a, kPetal, 0, 0, team); reset(b, kPetal, 0, 0, enemy);
    a.set_petal_id(PetalID::kMeteor); b.set_petal_id(PetalID::kMeteor);
    check(!_should_interact(a, b), "planted meteors can overlap");
    reset(a, kFlower, 0, 0, team);
    check(!_should_interact(a, b) && !_should_interact(b, a), "players pass meteors");
    reset(b, kFlower, 5, 0, enemy);
    a.add_component(kHealth); b.add_component(kHealth);
    players = true;
    unsigned before = damage_calls;
    on_collide(sim.get(), a, b);
    // Soft-block: cancel_movement may add velocity to keep bodies from nesting;
    // still no contact damage, and collision_velocity must separate.
    check(damage_calls == before
          && a.collision_velocity.x < 0.f && b.collision_velocity.x > 0.f,
          "player collision keeps soft push without PvP damage or knockback");
    // Shallow overlap still gets a separation floor (no slow sink-through).
    reset(a, kFlower, 0, 0, team); reset(b, kFlower, 19.5f, 0, enemy);
    a.add_component(kHealth); b.add_component(kHealth);
    a.mass = b.mass = 1.f;
    on_collide(sim.get(), a, b);
    check(a.collision_velocity.x < -1.f && b.collision_velocity.x > 1.f,
          "shallow player overlap still soft-blocks with separation floor");
    players = false;

    // Regression for sqrt underflow: a nonzero direction can have zero length.
    // Preserve Vector::normalize guards while avoiding a second square root.
    for (float delta : {0.f, 1e-30f, 1e-20f, 1e-10f, .001f, 1.f}) {
        reset(a, kMob, 0, 0, team); reset(b, kMob, delta, 0, team);
        on_collide(sim.get(), a, b);
        check(finite_motion(a) && finite_motion(b), "degenerate contact keeps finite motion");
        if (delta == 1e-30f) check(a.collision_velocity.x < 0.f,
                                 "underflow preserves finite tiny separation direction");
    }
    reset(a, kMob, 0, 0, team); reset(b, kMob, 1, 1, team);
    a.mass = b.mass = 0.f;
    on_collide(sim.get(), a, b);
    check(finite_motion(a) && finite_motion(b), "zero total mass cannot corrupt motion");
    b.set_x(std::numeric_limits<float>::quiet_NaN());
    on_collide(sim.get(), a, b);
    check(finite_motion(a) && finite_motion(b), "nonfinite coordinate cannot corrupt motion");

    // Independent analytic oracle for ordinary circle push. Away from exact
    // tangency, squared-distance rejection and normalized push must agree.
    std::mt19937 rng(20261023);
    std::uniform_real_distribution<float> pos(-40.f, 40.f);
    for (unsigned i = 0; i < 100000; ++i) {
        float x = pos(rng), y = pos(rng);
        reset(a, kMob, 0, 0, team); reset(b, kMob, x, y, team);
        on_collide(sim.get(), a, b);
        double distance = std::sqrt(double(x) * x + double(y) * y);
        double push = distance > 0 && distance < 20 ? (20 - distance) * .5 / distance : 0;
        check(std::abs(a.collision_velocity.x + x * push) < .00002
              && std::abs(a.collision_velocity.y + y * push) < .00002
              && std::abs(b.collision_velocity.x - x * push) < .00002
              && std::abs(b.collision_velocity.y - y * push) < .00002,
              "random circle contact matches independent push oracle");
    }
    std::printf("PASS %u collision checks (pickup, private loot, web, meteor, PvP, degenerate, random geometry)\n", checks);
}
