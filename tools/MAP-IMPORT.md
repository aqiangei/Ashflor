# Florr Tiled map import

From the workspace root run `python ashflor/tools/bake_florr_tmj.py`.
The source is `reference/florr-clone-main (1)/florr-clone-main/maps`;
the four generated `Shared/FlorrTmj*.inc` files share one uniform scale
and offset for terrain, hubs, extents and spawn zones.

The importer reads external tilesets, collision object origins, polygons,
layer order, bridge negation and all eight Tiled GID orientations. Tiles
without collision shapes remain walkable even on a blocking layer.
Contours are sampled at four subcells per tile (64 source pixels); the
resulting grid is merged without loss. Fine polygon edges are approximated
at that resolution. Walls are never eroded or coarsened to fit a budget;
over-budget imports fail explicitly. The current authored maps use square
256-pixel tiles and unrotated collision objects.

Run `python ashflor/tools/test_tmj_import.py` for orientation, bridge and
shipped-map merge checks. From the repository root, using native Clang:

```sh
clang++ -Wno-reorder-init-list -std=c++20 -I. -O1 \
  tools/test_map_geometry.cc Shared/Map.cc Shared/StaticData.cc \
  Helpers/Math.cc Helpers/Vector.cc -o /tmp/map-geometry
/tmp/map-geometry
```

This covers narrow portal routes, local rectangle subtraction, terrain
capacity and spawn clearance. Rebuild both client and server after import.
Runtime hub and portal pads intentionally clear small local areas. Do not
replace a diagonal portal route with its bounding rectangle: that clears
the entire intervening quadrant and paints a large untextured dirt area.
