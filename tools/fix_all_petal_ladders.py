#!/usr/bin/env python3
"""Fix ALL petal families: combat ladders + linear mech attrs + poison time.

Pass 1 — combat health/damage:
  Flat or regressing → pick design anchor, rebuild ×3(<Mythic)/×2 ladder.

Pass 2 — poison/burn .time:
  If duration exploded (>3×), pin time to design rung; keep .damage laddered.

Pass 3 — mechanism attrs that are flat across Common..Unique:
  Linear lerp Common→Unique (direction-aware).

Does NOT touch Coin/None. Re-run tools/audit_all_petal_attrs.py after.
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
EPS = 1e-4


def lerp(a: float, b: float, t: float) -> float:
    return a + (b - a) * t


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
    if attrs.strip() in ("", "{}"):
        return "{ " + repl + " }"
    body = attrs.strip()[1:-1].rstrip().rstrip(",")
    return "{ " + body + ", " + repl + " }"


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
        f"        .health = {fmt(health)},\n"
        f"        .damage = {fmt(damage)},\n"
        f"        .radius = {fmt(radius)},\n"
        f"        .reload = {fmt(reload)},\n"
        f"        .count = {count},\n"
        f"        .rarity = RarityID::k{rarity},\n"
        f"        .attributes = {attrs}\n"
        "    }"
    )


def series_status(vals: list[float], want: str) -> str:
    if max(vals) - min(vals) <= EPS:
        return "flat"
    rises = falls = 0
    for a, b in zip(vals, vals[1:]):
        if b > a + EPS:
            rises += 1
        elif b < a - EPS:
            falls += 1
    if want == "asc" and falls:
        return "regress"
    if want == "desc" and rises:
        return "regress"
    return "ok"


def pick_combat_anchor(vals: list[float]) -> tuple[int, float]:
    """Pick (rarity_index, value) as design base for rebuilding the ladder."""
    # Drop stale Unique if much smaller than Super
    usable = list(range(9))
    if vals[8] + EPS < vals[7] * 0.5 and vals[7] > 1.5:
        usable = list(range(8))

    # Prefer first drop point: values after a drop are often the real design ladder
    # e.g. [1,3,9,2.5,7.5,...] → anchor Epic=2.5
    for i in range(1, 9):
        if i not in usable:
            continue
        if vals[i] + EPS < vals[i - 1] and vals[i] > EPS:
            # if early rung looks synthetic (≈1 then ×3), prefer post-drop
            if vals[0] <= 1.0 + EPS or (vals[0] < vals[i] and i >= 2):
                return i, vals[i]

    # Else: highest value among usable (usually Unique/Super of a good ladder)
    ri = max(usable, key=lambda r: vals[r])
    return ri, vals[ri]


def needs_combat_fix(vals: list[float], allow_zero: bool) -> bool:
    if allow_zero and max(vals) <= EPS:
        return False
    st = series_status(vals, "asc")
    return st in ("flat", "regress")


# Mech keys: (direction, how to map flat current → (lo, hi))
# direction: asc | desc
# mapper receives current flat value → (common, unique)
def mech_range(key: str, cur: float) -> tuple[str, float, float] | None:
    if abs(cur) <= EPS and key not in ("damage",):
        return None
    # lower-is-better
    if key in ("vision_factor", "extra_reload_factor", "secondary_reload"):
        lo = cur  # Common (worse)
        if key == "vision_factor":
            hi = max(0.15, cur * 0.45)
        elif key == "extra_reload_factor":
            hi = max(0.70, cur - 0.15)
        else:  # secondary_reload
            hi = max(0.12, cur * 0.45)
        return "desc", lo, hi

    # ratios / factors slightly above 1
    if key in ("move_speed_factor", "extra_damage_factor", "hit_cycle_factor", "execute_factor"):
        lo = cur
        hi = cur + max(0.12, (cur - 1.0) * 2.5) if cur >= 1.0 else cur * 1.8
        if key == "move_speed_factor":
            hi = min(1.60, max(cur + 0.25, cur * 1.25))
        if key == "extra_damage_factor":
            hi = min(1.55, max(cur + 0.20, cur * 1.2))
        if key == "hit_cycle_factor":
            hi = min(3.0, cur * 1.35)
        if key == "execute_factor":
            hi = min(2.5, cur * 1.35)
        return "asc", lo, hi

    if key == "damage_reflection":
        lo = cur
        hi = min(0.55, cur + 0.28)
        return "asc", lo, hi

    if key in ("armor_penetration", "bonus_vs_poisoned", "bonus_vs_slowed", "execute_threshold"):
        lo = cur
        hi = min(0.75 if key != "execute_threshold" else 0.45, cur * 2.2)
        return "asc", lo, hi

    if key in (
        "armor", "poison_armor", "constant_heal", "burst_heal", "on_hit_heal",
        "extra_health", "extra_body_damage", "extra_range", "extra_rotation_speed",
        "magnet_radius", "aura_heal", "aura_radius", "aura_duration", "aura_slow_seconds",
        "slow_inflict_seconds", "snare_seconds", "dandelion_inflict_seconds",
        "splash_damage", "splash_radius", "taunt_radius",
        "radial_pulse_damage", "radial_pulse_radius", "mass",
    ):
        lo = cur * 0.55 if cur > 1 else cur
        # keep current near mid-low; Unique ≈ 2.2–3.5× current
        hi = cur * (2.8 if cur >= 1 else 2.2)
        if key == "mass" and cur >= 10:
            lo, hi = cur * 0.45, cur * 2.2
        if key in ("slow_inflict_seconds", "snare_seconds", "dandelion_inflict_seconds", "aura_slow_seconds"):
            lo, hi = cur * 0.7, cur * 2.0
        if key == "taunt_radius":
            lo, hi = cur * 0.6, cur * 2.0
        if key == "extra_rotation_speed":
            lo, hi = max(0.08, cur * 0.6), cur * 3.0
        return "asc", lo, hi

    return None


MECH_F_SUFFIX = {
    "vision_factor", "extra_reload_factor", "move_speed_factor",
    "extra_damage_factor", "damage_reflection", "armor_penetration",
    "bonus_vs_poisoned", "bonus_vs_slowed", "hit_cycle_factor",
    "execute_factor", "execute_threshold",
}


def main() -> int:
    text = STATIC_CC.read_text(encoding="utf-8")
    entries = parse_blocks(text)
    by_name: dict[str, dict[int, tuple[int, int, str]]] = {}
    for a, b, block in entries:
        nm = re.search(r'\.name\s*=\s*"([^"]*)"', block)
        rar = re.search(r"\.rarity\s*=\s*RarityID::k(\w+)", block)
        if not nm or not rar:
            continue
        name = nm.group(1)
        if name in ("None", "Coin"):
            continue
        try:
            ri = RARITY.index(rar.group(1))
        except ValueError:
            continue
        by_name.setdefault(name, {})[ri] = (a, b, block)

    replacements: list[tuple[int, int, str]] = []
    stats = {"combat": 0, "poison_time": 0, "mech": 0, "families": 0}

    for name, fam in sorted(by_name.items()):
        if len(fam) < 9:
            continue
        blocks = {ri: fam[ri][2] for ri in range(9)}
        hs = [get_num(blocks[r], "health", 0.0) or 0.0 for r in range(9)]
        ds = [get_num(blocks[r], "damage", 0.0) or 0.0 for r in range(9)]

        fix_combat = needs_combat_fix(hs, False) or needs_combat_fix(ds, True)

        # design attrs template: prefer Legendary, else Unusual, else Common
        design_r = 4 if 4 in fam else (1 if 1 in fam else 0)
        if fix_combat:
            # choose combat anchors independently
            ah_r, ah_v = pick_combat_anchor(hs)
            ad_r, ad_v = pick_combat_anchor(ds) if max(ds) > EPS else (ah_r, 0.0)
            # prefer same design rarity when both need fix — use health's anchor rarity
            # but keep each field's own value at its anchor
            design_r = ah_r
            bh, bd = ah_v, (ds[ah_r] if max(ds) > EPS else 0.0)
            # if damage had better post-drop anchor and health didn't drop, still ok
            if needs_combat_fix(ds, True) and not needs_combat_fix(hs, False):
                design_r = ad_r
                bh, bd = hs[ad_r], ad_v
            elif needs_combat_fix(ds, True) and needs_combat_fix(hs, False):
                # if damage drop index differs, use the later drop for both stats from that rung
                design_r = max(ah_r, ad_r) if abs(ah_r - ad_r) <= 2 else ah_r
                bh = hs[design_r]
                bd = ds[design_r]

        base_block = blocks[design_r]
        br = get_num(base_block, "radius", 10.0) or 10.0
        brel = get_num(base_block, "reload", 2.5) or 2.5
        bc = get_int(base_block, "count", 1)
        base_attrs = extract_attrs(base_block)

        # poison/burn time pin
        poison_time = nest_field(base_attrs, "poison_damage", "time")
        burn_time = nest_field(base_attrs, "burn_damage", "time")
        poison_dmgs = [nest_field(extract_attrs(blocks[r]), "poison_damage", "damage") for r in range(9)]
        burn_dmgs = [nest_field(extract_attrs(blocks[r]), "burn_damage", "damage") for r in range(9)]
        poison_times = [nest_field(extract_attrs(blocks[r]), "poison_damage", "time") for r in range(9)]
        burn_times = [nest_field(extract_attrs(blocks[r]), "burn_damage", "time") for r in range(9)]

        def time_exploded(times: list[float | None]) -> bool:
            present = [t for t in times if t is not None]
            return len(present) >= 2 and max(present) > min(present) * 3 + EPS

        fix_poison_t = time_exploded(poison_times)
        fix_burn_t = time_exploded(burn_times)

        # Collect flat mech keys from Common attrs (present on all rungs, flat)
        common_attrs = extract_attrs(blocks[0])
        mech_plan: dict[str, tuple[float, float]] = {}  # key → (lo, hi)
        for m in re.finditer(r"\.([a-z_][a-z0-9_]*)\s*=\s*([0-9.eE+-]+f?)", common_attrs):
            key = m.group(1)
            if key in ("icon_angle", "defend_only", "equipment", "rotation_style",
                       "spawns", "split_projectile", "hit_cycle", "spawn_count",
                       "clump_radius"):
                continue
            # skip nested handled separately
            if key in ("damage", "time"):
                continue
            vals = []
            ok_all = True
            for r in range(9):
                v = get_num(extract_attrs(blocks[r]), key)
                if v is None:
                    ok_all = False
                    break
                vals.append(v)
            if not ok_all:
                continue
            # already monotonic? skip
            rng = mech_range(key, vals[0])
            if rng is None:
                continue
            direction, lo, hi = rng
            st = series_status(vals, direction)
            if st == "ok":
                continue
            # flat or wrong direction → re-linearize from plan
            mech_plan[key] = (lo, hi)

        if not (fix_combat or fix_poison_t or fix_burn_t or mech_plan):
            continue

        stats["families"] += 1
        if fix_combat:
            stats["combat"] += 1
            bh = bh if fix_combat else hs[design_r]
            bd = bd if fix_combat else ds[design_r]
        else:
            bh, bd = hs[design_r], ds[design_r]

        # poison/burn damage at design for scaling
        base_poison_d = nest_field(base_attrs, "poison_damage", "damage")
        base_burn_d = nest_field(base_attrs, "burn_damage", "damage")
        if fix_poison_t and poison_time is None and poison_times[design_r] is not None:
            poison_time = poison_times[design_r]
        if fix_burn_t and burn_time is None and burn_times[design_r] is not None:
            burn_time = burn_times[design_r]
        # if time exploded, also rebuild poison damage from design if regressing
        if base_poison_d is not None and any(x is not None for x in poison_dmgs):
            pv = [x if x is not None else 0.0 for x in poison_dmgs]
            if series_status(pv, "asc") != "ok":
                fix_combat = True  # ensure we rewrite with scaled poison
        if base_burn_d is not None and any(x is not None for x in burn_dmgs):
            bv = [x if x is not None else 0.0 for x in burn_dmgs]
            if series_status(bv, "asc") != "ok":
                fix_combat = True

        if mech_plan:
            stats["mech"] += 1
        if fix_poison_t or fix_burn_t:
            stats["poison_time"] += 1

        for ri in range(9):
            a, b, block = fam[ri]
            mult = step_mult(design_r, ri)
            t = ri / 8.0

            if fix_combat:
                health = bh * mult
                damage = bd * mult
            else:
                health = hs[ri]
                damage = ds[ri]

            # start from this rarity's attrs (preserve equipment flags etc.), then patch
            attrs = extract_attrs(block)

            # Only rewrite DoT damage when combat ladder or time explosion forces a rebuild
            rewrite_dot = fix_combat or fix_poison_t or fix_burn_t
            if rewrite_dot and base_poison_d is not None:
                attrs = set_nest_field(attrs, "poison_damage", "damage", base_poison_d * mult)
            if rewrite_dot and base_burn_d is not None:
                attrs = set_nest_field(attrs, "burn_damage", "damage", base_burn_d * mult)
            if fix_poison_t and poison_time is not None:
                attrs = set_nest_field(attrs, "poison_damage", "time", poison_time)
            if fix_burn_t and burn_time is not None:
                attrs = set_nest_field(attrs, "burn_damage", "time", burn_time)

            for key, (lo, hi) in mech_plan.items():
                val = lerp(lo, hi, t)
                attrs = set_attr_scalar(attrs, key, val, as_f=(key in MECH_F_SUFFIX))

            # radius/reload/count: keep per-rung unless combat rebuild (then use design)
            if fix_combat:
                radius, reload, count = br, brel, bc
            else:
                radius = get_num(block, "radius", br) or br
                reload = get_num(block, "reload", brel) or brel
                count = get_int(block, "count", bc)

            # Web-style count climbs already in data — preserve existing count if not combat-only material
            if not fix_combat:
                count = get_int(block, "count", bc)

            new = rewrite_block(block, health, damage, radius, reload, count, RARITY[ri], attrs)
            replacements.append((a, b, new))

    replacements.sort(key=lambda t: t[0], reverse=True)
    out = text
    for a, b, new in replacements:
        out = out[:a] + new + out[b:]
    STATIC_CC.write_text(out, encoding="utf-8")
    print(
        f"fixed families={stats['families']} "
        f"combat={stats['combat']} poison_time={stats['poison_time']} "
        f"mech={stats['mech']} entries={len(replacements)}"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
