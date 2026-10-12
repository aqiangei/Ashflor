#pragma once

#include <Shared/StaticData.hh>

#include <cstdint>

// Deterministic per-map layout (zones + impassable terrain corridors).
// Same seed on client and server — no sync needed.
namespace MapGen {
    void ensure(uint8_t map_id);
    MapZones const &zones(uint8_t map_id);
    MapTerrain const &terrain(uint8_t map_id);
}
