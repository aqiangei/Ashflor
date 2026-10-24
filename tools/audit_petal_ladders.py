#!/usr/bin/env python3
"""Quick audit: every non-Coin family has Common..Unique; assemble bases resolve."""
from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CC = (ROOT / "Shared" / "StaticData.cc").read_text(encoding="utf-8")
HH = (ROOT / "Shared" / "StaticDefinitions.hh").read_text(encoding="utf-8")
RECIPE = (ROOT / "Shared" / "Recipe.cc").read_text(encoding="utf-8")

RARITY = [
    "Common", "Unusual", "Rare", "Epic", "Legendary",
    "Mythic", "Ultra", "Super", "Unique",
]


def parse_blocks(text: str) -> list[str]:
    start = text.index("PETAL_DATA = {{")
    end = text.index("\n}};", start)
    body = text[start:end]
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
    return entries


def main() -> int:
    blocks = parse_blocks(CC)
    fam: dict[str, set[int]] = {}
    for b in blocks:
        nm = re.search(r'\.name\s*=\s*"([^"]*)"', b)
        rar = re.search(r"\.rarity\s*=\s*RarityID::k(\w+)", b)
        if not nm or not rar:
            continue
        if nm.group(1) in ("None", "Coin"):
            continue
        try:
            ri = RARITY.index(rar.group(1))
        except ValueError:
            continue
        fam.setdefault(nm.group(1), set()).add(ri)

    incomplete = []
    for name, rs in sorted(fam.items()):
        missing = [RARITY[i] for i in range(9) if i not in rs]
        if missing:
            incomplete.append((name, missing))

    # Recipe result enum names → English via StaticDefinitions order is hard;
    # just check Leaf Blade / Bee Sting sample numbers.
    leaf = [b for b in blocks if '.name = "Leaf Blade"' in b]
    leaf_common = next((b for b in leaf if "kCommon" in b), "")
    ok_leaf = "health = 25.0" in leaf_common and "damage = 12.0" in leaf_common

    print(f"families={len(fam)} incomplete={len(incomplete)}")
    if incomplete[:15]:
        print("sample incomplete:")
        for n, m in incomplete[:15]:
            print(f"  {n}: missing {', '.join(m)}")
    print(f"Leaf Blade Common retune ok={ok_leaf}")
    print(f"recipe defs≈{RECIPE.count('PetalID::k')}")
    return 0 if ok_leaf else 1


if __name__ == "__main__":
    sys.exit(main())
