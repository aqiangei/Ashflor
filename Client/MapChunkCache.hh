#pragma once

#include <cstdint>

class Renderer;

namespace MapChunkCache {
    // World-space static map layers: zones, weave, and terrain.
    void draw(Renderer &target, uint8_t map_id, uint32_t difficulty,
              float left, float top, float right, float bottom, bool mobile);
    void clear();
}
