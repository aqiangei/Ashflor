#pragma once

#include <cstdint>

class Renderer;

// florr_clone getMobLabelCanvas: bake static name/rarity text once, blit each frame.
namespace MobLabelCache {
    // Returns a cached offscreen renderer (already drawn). Caller draws via draw_image_at.
    // layout: 0 = name+rarity horizontal row; 1 = name only; 2 = rarity only.
    Renderer *get(uint8_t layout, uint8_t mob_id, uint8_t rarity, char const *name, char const *rarity_label, uint32_t rarity_color);
    void clear();
}
