#!/usr/bin/env python3
"""Fill every petal family (except Coin/None) to Common..Unique same-name ladders.

Materials (kMat*): copy base stats, set rarity — no special combat curve.
Combat petals: scale HP/dmg ×3 per step up to Mythic, ×2 above; inverse when filling down.
"""
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
    if to_r == from_r:
        return 1.0
    ascending = to_r > from_r
    a, b = (from_r, to_r) if ascending else (to_r, from_r)
    m = 1.0
    for r in range(a, b):
        m *= 2.0 if r >= 5 else 3.0
    return m if ascending else (1.0 / m)


def sanitize_enum(name: str) -> str:
    parts = re.sub(r"[^A-Za-z0-9]+", " ", name).strip().split()
    return "".join(p[:1].upper() + p[1:] for p in parts)


def parse_petal_blocks(text: str) -> list[dict]:
    start = text.index("PETAL_DATA = {{")
    end = text.index("\n}};", start)
    body = text[start:end]
    entries: list[str] = []
    depth = 0
    begin = None
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
        # Capture attributes object only (brace-balanced from .attributes =)
        attrs_s = "{}"
        am = re.search(r"\.attributes\s*=\s*\{", block)
        if am:
            i0 = am.end() - 1
            d = 0
            for k in range(i0, len(block)):
                if block[k] == "{":
                    d += 1
                elif block[k] == "}":
                    d -= 1
                    if d == 0:
                        attrs_s = block[i0 : k + 1]
                        break
        if not nm or not rar:
            continue
        try:
            ri = RARITY_NAMES.index(rar.group(1))
        except ValueError:
            continue
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
                "attrs": attrs_s.strip(),
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
        if line.startswith("k"):
            ids.append(line)
    return ids


def parse_zh_section(text: str, marker: str, next_marker: str) -> list[str]:
    a = text.index(marker)
    b = text.index(next_marker)
    return re.findall(r'"(?:\\.|[^"\\])*"', text[a:b])


def fmt_num(x: float) -> str:
    if abs(x - round(x)) < 1e-6:
        return f"{int(round(x))}.0"
    s = f"{x:.4f}".rstrip("0").rstrip(".")
    if "." not in s:
        s += ".0"
    return s


def scale_attrs(attrs: str, mult: float, is_mat: bool) -> str:
    if is_mat or attrs.strip() in ("{}", ""):
        return attrs if attrs.strip() else "{}"
    if abs(mult - 1.0) < 1e-9:
        return attrs

    def repl_poison(m: re.Match) -> str:
        inner = m.group(1)

        def sc(n: re.Match) -> str:
            return f"{n.group(1)}{fmt_num(float(n.group(2)) * mult)}"

        return ".poison_damage = {" + re.sub(r"(\.\w+\s*=\s*)([0-9.eE+-]+)", sc, inner) + "}"

    out = re.sub(r"\.poison_damage\s*=\s*\{([^{}]*)\}", repl_poison, attrs)

    def repl_field(key: str, s: str) -> str:
        def sc(m: re.Match) -> str:
            return f"{m.group(1)}{fmt_num(float(m.group(2)) * mult)}"

        return re.sub(rf"(\.{key}\s*=\s*)([0-9.eE+-]+)", sc, s)

    for k in ("aura_heal", "aura_radius", "heal", "constant_heal",
              "radial_pulse_damage", "radial_pulse_radius", "extra_range"):
        out = repl_field(k, out)
    return out


def main() -> int:
    cc = STATIC_CC.read_text(encoding="utf-8")
    hh = STATIC_HH.read_text(encoding="utf-8")
    i18n = I18N.read_text(encoding="utf-8")

    petals = parse_petal_blocks(cc)
    enum_ids = parse_enum_ids(hh)
    zh_names = parse_zh_section(i18n, "PETAL_NAME_ZH", "PETAL_DESC_ZH")
    zh_descs = parse_zh_section(i18n, "PETAL_DESC_ZH", "MOB_NAME_ZH")
    # strip quotes
    zh_names = [s[1:-1] for s in zh_names]
    zh_descs = [s[1:-1] for s in zh_descs]

    n = min(len(petals), len(enum_ids), len(zh_names), len(zh_descs))
    print(f"parsed petals={len(petals)} enum={len(enum_ids)} zh={len(zh_names)}/{len(zh_descs)}")

    skip_names = {"None", "Coin"}
    mat_idx = {i for i, e in enumerate(enum_ids[:n]) if e.startswith("kMat")}

    families: dict[str, dict[int, dict]] = {}
    for i in range(n):
        p = petals[i]
        if p["name"] in skip_names:
            continue
        fam = families.setdefault(p["name"], {})
        if p["rarity"] not in fam or i > fam[p["rarity"]]["idx"]:
            fam[p["rarity"]] = {
                **p,
                "idx": i,
                "enum": enum_ids[i],
                "zh": zh_names[i],
                "zh_desc": zh_descs[i],
                "mat": i in mat_idx or enum_ids[i].startswith("kMat"),
            }

    existing_enums = set(enum_ids)
    new_enums: list[str] = []
    new_data: list[str] = []
    new_zh_name: list[str] = []
    new_zh_desc: list[str] = []

    for name, by_r in sorted(families.items(), key=lambda x: x[0]):
        present = sorted(by_r.keys())
        if not present:
            continue
        is_mat = any(by_r[r].get("mat") for r in present)
        # Fill every rarity Common..Unique
        for r in range(9):
            if r in by_r:
                continue
            # Nearest existing source (prefer lower, else higher)
            below = [rr for rr in present if rr < r]
            above = [rr for rr in present if rr > r]
            if below:
                src_r = max(below)
            else:
                src_r = min(above)
            src = by_r[src_r]
            if is_mat:
                mult = 1.0
                hp, dmg = src["health"], src["damage"]
            else:
                mult = step_mult(src_r, r)
                hp = max(1.0, src["health"] * mult)
                dmg = max(1.0, src["damage"] * mult)
            attrs = scale_attrs(src["attrs"], 1.0 if is_mat else mult, is_mat)

            base_enum = sanitize_enum(name)
            suffix = RARITY_NAMES[r]
            eid = f"k{base_enum}{suffix}"
            n_try = 0
            while eid in existing_enums or eid in new_enums:
                n_try += 1
                eid = f"k{base_enum}{suffix}X{n_try}"
            new_enums.append(eid)
            existing_enums.add(eid)
            by_r[r] = {
                **src,
                "rarity": r,
                "health": hp,
                "damage": dmg,
                "attrs": attrs,
                "idx": 10_000_000 + len(new_enums),
                "mat": is_mat,
            }
            present = sorted(by_r.keys())

            block = (
                f"    {{\n"
                f"        .name = \"{name}\",\n"
                f"        .description = \"{src['description']}\",\n"
                f"        .health = {fmt_num(hp)},\n"
                f"        .damage = {fmt_num(dmg)},\n"
                f"        .radius = {fmt_num(src['radius'])},\n"
                f"        .reload = {fmt_num(src['reload'])},\n"
                f"        .count = {src['count']},\n"
                f"        .rarity = {RARITY_ENUM[r]},\n"
                f"        .attributes = {attrs}\n"
                f"    }}"
            )
            new_data.append(block)
            new_zh_name.append(src["zh"])
            new_zh_desc.append(src["zh_desc"])

    print(f"families={len(families)} new_rungs={len(new_enums)}")
    if not new_enums:
        print("nothing to add")
        return 0

    # Patch enum: insert before kNumPetals
    if "        kNumPetals" not in hh:
        raise SystemExit("kNumPetals anchor missing")
    hh2 = hh.replace(
        "        kNumPetals",
        "        // Full Common..Unique ladders (gen_petal_ladders.py; excl Coin)\n"
        + "".join(f"        {e},\n" for e in new_enums)
        + "        kNumPetals",
        1,
    )
    STATIC_HH.write_text(hh2, encoding="utf-8")

    # Patch PETAL_DATA: append before closing }};
    # Find last petal entry end — after auto-filled section or Golden Leaf
    mob = cc.index("\nstd::array<struct MobData")
    petal_end = cc.rfind("\n}};", 0, mob)
    if petal_end < 0:
        raise SystemExit("PETAL_DATA end not found")
    # Ensure comma after previous last entry
    before = cc[:petal_end].rstrip()
    if not before.endswith(","):
        # last entry ends with `}` — add comma
        if before.endswith("}"):
            before = before + ","
    insert = (
        before
        + "\n    // ---- Full rarity ladders (gen_petal_ladders.py) ----\n"
        + ",\n".join(new_data)
        + "\n}};"
        + cc[petal_end + len("\n}};") :]
    )
    STATIC_CC.write_text(insert, encoding="utf-8")

    # Patch I18n by section rewrite (NAME then DESC)
    def append_zh(src: str, marker: str, next_m: str, adds: list[str]) -> str:
        a = src.index(marker)
        # find opening brace after marker
        ob = src.index("{", a)
        b = src.index(next_m)
        close = src.rfind("}", ob, b)
        body = src[ob + 1 : close].rstrip()
        if body and not body.endswith(","):
            # last line is "foo" without comma — OK for C, we'll add comma before new
            pass
        extra = "".join(f',\n    "{a}"' for a in adds)
        # if body ends with "foo" add comma via extra starting with ,
        return src[: close] + extra + "\n}" + src[close + 1 :]

    i18n2 = append_zh(i18n, "PETAL_NAME_ZH", "PETAL_DESC_ZH", new_zh_name)
    i18n2 = append_zh(i18n2, "PETAL_DESC_ZH", "MOB_NAME_ZH", new_zh_desc)
    I18N.write_text(i18n2, encoding="utf-8")

    print(f"wrote {len(new_enums)} ladder rungs")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
