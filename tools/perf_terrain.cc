// Native regression/benchmark (run from the repository root):
// clang++ -std=c++20 -O3 -Wno-narrowing -Wno-reorder-init-list -I. \
//   -ffunction-sections -fdata-sections tools/perf_terrain.cc Shared/MapGen.cc \
//   Shared/StaticData.cc Helpers/Math.cc -Wl,--gc-sections -o /tmp/perf-terrain
// /tmp/perf-terrain
// Include the implementation so the oracle can retain the original narrow
// phase while independently checking the indexed broad phase against a scan.
#ifndef ASHFLOR_MAP_IMPL
#define ASHFLOR_MAP_IMPL "Shared/Map.cc"
#endif
#include ASHFLOR_MAP_IMPL
#include <chrono>
#include <cstdio>
#include <random>

static uint8_t linear_query(uint8_t map, float x, float y, float r) {
    auto const &terrain = map_terrain(map);
    for (auto const &f : terrain)
        if (f.kind == TerrainKind::kChannel && circle_hits_feature(f, x, y, r)) return 0;
    for (auto const &f : terrain)
        if ((f.flags & TerrainFlags::kImpassable) && circle_hits_feature(f, x, y, r)) return 1;
    return 0;
}

int main() {
    std::mt19937 rng(20261023);
    size_t checked = 0;
    double linear_ms = 0, indexed_ms = 0;
    for (uint8_t map = 0; map < MapID::kNumMaps; ++map) {
        auto const &terrain = map_terrain(map);
        struct Query { float x, y, r; };
        std::vector<Query> queries;
        float radii[] = {0.f, 8.f, 25.f, 60.f, 256.f, 512.f, 1024.f, 4096.f};
        // Include off-map positions, cell seams, giant bosses and tiny drops.
        for (int i = 0; i < 100000; ++i) {
            float x = (float)(rng() % (ARENA_WIDTH + 2048)) - 1024.f;
            float y = (float)(rng() % (ARENA_HEIGHT + 2048)) - 1024.f;
            queries.push_back({x, y, radii[i % 8]});
        }
        for (auto const &f : terrain) {
            float xs[64], ys[64];
            uint32_t n = Map::terrain_outline(f, xs, ys, 64);
            if (!n) continue;
            float l = xs[0], t = ys[0], r = xs[0], b = ys[0];
            for (uint32_t i = 1; i < n; ++i) {
                l = std::min(l, xs[i]); r = std::max(r, xs[i]);
                t = std::min(t, ys[i]); b = std::max(b, ys[i]);
            }
            for (float radius : radii) {
                for (float x : {l - radius, l, (l + r) * .5f, r, r + radius})
                    for (float y : {t - radius, t, (t + b) * .5f, b, b + radius})
                        queries.push_back({x, y, radius});
            }
        }
        std::vector<uint8_t> expected;
        uint64_t result_hash = 1469598103934665603ull;
        auto start = std::chrono::steady_clock::now();
        for (auto q : queries) expected.push_back(linear_query(map, q.x, q.y, q.r));
        for (uint8_t result : expected) result_hash = (result_hash ^ result) * 1099511628211ull;
        linear_ms += std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
        start = std::chrono::steady_clock::now();
        for (size_t i = 0; i < queries.size(); ++i) {
            auto q = queries[i];
            if (Map::circle_hits_impassable(map, q.x, q.y, q.r) != expected[i]) {
                std::fprintf(stderr, "Mismatch map=%u x=%g y=%g r=%g\n", map, q.x, q.y, q.r);
                return 1;
            }
        }
        indexed_ms += std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
        checked += queries.size();
        // Layout checksum also allows comparison against the pre-change build:
        // MapGen uses collision queries while carving new connecting channels.
        uint64_t hash = 1469598103934665603ull;
        for (auto const &f : terrain) {
            for (float v : {f.left, f.top, f.right, f.bottom}) {
                uint32_t bits;
                std::memcpy(&bits, &v, sizeof(bits));
                hash = (hash ^ bits) * 1099511628211ull;
            }
            hash = (hash ^ f.seed ^ f.kind ^ f.shape ^ f.flags) * 1099511628211ull;
        }
        std::printf("map=%u features=%u layout=%llu results=%llu queries=%zu OK\n", map, terrain.size(),
                    (unsigned long long)hash, (unsigned long long)result_hash, queries.size());
    }
    std::printf("PASS %zu queries; linear=%.1f ms indexed=%.1f ms speedup=%.2fx\n",
                checked, linear_ms, indexed_ms, linear_ms / indexed_ms);
}
