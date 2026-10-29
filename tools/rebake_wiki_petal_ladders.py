#!/usr/bin/env python3
"""Rebake PETAL_DATA ladders to wiki rarity curves.

Combat (health/damage/DoT damage/stack/splash/lightning/max_damage…):
  Common→Unique = 1,3,9,27,81,243,729,2187,6561  (×3 every tier)

Heal/armor-style (constant_heal/burst_heal/aura_heal/armor/poison_armor/
damage_absorb/extra_health when flat HP):
  ×3 to Mythic, then ×√3 → Unique ≈ 1262.665× Common

Anchors on the lowest rarity rung that has a positive value (usually Common).
Does not touch Coin/None, factor attrs (speed/reload/vision), reload/count/radius.
"""
from __future__ import annotations

import math
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
STATIC_CC = ROOT / "Shared" / "StaticData.cc"

RARITY = [
    "Common", "Unusual", "Rare", "Epic", "Legendary",
    "Mythic", "Ultra", "Super", "Unique",
]
SQRT3 = math.sqrt(3.0)
COMBAT = [1.0, 3.0, 9.0, 27.0, 81.0, 243.0, 729.0, 2187.0, 6561.0]
HEAL = [
    1.0, 3.0, 9.0, 27.0, 81.0, 243.0,
    243.0 * SQRT3, 729.0, 729.0 * SQRT3,
]
EPS = 1e-6
SKIP = {"None", "Coin"}

# Attr keys scaled on combat curve (from Common/base anchor).
COMBAT_ATTRS = (
    "stack_damage", "splash_damage", "lightning_damage",
    "tomato_max_damage", "clay_max_damage", "domino_max_damage",
    "armor_shred", "extra_body_damage", "radial_pulse_damage",
    "on_hit_heal",  # flat heal-on-hit scales like damage feel
    "privet_extra_poison",
)
# Attr keys scaled on heal curve.
HEAL_ATTRS = (
    "constant_heal", "burst_heal", "aura_heal",
    "armor", "poison_armor", "damage_absorb",
)


def fmt(x: float) -> str:
    if abs(x) < 1e-12:
        return "0.0"
    if abs(x - round(x)) < 1e-6 and abs(x) < 1e12:
        return f"{int(round(x))}.0"
    # keep readable precision
    s = f"{x:.6g}"
    if "e" in s or "E" in s:
        return f"{x:.6g}"
    if "." not in s:
        return s + ".0"
    return s


def parse_blocks(text: str) -> list[tuple[int, int, str]]:
    start = text.index("PETAL_DATA = {{")
    end = text.index("\n}};", start)
    body_start = start
    body = text[start:end]
    out: list[tuple[int, int, str]] = []
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


def get_num(block: str, key: str, default: float | None = None) -> float | None:
    m = re.search(rf"\.{key}\s*=\s*([0-9.eE+-]+f?)", block)
    if not m:
        return default
    return float(m.group(1).rstrip("fF"))


def get_int(block: str, key: str, default: int = 1) -> int:
    m = re.search(rf"\.{key}\s*=\s*([0-9]+)", block)
    return int(m.group(1)) if m else default


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


def nest_field(attrs: str, nest: str, field: str) -> float | None:
    m = re.search(rf"\.{nest}\s*=\s*\{{([^}}]*)\}}", attrs)
    if not m:
        return None
    fm = re.search(rf"\.{field}\s*=\s*([0-9.eE+-]+f?)", m.group(1))
    return float(fm.group(1).rstrip("fF")) if fm else None


def set_nest_field(attrs: str, nest: str, field: str, value: float) -> str:
    m = re.search(rf"\.{nest}\s*=\s*\{{([^}}]*)\}}", attrs)
    if not m:
        return attrs
    inner = m.group(1)
    if re.search(rf"\.{field}\s*=", inner):
        inner = re.sub(
            rf"\.{field}\s*=\s*[0-9.eE+-]+f?",
            f".{field} = {fmt(value)}",
            inner,
        )
    else:
        inner = inner.rstrip().rstrip(",") + f", .{field} = {fmt(value)}"
    return re.sub(rf"\.{nest}\s*=\s*\{{[^}}]*\}}", f".{nest} = {{{inner}}}", attrs)


def set_attr_scalar(attrs: str, key: str, value: float, as_f: bool = False) -> str:
    suf = "f" if as_f else ""
    pat = rf"\.{key}\s*=\s*[0-9.eE+-]+f?"
    repl = f".{key} = {fmt(value)}{suf}"
    if re.search(pat, attrs):
        return re.sub(pat, repl, attrs)
    return attrs


def rewrite_block(old: str, health: float, damage: float, attrs: str) -> str:
    name_m = re.search(r'\.name\s*=\s*"([^"]*)"', old)
    desc_m = re.search(r'\.description\s*=\s*"((?:\\.|[^"\\])*)"', old)
    rar = re.search(r"\.rarity\s*=\s*RarityID::k(\w+)", old)
    name = name_m.group(1) if name_m else "?"
    desc = desc_m.group(1) if desc_m else ""
    rarity = rar.group(1) if rar else "Common"
    radius = get_num(old, "radius", 10.0) or 10.0
    reload = get_num(old, "reload", 2.5) or 2.5
    count = get_int(old, "count", 1)
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


def scale_from(anchor_ri: int, anchor_v: float, table: list[float], ri: int) -> float:
    if anchor_v <= EPS:
        return 0.0
    base = table[anchor_ri]
    if base < 1e-12:
        base = 1.0
    return anchor_v * (table[ri] / base)


def needs_rebake(vals: list[float], table: list[float], anchor_ri: int) -> bool:
    """True if Unique/Super ratio off ~3 or any rung far from wiki curve."""
    if max(vals) <= EPS:
        return False
    av = vals[anchor_ri]
    if av <= EPS:
        return True
    for ri, v in enumerate(vals):
        expect = scale_from(anchor_ri, av, table, ri)
        if expect <= EPS:
            continue
        # tolerate 2% relative error
        if abs(v - expect) / expect > 0.02:
            return True
    return False


def pick_anchor(vals: list[float]) -> int:
    for ri, v in enumerate(vals):
        if v > EPS:
            return ri
    return 0


def main() -> int:
    text = STATIC_CC.read_text(encoding="utf-8")
    entries = parse_blocks(text)
    # name → ri → list of (a,b,block) — keep ALL occurrences so duplicates all update
    by_name: dict[str, dict[int, list[tuple[int, int, str]]]] = {}
    for a, b, block in entries:
        nm = re.search(r'\.name\s*=\s*"([^"]*)"', block)
        rar = re.search(r"\.rarity\s*=\s*RarityID::k(\w+)", block)
        if not nm or not rar:
            continue
        name = nm.group(1)
        if name in SKIP:
            continue
        try:
            ri = RARITY.index(rar.group(1))
        except ValueError:
            continue
        by_name.setdefault(name, {}).setdefault(ri, []).append((a, b, block))

    replacements: list[tuple[int, int, str]] = []
    fam_ok = fam_fix = 0

    for name, fam in sorted(by_name.items()):
        if len(fam) < 9:
            continue
        # Use first occurrence of each rarity as design source
        blocks = {ri: fam[ri][0][2] for ri in range(9)}
        hs = [get_num(blocks[r], "health", 0.0) or 0.0 for r in range(9)]
        ds = [get_num(blocks[r], "damage", 0.0) or 0.0 for r in range(9)]
        ah = pick_anchor(hs)
        ad = pick_anchor(ds) if max(ds) > EPS else ah

        # Collect combat/heal attr anchors from lowest rung that has them
        attr_anchors: dict[str, tuple[int, float, str]] = {}  # key → (ri, val, curve)
        for ri in range(9):
            attrs = extract_attrs(blocks[ri])
            for key in COMBAT_ATTRS:
                if key in attr_anchors:
                    continue
                v = get_num(attrs, key)
                if v is not None and v > EPS:
                    attr_anchors[key] = (ri, v, "combat")
            for key in HEAL_ATTRS:
                if key in attr_anchors:
                    continue
                v = get_num(attrs, key)
                if v is not None and abs(v) > EPS:
                    attr_anchors[key] = (ri, v, "heal")
            # extra_health: only flat HP (>=2), not Relic fraction
            if "extra_health" not in attr_anchors:
                v = get_num(attrs, "extra_health")
                if v is not None and v >= 2.0 - EPS:
                    attr_anchors["extra_health"] = (ri, v, "heal")
            for nest in ("poison_damage", "burn_damage"):
                k = f"{nest}.damage"
                if k in attr_anchors:
                    continue
                v = nest_field(attrs, nest, "damage")
                if v is not None and v > EPS:
                    attr_anchors[k] = (ri, v, "combat")

        combat_needed = needs_rebake(hs, COMBAT, ah) or (
            max(ds) > EPS and needs_rebake(ds, COMBAT, ad)
        )
        # Always force Unique==Super combat fix
        if hs[8] + EPS < hs[7] * 2.5 or (max(ds) > EPS and ds[8] + EPS < ds[7] * 2.5):
            combat_needed = True

        attr_needed = False
        for key, (ari, av, curve) in attr_anchors.items():
            table = COMBAT if curve == "combat" else HEAL
            series = []
            for ri in range(9):
                attrs = extract_attrs(blocks[ri])
                if "." in key:
                    nest, field = key.split(".", 1)
                    series.append(nest_field(attrs, nest, field) or 0.0)
                else:
                    series.append(get_num(attrs, key) or 0.0)
            if needs_rebake(series, table, ari):
                attr_needed = True
                break

        if not combat_needed and not attr_needed:
            fam_ok += 1
            continue

        fam_fix += 1
        for ri in range(9):
            for a, b, block in fam[ri]:
                health = scale_from(ah, hs[ah], COMBAT, ri) if combat_needed else hs[ri]
                damage = (
                    scale_from(ad, ds[ad], COMBAT, ri)
                    if combat_needed and max(ds) > EPS
                    else ds[ri]
                )
                attrs = extract_attrs(block)
                for key, (ari, av, curve) in attr_anchors.items():
                    table = COMBAT if curve == "combat" else HEAL
                    nv = scale_from(ari, av, table, ri)
                    if "." in key:
                        nest, field = key.split(".", 1)
                        attrs = set_nest_field(attrs, nest, field, nv)
                    else:
                        attrs = set_attr_scalar(attrs, key, nv)
                new = rewrite_block(block, health, damage, attrs)
                replacements.append((a, b, new))

    replacements.sort(key=lambda t: t[0], reverse=True)
    out = text
    for a, b, new in replacements:
        out = out[:a] + new + out[b:]
    STATIC_CC.write_text(out, encoding="utf-8")
    print(
        f"wiki rebake: families_fixed={fam_fix} already_ok={fam_ok} "
        f"blocks_rewritten={len(replacements)}"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
