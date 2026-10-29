#!/usr/bin/env python3
"""Retune special-mechanism petals: linear mech attrs by rarity (0..8).

HP/dmg still use the ×3/×2 combat ladder from each family's design-base rarity.
Mechanism fields use lerp(lo, hi, r/8) so Common ≠ Unique.
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

def lerp(a: float, b: float, t: float) -> float:
    return a + (b - a) * t

def t_of(r: int) -> float:
    return r / 8.0

def fmt(x: float) -> str:
    if abs(x - round(x)) < 1e-6:
        return f"{int(round(x))}.0"
    s = f"{x:.4f}".rstrip("0").rstrip(".")
    return s if "." in s else s + ".0"

def step_mult(from_r: int, to_r: int) -> float:
    if to_r == from_r:
        return 1.0
    ascending = to_r > from_r
    a, b = (from_r, to_r) if ascending else (to_r, from_r)
    m = 1.0
    for r in range(a, b):
        m *= 2.0 if r >= 5 else 3.0
    return m if ascending else (1.0 / m)

# Design base rarity + combat stats + linear mech ranges (Common→Unique).
# attrs_extra: non-scaling flags / style kept on every rung.
SPECS: dict[str, dict] = {
    "Magnet": {
        "base_r": 1, "health": 10, "damage": 5, "radius": 12, "reload": 2.5, "count": 1,
        "mech": {"magnet_radius": (500.0, 2200.0)},
        "extra": ".equipment = EquipmentFlags::kMagnet",
    },
    "Accelerator": {
        "base_r": 1, "health": 8, "damage": 5, "radius": 12, "reload": 2.5, "count": 1,
        "mech": {"move_speed_factor": (1.12, 1.55)},
        "extra": ".equipment = EquipmentFlags::kAccelerator",
    },
    "Antennae": {
        "base_r": 4, "health": 5, "damage": 3, "radius": 12.5, "reload": 0.0, "count": 0,
        # lower vision_factor = more FOV
        "mech": {"vision_factor": (0.85, 0.40)},
        "extra": ".equipment = EquipmentFlags::kAntennae",
    },
    "Observer": {
        "base_r": 5, "health": 12, "damage": 5, "radius": 12.5, "reload": 0.0, "count": 0,
        "mech": {"vision_factor": (0.55, 0.18)},
        "extra": ".equipment = EquipmentFlags::kObserver",
    },
    "Faster": {
        "base_r": 1, "health": 10, "damage": 8, "radius": 7, "reload": 0.5, "count": 1,
        "mech": {"extra_rotation_speed": (0.45, 2.20)},
        "extra": "",
    },
    "Golden Leaf": {
        "base_r": 2, "health": 15, "damage": 10, "radius": 10, "reload": 1.0, "count": 1,
        # lower reload factor = faster cooldowns
        "mech": {"extra_reload_factor": (0.97, 0.82), "constant_heal": (1.0, 6.0)},
        "extra": ".icon_angle = -1.0",
    },
    "Third Eye": {
        "base_r": 5, "health": 10, "damage": 5, "radius": 10, "reload": 2.5, "count": 1,
        "mech": {"extra_range": (25.0, 120.0)},
        "extra": ".equipment = EquipmentFlags::kThirdEye",
    },
    "Cutter": {
        "base_r": 3, "health": 15, "damage": 25, "radius": 40, "reload": 2.5, "count": 1,
        "mech": {"extra_body_damage": (12.0, 55.0)},
        "extra": ".equipment = EquipmentFlags::kCutter",
    },
    "Moon": {
        "base_r": 8, "health": 200, "damage": 120, "radius": 50, "reload": 10.0, "count": 1,
        "mech": {"mass": (50.0, 220.0)},
        "extra": "",
    },
    "Lotus": {
        "base_r": 4, "health": 20, "damage": 0, "radius": 12, "reload": 2.0, "count": 1,
        "mech": {"poison_armor": (6.0, 36.0), "constant_heal": (1.0, 8.0)},
        "extra": ".icon_angle = 0.1",
    },
    "Bone": {
        "base_r": 4, "health": 40, "damage": 8, "radius": 12, "reload": 2.5, "count": 1,
        "mech": {"armor": (8.0, 48.0)},
        "extra": ".icon_angle = 1.0",
    },
    "Salt": {
        "base_r": 2, "health": 10, "damage": 12, "radius": 10, "reload": 2.5, "count": 1,
        "mech": {"damage_reflection": (0.12, 0.40)},
        "extra": "",
    },
    "Bubble": {
        "base_r": 1, "health": 5, "damage": 0, "radius": 12, "reload": 1.75, "count": 1,
        # lower secondary_reload = snappier pop cycle
        "mech": {"secondary_reload": (0.55, 0.18)},
        "extra": ".defend_only = 1",
    },
    "Web": {
        "base_r": 1, "health": 5, "damage": 3, "radius": 10, "reload": 3.0, "count": 1,
        "mech": {"secondary_reload": (0.70, 0.28)},
        "extra": ".defend_only = 1",
    },
    "Monstera": {
        "base_r": 4, "health": 30, "damage": 5, "radius": 15, "reload": 4.0, "count": 1,
        "mech": {
            "aura_heal": (4.0, 22.0),
            "aura_radius": (140.0, 420.0),
            "aura_duration": (12.0, 28.0),
            "secondary_reload": (0.70, 0.35),
        },
        "extra": ".defend_only = 1, .icon_angle = -0.35f",
    },
    "Honey": {
        "base_r": 2, "health": 15, "damage": 10, "radius": 12, "reload": 2.5, "count": 1,
        "mech": {"mass": (0.25, 0.70), "secondary_reload": (0.65, 0.30)},
        "extra": "",
    },
    "Meteor": {
        "base_r": 4, "health": 20, "damage": 40, "radius": 18, "reload": 3.5, "count": 1,
        "mech": {
            "mass": (0.40, 1.10),
            "aura_duration": (12.0, 30.0),
            "secondary_reload": (0.65, 0.30),
        },
        "extra": "",
    },
    "Heaviest": {
        "base_r": 4, "health": 200, "damage": 10, "radius": 12, "reload": 15.0, "count": 1,
        "mech": {},
        "extra": "",
    },
    "Yggdrasil": {
        "base_r": 8, "health": 1200, "damage": 0, "radius": 12, "reload": 10.0, "count": 1,
        "mech": {},
        "extra": ".defend_only = 1, .icon_angle = M_PI",
    },
    "Yin Yang": {
        "base_r": 3, "health": 20, "damage": 20, "radius": 10, "reload": 2.5, "count": 1,
        "mech": {"extra_rotation_speed": (0.15, 0.90)},
        "extra": "",
    },
}


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


def build_attrs(spec: dict, r: int) -> str:
    parts = []
    tt = t_of(r)
    for key, (lo, hi) in spec.get("mech", {}).items():
        v = lerp(lo, hi, tt)
        # factors often written with f suffix
        if key.endswith("_factor") or key == "damage_reflection":
            parts.append(f".{key} = {fmt(v)}f")
        else:
            parts.append(f".{key} = {fmt(v)}")
    extra = (spec.get("extra") or "").strip().rstrip(",")
    if extra:
        parts.append(extra)
    if not parts:
        return "{}"
    return "{ " + ", ".join(parts) + " }"


def rewrite(old: str, health: float, damage: float, radius: float,
            reload: float, count: int, rarity: str, attrs: str) -> str:
    name_m = re.search(r'\.name\s*=\s*"([^"]*)"', old)
    desc_m = re.search(r'\.description\s*=\s*"((?:\\.|[^"\\])*)"', old)
    name = name_m.group(1) if name_m else "?"
    desc = desc_m.group(1) if desc_m else ""
    return (
        "{\n"
        f'        .name = "{name}",\n'
        f'        .description = "{desc}",\n'
        f"        .health = {fmt(health)},\n"
        f"        .damage = {fmt(damage)},\n"
        f"        .radius = {fmt(radius)},\n"
        f"        .reload = {fmt(reload)},\n"
        f"        .count = {count},\n"
        f"        .rarity = RarityID::k{rarity},\n"
        f"        .attributes = {attrs}\n"
        "    }"
    )


def main() -> int:
    text = STATIC_CC.read_text(encoding="utf-8")
    entries = parse_blocks(text)
    by_name: dict[str, dict[int, tuple[int, int, str]]] = {}
    for a, b, block in entries:
        nm = re.search(r'\.name\s*=\s*"([^"]*)"', block)
        rar = re.search(r"\.rarity\s*=\s*RarityID::k(\w+)", block)
        if not nm or not rar:
            continue
        if nm.group(1) not in SPECS:
            continue
        ri = RARITY.index(rar.group(1))
        by_name.setdefault(nm.group(1), {})[ri] = (a, b, block)

    replacements = []
    n = 0
    for name, spec in SPECS.items():
        fam = by_name.get(name)
        if not fam:
            print(f"WARN missing {name}")
            continue
        base_r = spec["base_r"]
        if base_r not in fam:
            base_r = min(fam.keys())
        bh, bd = float(spec["health"]), float(spec["damage"])
        br, brel, bc = float(spec["radius"]), float(spec["reload"]), int(spec["count"])
        for ri, (a, b, block) in fam.items():
            mult = step_mult(base_r, ri)
            attrs = build_attrs(spec, ri)
            # Web Mythic+ historically used count=3 — keep mild count climb
            count = bc
            if name == "Web" and ri >= 5:
                count = 3
            elif name == "Web" and ri >= 3:
                count = 2
            new = rewrite(
                block, bh * mult, bd * mult, br, brel, count, RARITY[ri], attrs
            )
            replacements.append((a, b, new))
            n += 1

    replacements.sort(key=lambda t: t[0], reverse=True)
    out = text
    for a, b, new in replacements:
        out = out[:a] + new + out[b:]
    STATIC_CC.write_text(out, encoding="utf-8")
    print(f"retuned {n} special-mech entries across {len(SPECS)} families")
    return 0


if __name__ == "__main__":
    sys.exit(main())
