#include <Client/MobLabelCache.hh>

#include <Client/I18n.hh>
#include <Client/Render/Renderer.hh>

#include <Shared/StaticDefinitions.hh>

#include <cstdint>
#include <algorithm>
#include <memory>
#include <unordered_map>

namespace {
constexpr size_t kMaxEntries = 256;
constexpr float kFont = 12.f;
constexpr float kPadX = 4.f;
constexpr float kPadY = 4.f;

struct Entry {
    std::unique_ptr<Renderer> canvas;
    uint64_t used = 0;
};

std::unordered_map<uint32_t, Entry> cache;
uint8_t cached_lang = 255;
uint64_t access_clock = 0;

uint32_t make_key(uint8_t layout, uint8_t mob_id, uint8_t rarity) {
    return ((uint32_t)layout << 24) | ((uint32_t)mob_id << 8) | (uint32_t)rarity;
}

void bake_row(Renderer &r, char const *name, char const *rarity_label, uint32_t rarity_color) {
    r.set_text_size(kFont);
    float name_w = name != nullptr ? r.get_text_size(name) : 0.f;
    float rare_w = rarity_label != nullptr ? r.get_text_size(rarity_label) : 0.f;
    float gap = (name_w > 0.f && rare_w > 0.f) ? 8.f : 0.f;
    float total = name_w + gap + rare_w;
    float w = total + kPadX * 2.f;
    float h = kFont + kPadY * 2.f;
    r.set_dimensions(w, h);
    r.reset_transform();
    r.clear();
    r.center_text_align();
    r.center_text_baseline();
    float x = kPadX;
    if (name != nullptr && name_w > 0.f) {
        r.translate(x + name_w * 0.5f, h * 0.5f);
        r.draw_text(name, {
            .fill = 0xffffffff,
            .stroke = 0xff000000,
            .size = kFont,
            .stroke_scale = 0.14f
        });
        r.reset_transform();
        x += name_w + gap;
    }
    if (rarity_label != nullptr && rare_w > 0.f) {
        r.translate(x + rare_w * 0.5f, h * 0.5f);
        r.draw_text(rarity_label, {
            .fill = rarity_color | 0xff000000,
            .stroke = 0xff000000,
            .size = kFont,
            .stroke_scale = 0.14f
        });
        r.reset_transform();
    }
}

void bake_single(Renderer &r, char const *text, uint32_t fill) {
    r.set_text_size(kFont);
    float tw = r.get_text_size(text);
    float w = tw + kPadX * 2.f;
    float h = kFont + kPadY * 2.f;
    r.set_dimensions(w, h);
    r.reset_transform();
    r.clear();
    r.center_text_align();
    r.center_text_baseline();
    r.translate(w * 0.5f, h * 0.5f);
    r.draw_text(text, {
        .fill = fill,
        .stroke = 0xff000000,
        .size = kFont,
        .stroke_scale = 0.14f
    });
    r.reset_transform();
}
}

namespace MobLabelCache {

Renderer *get(uint8_t layout, uint8_t mob_id, uint8_t rarity,
    char const *name, char const *rarity_label, uint32_t rarity_color)
{
    if (layout == 0 && name == nullptr && rarity_label == nullptr) return nullptr;
    if (layout == 1 && (name == nullptr || name[0] == '\0')) return nullptr;
    if (layout == 2 && (rarity_label == nullptr || rarity_label[0] == '\0')) return nullptr;

    uint8_t lang = I18n::use_chinese ? 1 : 0;
    if (lang != cached_lang) {
        cache.clear();
        cached_lang = lang;
    }

    uint32_t key = make_key(layout, mob_id, rarity);
    auto it = cache.find(key);
    if (it != cache.end()) {
        it->second.used = ++access_clock;
        return it->second.canvas.get();
    }

    // Keep hot labels when exploring new species/rarities. Clearing everything
    // at capacity repeatedly rebaked all the labels in a crowded viewport.
    if (cache.size() >= kMaxEntries) {
        auto oldest = std::min_element(cache.begin(), cache.end(), [](auto const &a, auto const &b) {
            return a.second.used < b.second.used;
        });
        cache.erase(oldest);
    }

    Entry e;
    e.used = ++access_clock;
    e.canvas = std::make_unique<Renderer>();
    if (layout == 0)
        bake_row(*e.canvas, name, rarity_label, rarity_color);
    else if (layout == 1)
        bake_single(*e.canvas, name, 0xffffffff);
    else
        bake_single(*e.canvas, rarity_label, rarity_color | 0xff000000);

    Renderer *out = e.canvas.get();
    cache.emplace(key, std::move(e));
    return out;
}

void clear() {
    cache.clear();
}

}
