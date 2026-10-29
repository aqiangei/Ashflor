#!/usr/bin/env python3
"""Retune assemble-product HP/dmg (+ optional fields) and resync ladders.

Keeps each family's lowest-rarity attributes as the template (mechanics stay),
scales combat numbers and scalable attr fields across Common..Unique.
Anchors from todo/make/花瓣制作组合.txt map-base crafts.
"""
from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
STATIC_CC = ROOT / "Shared" / "StaticData.cc"

RARITY_NAMES = [
    "Common", "Unusual", "Rare", "Epic", "Legendary",
    "Mythic", "Ultra", "Super", "Unique",
]

# English name → overrides at base_rarity (default: family's lowest present), then laddered.
# Omit a key to keep the existing base value. "base_rarity": "Unusual"|… pins the design rung.
ANCHORS: dict[str, dict] = {
    "Leaf Blade": {"health": 25.0, "damage": 12.0, "radius": 12.0, "reload": 1.6, "count": 3},
    "Bee Sting": {
        "base_rarity": "Unusual", "health": 18.0, "damage": 18.0,
        "poison_damage": 3.0, "poison_time": 2.0,
    },
    "Healing Bloom": {
        "base_rarity": "Unusual", "health": 18.0, "damage": 0.0, "constant_heal": 6.0,
    },
    "Sand Spike": {"base_rarity": "Unusual", "health": 32.0, "damage": 16.0},
    "Craft Cactus": {
        "base_rarity": "Unusual", "health": 65.0, "damage": 10.0, "damage_reflection": 0.20,
    },
    "Quicksand Orb": {
        "base_rarity": "Rare", "health": 22.0, "damage": 8.0, "slow_inflict_seconds": 2.0,
    },
    "Water Bubble": {"base_rarity": "Rare", "health": 15.0, "damage": 10.0},
    "Starfish": {"base_rarity": "Rare", "health": 40.0, "damage": 15.0},
    "Tide Pearl": {"base_rarity": "Rare", "health": 28.0, "damage": 12.0},
    "Ice Spike": {
        "base_rarity": "Unusual", "health": 22.0, "damage": 20.0, "slow_inflict_seconds": 1.5,
    },
    "Snowball": {"base_rarity": "Unusual", "health": 45.0, "damage": 11.0},
    "Ice Shield": {"base_rarity": "Rare", "health": 110.0, "damage": 6.0},
    "Thorn Vine": {"base_rarity": "Unusual", "health": 55.0, "damage": 12.0},
    "Poison Spore": {
        "health": 12.0, "damage": 4.0, "poison_damage": 4.0, "poison_time": 4.0,
    },
    "Rotwood": {
        "base_rarity": "Rare", "health": 120.0, "damage": 8.0,
        "poison_damage": 6.0, "poison_time": 2.0,
    },
    "Ant Fang": {"base_rarity": "Unusual", "health": 30.0, "damage": 19.0},
    "Acid Flask": {
        "base_rarity": "Unusual", "health": 16.0, "damage": 9.0,
        "poison_damage": 3.0, "poison_time": 4.0,
    },
    "Fireball": {
        "base_rarity": "Unusual", "health": 20.0, "damage": 22.0,
        "burn_damage": 4.0, "burn_time": 3.0,
    },
    "Magma Core": {
        "base_rarity": "Unusual", "health": 75.0, "damage": 28.0,
        "burn_damage": 2.0, "burn_time": 3.0,
    },
    "Obsidian Blade": {"base_rarity": "Unusual", "health": 24.0, "damage": 34.0},
    "Steel Sword": {"base_rarity": "Unusual", "health": 50.0, "damage": 32.0, "reload": 1.9},
    "Holy Orb": {"base_rarity": "Unusual", "health": 24.0, "damage": 18.0},
    "Mech Gear": {"base_rarity": "Unusual", "health": 80.0, "damage": 20.0},
}

# Attr keys scaled with rarity (ratios like reflection stay absolute).
SCALE_ATTR_KEYS = (
    "aura_heal", "aura_radius", "constant_heal", "burst_heal",
    "radial_pulse_damage", "radial_pulse_radius", "extra_range",
    "extra_health", "armor", "magnet_radius", "splash_damage", "splash_radius",
)
ABS_ATTR_KEYS = (
    "damage_reflection", "extra_damage_factor", "extra_reload_factor",
    "move_speed_factor", "bonus_vs_poisoned", "hit_cycle_factor",
    "slow_inflict_seconds", "snare_seconds", "secondary_reload",
    "defend_only", "hit_cycle", "clump_radius", "icon_angle",
)


def step_mult(from_r: int, to_r: int) -> float:
    if to_r == from_r:
        return 1.0
    ascending = to_r > from_r
    a, b = (from_r, to_r) if ascending else (to_r, from_r)
    m = 1.0
    for r in range(a, b):
        m *= 2.0 if r >= 5 else 3.0
    return m if ascending else (1.0 / m)


def fmt_num(x: float) -> str:
    if abs(x - round(x)) < 1e-6:
        return f"{int(round(x))}.0"
    s = f"{x:.4f}".rstrip("0").rstrip(".")
    if "." not in s:
        s += ".0"
    return s


def parse_petal_blocks(text: str) -> list[tuple[int, int, str]]:
    start = text.index("PETAL_DATA = {{")
    end = text.index("\n}};", start)
    body_start = start
    body = text[start:end]
    entries: list[tuple[int, int, str]] = []
    depth = 0
    begin = None
    for j, ch in enumerate(body):
        if ch == "{":
            depth += 1
            if depth == 3 and begin is None:
                begin = j
        elif ch == "}":
            if depth == 3 and begin is not None:
                abs_b = body_start + begin
                abs_e = body_start + j + 1
                entries.append((abs_b, abs_e, body[begin : j + 1]))
                begin = None
            depth -= 1
    return entries


def extract_attrs(block: str) -> str:
    am = re.search(r"\.attributes\s*=\s*\{", block)
    if not am:
        return "{}"
    i0 = am.end() - 1
    d = 0
    for k in range(i0, len(block)):
        if block[k] == "{":
            d += 1
        elif block[k] == "}":
            d -= 1
            if d == 0:
                return block[i0 : k + 1]
    return "{}"


def get_num(block: str, key: str, default: float) -> float:
    m = re.search(rf"\.{key}\s*=\s*([0-9.eE+-]+)", block)
    return float(m.group(1)) if m else default


def get_int(block: str, key: str, default: int) -> int:
    m = re.search(rf"\.{key}\s*=\s*([0-9]+)", block)
    return int(m.group(1)) if m else default


def scale_attrs(attrs: str, mult: float) -> str:
    attrs = attrs.strip()
    if attrs in ("{}", "") or abs(mult - 1.0) < 1e-9:
        return attrs if attrs else "{}"

    def repl_dot_nested(key: str, s: str) -> str:
        # Scale .damage only; keep .time absolute.
        def repl(m: re.Match) -> str:
            inner = m.group(1)

            def sc(n: re.Match) -> str:
                field = n.group(1)
                raw = n.group(2).rstrip("fF")
                if field.strip().startswith(".time"):
                    return f"{field}{fmt_num(float(raw))}"
                return f"{field}{fmt_num(float(raw) * mult)}"

            return f".{key} = {{" + re.sub(
                r"(\.\w+\s*=\s*)([0-9.eE+-]+f?)", sc, inner
            ) + "}"

        return re.sub(rf"\.{key}\s*=\s*\{{([^{{}}]*)\}}", repl, s)

    out = attrs
    for k in ("poison_damage", "burn_damage"):
        out = repl_dot_nested(k, out)

    def repl_field(key: str, s: str) -> str:
        def sc(m: re.Match) -> str:
            raw = m.group(2).rstrip("fF")
            return f"{m.group(1)}{fmt_num(float(raw) * mult)}"

        return re.sub(rf"(\.{key}\s*=\s*)([0-9.eE+-]+f?)", sc, s)

    for k in SCALE_ATTR_KEYS:
        out = repl_field(k, out)
    return out


def patch_attr_scalar(attrs: str, key: str, value: float) -> str:
    """Set or insert a scalar attr inside `{ ... }`."""
    if attrs.strip() in ("", "{}"):
        return "{ ." + key + " = " + fmt_num(value) + " }"
    pat = rf"\.{key}\s*=\s*[0-9.eE+-]+f?"
    if re.search(pat, attrs):
        return re.sub(pat, f".{key} = {fmt_num(value)}", attrs)
    inner = attrs.strip()
    if not inner.startswith("{"):
        return attrs
    body = inner[1:-1].rstrip().rstrip(",")
    if body.strip():
        return "{ " + body + ", ." + key + " = " + fmt_num(value) + " }"
    return "{ ." + key + " = " + fmt_num(value) + " }"


def patch_nested_dot(attrs: str, nest: str, dmg: float | None, time: float | None) -> str:
    if dmg is None and time is None:
        return attrs
    m = re.search(rf"\.{nest}\s*=\s*\{{([^{{}}]*)\}}", attrs)
    if m:
        inner = m.group(1)
        if dmg is not None:
            if re.search(r"\.damage\s*=", inner):
                inner = re.sub(r"\.damage\s*=\s*[0-9.eE+-]+f?",
                               f".damage = {fmt_num(dmg)}", inner)
            else:
                inner = inner.rstrip() + f", .damage = {fmt_num(dmg)}"
        if time is not None:
            if re.search(r"\.time\s*=", inner):
                inner = re.sub(r"\.time\s*=\s*[0-9.eE+-]+f?",
                               f".time = {fmt_num(time)}", inner)
            else:
                inner = inner.rstrip() + f", .time = {fmt_num(time)}"
        return re.sub(rf"\.{nest}\s*=\s*\{{[^{{}}]*\}}",
                      f".{nest} = {{{inner}}}", attrs)
    # insert nest
    chunk = f".{nest} = {{"
    parts = []
    if dmg is not None:
        parts.append(f".damage = {fmt_num(dmg)}")
    if time is not None:
        parts.append(f".time = {fmt_num(time)}")
    chunk += " " + ", ".join(parts) + " }"
    if attrs.strip() in ("", "{}"):
        return "{ " + chunk + " }"
    body = attrs.strip()[1:-1].rstrip().rstrip(",")
    return "{ " + body + ", " + chunk + " }"


def rewrite_block(old: str, health: float, damage: float, radius: float,
                  reload: float, count: int, rarity: str, attrs: str) -> str:
    name_m = re.search(r'\.name\s*=\s*"([^"]*)"', old)
    desc_m = re.search(r'\.description\s*=\s*"((?:\\.|[^"\\])*)"', old)
    name = name_m.group(1) if name_m else "?"
    desc = desc_m.group(1) if desc_m else ""
    return (
        "{\n"
        f'        .name = "{name}",\n'
        f'        .description = "{desc}",\n'
        f"        .health = {fmt_num(health)},\n"
        f"        .damage = {fmt_num(damage)},\n"
        f"        .radius = {fmt_num(radius)},\n"
        f"        .reload = {fmt_num(reload)},\n"
        f"        .count = {count},\n"
        f"        .rarity = RarityID::k{rarity},\n"
        f"        .attributes = {attrs}\n"
        "    }"
    )


def main() -> int:
    text = STATIC_CC.read_text(encoding="utf-8")
    entries = parse_petal_blocks(text)
    by_name: dict[str, dict[int, tuple[int, int, str]]] = {}
    for abs_b, abs_e, block in entries:
        nm = re.search(r'\.name\s*=\s*"([^"]*)"', block)
        rar = re.search(r"\.rarity\s*=\s*RarityID::k(\w+)", block)
        if not nm or not rar:
            continue
        try:
            ri = RARITY_NAMES.index(rar.group(1))
        except ValueError:
            continue
        by_name.setdefault(nm.group(1), {})[ri] = (abs_b, abs_e, block)

    replacements: list[tuple[int, int, str]] = []
    tuned = 0
    for name, ov in ANCHORS.items():
        fam = by_name.get(name)
        if not fam:
            print(f"WARN missing family: {name}")
            continue
        if "base_rarity" in ov:
            try:
                base_r = RARITY_NAMES.index(ov["base_rarity"])
            except ValueError:
                base_r = min(fam.keys())
            if base_r not in fam:
                base_r = min(fam.keys())
        else:
            base_r = min(fam.keys())
        _, _, base_block = fam[base_r]
        bh = ov.get("health", get_num(base_block, "health", 10.0))
        bd = ov.get("damage", get_num(base_block, "damage", 10.0))
        br = ov.get("radius", get_num(base_block, "radius", 10.0))
        brel = ov.get("reload", get_num(base_block, "reload", 2.5))
        bc = int(ov.get("count", get_int(base_block, "count", 1)))
        battrs = extract_attrs(base_block)
        for k in (
            "constant_heal", "damage_reflection", "slow_inflict_seconds", "extra_health",
        ):
            if k in ov:
                battrs = patch_attr_scalar(battrs, k, float(ov[k]))
        if "poison_damage" in ov or "poison_time" in ov:
            battrs = patch_nested_dot(
                battrs, "poison_damage",
                ov.get("poison_damage"), ov.get("poison_time"),
            )
        if "burn_damage" in ov or "burn_time" in ov:
            battrs = patch_nested_dot(
                battrs, "burn_damage",
                ov.get("burn_damage"), ov.get("burn_time"),
            )

        for ri, (abs_b, abs_e, block) in fam.items():
            mult = step_mult(base_r, ri)
            attrs = scale_attrs(battrs, mult)
            for key in ABS_ATTR_KEYS:
                m = re.search(rf"\.{key}\s*=\s*([0-9.eE+-]+f?)", battrs)
                if not m:
                    continue
                raw = m.group(1).rstrip("f")
                val = float(raw)
                if key == "extra_damage_factor":
                    val = val + 0.02 * max(0, ri - base_r)
                attrs = re.sub(
                    rf"\.{key}\s*=\s*[0-9.eE+-]+f?",
                    f".{key} = {fmt_num(val)}" + ("f" if m.group(1).endswith("f") else ""),
                    attrs,
                )
            new_block = rewrite_block(
                block,
                health=bh * mult,
                damage=bd * mult,
                radius=br,
                reload=brel,
                count=bc,
                rarity=RARITY_NAMES[ri],
                attrs=attrs,
            )
            replacements.append((abs_b, abs_e, new_block))
            tuned += 1

    replacements.sort(key=lambda t: t[0], reverse=True)
    out = text
    for abs_b, abs_e, new_block in replacements:
        out = out[:abs_b] + new_block + out[abs_e:]
    STATIC_CC.write_text(out, encoding="utf-8")
    print(f"retuned {tuned} entries across {len(ANCHORS)} assemble families")
    return 0


if __name__ == "__main__":
    sys.exit(main())
