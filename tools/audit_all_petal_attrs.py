#!/usr/bin/env python3
"""Full-petal ladder audit: combat + mechanism attrs across Common..Unique.

Reports (per family):
  - missing rarity rungs
  - non-monotonic combat (health/damage should rise when >0)
  - flat or regressing mechanism attrs (direction-aware)
  - poison/burn .damage must rise; .time must NOT explode with rarity

Usage:
  python tools/audit_all_petal_attrs.py            # full report
  python tools/audit_all_petal_attrs.py --batch 0  # families 0..49
  python tools/audit_all_petal_attrs.py --batch 1
  python tools/audit_all_petal_attrs.py --summary
"""
from __future__ import annotations

import argparse
import re
import sys
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CC = (ROOT / "Shared" / "StaticData.cc").read_text(encoding="utf-8")

RARITY = [
    "Common", "Unusual", "Rare", "Epic", "Legendary",
    "Mythic", "Ultra", "Super", "Unique",
]

# Higher rarity should INCREASE these (when present / non-default)
ASCEND = {
    "health", "damage", "radius",  # combat / size often grows
    "magnet_radius", "move_speed_factor", "extra_rotation_speed",
    "extra_range", "extra_body_damage", "extra_damage_factor",
    "extra_health", "damage_reflection", "armor", "poison_armor",
    "constant_heal", "burst_heal", "on_hit_heal", "mass",
    "aura_heal", "aura_radius", "aura_duration", "aura_slow_seconds",
    "armor_penetration", "bonus_vs_poisoned", "bonus_vs_slowed",
    "snare_seconds", "splash_damage", "splash_radius",
    "execute_threshold", "execute_factor", "taunt_radius",
    "radial_pulse_damage", "radial_pulse_radius",
    "dandelion_inflict_seconds", "slow_inflict_seconds",
    "hit_cycle_factor", "spawn_count", "clump_radius",
    "poison_damage", "burn_damage",  # DoT damage component
}

# Higher rarity should DECREASE (better = smaller)
DESCEND = {
    "vision_factor",       # lower = more FOV
    "extra_reload_factor", # lower = faster CD
    "secondary_reload",   # often snappier cycle
}

# Ignore for monotonic checks (flags / presentation / intentional flat)
IGNORE = {
    "icon_angle", "defend_only", "equipment", "rotation_style",
    "spawns", "split_projectile", "hit_cycle", "count",
    "reload",  # base reload usually fixed per design; CD buffs use extra_reload_factor
    "poison_time", "burn_time",  # duration often stays fixed by design
    "radius",  # petal hitbox often fixed; size growth is optional
    "clump_radius",  # formation spacing often fixed
    "spawn_count",  # summon count is design-fixed more often than not
}

# entity-missing-audit: special attrs pinned across ladder; only hp/dmg grow
FIXED_SPECIAL_FAMILIES = {
    "Frost Starfish",
    "Molten Sand Blade",
    "Heartwood",
    "Sand Spike",
    "Snowball",
    "Chitin Needle",
    "Mantis Claw",
}

BATCH_SIZE = 50
EPS = 1e-4


def parse_blocks(text: str) -> list[str]:
    start = text.index("PETAL_DATA = {{")
    end = text.index("\n}};", start)
    body = text[start:end]
    entries, depth, begin = [], 0, None
    for j, ch in enumerate(body):
        if ch == "{":
            depth += 1
            if depth == 3 and begin is None:
                begin = j
        elif ch == "}":
            if depth == 3 and begin is not None:
                entries.append(body[begin : j + 1])
                begin = None
            depth -= 1
    return entries


def parse_float(s: str) -> float:
    return float(s.rstrip("fF"))


def extract_nums(block: str) -> dict[str, float]:
    """Top-level petal fields + attributes + poison/burn damage (not time)."""
    out: dict[str, float] = {}
    for key in ("health", "damage", "radius", "reload", "count"):
        m = re.search(rf"\.{key}\s*=\s*([0-9.eE+-]+f?)", block)
        if m:
            out[key] = parse_float(m.group(1))

    attrs_m = re.search(r"\.attributes\s*=\s*\{", block)
    if not attrs_m:
        return out
    # slice attributes brace
    i = attrs_m.end() - 1
    depth = 0
    end = None
    for j in range(i, len(block)):
        if block[j] == "{":
            depth += 1
        elif block[j] == "}":
            depth -= 1
            if depth == 0:
                end = j
                break
    if end is None:
        return out
    attrs = block[i : end + 1]

    # nested poison/burn
    for nest in ("poison_damage", "burn_damage"):
        nm = re.search(rf"\.{nest}\s*=\s*\{{([^}}]*)\}}", attrs)
        if nm:
            dm = re.search(r"\.damage\s*=\s*([0-9.eE+-]+f?)", nm.group(1))
            tm = re.search(r"\.time\s*=\s*([0-9.eE+-]+f?)", nm.group(1))
            if dm:
                out[nest] = parse_float(dm.group(1))
            if tm:
                out[f"{nest}_time"] = parse_float(tm.group(1))

    # flat attribute floats (skip nested already handled)
    stripped = re.sub(r"\.(?:poison_damage|burn_damage)\s*=\s*\{[^}]*\}", "", attrs)
    for m in re.finditer(r"\.([a-z_][a-z0-9_]*)\s*=\s*([0-9.eE+-]+f?)", stripped):
        k, v = m.group(1), m.group(2)
        if k in IGNORE:
            continue
        out[k] = parse_float(v)
    return out


def series_ok(vals: list[float], direction: str) -> tuple[str, list[str]]:
    """Return status + issue strings. status: ok|flat|regress|mixed."""
    issues = []
    if len(vals) < 2:
        return "ok", issues
    # all equal?
    if max(vals) - min(vals) <= EPS:
        return "flat", ["all equal"]
    rises = falls = 0
    for a, b in zip(vals, vals[1:]):
        if b > a + EPS:
            rises += 1
        elif b < a - EPS:
            falls += 1
    if direction == "asc":
        if falls:
            return "regress", [f"decreased between steps (rises={rises} falls={falls})"]
        return "ok", issues
    if direction == "desc":
        if rises:
            return "regress", [f"increased between steps (rises={rises} falls={falls})"]
        return "ok", issues
    return "ok", issues


def audit_family(name: str, rows: dict[int, dict[str, float]]) -> list[str]:
    problems: list[str] = []
    missing = [RARITY[i] for i in range(9) if i not in rows]
    if missing:
        problems.append(f"MISSING rarities: {', '.join(missing)}")
        return problems

    # union of keys across ladder
    keys = set()
    for r in range(9):
        keys |= set(rows[r].keys())

    for key in sorted(keys):
        if key in IGNORE or key.endswith("_time"):
            # soft: poison/burn time should not explode (>3x from common)
            if key.endswith("_time"):
                vals = [rows[r].get(key) for r in range(9)]
                present = [v for v in vals if v is not None]
                if len(present) >= 2 and max(present) > min(present) * 3 + EPS:
                    problems.append(
                        f"{key}: duration exploded {min(present):g}→{max(present):g} (should stay ~flat)"
                    )
            continue

        vals = []
        for r in range(9):
            if key not in rows[r]:
                vals = None
                break
            vals.append(rows[r][key])
        if vals is None:
            # key not on every rung — report if present on some but not all
            present_r = [RARITY[r] for r in range(9) if key in rows[r]]
            if 0 < len(present_r) < 9:
                problems.append(f"{key}: only on {', '.join(present_r)}")
            continue

        # skip all-zero combat damage ladders (healers etc.)
        if key == "damage" and max(vals) <= EPS:
            continue
        if key == "health" and max(vals) <= EPS:
            continue

        if key in DESCEND:
            st, iss = series_ok(vals, "desc")
            if st == "flat":
                if name in FIXED_SPECIAL_FAMILIES:
                    continue
                problems.append(f"{key}: FLAT {vals[0]:g} (should decrease Common→Unique)")
            elif st == "regress":
                problems.append(f"{key}: BAD DIR {vals} ({'; '.join(iss)})")
        elif key in ASCEND or key in ("health", "damage"):
            st, iss = series_ok(vals, "asc")
            if st == "flat":
                if key == "mass" and vals[0] <= 0.11:
                    # default mass 0.1 — ignore if never set meaningfully
                    continue
                # pinned specials OK; combat hp/dmg must still rise
                if (
                    name in FIXED_SPECIAL_FAMILIES
                    and key not in ("health", "damage")
                ):
                    continue
                problems.append(f"{key}: FLAT {vals[0]:g} across all rarities")
            elif st == "regress":
                problems.append(f"{key}: REGRESS {vals} ({'; '.join(iss)})")
        else:
            # unknown numeric present on ladder: only flag REGRESS, not flat
            st, iss = series_ok(vals, "asc")
            if st == "regress":
                problems.append(f"{key}: REGRESS {vals} ({'; '.join(iss)})")

    return problems


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--batch", type=int, default=None, help="0-based batch index")
    ap.add_argument("--summary", action="store_true")
    ap.add_argument("--only-problems", action="store_true", default=True)
    ap.add_argument("--all-families", action="store_true", help="print ok families too")
    args = ap.parse_args()

    blocks = parse_blocks(CC)
    fam: dict[str, dict[int, dict[str, float]]] = defaultdict(dict)
    for b in blocks:
        nm = re.search(r'\.name\s*=\s*"([^"]*)"', b)
        rar = re.search(r"\.rarity\s*=\s*RarityID::k(\w+)", b)
        if not nm or not rar:
            continue
        name = nm.group(1)
        if name in ("None", "Coin"):
            continue
        try:
            ri = RARITY.index(rar.group(1))
        except ValueError:
            continue
        fam[name][ri] = extract_nums(b)

    names = sorted(fam.keys())
    if args.batch is not None:
        lo = args.batch * BATCH_SIZE
        hi = lo + BATCH_SIZE
        names = names[lo:hi]
        print(f"# batch {args.batch}: families [{lo}:{hi}) of {len(fam)} total → {len(names)}")

    bad = 0
    ok = 0
    problem_lines: list[str] = []
    for name in names:
        probs = audit_family(name, fam[name])
        if probs:
            bad += 1
            problem_lines.append(f"\n## {name}")
            for p in probs:
                problem_lines.append(f"  - {p}")
        else:
            ok += 1
            if args.all_families:
                problem_lines.append(f"\n## {name}: OK")

    if args.summary:
        # recount all families for summary
        all_bad = 0
        issue_counts: dict[str, int] = defaultdict(int)
        for name in sorted(fam.keys()):
            probs = audit_family(name, fam[name])
            if probs:
                all_bad += 1
                for p in probs:
                    key = p.split(":")[0].strip()
                    issue_counts[key] += 1
        print(f"families={len(fam)} ok={len(fam)-all_bad} problems={all_bad}")
        print("top issue fields:")
        for k, c in sorted(issue_counts.items(), key=lambda x: -x[1])[:40]:
            print(f"  {c:4d}  {k}")
        return 0 if all_bad == 0 else 1

    print("\n".join(problem_lines) if problem_lines else "(no problems in this batch)")
    print(f"\n# batch result: ok={ok} problems={bad}")
    return 0 if bad == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
