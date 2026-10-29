#!/usr/bin/env python3
"""Align selected petal families' health/damage to wiki_en Unique/Common ×3.

Only touches families listed in TARGETS (batch E). Does not rewrite Bubble —
wiki Bubble Unique uses mob-HP table (parse error); ash petal curve is kept.
"""
from __future__ import annotations

import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
STATIC = ROOT / "Shared" / "StaticData.cc"
WIKI = ROOT.parent / "temp" / "wiki_research" / "wiki_en_petals_scaled.json"

RARITY = [
    "Common", "Unusual", "Rare", "Epic", "Legendary",
    "Mythic", "Ultra", "Super", "Unique",
]
COMBAT = [1.0, 3.0, 9.0, 27.0, 81.0, 243.0, 729.0, 2187.0, 6561.0]

# Only petals where wiki combat ladder is trustworthy petal-scale
TARGETS = ("Faster",)


def fmt(x: float) -> str:
    if abs(x - round(x)) < 1e-6:
        return f"{int(round(x))}.0"
    s = f"{x:.6g}"
    return s if "." in s or "e" in s.lower() else s + ".0"


def parse_blocks(text: str) -> list[tuple[int, int, str]]:
    start = text.index("PETAL_DATA = {{")
    end = text.index("\n}};", start)
    body_start = start
    body = text[start:end]
    out = []
    depth = 0
    begin = None
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


def main() -> int:
    wiki = json.loads(WIKI.read_text(encoding="utf-8"))
    text = STATIC.read_text(encoding="utf-8")
    entries = parse_blocks(text)
    by: dict[str, dict[int, tuple[int, int, str]]] = {}
    for a, b, block in entries:
        nm = re.search(r'\.name\s*=\s*"([^"]*)"', block)
        rar = re.search(r"\.rarity\s*=\s*RarityID::k(\w+)", block)
        if not nm or not rar:
            continue
        name = nm.group(1)
        if name not in TARGETS:
            continue
        ri = RARITY.index(rar.group(1))
        by.setdefault(name, {})[ri] = (a, b, block)

    reps: list[tuple[int, int, str]] = []
    for name in TARGETS:
        w = wiki.get(name) or {}
        wc = w.get("Common") or {}
        if "health" not in wc and "damage" not in wc:
            print(f"skip {name}: no wiki Common")
            continue
        fam = by.get(name)
        if not fam or len(fam) < 9:
            print(f"skip {name}: incomplete ash ladder")
            continue
        bh = float(wc.get("health") or 0.0)
        bd = float(wc.get("damage") or 0.0)
        for ri in range(9):
            a, b, block = fam[ri]
            h = bh * (COMBAT[ri] / COMBAT[0]) if bh else None
            d = bd * (COMBAT[ri] / COMBAT[0]) if bd else None
            new = block
            if h is not None:
                new = re.sub(r"\.health\s*=\s*[0-9.eE+-]+", f".health = {fmt(h)}", new, count=1)
            if d is not None:
                new = re.sub(r"\.damage\s*=\s*[0-9.eE+-]+", f".damage = {fmt(d)}", new, count=1)
            reps.append((a, b, new))
        print(f"{name}: Common hp={bh} dmg={bd} → Unique hp={bh*6561} dmg={bd*6561}")

    reps.sort(key=lambda t: t[0], reverse=True)
    out = text
    for a, b, new in reps:
        out = out[:a] + new + out[b:]
    STATIC.write_text(out, encoding="utf-8")
    print(f"rewrote {len(reps)} blocks")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
