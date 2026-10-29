#pragma once

#include <Shared/StaticDefinitions.hh>

class Renderer;

// Original florr.io sprite overlays from florrclone assets.
// Returns 1 if drawn; 0 = caller keeps procedural ashflor art.
namespace FlorrArt {
    enum WornKind : uint8_t {
        kWornBandage = 0,
        kWornChampionCrown,
        kWornPharaohCrown,
        kWornGoggles,
        kWornCount
    };

    uint8_t try_draw_petal(PetalID::T id, Renderer &ctx, float radius);
    // flags: bit1 = body segment (centipedes). rarity: skip florr PNG at Super+ (soft upscale).
    uint8_t try_draw_mob(MobID::T id, Renderer &ctx, float radius,
        uint32_t flags = 0, uint8_t rarity = 0);
    // Body-mounted overlays (bandage wraps, crowns, goggles). Size is full draw width/height.
    uint8_t try_draw_worn(WornKind kind, Renderer &ctx, float size);
    // True when this petal should sit on the flower (worn art) instead of orbiting visually.
    uint8_t is_body_worn_petal(PetalID::T id);
    // Mapped florrclone URL exists (image may still be loading). Raster caches must not
    // bake procedural for these IDs or the first unloaded frame locks out sprites forever.
    uint8_t has_petal_sprite(PetalID::T id);
    uint8_t has_mob_sprite(MobID::T id);
}
