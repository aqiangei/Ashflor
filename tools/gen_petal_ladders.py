#!/usr/bin/env python3
"""Append missing same-name rarity ladder rungs for craft (min→Unique)."""
from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
STATIC_CC = ROOT / "Shared" / "StaticData.cc"
STATIC_HH = ROOT / "Shared" / "StaticDefinitions.hh"
I18N = ROOT / "Client" / "I18nData.inc"

RARITY_NAMES = [
    "Common", "Unusual", "Rare", "Epic", "Legendary",
    "Mythic", "Ultra", "Super", "Unique",
]
RARITY_ENUM = [f"RarityID::k{n}" for n in RARITY_NAMES]


def step_mult(from_r: int, to_r: int) -> float:
    """Combat HP/dmg multiplier for one rarity step."""
    m = 1.0
    for r in range(from_r, to_r):
        # Common→Mythic ×3; Mythic→Unique ×2
        m *= 2.0 if r >= 5 else 3.0
    return m


def sanitize_enum(name: str) -> str:
    parts = re.sub(r"[^A-Za-z0-9]+", " ", name).strip().split()
    return "".join(p[:1].upper() + p[1:] for p in parts)


def parse_petal_blocks(text: str) -> list[dict]:
    start = text.index("PETAL_DATA = {{")
    # Outer array uses `{{` … `}}`; petal entries sit at brace-depth 2.
    end = text.index("\n}};", start)
    body = text[start:end]
    entries: list[str] = []
    depth = 0
    begin = None
    for j, ch in enumerate(body):
        if ch == "{":
            depth += 1
            if depth == 3 and begin is None:
                # depth: 1=`{` of `{{`, 2=second `{`, 3=petal open
                begin = j
        elif ch == "}":
            if depth == 3 and begin is not None:
                entries.append(body[begin : j + 1])
                begin = None
            depth -= 1
    parsed = []
    for idx, block in enumerate(entries):
        nm = re.search(r'\.name\s*=\s*"([^"]*)"', block)
        rar = re.search(r"\.rarity\s*=\s*RarityID::k(\w+)", block)
        hp = re.search(r"\.health\s*=\s*([0-9.eE+-]+)", block)
        dmg = re.search(r"\.damage\s*=\s*([0-9.eE+-]+)", block)
        rad = re.search(r"\.radius\s*=\s*([0-9.eE+-]+)", block)
        rel = re.search(r"\.reload\s*=\s*([0-9.eE+-]+)", block)
        cnt = re.search(r"\.count\s*=\s*([0-9]+)", block)
        desc = re.search(r'\.description\s*=\s*"([^"]*)"', block)
        am = re.search(r"\.attributes\s*=\s*(\{[\s\S]*\})\s*$", block.strip())
        attrs_s = am.group(1).strip() if am else "{}"
        if not nm or not rar:
            continue
        try:
            ri = RARITY_NAMES.index(rar.group(1))
        except ValueError:
            continue
        # Escape description quotes for re-emit
        desc_s = (desc.group(1) if desc else "").replace("\\", "\\\\").replace('"', '\\"')
        parsed.append(
            {
                "idx": idx,
                "name": nm.group(1),
                "rarity": ri,
                "health": float(hp.group(1)) if hp else 10.0,
                "damage": float(dmg.group(1)) if dmg else 10.0,
                "radius": float(rad.group(1)) if rad else 10.0,
                "reload": float(rel.group(1)) if rel else 2.5,
                "count": int(cnt.group(1)) if cnt else 1,
                "description": desc_s,
                "attrs": attrs_s,
                "block": block,
            }
        )
    return parsed


def parse_enum_ids(hh: str) -> list[str]:
    m = re.search(r"namespace PetalID\s*\{.*?enum\s*:\s*T\s*\{(.*?)\bkNumPetals", hh, re.S)
    if not m:
        raise SystemExit("PetalID enum not found")
    ids = []
    for line in m.group(1).splitlines():
        line = line.split("//")[0].strip().rstrip(",")
        if not line or not line.startswith("k"):
            continue
        ids.append(line)
    return ids


def parse_zh_array(text: str, marker: str) -> list[str]:
    m = re.search(
        rf"static char const \*const {marker}\[PetalID::kNumPetals\] = \{{(.*?)\n\}};",
        text,
        re.S,
    )
    if not m:
        raise SystemExit(f"{marker} not found")
    body = m.group(1)
    return re.findall(r'"((?:\\.|[^"\\])*)"', body)


def fmt_num(x: float) -> str:
    if abs(x - round(x)) < 1e-6:
        return f"{int(round(x))}.0"
    s = f"{x:.4f}".rstrip("0").rstrip(".")
    if "." not in s:
        s += ".0"
    return s


def scale_attrs(attrs: str, mult: float) -> str:
    """Scale common numeric combat fields inside attributes initializer."""
    keys = (
        "aura_heal",
        "aura_radius",
        "poison_damage",
        "heal",
        "constant_heal",
        "damage",
        "health",
    )

    def repl_poison(m: re.Match) -> str:
        inner = m.group(1)
        def sc(n: re.Match) -> str:
            return f"{n.group(1)}{fmt_num(float(n.group(2)) * mult)}"
        inner2 = re.sub(r"(\.\w+\s*=\s*)([0-9.eE+-]+)", sc, inner)
        return ".poison_damage = {" + inner2 + "}"

    out = re.sub(
        r"\.poison_damage\s*=\s*\{([^{}]*)\}",
        repl_poison,
        attrs,
    )

    def repl_field(key: str, s: str) -> str:
        def sc(m: re.Match) -> str:
            return f"{m.group(1)}{fmt_num(float(m.group(2)) * mult)}"
        return re.sub(rf"(\.{key}\s*=\s*)([0-9.eE+-]+)", sc, s)

    for k in ("aura_heal", "aura_radius", "heal", "constant_heal"):
        out = repl_field(k, out)
    # extra_reload_factor: leave as-is (Golden Leaf already complete)
    return out


def main() -> int:
    cc = STATIC_CC.read_text(encoding="utf-8")
    hh = STATIC_HH.read_text(encoding="utf-8")
    i18n = I18N.read_text(encoding="utf-8")

    petals = parse_petal_blocks(cc)
    enum_ids = parse_enum_ids(hh)
    zh_names = parse_zh_array(i18n, "PETAL_NAME_ZH")
    zh_descs = parse_zh_array(i18n, "PETAL_DESC_ZH")

    if len(petals) != len(enum_ids):
        print(
            f"warn: petal blocks {len(petals)} vs enum {len(enum_ids)}",
            file=sys.stderr,
        )
    n = min(len(petals), len(enum_ids), len(zh_names), len(zh_descs))

    # Material / skip ranges from enum names
    skip_names = {"None", "Coin"}
    mat_ids = set()
    for i, eid in enumerate(enum_ids[:n]):
        if eid.startswith("kMat") or eid == "kCoin" or eid == "kNone":
            mat_ids.add(i)
            skip_names.add(petals[i]["name"])

    # Group by English name → best entry per rarity
    families: dict[str, dict[int, dict]] = {}
    for i in range(n):
        if i in mat_ids:
            continue
        p = petals[i]
        if p["name"] in skip_names:
            continue
        fam = families.setdefault(p["name"], {})
        # Prefer higher id on collision
        if p["rarity"] not in fam or i > fam[p["rarity"]]["idx"]:
            fam[p["rarity"]] = {**p, "idx": i, "enum": enum_ids[i], "zh": zh_names[i], "zh_desc": zh_descs[i]}

    existing_enums = set(enum_ids)
    new_enums: list[str] = []
    new_data: list[str] = []
    new_zh_name: list[str] = []
    new_zh_desc: list[str] = []

    for name, by_r in sorted(families.items(), key=lambda x: x[0]):
        present = sorted(by_r.keys())
        if not present:
            continue
        rmin = present[0]
        # Fill every missing rarity from rmin..Unique inclusive
        for r in range(rmin, 9):
            if r in by_r:
                continue
            # Source = highest existing below r
            src_r = max(rr for rr in present if rr < r)
            src = by_r[src_r]
            mult = step_mult(src_r, r)
            hp = src["health"] * mult
            dmg = src["damage"] * mult
            attrs = scale_attrs(src["attrs"], mult)
            base_enum = sanitize_enum(name)
            suffix = RARITY_NAMES[r]
            eid = f"k{base_enum}{suffix}"
            # Avoid collisions
            n_try = 0
            while eid in existing_enums or eid in new_enums:
                n_try += 1
                eid = f"k{base_enum}{suffix}X{n_try}"
            new_enums.append(eid)
            existing_enums.add(eid)
            # remember for further steps in same family
            by_r[r] = {
                **src,
                "rarity": r,
                "health": hp,
                "damage": dmg,
                "attrs": attrs,
                "idx": 10_000_000 + len(new_enums),
            }
            present = sorted(by_r.keys())

            desc = src["description"]
            block = f"""    {{
        .name = "{name}",
        .description = "{desc}",
        .health = {fmt_num(hp)},
        .damage = {fmt_num(dmg)},
        .radius = {fmt_num(src['radius'])},
        .reload = {fmt_num(src['reload'])},
        .count = {src['count']},
        .rarity = {RARITY_ENUM[r]},
        .attributes = {attrs}
    }}"""
            new_data.append(block)
            new_zh_name.append(src["zh"])
            new_zh_desc.append(src["zh_desc"])

    print(f"families={len(families)} new_rungs={len(new_enums)}")
    if not new_enums:
        print("nothing to add")
        return 0

    # --- Patch StaticDefinitions.hh ---
    hh2 = hh.replace(
        "        kGoldenLeafUnique,\n        kNumPetals",
        "        kGoldenLeafUnique,\n"
        "        // Auto-filled rarity ladders (same English name; craft +1 only)\n"
        + "".join(f"        {e},\n" for e in new_enums)
        + "        kNumPetals",
    )
    if hh2 == hh:
        raise SystemExit("failed to patch StaticDefinitions.hh anchor")
    STATIC_HH.write_text(hh2, encoding="utf-8")

    # --- Patch StaticData.cc ---
    # Insert before closing `}};` of PETAL_DATA (before MOB_DATA)
    anchor = """        .attributes = { .extra_reload_factor = 0.74f, .icon_angle = -1 } // -26%
    }
}};"""
    insert = (
        "        .attributes = { .extra_reload_factor = 0.74f, .icon_angle = -1 } // -26%\n"
        "    },\n"
        "    // ---- Auto-filled rarity ladders (gen_petal_ladders.py) ----\n"
        + ",\n".join(new_data)
        + "\n}};"
    )
    if anchor not in cc:
        raise SystemExit("failed to find PETAL_DATA end anchor")
    cc2 = cc.replace(anchor, insert, 1)
    STATIC_CC.write_text(cc2, encoding="utf-8")

    # --- Patch I18nData.inc NAME + DESC ---
    def patch_zh(src: str, marker: str, adds: list[str]) -> str:
        # Find closing of array: last golden leaf strings before `};`
        m = re.search(
            rf"(static char const \*const {marker}\[PetalID::kNumPetals\] = \{{)(.*?)(\n\}});",
            src,
            re.S,
        )
        if not m:
            raise SystemExit(f"patch {marker} failed")
        extra = "".join(f',\n    "{a}"' for a in adds)
        # append before closing
        body = m.group(2).rstrip()
        if not body.endswith('"'):
            pass
        return src[: m.start()] + m.group(1) + body + extra + m.group(3) + src[m.end() :]

    i18n2 = patch_zh(i18n, "PETAL_NAME_ZH", new_zh_name)
    i18n2 = patch_zh(i18n2, "PETAL_DESC_ZH", new_zh_desc)
    I18N.write_text(i18n2, encoding="utf-8")

    print(f"wrote {len(new_enums)} ladder rungs")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
