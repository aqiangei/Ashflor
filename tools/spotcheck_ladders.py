#!/usr/bin/env python3
from __future__ import annotations
import re
from collections import defaultdict
from pathlib import Path

CC = (Path(__file__).resolve().parents[1] / "Shared" / "StaticData.cc").read_text(encoding="utf-8")
RARITY = ["Common", "Unusual", "Rare", "Epic", "Legendary", "Mythic", "Ultra", "Super", "Unique"]
WATCH = [
    "Corn", "Crown Fragment", "Square", "Stick", "Acid Droplet", "Cactus",
    "Magnet", "Queen's Emblem", "Soldier Egg", "Wet Leaf", "Colony Core",
    "Queen's Emblem", "Pincer", "Royal Rose",
]

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
            entries.append(body[begin : j + 1])
            begin = None
        depth -= 1

fam = defaultdict(dict)
for b in entries:
    nm = re.search(r'\.name\s*=\s*"([^"]*)"', b)
    rar = re.search(r"\.rarity\s*=\s*RarityID::k(\w+)", b)
    if not nm or not rar or nm.group(1) not in WATCH:
        continue
    ri = RARITY.index(rar.group(1))
    h = re.search(r"\.health\s*=\s*([0-9.eE+-]+)", b)
    d = re.search(r"\.damage\s*=\s*([0-9.eE+-]+)", b)
    row = {"h": h.group(1) if h else "?", "d": d.group(1) if d else "?"}
    for k in (
        "damage_reflection", "move_speed_factor", "magnet_radius",
        "secondary_reload", "slow_inflict_seconds", "spawn_count",
    ):
        m = re.search(rf"\.{k}\s*=\s*([0-9.eE+-]+f?)", b)
        if m:
            row[k] = m.group(1)
    pm = re.search(r"\.poison_damage\s*=\s*\{([^}]*)\}", b)
    if pm:
        row["poison"] = pm.group(1).strip()
    fam[nm.group(1)][ri] = row

for name in WATCH:
    print("==", name)
    for ri in range(9):
        print(" ", RARITY[ri], fam[name].get(ri))
