// White-box geometry regression: compile this TU instead of Shared/MapGen.cc.
#include <Shared/MapGen.cc>
#include <cassert>
#include <cstdio>

int main() {
    TerrainFeature wall{};
    wall.left = 0; wall.top = 0; wall.right = 2000; wall.bottom = 2000;
    StaticArray<TerrainFeature, 48> pieces;
    emit_aabb_minus_rect(pieces, wall, 800, 800, 1200, 1200);
    float area = 0;
    for (auto const &p : pieces)
        area += (p.right-p.left)*(p.bottom-p.top);
    assert(area == 3840000); // Short crossing removes only its actual intersection.
    MapTerrain portals;
    clear_portal_pads(portals, kThemes[MapID::kGarden], MapID::kGarden);
    for (auto const &p : portals) {
        if (p.seed != 251) continue;
        assert(std::min(p.right-p.left, p.bottom-p.top) <= 441);
        // Northeast quadrant used to be swallowed by the diagonal gate AABB.
        assert(!Map::point_in_feature(p, 23000, 13000));
    }
    for (uint8_t m = 0; m < MapID::kNumMaps; ++m) {
        MapGen::ensure(m);
        assert(map_terrain(m).size() < MAX_TERRAIN_PER_MAP);
        float hx, hy;
        Map::hub_xy(m, hx, hy);
        assert(!Map::circle_hits_impassable(m, hx, hy, 40));
    }
    std::puts("map geometry: local carve, narrow garden routes, terrain budget and hubs OK");
}
