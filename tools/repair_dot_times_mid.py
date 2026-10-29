#!/usr/bin/env python3
"""Fix flat DoT times in (6,12] that are Legendary-scaled (T/81 in [0.1, 0.5]).

Does NOT touch real 2–6s durations (Iris/Fireball/etc.).
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


def nest_time(block, nest):
    m = re.search(rf"\.{nest}\s*=\s*\{{([^}}]*)\}}", block)
    if not m:
        return None
    tm = re.search(r"\.time\s*=\s*([0-9.eE+-]+f?)", m.group(1))
    return float(tm.group(1).rstrip("fF")) if tm else None


def set_nest_time(block, nest, time):
    m = re.search(rf"\.{nest}\s*=\s*\{{([^}}]*)\}}", block)
    if not m:
        return block
    inner = re.sub(r"\.time\s*=\s*[0-9.eE+-]+f?", f".time = {fmt(time)}", m.group(1))
    return block[: m.start()] + f".{nest} = {{{inner}}}" + block[m.end() :]


def main() -> int:
    text = STATIC_CC.read_text(encoding="utf-8")
    entries = parse_blocks(text)
    by_name = {}
    for a, b, block in entries:
        nm = re.search(r'\.name\s*=\s*"([^"]*)"', block)
        rar = re.search(r"\.rarity\s*=\s*RarityID::k(\w+)", block)
        if not nm or not rar or nm.group(1) in ("None", "Coin"):
            continue
        by_name.setdefault(nm.group(1), {})[RARITY.index(rar.group(1))] = (a, b, block)

    reps = []
    for name, fam in sorted(by_name.items()):
        if len(fam) < 9:
            continue
        for nest in ("poison_damage", "burn_damage"):
            times = [nest_time(fam[r][2], nest) for r in range(9)]
            if any(t is None for t in times):
                continue
            if max(times) - min(times) > 1e-3:
                continue
            t = times[0]
            if not (6.0 < t <= 12.0):
                continue
            d = t / 81.0
            if not (0.1 <= d <= 0.5):
                continue
            print(f"{name}.{nest}: {t:g} → {d:g}")
            for ri in range(9):
                a, b, block = fam[ri]
                new = set_nest_time(block, nest, d)
                reps.append((a, b, new))

    reps.sort(key=lambda x: x[0], reverse=True)
    out = text
    for a, b, new in reps:
        out = out[:a] + new + out[b:]
    STATIC_CC.write_text(out, encoding="utf-8")
    print(f"mid-pass entries={len(reps)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
