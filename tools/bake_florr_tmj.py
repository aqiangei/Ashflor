#!/usr/bin/env python3
"""Bake florr-clone .tmj → ashflor collision + spawn hubs + spawn zones.

Rules:
  - Uniform fit-scale into ARENA (preserve aspect — no stretch / mirror)
  - Center the layout in the square arena; letterbox is framed as walls
  - Resolve external tilesets and transform authored collision shapes per GID
  - Keep authored walls and corridors; never erode or coarsen the map
  - player_spawns → map hub; spawns layer → zone AABBs
"""
from __future__ import annotations
import json
from pathlib import Path

ARENA = 36000.0
MAX_BOXES = 1200  # keep under MAX_TERRAIN_PER_MAP (1536) after portal pads
ROOT = Path(__file__).resolve().parents[2]
MAPS_DIR = ROOT / "reference" / "florr-clone-main (1)" / "florr-clone-main" / "maps"
OUT_TERRAIN = ROOT / "ashflor" / "Shared" / "FlorrTmjTerrain.inc"
OUT_ZONES = ROOT / "ashflor" / "Shared" / "FlorrTmjZones.inc"
OUT_HUBS = ROOT / "ashflor" / "Shared" / "FlorrTmjHubs.inc"
OUT_EXTENTS = ROOT / "ashflor" / "Shared" / "FlorrTmjExtents.inc"
PORTAL_OFF = 560.0

MAP_FILES = {
    "kPrimeGarden": "garden",
    "kSewers": "sewers",
    "kPrimeDesert": "desert",
    "kWildwood": "jungle",
    "kHel": "hel",
    "kAntHell": "ant_hell",
    "kPrimeOcean": "ocean",
}

GID_FLIP = 0x80000000 | 0x40000000 | 0x20000000
SUBDIV = 4  # collision edge approximation: at most half a subcell per axis
RARITY_COLORS = [
    0xff7eef6d, 0xffffe65d, 0xff4d52e3, 0xff861fde,
    0xffde1f1f, 0xff1fdbde, 0xffff2b75, 0xff2bffa3,
]


def load_tmj(stem: str) -> dict:
    j = json.loads((MAPS_DIR / f"{stem}.tmj").read_text(encoding="utf-8"))
    if j["tilewidth"] != j["tileheight"]:
        raise ValueError(f"{stem}: rectangular map tiles need separate axis scaling")
    return j


def florr_diff_to_rarity(d: float) -> int:
    if d < 0:
        return 0
    if d <= 8:
        return 0
    if d <= 22:
        return 1
    if d <= 40:
        return 2
    if d <= 60:
        return 3
    if d <= 75:
        return 4
    if d <= 90:
        return 5
    if d <= 98:
        return 6
    return 7


def collision_and_water(j: dict):
    w, h = j["width"] * SUBDIV, j["height"] * SUBDIV
    solid = [[False] * w for _ in range(h)]
    water = [[False] * w for _ in range(h)]
    tiles = {}
    for ts in j.get("tilesets", []):
        first = int(ts.get("firstgid", 1))
        if "source" in ts:
            ts = json.loads((MAPS_DIR / ts["source"]).read_text(encoding="utf-8"))
        for tile in ts.get("tiles", []) or []:
            props = {p["name"]: p.get("value") for p in tile.get("properties", [])}
            tiles[first + tile["id"]] = (tile, bool(props.get("water")), ts["tilewidth"], ts["tileheight"])
    stamps = {}

    def inside(o, x, y):
        x -= o.get("x", 0)
        y -= o.get("y", 0)
        if o.get("rotation", 0):
            raise ValueError("Rotated collision objects need explicit support")
        if "polygon" not in o:
            return 0 <= x < o.get("width", 0) and 0 <= y < o.get("height", 0)
        pts = o["polygon"]
        hit = False
        a = pts[-1]
        for b in pts:
            if (a["y"] > y) != (b["y"] > y):
                if x < a["x"] + (y-a["y"])*(b["x"]-a["x"])/(b["y"]-a["y"]):
                    hit = not hit
            a = b
        return hit

    def stamp(gid, negate):
        key = (gid, negate)
        if key in stamps:
            return stamps[key]
        tile, wet, tw, th = tiles[gid & ~GID_FLIP]
        objects = tile.get("objectgroup", {}).get("objects", [])
        cells = []
        for sy in range(SUBDIV):
            for sx in range(SUBDIV):
                # Inverse of Tiled's diagonal-then-horizontal-then-vertical.
                x, y = (sx+.5)/SUBDIV, (sy+.5)/SUBDIV
                if gid & 0x80000000: x = 1-x
                if gid & 0x40000000: y = 1-y
                if gid & 0x20000000: x, y = y, x
                if (negate and not objects) or any(inside(o, x*tw, y*th) for o in objects):
                    cells.append((sx, sy))
        stamps[key] = (cells, wet)
        return cells, wet
    for layer in j["layers"]:
        if layer.get("type") != "tilelayer":
            continue
        props = {p["name"]: p.get("value") for p in layer.get("properties", [])}
        data = layer.get("data") or []
        negate = bool(props.get("negate_collision"))
        collides = bool(props.get("has_collision"))
        for i, gid in enumerate(data):
            lw = layer.get("width", j["width"])
            if i >= lw * layer.get("height", j["height"]):
                break
            raw = gid & ~GID_FLIP
            if raw == 0:
                continue
            if not collides and not negate:
                continue
            cells, wet = stamp(gid, negate)
            x, y = i % lw, i // lw
            dx = layer.get("x", 0) + layer.get("offsetx", 0) / j["tilewidth"]
            dy = layer.get("y", 0) + layer.get("offsety", 0) / j["tileheight"]
            for sx, sy in cells:
                px, py = round((x+dx)*SUBDIV)+sx, round((y+dy)*SUBDIV)+sy
                if 0 <= px < w and 0 <= py < h:
                    solid[py][px] = not negate
                    water[py][px] = wet and not negate
    return solid, water


def merge_runs(mask, cell: float, ox: float = 0.0, oy: float = 0.0):
    h = len(mask)
    w = len(mask[0]) if h else 0
    boxes = []
    for y in range(h):
        x = 0
        while x < w:
            if not mask[y][x]:
                x += 1
                continue
            x0 = x
            while x < w and mask[y][x]:
                x += 1
            boxes.append([
                ox + x0 * cell,
                oy + y * cell,
                ox + x * cell,
                oy + (y + 1) * cell,
            ])
    boxes.sort(key=lambda b: (b[0], b[2], b[1]))
    merged = []
    for b in boxes:
        if (
            merged
            and abs(merged[-1][0] - b[0]) < 0.01
            and abs(merged[-1][2] - b[2]) < 0.01
            and abs(merged[-1][3] - b[1]) < 0.01
        ):
            merged[-1][3] = b[3]
        else:
            merged.append(b)
    return merged


def player_spawn(j: dict):
    for layer in j["layers"]:
        if layer.get("type") != "objectgroup" or layer.get("name") != "player_spawns":
            continue
        objs = layer.get("objects") or []
        if not objs:
            continue
        o = objs[0]
        # florr uses object origin (not AABB center) — rects can span half the map.
        return (float(o.get("x", 0)), float(o.get("y", 0)))
    tw = float(j["tilewidth"])
    return j["width"] * tw * 0.5, j["height"] * tw * 0.5


def spawn_zones(j: dict, world_w: float, world_h: float):
    pts = []
    for layer in j["layers"]:
        if layer.get("type") != "objectgroup" or layer.get("name") != "spawns":
            continue
        for o in layer.get("objects") or []:
            props = {p["name"]: p.get("value") for p in o.get("properties", [])}
            d = props.get("difficulty", 0)
            try:
                d = float(d)
            except (TypeError, ValueError):
                d = 0.0
            pts.append((o.get("x", 0), o.get("y", 0), d, props.get("mobs", "")))
    if not pts:
        return [(0.0, 0.0, world_w, world_h, 0)]
    zones = []
    for i, (x, y, d, _m) in enumerate(pts):
        dist = 1e18
        for j2, (x2, y2, _, _) in enumerate(pts):
            if i == j2:
                continue
            dist = min(dist, ((x - x2) ** 2 + (y - y2) ** 2) ** 0.5)
        half = dist * 0.55 if dist < 1e17 else min(world_w, world_h) * 0.22
        half = max(900.0, min(half, 4200.0))
        if d < 0:  # Hel whole-map
            zones.append((0.0, 0.0, world_w, world_h, florr_diff_to_rarity(d)))
            continue
        L = max(0.0, x - half)
        T = max(0.0, y - half)
        R = min(world_w, x + half)
        B = min(world_h, y + half)
        zones.append((L, T, R, B, florr_diff_to_rarity(d), x, y))
    zones.sort(key=lambda z: z[4])
    return zones


def main():
    if not MAPS_DIR.is_dir():
        raise SystemExit(f"maps dir missing: {MAPS_DIR}")

    terrain_chunks = []
    zone_chunks = []
    hub_lines = []
    extent_lines = []
    hubs = {}
    for enum_name, stem in MAP_FILES.items():
        j = load_tmj(stem)
        tw = float(j["tilewidth"])
        world_w = j["width"] * tw
        world_h = j["height"] * tw
        # Uniform fit into square arena — preserve aspect, never stretch axes independently.
        scale = min(ARENA / world_w, ARENA / world_h)
        ox = (ARENA - world_w * scale) * 0.5
        oy = (ARENA - world_h * scale) * 0.5
        solid, water = collision_and_water(j)
        # Keep every authored cell. Water replaces wall material; it must not
        # leave a second hill underneath after a bridge clears it.
        hills = [[s and not wet for s, wet in zip(sr, wr)]
                 for sr, wr in zip(solid, water)]
        factor = 1
        cell = tw * scale / SUBDIV
        hill_boxes = merge_runs(hills, cell, ox, oy)
        water_boxes = merge_runs(water, cell, ox, oy)
        if len(hill_boxes) + len(water_boxes) > MAX_BOXES:
            raise ValueError(f"{stem}: collision budget exceeded; refusing to distort layout")

        sx, sy = player_spawn(j)
        sx = sx * scale + ox
        sy = sy * scale + oy
        mw = world_w * scale
        mh = world_h * scale
        hub_lines.append(
            f"    case MapID::{enum_name}: x = {sx:.1f}f; y = {sy:.1f}f; return;"
        )
        # Playable AABB inside the square arena (letterbox outside is framed).
        extent_lines.append(
            f"    case MapID::{enum_name}: "
            f"x0 = {ox:.1f}f; y0 = {oy:.1f}f; x1 = {ox + mw:.1f}f; y1 = {oy + mh:.1f}f; return;"
        )
        hubs[enum_name] = (sx, sy, mw, mh, ox, oy)

        tlines = [
            f"// {stem}.tmj fit scale={scale:.4f} ox={ox:.1f} oy={oy:.1f} "
            f"factor={factor} hills={len(hill_boxes)} water={len(water_boxes)} subdiv={SUBDIV} erode=0"
        ]
        tlines.append(f"case MapID::{enum_name}: {{")
        for i, (L, T, R, B) in enumerate(hill_boxes):
            if R - L < 20 or B - T < 20:
                continue
            tlines.append(
                "    push_feat(t, TerrainKind::kHill, TerrainShape::kAABB, TerrainFlags::kImpassable, "
                f"{L:.1f}f, {T:.1f}f, {R:.1f}f, {B:.1f}f, pal.hill, {200 + i % 50}, -1.f);"
            )
        for i, (L, T, R, B) in enumerate(water_boxes):
            if R - L < 20 or B - T < 20:
                continue
            tlines.append(
                "    push_feat(t, TerrainKind::kWater, TerrainShape::kAABB, TerrainFlags::kImpassable, "
                f"{L:.1f}f, {T:.1f}f, {R:.1f}f, {B:.1f}f, pal.water, {10 + i % 40}, -1.f);"
            )
        tlines.append("    break; }")
        terrain_chunks.append("\n".join(tlines))

        zlines = [f"case MapID::{enum_name}: {{"]
        zlines.append(
            f"    // florr spawn bands  raw={world_w:.0f}x{world_h:.0f} "
            f"fit={mw:.0f}x{mh:.0f} @({ox:.0f},{oy:.0f})"
        )
        zlines.append(
            "    {"
            f" ZoneDefinition z = soft; z.difficulty = 0; z.color = 0x{RARITY_COLORS[0]:08x}; "
            f"z.name = \"{stem}\"; z.density = 2.2f; z.drop_multiplier = zone_drop_tilt(0); "
            f"z.anchor_x = {sx:.1f}f; z.anchor_y = {sy:.1f}f; "
            f"push_zone(zones, z, {ox:.1f}f, {oy:.1f}f, {ox + mw:.1f}f, {oy + mh:.1f}f, 0, z.color); }}"
        )
        for z in spawn_zones(j, world_w, world_h):
            if len(z) == 5:
                L, T, R, B, rar = z
                ax, ay = (L + R) * 0.5, (T + B) * 0.5
            else:
                L, T, R, B, rar, ax, ay = z
            L, T, R, B = L * scale + ox, T * scale + oy, R * scale + ox, B * scale + oy
            ax, ay = ax * scale + ox, ay * scale + oy
            col = RARITY_COLORS[min(rar, 7)]
            pool = "soft" if rar <= 1 else ("mid" if rar <= 3 else "hard")
            zlines.append(
                "    {"
                f" ZoneDefinition z = {pool}; z.difficulty = {rar}; z.color = 0x{col:08x}; "
                f"z.name = \"{stem}\"; z.density = {2.0 + rar * 0.25:.2f}f; "
                f"z.drop_multiplier = zone_drop_tilt({rar}); "
                f"z.anchor_x = {ax:.1f}f; z.anchor_y = {ay:.1f}f; "
                f"push_zone(zones, z, {L:.1f}f, {T:.1f}f, {R:.1f}f, {B:.1f}f, {rar}, z.color); }}"
            )
        zlines.append("    break; }")
        zone_chunks.append("\n".join(zlines))
        print(
            enum_name, stem, f"fit*{scale:.3f}", f"off=({ox:.0f},{oy:.0f})",
            "factor", factor, "hills", len(hill_boxes), "water", len(water_boxes),
            "hub", round(sx), round(sy),
        )

    hdr = (
        "// AUTO-GENERATED by tools/bake_florr_tmj.py — florr fit-scale (aspect preserved). "
        "Do not edit.\n\n"
    )
    OUT_TERRAIN.write_text(hdr + "\n".join(terrain_chunks) + "\n", encoding="utf-8")
    OUT_ZONES.write_text(hdr + "\n".join(zone_chunks) + "\n", encoding="utf-8")
    OUT_HUBS.write_text(
        hdr + "\n".join(hub_lines) + "\n    default: x = MAP_SPAWN_X; y = MAP_SPAWN_Y; return;\n",
        encoding="utf-8",
    )
    OUT_EXTENTS.write_text(
        hdr
        + "\n".join(extent_lines)
        + "\n    default: x0 = 0.f; y0 = 0.f; x1 = (float)ARENA_WIDTH; y1 = (float)ARENA_HEIGHT; return;\n",
        encoding="utf-8",
    )
    print("wrote", OUT_TERRAIN)
    print("wrote", OUT_ZONES)
    print("wrote", OUT_HUBS)
    print("wrote", OUT_EXTENTS)
    print("--- hub portal pads (for MAP_PORTALS) ---")
    for name, (sx, sy, ww, hh, ox, oy) in hubs.items():
        print(f"  {name} hub=({sx:.0f},{sy:.0f}) extent={ww:.0f}x{hh:.0f} origin=({ox:.0f},{oy:.0f})")
        print(
            f"    N ({sx:.0f},{sy - PORTAL_OFF:.0f}) S ({sx:.0f},{sy + PORTAL_OFF:.0f}) "
            f"W ({sx - PORTAL_OFF:.0f},{sy:.0f}) E ({sx + PORTAL_OFF:.0f},{sy:.0f})"
        )


if __name__ == "__main__":
    main()
