// Build twice with ASHFLOR_MAP_IMPL pointing at the before/after Map.cc.
// Compare the binary outputs, including CCD wall slides and ejection results.
#ifndef ASHFLOR_MAP_IMPL
#define ASHFLOR_MAP_IMPL "Shared/Map.cc"
#endif
#include ASHFLOR_MAP_IMPL
#include <bit>
#include <chrono>
#include <cstdio>
#include <random>

struct MotionCase { uint8_t map; float x, y, dx, dy, radius; };

int main(int argc, char **argv) {
    std::mt19937 rng(20261102);
    std::vector<MotionCase> cases;
    float const radii[] = {5.f, 25.f, 60.f, 256.f, 1024.f};
    for (uint8_t map = 0; map < MapID::kNumMaps; ++map) {
        auto add = [&](float x, float y, unsigned i) {
            float dx = (float)(rng() % 801) - 400.f;
            float dy = (float)(rng() % 801) - 400.f;
            cases.push_back({map, x, y, dx, dy, radii[i % 5]});
        };
        for (unsigned i = 0; i < 3000; ++i)
            add((float)(rng() % (ARENA_WIDTH + 1024)) - 512.f,
                (float)(rng() % (ARENA_HEIGHT + 1024)) - 512.f, i);
        // Real wall/channel edges, thin imported tiles, bends and giant bosses.
        for (auto const &f : map_terrain(map)) {
            float xs[64], ys[64];
            unsigned n = Map::terrain_outline(f, xs, ys, 64);
            for (unsigned j = 0; j < n; j += 8)
                for (unsigned i = 0; i < 5; ++i)
                    add(xs[j] + radii[i], ys[j] - radii[i], i);
        }
    }
    FILE *out = argc > 1 ? std::fopen(argv[1], "wb") : nullptr;
    if (argc > 1 && !out) return 2;
    uint64_t hash = 1469598103934665603ull;
    auto start = std::chrono::steady_clock::now();
    for (auto c : cases) {
        float x = c.x + c.dx, y = c.y + c.dy;
        Map::ccd_move_circle(c.map, x, y, c.x, c.y, c.radius);
        float result[] = {x, y};
        if (out) std::fwrite(result, sizeof(result), 1, out);
        for (unsigned char b : std::bit_cast<std::array<unsigned char, sizeof(result)>>(result))
            hash = (hash ^ b) * 1099511628211ull;
    }
    if (out) std::fclose(out);
    double ms = std::chrono::duration<double, std::milli>(
        std::chrono::steady_clock::now() - start).count();
    std::printf("CCD cases=%zu hash=%llu ms=%.2f\n", cases.size(),
                (unsigned long long)hash, ms);
}
