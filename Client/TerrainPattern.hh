#pragma once

#include <cstdint>

class Renderer;
struct TerrainFeature;

namespace TerrainPattern {
    // Deterministic world-space ground art. The caller clips it to a map chunk.
    void draw(Renderer &, uint8_t map_id, float left, float top, float right, float bottom);
    void draw_wall(Renderer &, TerrainFeature const &, uint8_t map_id);
}
