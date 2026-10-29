#pragma once

#include <Shared/StaticDefinitions.hh>

#include <cstdint>

class Renderer;

// Stamp original florr.io world-tile SVGs onto chunk ground / clipped walls.
// Missing or not-yet-loaded images → caller keeps procedural ThemeMaterial art.
namespace FlorrTerrain {
    // Returns how many tiles painted (0 = fully procedural this pass).
    uint32_t paint_ground(Renderer &ctx, uint8_t map_id,
                          float left, float top, float right, float bottom);
    // Wall / water / channel stamps are intentionally no-ops for now —
    // opaque mismatched tiles flashed as "格子" on title and in-game.
    uint32_t paint_wall_fill(Renderer &, uint8_t, float, float, float, float);
    uint32_t paint_water_fill(Renderer &, uint8_t, float, float, float, float);
    uint32_t paint_channel_fill(Renderer &, uint8_t, float, float, float, float);

    // 1 once tile images are warm enough that a ground stamp would succeed.
    uint8_t tiles_ready();
    // Kick off /florr-art/urls.json preload (idempotent).
    void ensure_preload();
}
