#!/usr/bin/env python3
"""Verify the 7 entity-missing-audit petals: fixed specials + Heartwood plant path."""
from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CC = (ROOT / "Shared" / "StaticData.cc").read_text(encoding="utf-8")
PETAL_CC = (ROOT / "Server" / "Process" / "Petal.cc").read_text(encoding="utf-8")

CHECKS = {
    "Frost Starfish": [("slow_inflict_seconds", 0.65)],
    "Molten Sand Blade": [("damage", 6.0), ("time", 3.0)],
    "Heartwood": [
        ("aura_heal", 14.0),
        ("aura_radius", 145.0),
        ("aura_duration", 16.0),
        ("secondary_reload", 0.45),
        ("defend_only", 1.0),
    ],
    "Sand Spike": [("armor_penetration", 0.12)],
    "Snowball": [("slow_inflict_seconds", 0.40)],
    "Chitin Needle": [("armor_penetration", 0.08)],
    "Mantis Claw": [("execute_threshold", 0.20), ("execute_factor", 1.30)],
}


def main() -> int:
    fails = 0
    print("## seven-petal fixed attrs")
    for name, pairs in CHECKS.items():
        n = 0
        bad = []
        for m in re.finditer(rf'\.name\s*=\s*"{re.escape(name)}"', CC):
            n += 1
            chunk = CC[m.start() : m.start() + 1000]
            am = re.search(
                r"\.attributes\s*=\s*(\{[^{}]*(?:\{[^{}]*\}[^{}]*)*\})", chunk
            )
            if not am:
                bad.append("no attrs")
                continue
            attrs = am.group(1)
            for k, v in pairs:
                km = re.search(rf"\.{k}\s*=\s*([0-9.]+)f?", attrs)
                if not km or abs(float(km.group(1)) - v) > 1e-6:
                    bad.append(f"{k}!={v}")
                    break
        ok = n == 9 and not bad
        mark = "PASS" if ok else "FAIL"
        detail = f"rungs={n}" + (f" {bad[:3]}" if bad else "")
        print(f"  [{mark}] {name} — {detail}")
        if not ok:
            fails += 1

    print("\n## Heartwood plant path")
    planted = (
        'petal_name_is(pid, "Heartwood")' in PETAL_CC
        and "set_split_projectile" in PETAL_CC
    )
    mark = "PASS" if planted else "FAIL"
    print(f"  [{mark}] Server/Process/Petal.cc Heartwood grounded aura")
    if not planted:
        fails += 1

    print(f"\n# seven-petal: {'PASS' if fails == 0 else f'{fails} FAIL(s)'}")
    return 0 if fails == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
