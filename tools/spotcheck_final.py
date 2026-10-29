#!/usr/bin/env python3
"""Final spot-check: combat monotonic + key mech ranges."""
from __future__ import annotations
import re
from collections import defaultdict
from pathlib import Path

CC = (Path(__file__).resolve().parents[1] / "Shared" / "StaticData.cc").read_text(encoding="utf-8")
RARITY = ["Common", "Unusual", "Rare", "Epic", "Legendary", "Mythic", "Ultra", "Super", "Unique"]
WATCH = {
    "Basic": ["health", "damage"],
    "Leaf Blade": ["health", "damage", "count"],
    "Corn": ["health", "damage"],
    "Cactus": ["health", "damage", "damage_reflection"],
    "Magnet": ["health", "magnet_radius"],
    "Accelerator": ["move_speed_factor"],
    "Antennae": ["vision_factor"],
    "Golden Leaf": ["extra_reload_factor", "constant_heal"],
    "Acid Droplet": ["poison"],
    "Pincer": ["poison"],
    "Fireball": ["burn"],
    "Iris": ["poison"],
    "Square": ["health", "damage"],
    "Stick": ["health", "damage", "secondary_reload"],
    "Wet Leaf": ["move_speed_factor", "slow_inflict_seconds"],
}

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
    row = {}
    for k in ("health", "damage", "count"):
        m = re.search(rf"\.{k}\s*=\s*([0-9.eE+-]+)", b)
        if m:
            row[k] = m.group(1)
    for k in (
        "magnet_radius", "move_speed_factor", "vision_factor",
        "extra_reload_factor", "constant_heal", "damage_reflection",
        "secondary_reload", "slow_inflict_seconds",
    ):
        m = re.search(rf"\.{k}\s*=\s*([0-9.eE+-]+f?)", b)
        if m:
            row[k] = m.group(1)
    pm = re.search(r"\.poison_damage\s*=\s*\{([^}]*)\}", b)
    if pm:
        row["poison"] = " ".join(pm.group(1).split())
    bm = re.search(r"\.burn_damage\s*=\s*\{([^}]*)\}", b)
    if bm:
        row["burn"] = " ".join(bm.group(1).split())
    fam[nm.group(1)][ri] = row

# sanity flags
flags = []
for name in sorted(WATCH):
    print(f"\n## {name}")
    for ri in range(9):
        r = fam[name].get(ri, {})
        keys = WATCH[name]
        bits = " ".join(f"{k}={r.get(k,'?')}" for k in keys)
        print(f"  {RARITY[ri]}: {bits}")
    hs = [float(fam[name][ri]["health"]) for ri in range(9) if "health" in fam[name].get(ri, {})]
    if len(hs) == 9:
        if hs[8] > 50000:
            flags.append(f"{name}: Unique HP {hs[8]:g} very high")
        if any(hs[i] > hs[i + 1] for i in range(8)):
            flags.append(f"{name}: HP regress")
    if name == "Leaf Blade":
        if fam[name][0].get("health") != "25.0":
            flags.append(f"Leaf Blade Common hp want 25 got {fam[name][0].get('health')}")
    if name == "Magnet":
        if fam[name][0].get("magnet_radius") != "500.0" or fam[name][8].get("magnet_radius") != "2200.0":
            flags.append(f"Magnet radius ladder off: {fam[name][0].get('magnet_radius')}→{fam[name][8].get('magnet_radius')}")
    if name == "Fireball" and "burn" in fam[name][0]:
        if "time = 0." in fam[name][0]["burn"] and "time = 0.1" in fam[name][0]["burn"]:
            flags.append("Fireball burn time looks over-shrunk")
        t = re.search(r"time = ([0-9.]+)", fam[name][0]["burn"])
        if t and float(t.group(1)) < 1.0:
            flags.append(f"Fireball burn time {t.group(1)} suspiciously low")

print("\n# FLAGS")
for f in flags or ["(none)"]:
    print(" ", f)
