#include <Client/Assets/MobRasterCache.hh>

#include <Client/Assets/FlorrArt.hh>
#include <Client/Render/Renderer.hh>
#include <Shared/StaticData.hh>
#include <Shared/StaticDefinitions.hh>

#include <algorithm>
#include <cmath>
#include <memory>
#include <unordered_map>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace {

constexpr size_t kMaxEntries = 384;
constexpr int kAnimBuckets = 24;
constexpr size_t kMaxBytes = 32u * 1024u * 1024u;
constexpr float kBakeScale = 2.f;

struct Entry {
    std::unique_ptr<Renderer> canvas;
    float bake_scale = kBakeScale;
    uint64_t used = 0;
    size_t bytes = 0;
};

std::unordered_map<uint64_t, Entry> cache;
uint64_t access_clock = 0;
uint8_t baking = 0;
size_t cache_bytes = 0;

uint64_t make_key(MobID::T id, MobRenderAttributes const &attrs) {
    int anim_q = (int) std::floor(attrs.animation * (float) kAnimBuckets / (float) (2 * M_PI));
    if (anim_q < 0) anim_q = -anim_q;
    anim_q %= kAnimBuckets;
    uint32_t radius_q = (uint32_t) (attrs.radius + 0.5f);
    if (radius_q > 0xfffu) radius_q = 0xfffu;
    uint32_t flags = attrs.flags & 0x7u;
    // Only ladybug spots depend on entity identity. Other species must share
    // poses, otherwise each entity continually evicts this small cache.
    uint32_t seed = (id == MobID::kLadybug || id == MobID::kDarkLadybug
        || id == MobID::kMassiveLadybug || id == MobID::kShinyLadybug)
        ? (attrs.seed & 0xffffu) : 0u;
    uint32_t rarity = attrs.rarity & 0xfu;
    return ((uint64_t) id)
        | ((uint64_t) anim_q << 11)
        | ((uint64_t) flags << 16)
        | ((uint64_t) attrs.color << 20)
        | ((uint64_t) radius_q << 28)
        | ((uint64_t) seed << 40)
        | ((uint64_t) rarity << 56);
}

MobRenderAttributes quantize(MobRenderAttributes attrs) {
    float step = (float) (2 * M_PI) / (float) kAnimBuckets;
    int anim_q = (int) std::floor(attrs.animation / step);
    if (anim_q < 0) anim_q = -anim_q;
    attrs.animation = (float) (anim_q % kAnimBuckets) * step;
    attrs.radius = std::floor(attrs.radius + 0.5f);
    return attrs;
}

void bake(MobID::T id, MobRenderAttributes attrs, Renderer &r, float scale) {
    attrs = quantize(attrs);
    float rad = attrs.radius;
    if (rad < 8.f) rad = 8.f;
    float half = rad * 3.2f * scale;
    float side = half * 2.f;
    r.set_dimensions(side, side);
    r.reset();
    r.clear();
    r.translate(side * 0.5f, side * 0.5f);
    r.scale(scale);
    baking = 1;
    draw_static_mob(id, r, attrs);
    baking = 0;
    r.reset_transform();
}

} // namespace

namespace MobRasterCache {

uint8_t try_draw(MobID::T id, MobRenderAttributes const &attrs, Renderer &dest) {
    if (baking) return 0;
    if (id >= MobID::kNumMobs) return 0;
    // Tall wooden cross + soft shadow — square bake used to show as a plate.
    if (id == MobID::kTrainingDummy) return 0;
    // Continuous layered rotation needs the live phase, not a looping pose.
    if (id == MobID::kSandstorm) return 0;
    // Centipede antennae / segment overhang clip in the fixed bake half-extent;
    // damage flash skips cache so only hit frames looked "correct".
    if (id < MobID::kNumMobs && MOB_DATA[id].attributes.segments > 1) return 0;
    // FlorrArt PNGs load async; baking procedural here would permanently hide them.
    // Super+ skip FlorrArt (crisp procedural) — allow bake for those rarities.
    if (FlorrArt::has_mob_sprite(id) && attrs.rarity < RarityID::kSuper) return 0;
    if (dest.context.amount > 0.02f) return 0;

    uint64_t key = make_key(id, attrs);
    auto it = cache.find(key);
    if (it == cache.end()) {
        float const radius = std::max(8.f, std::floor(attrs.radius + 0.5f));
        // Fit the entire silhouette, including antennae/shadows. Clamping only
        // the canvas size used to crop large mobs while keeping the old scale.
        float const scale = std::min(kBakeScale, 1024.f / (radius * 6.4f));
        size_t const side = (size_t)std::ceil(radius * 6.4f * scale);
        size_t const bytes = side * side * 4u;
        while (!cache.empty() && (cache.size() >= kMaxEntries || cache_bytes + bytes > kMaxBytes)) {
            auto oldest = std::min_element(cache.begin(), cache.end(),
                [](auto const &a, auto const &b) {
                    return a.second.used < b.second.used;
                });
            cache_bytes -= oldest->second.bytes;
            cache.erase(oldest);
        }
        Entry e;
        e.used = ++access_clock;
        e.bake_scale = scale;
        e.bytes = bytes;
        e.canvas = std::make_unique<Renderer>();
        bake(id, attrs, *e.canvas, e.bake_scale);
        it = cache.emplace(key, std::move(e)).first;
        cache_bytes += bytes;
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
    cache_bytes = 0;
}

}
