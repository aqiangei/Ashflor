#!/usr/bin/env python3
"""Fix combat ladders with absurd Unique HP or near-zero Common damage.

Pulls design anchors from git HEAD when available; otherwise uses sensible
classic bases. Rebuilds ×3/×2 ladder from Unusual (r=1) by default.
"""
from __future__ import annotations

import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
STATIC_CC = ROOT / "Shared" / "StaticData.cc"
RARITY = [
    "Common", "Unusual", "Rare", "Epic", "Legendary",
    "Mythic", "Ultra", "Super", "Unique",
]

# Manual overrides when HEAD has no full ladder / bad values
ANCHORS: dict[str, dict] = {
    # Classic corn: chunky body, low chip damage at Unusual
    "Corn": {"base_r": 1, "health": 200.0, "damage": 5.0},
    "Square": {"base_r": 1, "health": 30.0, "damage": 10.0},
    "Crown Fragment": {"base_r": 1, "health": 12.0, "damage": 8.0},
    "Stick": {"base_r": 4, "health": 10.0, "damage": 1.0},  # Legendary design
    "Queen's Emblem": {"base_r": 1, "health": 25.0, "damage": 5.0},
    "Soldier Egg": {"base_r": 2, "health": 40.0, "damage": 1.0},
    "Rice": {"base_r": 1, "health": 5.0, "damage": 8.0},
    "Ancient Seed": {"base_r": 1, "health": 15.0, "damage": 5.0},
    "Colony Heart": {"base_r": 1, "health": 20.0, "damage": 8.0},
    "Guard Egg": {"base_r": 1, "health": 35.0, "damage": 1.0},
    "Heartwood": {"base_r": 1, "health": 40.0, "damage": 12.0},
    "King's Oath": {"base_r": 1, "health": 30.0, "damage": 20.0},
    "Spore Cloud": {"base_r": 1, "health": 12.0, "damage": 5.0},
}


def step_mult(from_r: int, to_r: int) -> float:
    if to_r == from_r:
        return 1.0
    ascending = to_r > from_r
    a, b = (from_r, to_r) if ascending else (to_r, from_r)
    m = 1.0
    for r in range(a, b):
        m *= 2.0 if r >= 5 else 3.0
    return m if ascending else (1.0 / m)


def fmt(x: float) -> str:
    if abs(x - round(x)) < 1e-6:
        return f"{int(round(x))}.0"
    s = f"{x:.4f}".rstrip("0").rstrip(".")
    return s if "." in s else s + ".0"


def parse_blocks(text: str):
    start = text.index("PETAL_DATA = {{")
    end = text.index("\n}};", start)
    body_start = start
    body = text[start:end]
    out, depth, begin = [], 0, None
    for j, ch in enumerate(body):
        if ch == "{":
            depth += 1
            if depth == 3 and begin is None:
                begin = j
        elif ch == "}":
            if depth == 3 and begin is not None:
                out.append((body_start + begin, body_start + j + 1, body[begin : j + 1]))
                begin = None
            depth -= 1
    return out


def get_num(block: str, key: str, default=None):
    m = re.search(rf"\.{key}\s*=\s*([0-9.eE+-]+f?)", block)
    return float(m.group(1).rstrip("fF")) if m else default


def get_int(block: str, key: str, default=1):
    m = re.search(rf"\.{key}\s*=\s*([0-9]+)", block)
    return int(m.group(1)) if m else default


def extract_attrs(block: str) -> str:
    am = re.search(r"\.attributes\s*=\s*\{", block)
    if not am:
        return "{}"
    i0 = am.end() - 1
    d = 0
    for k in range(i0, len(block)):
        if block[k] == "{":
            d += 1
        elif block[k] == "}":
            d -= 1
            if d == 0:
                return block[i0 : k + 1]
    return "{}"


def rewrite(old, health, damage, radius, reload, count, rarity, attrs):
    name_m = re.search(r'\.name\s*=\s*"([^"]*)"', old)
    desc_m = re.search(r'\.description\s*=\s*"((?:\\.|[^"\\])*)"', old)
    name = name_m.group(1) if name_m else "?"
    desc = desc_m.group(1) if desc_m else ""
    return (
        "{\n"
        f'        .name = "{name}",\n'
        f'        .description = "{desc}",\n'
        f"        .health = {fmt(health)},\n"
        f"        .damage = {fmt(damage)},\n"
        f"        .radius = {fmt(radius)},\n"
        f"        .reload = {fmt(reload)},\n"
        f"        .count = {count},\n"
        f"        .rarity = RarityID::k{rarity},\n"
        f"        .attributes = {attrs}\n"
        "    }"
    )


def is_outlier(hs, ds) -> bool:
    # Unique HP absurd vs Common, or Common dmg near-zero while Unique huge
    if hs[8] >= 20000:
        return True
    if ds[0] < 0.5 and ds[8] >= 50:
        return True
    if hs[0] < 0.5 and hs[8] >= 100:
        return True
    if hs[0] <= 1.01 and hs[8] >= 1000 and name_is_synthetic(hs):
        return True
    return False


def name_is_synthetic(hs):
    # 1,3,9,27... pattern
    return abs(hs[0] - 1.0) < 0.05 and abs(hs[1] - 3.0) < 0.05


def main() -> int:
    text = STATIC_CC.read_text(encoding="utf-8")
    entries = parse_blocks(text)
    by_name = {}
    for a, b, block in entries:
        nm = re.search(r'\.name\s*=\s*"([^"]*)"', block)
        rar = re.search(r"\.rarity\s*=\s*RarityID::k(\w+)", block)
        if not nm or not rar or nm.group(1) in ("None", "Coin"):
            continue
        ri = RARITY.index(rar.group(1))
        by_name.setdefault(nm.group(1), {})[ri] = (a, b, block)

    # auto-detect outliers + explicit anchors
    targets = set(ANCHORS.keys())
    for name, fam in by_name.items():
        if len(fam) < 9:
            continue
        hs = [get_num(fam[r][2], "health", 0) for r in range(9)]
        ds = [get_num(fam[r][2], "damage", 0) for r in range(9)]
        if is_outlier(hs, ds):
            targets.add(name)

    reps = []
    for name in sorted(targets):
        fam = by_name.get(name)
        if not fam or len(fam) < 9:
            print(f"SKIP {name}")
            continue
        ov = ANCHORS.get(name)
        hs = [get_num(fam[r][2], "health", 0) for r in range(9)]
        ds = [get_num(fam[r][2], "damage", 0) for r in range(9)]
        if ov:
            base_r = int(ov["base_r"])
            bh, bd = float(ov["health"]), float(ov["damage"])
        else:
            base_r = 1
            for r in (1, 2, 3, 4, 0):
                if 8 <= hs[r] <= 400:
                    base_r = r
                    break
            else:
                base_r = 1
            bh, bd = hs[base_r], ds[base_r]
            if hs[8] >= 20000:
                bh = max(10.0, hs[8] / step_mult(base_r, 8))
                bd = max(3.0, ds[8] / step_mult(base_r, 8)) if ds[8] > 0 else bd
            if bh <= 1.01:
                bh = 15.0
            if 0 < bd <= 1.01:
                bd = 8.0

        base = fam[base_r][2]
        br = get_num(base, "radius", 10) or 10
        brel = get_num(base, "reload", 2.5) or 2.5
        bc = get_int(base, "count", 1)
        # keep per-rung attrs (already linear mech)
        print(f"FIX {name}: base_r={base_r} hp={bh:g} dmg={bd:g}")
        for ri in range(9):
            a, b, block = fam[ri]
            mult = step_mult(base_r, ri)
            attrs = extract_attrs(block)
            new = rewrite(
                block, bh * mult, bd * mult, br, brel,
                get_int(block, "count", bc), RARITY[ri], attrs,
            )
            reps.append((a, b, new))

    reps.sort(key=lambda t: t[0], reverse=True)
    out = text
    for a, b, new in reps:
        out = out[:a] + new + out[b:]
    STATIC_CC.write_text(out, encoding="utf-8")
    print(f"fixed {len(targets)} families, {len(reps)} entries")
    return 0


if __name__ == "__main__":
    sys.exit(main())
