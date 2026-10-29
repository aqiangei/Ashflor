#!/usr/bin/env python3
"""Pin known DoT durations that were recovered too short from scaled junk."""
from __future__ import annotations
import re
import sys
from pathlib import Path

STATIC_CC = Path(__file__).resolve().parents[1] / "Shared" / "StaticData.cc"
RARITY = [
    "Common", "Unusual", "Rare", "Epic", "Legendary",
    "Mythic", "Ultra", "Super", "Unique",
]
# name → (nest, time)
PINS = {
    "Pincer": ("poison_damage", 2.0),
    "Royal Rose": ("poison_damage", 3.0),
    "Chaos Prism": ("poison_damage", 2.5),
    "Thorn Trap": ("poison_damage", 2.0),
    "Venom Cocoon": ("poison_damage", 2.5),
    "Acid Droplet": ("poison_damage", 1.5),
}


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
    for name, (nest, t) in PINS.items():
        fam = by_name.get(name)
        if not fam:
            print(f"WARN {name}")
            continue
        print(f"{name}.{nest} → {t}")
        for ri, (a, b, block) in fam.items():
            m = re.search(rf"\.{nest}\s*=\s*\{{([^}}]*)\}}", block)
            if not m:
                continue
            inner = re.sub(r"\.time\s*=\s*[0-9.eE+-]+f?", f".time = {fmt(t)}", m.group(1))
            new = block[: m.start()] + f".{nest} = {{{inner}}}" + block[m.end() :]
            reps.append((a, b, new))

    reps.sort(key=lambda x: x[0], reverse=True)
    out = text
    for a, b, new in reps:
        out = out[:a] + new + out[b:]
    STATIC_CC.write_text(out, encoding="utf-8")
    print(f"pinned {len(reps)} entries")
    return 0


if __name__ == "__main__":
    sys.exit(main())
