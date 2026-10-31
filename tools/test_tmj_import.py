"""Regression checks for Tiled orientation, layer semantics and lossless box merging."""
import unittest
import bake_florr_tmj as bake


class ImportTests(unittest.TestCase):
    def fixture(self, gid, extra=()):
        return dict(width=1, height=1, tilewidth=256, tileheight=256,
                    tilesets=[dict(firstgid=1, tilewidth=256, tileheight=256, tiles=[
                        dict(id=0, objectgroup=dict(objects=[dict(x=0, y=0, width=64, height=128)])),
                        dict(id=1, properties=[dict(name='water', value=True)],
                             objectgroup=dict(objects=[dict(x=0, y=0, width=256, height=256)])),
                        dict(id=2)])],
                    layers=[dict(type='tilelayer', data=[gid], properties=[dict(name='has_collision', value=True)]), *extra])

    def test_all_eight_orientations(self):
        # Expected from the reference renderer's quarter-turn/mirror table.
        expected = [{(0, 0), (0, 1)}, {(3, 0), (3, 1)},
                    {(0, 2), (0, 3)}, {(3, 2), (3, 3)},
                    {(0, 0), (1, 0)}, {(2, 0), (3, 0)},
                    {(0, 3), (1, 3)}, {(2, 3), (3, 3)}]
        for flags in range(8):
            gid = 1 | ((flags & 1) << 31) | ((flags & 2) << 29) | ((flags & 4) << 27)
            solid, _ = bake.collision_and_water(self.fixture(gid))
            self.assertEqual({(x, y) for y, row in enumerate(solid) for x, v in enumerate(row) if v}, expected[flags])

    def test_unshaped_tile_and_bridge(self):
        self.assertFalse(any(map(any, bake.collision_and_water(self.fixture(3))[0])))
        bridge = dict(type='tilelayer', data=[3], properties=[dict(name='negate_collision', value=True)])
        solid, water = bake.collision_and_water(self.fixture(2, [bridge]))
        self.assertFalse(any(map(any, solid)))
        self.assertFalse(any(map(any, water)))
        wall = dict(type='tilelayer', data=[1], properties=[dict(name='has_collision', value=True)])
        solid, _ = bake.collision_and_water(self.fixture(2, [bridge, wall]))
        self.assertEqual(sum(map(sum, solid)), 2)

    def test_shipped_maps_merge_without_geometry_loss(self):
        for stem in bake.MAP_FILES.values():
            solid, water = bake.collision_and_water(bake.load_tmj(stem))
            hills = [[s and not w for s, w in zip(sr, wr)] for sr, wr in zip(solid, water)]
            total = 0
            for mask in [hills, water]:
                boxes = bake.merge_runs(mask, 1)
                total += len(boxes)
                restored = [[False] * len(mask[0]) for _ in mask]
                for l, t, r, b in boxes:
                    for y in range(int(t), int(b)):
                        restored[y][int(l):int(r)] = [True] * int(r-l)
                self.assertEqual(mask, restored, stem)
            self.assertLessEqual(total, bake.MAX_BOXES, stem)


if __name__ == '__main__':
    unittest.main()
