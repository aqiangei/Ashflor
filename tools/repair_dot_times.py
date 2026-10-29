#!/usr/bin/env python3
"""Repair poison/burn .time values that were wrongly combat-scaled then flattened.

If all 9 rarities share the same large time T, recover design duration as
T / step_mult(0, r) for the r that yields a sensible duration in [0.25, 12].
Prefer Legendary (r=4, ÷81) which was the default design rung in fix_all.
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


def parse_blocks(text: str) -> list[tuple[int, int, str]]:
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


def nest_time(block: str, nest: str) -> float | None:
    m = re.search(rf"\.{nest}\s*=\s*\{{([^}}]*)\}}", block)
    if not m:
        return None
    tm = re.search(r"\.time\s*=\s*([0-9.eE+-]+f?)", m.group(1))
    return float(tm.group(1).rstrip("fF")) if tm else None


def set_nest_time(block: str, nest: str, time: float) -> str:
    m = re.search(rf"\.{nest}\s*=\s*\{{([^}}]*)\}}", block)
    if not m:
        return block
    inner = m.group(1)
    if re.search(r"\.time\s*=", inner):
        inner = re.sub(r"\.time\s*=\s*[0-9.eE+-]+f?", f".time = {fmt(time)}", inner)
    else:
        inner = inner.rstrip().rstrip(",") + f", .time = {fmt(time)}"
    return (
        block[: m.start()]
        + f".{nest} = {{{inner}}}"
        + block[m.end() :]
    )


def recover_time(t: float) -> float | None:
    """If T looks combat-scaled, recover design duration; else None (keep)."""
    if t <= 12.0:
        return None  # already plausible
    # try design rungs from high to mid — Legendary first
    candidates = []
    for r in (4, 5, 3, 6, 2, 7, 1, 8, 0):
        d = t / step_mult(0, r)
        if 0.25 <= d <= 12.0:
            candidates.append((abs(r - 4), r, d))
    if not candidates:
        # last resort: clamp by dividing Unique mult
        d = t / step_mult(0, 8)
        if 0.05 <= d <= 20.0:
            return d
        return None
    candidates.sort()
    return candidates[0][2]


def main() -> int:
    text = STATIC_CC.read_text(encoding="utf-8")
    entries = parse_blocks(text)
    by_name: dict[str, dict[int, tuple[int, int, str]]] = {}
    for a, b, block in entries:
        nm = re.search(r'\.name\s*=\s*"([^"]*)"', block)
        rar = re.search(r"\.rarity\s*=\s*RarityID::k(\w+)", block)
        if not nm or not rar:
            continue
        if nm.group(1) in ("None", "Coin"):
            continue
        try:
            ri = RARITY.index(rar.group(1))
        except ValueError:
            continue
        by_name.setdefault(nm.group(1), {})[ri] = (a, b, block)

    replacements: list[tuple[int, int, str]] = []
    fixed_fam = 0
    for name, fam in sorted(by_name.items()):
        if len(fam) < 9:
            continue
        changed = False
        for nest in ("poison_damage", "burn_damage"):
            times = [nest_time(fam[r][2], nest) for r in range(9)]
            present = [t for t in times if t is not None]
            if len(present) < 9:
                continue
            # all equal (or nearly)
            if max(present) - min(present) > 1e-3:
                # still exploded ladder — take min as design
                design = min(present)
                if design > 12:
                    recovered = recover_time(design)
                    design = recovered if recovered is not None else design
            else:
                recovered = recover_time(present[0])
                if recovered is None:
                    continue
                design = recovered
            for ri in range(9):
                a, b, block = fam[ri]
                new = set_nest_time(block, nest, design)
                if new != block:
                    fam[ri] = (a, b, new)
                    changed = True
            print(f"{name}.{nest}: {present[0]:g} → {design:g}")
        if changed:
            fixed_fam += 1
            for ri in range(9):
                a, b, block = fam[ri]
                replacements.append((a, b, block))

    replacements.sort(key=lambda t: t[0], reverse=True)
    out = text
    for a, b, new in replacements:
        out = out[:a] + new + out[b:]
    STATIC_CC.write_text(out, encoding="utf-8")
    print(f"repaired DoT time families={fixed_fam} entries={len(replacements)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
