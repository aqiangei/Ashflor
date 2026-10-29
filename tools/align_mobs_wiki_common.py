#!/usr/bin/env python3
"""Align MOB_DATA base rarity/hp/damage to wiki ZH Common rows.

Handles both multi-line and single-line field layouts in StaticData.cc.
"""
from __future__ import annotations

import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
STATIC = ROOT / "Shared" / "StaticData.cc"
WIKI = ROOT.parent / "temp" / "wiki_research" / "wiki_zh_mobs_reparsed.json"

# wiki ZH → Ashflor MOB_DATA .name (first block with that name)
ALIGN: dict[str, str] = {
    "兵蚁": "Soldier Ant",
    "工蚁": "Worker Ant",
    "幼蚁": "Baby Ant",
    "火兵蚁": "Soldier Fire Ant",
    "蚁后": "Queen Ant",
    "黄蜂": "Hornet",
    "胡蜂": "Jungle Wasp",
    "蜘蛛": "Spider",
    "甲虫": "Beetle",
    "蝎子": "Scorpion",
    "蜈蚣_邪恶_": "Evil Centipede",
    "白兵蚁": "Soldier Termite",
    "水母": "Moon Jelly",
    "海星_生物_": "Coral Star",
    "螃蟹": "Reef Crab",
    "水蛭": "Sea Leech",
    "螳螂": "Emerald Mantis",
    "瓢虫_深色_": "Ladybug",
    # Skip 蠕虫→Sandworm Monarch (Unique boss; baked Unique base, not Common ladder)
    # Skip 火蚁后 (no dedicated mob)
}


def fmt(x: float) -> str:
    if abs(x - round(x)) < 1e-6:
        return f"{int(round(x))}.0"
    return f"{x:.4g}"


def patch_block(block: str, hp: float, dmg: float) -> str | None:
    """Rewrite rarity/health/damage inside one mob struct block."""
    if ".rarity = " not in block or ".health = " not in block:
        return None
    out = re.sub(
        r"\.rarity = RarityID::k\w+",
        ".rarity = RarityID::kCommon",
        block,
        count=1,
    )
    out = re.sub(
        r"\.health = \{?\s*[0-9.eE+-]+\s*\}?",
        f".health = {{{fmt(hp)}}}",
        out,
        count=1,
    )
    out = re.sub(
        r"\.damage = [0-9.eE+-]+",
        f".damage = {fmt(dmg)}",
        out,
        count=1,
    )
    return out


def main() -> int:
    wiki = json.loads(WIKI.read_text(encoding="utf-8"))
    text = STATIC.read_text(encoding="utf-8")
    m = re.search(r"MOB_DATA\s*=\s*\{\{", text)
    if not m:
        raise SystemExit("MOB_DATA missing")
    i = text.index("{{", m.start()) + 1
    depth = 0
    end = None
    for j in range(i, len(text)):
        if text[j] == "{":
            depth += 1
        elif text[j] == "}":
            depth -= 1
            if depth == 0:
                end = j + 1
                break
    assert end is not None
    head, mid, tail = text[: m.start()], text[m.start() : end], text[end:]

    # Split mid into mob blocks at depth-1 of content after {{
    # Simpler: find each `.name = "X"` and expand to enclosing `{ ... }`
    changed = []
    for zh, ash_name in ALIGN.items():
        w = wiki.get(zh) or {}
        c = w.get("Common") or {}
        if "health" not in c:
            print(f"skip {zh}: no wiki Common")
            continue
        hp = float(c["health"])
        dmg = float(c.get("damage", 0.0))
        needle = f'.name = "{ash_name}"'
        pos = mid.find(needle)
        if pos < 0:
            print(f"MISS name {ash_name} ({zh})")
            continue
        # walk back to opening `{` of this struct
        lb = mid.rfind("{", 0, pos)
        if lb < 0:
            print(f"MISS open brace {ash_name}")
            continue
        depth = 0
        rb = None
        for j in range(lb, len(mid)):
            if mid[j] == "{":
                depth += 1
            elif mid[j] == "}":
                depth -= 1
                if depth == 0:
                    rb = j + 1
                    break
        if rb is None:
            print(f"MISS close brace {ash_name}")
            continue
        old = mid[lb:rb]
        new = patch_block(old, hp, dmg)
        if new is None or new == old:
            # still report if already correct
            if new == old:
                print(f"unchanged {ash_name} (already aligned?)")
            continue
        mid = mid[:lb] + new + mid[rb:]
        changed.append(f"{ash_name}: Common hp={fmt(hp)} dmg={fmt(dmg)} ← {zh}")

    STATIC.write_text(head + mid + tail, encoding="utf-8")
    print(f"updated {len(changed)} mobs:")
    for line in changed:
        print(" ", line)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
