#!/usr/bin/env python3
"""Dump mechanism attrs across rarities for key special petals."""
from __future__ import annotations
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CC = (ROOT / "Shared" / "StaticData.cc").read_text(encoding="utf-8")
RARITY = ["Common","Unusual","Rare","Epic","Legendary","Mythic","Ultra","Super","Unique"]
WATCH = {
    "Magnet", "Accelerator", "Antennae", "Moon", "Golden Leaf", "Faster",
    "Third Eye", "Observer", "Cutter", "Yggdrasil", "Lotus", "Honey", "Meteor",
    "Bubble", "Web", "Yin Yang", "Heaviest", "Bone", "Salt", "Monstera",
}
KEYS = (
    "magnet_radius", "move_speed_factor", "vision_factor", "extra_rotation_speed",
    "extra_range", "extra_reload_factor", "extra_body_damage", "extra_damage_factor",
    "damage_reflection", "constant_heal", "poison_armor", "armor", "mass",
    "aura_heal", "aura_radius", "aura_duration", "secondary_reload", "health", "damage",
)

start = CC.index("PETAL_DATA = {{")
end = CC.index("\n}};", start)
body = CC[start:end]
entries, depth, begin = [], 0, None
for j, ch in enumerate(body):
    if ch == "{":
        depth += 1
        if depth == 3 and begin is None:
            begin = j
    elif ch == "}":
        if depth == 3 and begin is not None:
            entries.append(body[begin:j+1]); begin = None
        depth -= 1

fam = {n: {} for n in WATCH}
for b in entries:
    nm = re.search(r'\.name\s*=\s*"([^"]*)"', b)
    rar = re.search(r"\.rarity\s*=\s*RarityID::k(\w+)", b)
    if not nm or not rar or nm.group(1) not in WATCH:
        continue
    ri = RARITY.index(rar.group(1))
    row = {}
    for k in KEYS:
        m = re.search(rf"\.{k}\s*=\s*([0-9.eE+-]+f?)", b)
        if m:
            row[k] = m.group(1).rstrip("f")
    fam[nm.group(1)][ri] = row

for name in sorted(WATCH):
    rows = fam[name]
    if not rows:
        print(f"\n## {name} MISSING")
        continue
    print(f"\n## {name}")
    for ri in range(9):
        if ri not in rows:
            print(f"  {RARITY[ri]}: MISSING")
            continue
        r = rows[ri]
        mech = {k: r[k] for k in r if k not in ("health", "damage")}
        print(f"  {RARITY[ri]}: hp={r.get('health','?')} dmg={r.get('damage','?')} {mech}")
