#pragma once

#include <Shared/StaticData.hh>

#include <cstdint>

// Deterministic per-map layout (zones + impassable terrain corridors).
// Same seed on client and server — no sync needed.
namespace MapGen {
    // 每图唯一的 Boss 锚点（世界坐标）；无 Boss 时 mob_id = 0
    struct BossAnchor {
        float x = 0.f;
        float y = 0.f;
        uint8_t mob_id = 0;
    };
    void ensure(uint8_t map_id);
    MapZones const &zones(uint8_t map_id);
    MapTerrain const &terrain(uint8_t map_id);
    BossAnchor const &boss_anchor(uint8_t map_id);
}
