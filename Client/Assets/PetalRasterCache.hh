#pragma once

#include <Shared/StaticData.hh>

#include <cstdint>

class Renderer;

// Bake procedural-only petals once, blit with drawImage each frame.
// FlorrArt-mapped IDs are excluded (see FlorrArt::has_petal_sprite).
// (same idea as MobLabelCache / florr_clone-web petal canvases).
namespace PetalRasterCache {
    // Returns 1 if drawn from cache (or freshly baked). 0 = caller should draw paths.
    // Skips when damage color-filter is active or while baking.
    uint8_t try_draw(PetalID::T art_id, Renderer &dest);
    void clear();
}
