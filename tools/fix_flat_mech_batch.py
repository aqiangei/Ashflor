#!/usr/bin/env python3
"""Pin mechanism attrs for the 7 entity-missing-audit petals.

Design (todo/entity-missing-audit): special attrs stay FIXED across the rarity
ladder; only hp/dmg scale. Do NOT linearize CC / pen / execute / aura by rarity
(that recreates permanent slow / overtuned Unique auras).
"""
from __future__ import annotations

import re
import sys
from pathlib import Path

STATIC_CC = Path(__file__).resolve().parents[1] / "Shared" / "StaticData.cc"
RARITY = [
    "Common", "Unusual", "Rare", "Epic", "Legendary",
    "Mythic", "Ultra", "Super", "Unique",
]

# Exact attribute blocks written to every rarity of that English name.
FIXED_ATTRS = {
    "Frost Starfish": "{ .slow_inflict_seconds = 0.65 }",
    "Molten Sand Blade": "{ .burn_damage = { .damage = 6.0, .time = 3.0 } }",
    "Heartwood": (
        "{ .secondary_reload = 0.45, .aura_heal = 14.0, .aura_radius = 145.0, "
        ".aura_duration = 16.0, .defend_only = 1 }"
    ),
    "Sand Spike": "{ .armor_penetration = 0.12f }",
    "Snowball": "{ .slow_inflict_seconds = 0.40 }",
    "Chitin Needle": "{ .armor_penetration = 0.08f }",
    "Mantis Claw": "{ .execute_threshold = 0.20f, .execute_factor = 1.30f }",
}


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


def set_attributes(block: str, attrs: str) -> str:
    return re.sub(
        r"\.attributes\s*=\s*\{[^{}]*(?:\{[^{}]*\}[^{}]*)*\}",
        f".attributes = {attrs}",
        block,
        count=1,
    )


def main() -> int:
    text = STATIC_CC.read_text(encoding="utf-8")
    entries = parse_blocks(text)
    reps = []
    counts = {n: 0 for n in FIXED_ATTRS}
    for a, b, block in entries:
        nm = re.search(r'\.name\s*=\s*"([^"]*)"', block)
        if not nm or nm.group(1) not in FIXED_ATTRS:
            continue
        name = nm.group(1)
        new = set_attributes(block, FIXED_ATTRS[name])
        reps.append((a, b, new))
        counts[name] += 1

    reps.sort(key=lambda x: x[0], reverse=True)
    out = text
    for a, b, new in reps:
        out = out[:a] + new + out[b:]
    STATIC_CC.write_text(out, encoding="utf-8")
    for name, n in counts.items():
        print(f"{name}: {n}")
    print(f"patched {len(reps)} entries (fixed attrs, no rarity lerp)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
