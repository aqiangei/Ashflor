// Native spawn-placement regression/benchmark; compile with -DSERVERSIDE=1,
// Shared/{MapGen,StaticData,Simulation,Entity,EntityDef,Arena,Binary}.cc,
// Helpers/{Math,Vector,UTF8}.cc and Server/SpatialHashCanonical.cc.
// This runs no clients, sockets, persistence or live-server mutations.
#ifndef ASHFLOR_MAP_IMPL
#define ASHFLOR_MAP_IMPL "Shared/Map.cc"
#endif
#include ASHFLOR_MAP_IMPL
#include <chrono>
#include <cassert>
#include <cstdio>
#include <memory>

void Simulation::on_tick() {}
void Simulation::post_tick() {}

int main() {
    auto sim = std::make_unique<Simulation>();
    auto &flower = sim->alloc_ent();
    flower.add_component(kPhysics);
    flower.add_component(kFlower);
    sim->refresh_active_entities();
    size_t total = 0, accepted = 0;
    uint64_t hash = 1469598103934665603ull;
    double total_ms = 0, worst_ms = 0;
    for (uint8_t map = 0; map < MapID::kNumMaps; ++map) {
        sim->map_id = map;
        auto const &zones = map_zones(map);
        std::srand(20261023);
        auto start = std::chrono::steady_clock::now();
        for (uint32_t zi = 0; zi < zones.size(); ++zi) {
            auto const &zone = zones[zi];
            flower.set_x((zone.left + zone.right) * .5f);
            flower.set_y((zone.top + zone.bottom) * .5f);
            for (auto const &spawn : zone.spawns) {
                for (int i = 0; i < 12; ++i) {
                    float x = 0, y = 0;
                    unsigned seed = 20261023u + static_cast<unsigned>(total);
                    std::srand(seed);
                    float clearance = MOB_DATA[spawn.id].radius.get_single(frand())
                        * MOB_RADIUS_MULTIPLIER * mob_rarity_radius_mult(MOB_DATA[spawn.id].rarity) + 8.f;
                    std::srand(seed);
                    auto before = std::chrono::steady_clock::now();
                    bool ok = Map::sample_spawn_point(sim.get(), zi, spawn.id, x, y, MOB_DATA[spawn.id].rarity);
                    worst_ms = std::max(worst_ms, std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - before).count());
                    ++total; accepted += ok;
                    if (ok) {
                        assert(std::isfinite(x) && std::isfinite(y));
                        assert(x >= zone.left + 60.f && x <= zone.right - 60.f);
                        assert(y >= zone.top + 60.f && y <= zone.bottom - 60.f);
                        float hw = 0.f, hh = 0.f;
                        spawn_aoi_half(hw, hh);
                        assert(point_near_flower_aoi(sim.get(), x, y, hw, hh));
                        // Channels override overlapping walls, so a smaller
                        // radius is not a valid oracle for placement clearance.
                        assert(!Map::circle_hits_impassable(map, x, y, clearance));
                        assert(!Map::in_safe_channel(map, x, y));
                        float dx = x - flower.get_x(), dy = y - flower.get_y();
                        assert(dx * dx + dy * dy >= kMinSpawnPlayerDist * kMinSpawnPlayerDist);
                    }
                    uint32_t xb, yb;
                    std::memcpy(&xb, &x, sizeof(xb)); std::memcpy(&yb, &y, sizeof(yb));
                    hash = (hash ^ xb ^ ((uint64_t)yb << 32) ^ ok) * 1099511628211ull;
                }
            }
        }
        double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
        total_ms += ms;
        std::printf("map=%u placement_ms=%.2f checksum=%llu\n", map, ms, (unsigned long long)hash);
    }
    std::printf("PASS attempts=%zu accepted=%zu total_ms=%.2f worst_attempt_ms=%.3f checksum=%llu\n",
                total, accepted, total_ms, worst_ms, (unsigned long long)hash);
}
