#!/usr/bin/env python3
"""Surgical combat fixes only — known-bad families with explicit anchors.

Does NOT auto-detect / rewrite unrelated petals.
Preserves existing attributes (mech already linear).
"""
from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
STATIC_CC = ROOT / "Shared" / "StaticData.cc"
RARITY = [
    "Common", "Unusual", "Rare", "Epic", "Legendary",
    "Mythic", "Ultra", "Super", "Unique",
]

# base_r health/damage at that rarity → ×3/<Mythic ×2 ladder
ANCHORS = {
    # Classic / material / broken Unique leftovers
    "Corn": {"base_r": 1, "health": 50.0, "damage": 5.0},
    "Square": {"base_r": 1, "health": 30.0, "damage": 10.0},
    "Crown Fragment": {"base_r": 1, "health": 12.0, "damage": 8.0},
    "Stick": {"base_r": 4, "health": 10.0, "damage": 1.0},
    "Rice": {"base_r": 1, "health": 5.0, "damage": 8.0},
    "Queen's Emblem": {"base_r": 1, "health": 25.0, "damage": 5.0},
    "Soldier Egg": {"base_r": 2, "health": 40.0, "damage": 1.0},
    "Guard Egg": {"base_r": 1, "health": 35.0, "damage": 1.0},
    "Ancient Seed": {"base_r": 1, "health": 15.0, "damage": 5.0},
    "Colony Heart": {"base_r": 1, "health": 20.0, "damage": 8.0},
    "Heartwood": {"base_r": 1, "health": 40.0, "damage": 12.0},
    "King's Oath": {"base_r": 1, "health": 30.0, "damage": 20.0},
    "Spore Cloud": {"base_r": 1, "health": 12.0, "damage": 5.0},
    # Flat materials (were identical HP/dmg all rarities)
    "Abyss Pearl": {"base_r": 0, "health": 18.0, "damage": 12.0},
    "Ancient Heart": {"base_r": 0, "health": 28.0, "damage": 18.0},
    "Ant Roe": {"base_r": 0, "health": 12.0, "damage": 8.0},
    "Ant Shell": {"base_r": 0, "health": 5.0, "damage": 3.0},
    "Antenna Scrap": {"base_r": 0, "health": 8.0, "damage": 5.0},
    "Bee Barb": {"base_r": 0, "health": 8.0, "damage": 5.0},
    "Blizzard Eye": {"base_r": 0, "health": 18.0, "damage": 12.0},
    "Cactus Sap": {"base_r": 0, "health": 8.0, "damage": 5.0},
    "Capital Badge": {"base_r": 0, "health": 12.0, "damage": 8.0},
    "Carapace": {"base_r": 0, "health": 12.0, "damage": 8.0},
    "Cold Iron": {"base_r": 0, "health": 12.0, "damage": 8.0},
    "Coral": {"base_r": 0, "health": 12.0, "damage": 8.0},
    "Corrupt Spore": {"base_r": 0, "health": 18.0, "damage": 12.0},
    "Fire Crystal": {"base_r": 0, "health": 8.0, "damage": 5.0},
    "Formic Acid": {"base_r": 0, "health": 8.0, "damage": 5.0},
    "Gear Tooth": {"base_r": 0, "health": 8.0, "damage": 5.0},
    "Heartwood Chip": {"base_r": 0, "health": 28.0, "damage": 18.0},
    "Hive Heart": {"base_r": 0, "health": 28.0, "damage": 18.0},
    "Holy Shard": {"base_r": 0, "health": 12.0, "damage": 8.0},
    "Ice Crystal": {"base_r": 0, "health": 5.0, "damage": 3.0},
    "Ice Scale": {"base_r": 0, "health": 12.0, "damage": 8.0},
    "Lava Rock": {"base_r": 0, "health": 12.0, "damage": 8.0},
    "Leaf Scrap": {"base_r": 0, "health": 5.0, "damage": 3.0},
    "Leyline Core": {"base_r": 0, "health": 28.0, "damage": 18.0},
    "Obsidian Shard": {"base_r": 0, "health": 12.0, "damage": 8.0},
    "Permafrost Heart": {"base_r": 0, "health": 28.0, "damage": 18.0},
    "Phoenix Ember": {"base_r": 0, "health": 18.0, "damage": 12.0},
    "Pollen Dust": {"base_r": 0, "health": 5.0, "damage": 3.0},
    "Queen Pheromone": {"base_r": 0, "health": 18.0, "damage": 12.0},
    "Refined Steel": {"base_r": 0, "health": 8.0, "damage": 5.0},
    "Resin": {"base_r": 0, "health": 8.0, "damage": 5.0},
    "Royal Jelly": {"base_r": 0, "health": 18.0, "damage": 12.0},
    "Royal Seal": {"base_r": 0, "health": 18.0, "damage": 12.0},
    "Sand Grain": {"base_r": 0, "health": 5.0, "damage": 3.0},
    "Sandstorm Eye": {"base_r": 0, "health": 18.0, "damage": 12.0},
    "Sandworm Core": {"base_r": 0, "health": 28.0, "damage": 18.0},
    "Sea Salt": {"base_r": 0, "health": 5.0, "damage": 3.0},
    "Seashell": {"base_r": 0, "health": 8.0, "damage": 5.0},
    "Snow Fluff": {"base_r": 0, "health": 5.0, "damage": 3.0},
    "Sovereign Core": {"base_r": 0, "health": 28.0, "damage": 18.0},
    "Spore Dust": {"base_r": 0, "health": 8.0, "damage": 5.0},
    "Sulfur": {"base_r": 0, "health": 5.0, "damage": 3.0},
    "Tide Core": {"base_r": 0, "health": 28.0, "damage": 18.0},
    "Twig": {"base_r": 0, "health": 5.0, "damage": 3.0},
    "Venom Barb": {"base_r": 0, "health": 8.0, "damage": 5.0},
    "Venom Sac": {"base_r": 0, "health": 12.0, "damage": 8.0},
    "Vine Strand": {"base_r": 0, "health": 5.0, "damage": 3.0},
    "Water Drop": {"base_r": 0, "health": 5.0, "damage": 3.0},
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
    body_start, body = start, text[start:end]
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


def get_num(block, key, default=None):
    m = re.search(rf"\.{key}\s*=\s*([0-9.eE+-]+f?)", block)
    return float(m.group(1).rstrip("fF")) if m else default


def get_int(block, key, default=1):
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
    return (
        "{\n"
        f'        .name = "{name_m.group(1) if name_m else "?"}",\n'
        f'        .description = "{desc_m.group(1) if desc_m else ""}",\n'
        f"        .health = {fmt(health)},\n"
        f"        .damage = {fmt(damage)},\n"
        f"        .radius = {fmt(radius)},\n"
        f"        .reload = {fmt(reload)},\n"
        f"        .count = {count},\n"
        f"        .rarity = RarityID::k{rarity},\n"
        f"        .attributes = {attrs}\n"
        "    }"
    )


def main() -> int:
    text = STATIC_CC.read_text(encoding="utf-8")
    entries = parse_blocks(text)
    by_name = {}
    for a, b, block in entries:
        nm = re.search(r'\.name\s*=\s*"([^"]*)"', block)
        rar = re.search(r"\.rarity\s*=\s*RarityID::k(\w+)", block)
        if not nm or not rar:
            continue
        by_name.setdefault(nm.group(1), {})[RARITY.index(rar.group(1))] = (a, b, block)

    reps = []
    for name, ov in ANCHORS.items():
        fam = by_name.get(name)
        if not fam or len(fam) < 9:
            print(f"WARN missing {name}")
            continue
        base_r = int(ov["base_r"])
        bh, bd = float(ov["health"]), float(ov["damage"])
        base = fam[base_r][2]
        br = get_num(base, "radius", 10) or 10
        brel = get_num(base, "reload", 2.5) or 2.5
        print(f"{name}: r{base_r} hp={bh:g} dmg={bd:g}")
        for ri in range(9):
            a, b, block = fam[ri]
            mult = step_mult(base_r, ri)
            new = rewrite(
                block, bh * mult, bd * mult, br, brel,
                get_int(block, "count", 1), RARITY[ri], extract_attrs(block),
            )
            reps.append((a, b, new))

    reps.sort(key=lambda t: t[0], reverse=True)
    out = text
    for a, b, new in reps:
        out = out[:a] + new + out[b:]
    STATIC_CC.write_text(out, encoding="utf-8")
    print(f"surgical combat: {len(ANCHORS)} families, {len(reps)} entries")
    return 0


if __name__ == "__main__":
    sys.exit(main())
