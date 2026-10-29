#include <Client/Assets/PetalRasterCache.hh>

#include <Client/Assets/Assets.hh>
#include <Client/Render/Renderer.hh>

#include <algorithm>
#include <cmath>
#include <memory>
#include <unordered_map>

namespace {

constexpr size_t kMaxEntries = 320;
// Bake sharper than world units so zoom / DPR≤2 blits stay crisp.
constexpr float kBakeScale = 2.f;

struct Entry {
    std::unique_ptr<Renderer> canvas;
    float bake_scale = kBakeScale;
    uint64_t used = 0;
};

std::unordered_map<uint32_t, Entry> cache;
uint64_t access_clock = 0;
uint8_t baking = 0;

float bake_half_extent(PetalID::T art) {
    float r = PETAL_DATA[art].radius;
    if (r < 6.f) r = 6.f;
    // Strokes / soft shadows / elongated petals (shovel, bandage) overrun radius.
    return r * 2.6f;
}

void bake(PetalID::T art, Renderer &r, float scale) {
    float half = bake_half_extent(art) * scale;
    float side = half * 2.f;
    if (side < 96.f) side = 96.f;
    if (side > 512.f) side = 512.f;
    r.set_dimensions(side, side);
    r.reset();
    r.clear();
    r.translate(side * 0.5f, side * 0.5f);
    r.scale(scale);
    baking = 1;
    draw_static_petal_single(art, r);
    baking = 0;
    r.reset_transform();
}

} // namespace

namespace PetalRasterCache {

uint8_t try_draw(PetalID::T art_id, Renderer &dest) {
    if (baking) return 0;
    if (art_id == PetalID::kNone || art_id >= PetalID::kNumPetals) return 0;
    // Damage flash tints via set_fill/set_stroke MIX — blit would skip it.
    if (dest.context.amount > 0.02f) return 0;

    uint32_t key = (uint32_t) art_id;
    auto it = cache.find(key);
    if (it == cache.end()) {
        if (cache.size() >= kMaxEntries) {
            auto oldest = std::min_element(cache.begin(), cache.end(),
                [](auto const &a, auto const &b) {
                    return a.second.used < b.second.used;
                });
            cache.erase(oldest);
        }
        Entry e;
        e.used = ++access_clock;
        e.bake_scale = kBakeScale;
        e.canvas = std::make_unique<Renderer>();
        bake(art_id, *e.canvas, e.bake_scale);
        it = cache.emplace(key, std::move(e)).first;
    } else {
        it->second.used = ++access_clock;
    }

    float s = it->second.bake_scale;
    if (s > 1e-3f && std::fabs(s - 1.f) > 1e-3f) {
        dest.scale(1.f / s);
        dest.draw_image(*it->second.canvas);
        dest.scale(s);
    } else {
        dest.draw_image(*it->second.canvas);
    }
    return 1;
}

void clear() {
    cache.clear();
}

}
