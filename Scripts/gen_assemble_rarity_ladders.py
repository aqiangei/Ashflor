#!/usr/bin/env python3
"""Append-only rarity ladders for assemble mats + recipe results.

Keeps existing PetalIDs stable. Adds same-English-name variants for every
RarityID not already present. Patches:
  Shared/StaticDefinitions.hh
  Shared/StaticData.cc
  Client/I18nData.inc
  Shared/PetalNameZh.inc

Run:  python Scripts/gen_assemble_rarity_ladders.py
"""
from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DEF_HH = ROOT / "Shared" / "StaticDefinitions.hh"
DATA_CC = ROOT / "Shared" / "StaticData.cc"
RECIPE_CC = ROOT / "Shared" / "Recipe.cc"
I18N_INC = ROOT / "Client" / "I18nData.inc"
ZH_INC = ROOT / "Shared" / "PetalNameZh.inc"

RARITY_NAMES = [
    "Common", "Unusual", "Rare", "Epic", "Legendary",
    "Mythic", "Ultra", "Super", "Unique",
]
RARITY_ENUM = [f"RarityID::k{n}" for n in RARITY_NAMES]
# Combat-ish ×3 per step relative to the family's existing baseline rarity.
RARITY_MULT = [1.0, 3.0, 9.0, 27.0, 81.0, 243.0, 500.0, 1000.0, 2000.0]

MARKER_BEGIN = "ASSEMBLE_RARITY_LADDERS_BEGIN"
MARKER_END = "ASSEMBLE_RARITY_LADDERS_END"


def parse_petal_enum(text: str) -> list[str]:
    m = re.search(
        r"namespace PetalID \{.*?enum\s*:\s*T\s*\{(.*?)\s*kNumPetals",
        text,
        re.S,
    )
    if not m:
        raise SystemExit("PetalID enum not found")
    ids = []
    for line in m.group(1).splitlines():
        line = line.split("//")[0].strip().rstrip(",")
        if re.fullmatch(r"k\w+", line):
            ids.append(line)
    return ids


def parse_petal_data_entries(text: str) -> list[dict]:
    m = re.search(
        r"std::array<struct PetalData,\s*PetalID::kNumPetals>\s*const\s*PETAL_DATA\s*=\s*\{\{",
        text,
    )
    if not m:
        raise SystemExit("PETAL_DATA = {{ not found")
    start = m.end()
    depth = 2  # already inside {{
    i = start
    entries: list[str] = []
    entry_start = None
    while i < len(text) and depth > 0:
        c = text[i]
        if c == '"':
            i += 1
            while i < len(text):
                if text[i] == "\\":
                    i += 2
                    continue
                if text[i] == '"':
                    i += 1
                    break
                i += 1
            continue
        if c == "{":
            if depth == 2:
                entry_start = i
            depth += 1
        elif c == "}":
            depth -= 1
            if depth == 2 and entry_start is not None:
                entries.append(text[entry_start : i + 1])
                entry_start = None
            if depth == 0:
                break
        i += 1

    blocks = []
    for p in entries:
        nm = re.search(r'\.name\s*=\s*"([^"]*)"', p)
        rar = re.search(r"\.rarity\s*=\s*RarityID::k(\w+)", p)
        if not nm or not rar:
            continue
        hp = re.search(r"\.health\s*=\s*([0-9.eE+-]+)", p)
        dmg = re.search(r"\.damage\s*=\s*([0-9.eE+-]+)", p)
        rad = re.search(r"\.radius\s*=\s*([0-9.eE+-]+)", p)
        rel = re.search(r"\.reload\s*=\s*([0-9.eE+-]+)", p)
        cnt = re.search(r"\.count\s*=\s*(\d+)", p)
        desc = re.search(r'\.description\s*=\s*"([^"]*)"', p)
        blocks.append(
            {
                "name": nm.group(1),
                "rarity": rar.group(1),
                "health": float(hp.group(1)) if hp else 5.0,
                "damage": float(dmg.group(1)) if dmg else 3.0,
                "radius": float(rad.group(1)) if rad else 10.0,
                "reload": float(rel.group(1)) if rel else 2.5,
                "count": int(cnt.group(1)) if cnt else 1,
                "desc": desc.group(1) if desc else "",
            }
        )
    return blocks


def parse_string_array(blob: str, token: str) -> list[str]:
    i = blob.find(token)
    if i < 0:
        return []
    j = blob.find("{", i)
    k = blob.find("};", j)
    chunk = blob[j + 1 : k]
    return re.findall(r'"((?:\\.|[^"\\])*)"', chunk)


def append_string_array(path: Path, token: str, values: list[str]) -> None:
    if not path.exists() or not values:
        return
    t = path.read_text(encoding="utf-8")
    i = t.find(token)
    if i < 0:
        print(f"warn: {token} not in {path.name}")
        return
    j = t.find("};", i)
    if j < 0:
        return
    extra = "".join(f'    "{v}",\n' for v in values)
    path.write_text(t[:j] + extra + t[j:], encoding="utf-8")


def main() -> int:
    def_text = DEF_HH.read_text(encoding="utf-8")
    data_text = DATA_CC.read_text(encoding="utf-8")
    recipe_text = RECIPE_CC.read_text(encoding="utf-8")

    if MARKER_BEGIN in def_text:
        print("Ladders already present — remove marker block to regenerate.")
        return 1

    enum_ids = parse_petal_enum(def_text)
    blocks = parse_petal_data_entries(data_text)
    if len(blocks) != len(enum_ids):
        print(f"PETAL_DATA {len(blocks)} != enum {len(enum_ids)}")
        return 1

    id_to_block = {enum_ids[i]: blocks[i] for i in range(len(enum_ids))}

    recipe_ids = set(re.findall(r"PetalID::(k\w+)", recipe_text))
    recipe_ids -= {"kCoin", "kNone", "kNumPetals"}

    bases: list[str] = []
    seen: set[str] = set()
    for bid in [i for i in enum_ids if i.startswith("kMat")] + sorted(
        i for i in recipe_ids if i in id_to_block and not i.startswith("kMat")
    ):
        if bid in seen:
            continue
        seen.add(bid)
        bases.append(bid)

    # Existing rarities keyed by English name (across ALL petals)
    name_rarities: dict[str, set[int]] = {}
    for b in blocks:
        try:
            ri = RARITY_NAMES.index(b["rarity"])
        except ValueError:
            continue
        name_rarities.setdefault(b["name"], set()).add(ri)

    i18n_text = I18N_INC.read_text(encoding="utf-8") if I18N_INC.exists() else ""
    zh_live = ZH_INC.read_text(encoding="utf-8") if ZH_INC.exists() else ""
    zh_names = parse_string_array(zh_live, "LIVE_PETAL_NAME_ZH") or parse_string_array(
        i18n_text, "PETAL_NAME_ZH"
    )
    zh_descs = parse_string_array(i18n_text, "PETAL_DESC_ZH")

    new_enums: list[str] = []
    new_data: list[str] = []
    new_zh_names: list[str] = []
    new_zh_descs: list[str] = []

    for base_id in bases:
        b = id_to_block[base_id]
        base_ri = RARITY_NAMES.index(b["rarity"])
        base_mult = RARITY_MULT[base_ri]
        existing = name_rarities.get(b["name"], set())
        base_idx = enum_ids.index(base_id)
        zh_n = zh_names[base_idx] if base_idx < len(zh_names) else b["name"]
        zh_d = zh_descs[base_idx] if base_idx < len(zh_descs) else b["desc"]

        for ri, rname in enumerate(RARITY_NAMES):
            if ri in existing:
                continue
            enum_name = f"{base_id}{rname}"
            if enum_name in enum_ids or enum_name in new_enums:
                enum_name = f"{base_id}_{rname}"
            new_enums.append(enum_name)
            scale = RARITY_MULT[ri] / base_mult
            hp = max(1.0, round(b["health"] * scale, 2))
            dmg = max(0.0, round(b["damage"] * scale, 2))
            desc = b["desc"].replace("\\", "\\\\").replace('"', '\\"')
            new_data.append(
                "    {\n"
                f'        .name = "{b["name"]}",\n'
                f'        .description = "{desc}",\n'
                f"        .health = {hp}, .damage = {dmg}, .radius = {b['radius']},\n"
                f"        .reload = {b['reload']}, .count = {b['count']},\n"
                f"        .rarity = {RARITY_ENUM[ri]},\n"
                "        .attributes = {}\n"
                "    }"
            )
            new_zh_names.append(zh_n.replace("\\", "\\\\").replace('"', '\\"'))
            new_zh_descs.append(zh_d.replace("\\", "\\\\").replace('"', '\\"'))
            existing.add(ri)  # avoid dup within same name in this run
            name_rarities[b["name"]] = existing

    if not new_enums:
        print("Nothing to add.")
        return 0

    # --- StaticDefinitions.hh ---
    marker = (
        f"        // {MARKER_BEGIN} (gen_assemble_rarity_ladders.py)\n"
        + "".join(f"        {e},\n" for e in new_enums)
        + f"        // {MARKER_END}\n"
    )
    def_text2, nsub = re.subn(
        r"(        kNumPetals\n)",
        marker + r"\1",
        def_text,
        count=1,
    )
    if nsub != 1:
        print("Failed to insert enum marker before kNumPetals")
        return 1
    DEF_HH.write_text(def_text2, encoding="utf-8")

    # --- StaticData.cc: insert before closing }}; ---
    # Find end of PETAL_DATA array body
    m = re.search(
        r"std::array<struct PetalData,\s*PetalID::kNumPetals>\s*const\s*PETAL_DATA\s*=\s*\{\{",
        data_text,
    )
    start = m.end()
    depth = 2
    i = start
    end_inner = None
    while i < len(data_text) and depth > 0:
        c = data_text[i]
        if c == '"':
            i += 1
            while i < len(data_text):
                if data_text[i] == "\\":
                    i += 2
                    continue
                if data_text[i] == '"':
                    i += 1
                    break
                i += 1
            continue
        if c == "{":
            depth += 1
        elif c == "}":
            depth -= 1
            if depth == 1:
                end_inner = i  # position of first } of }};
                break
        i += 1
    if end_inner is None:
        print("Failed to find PETAL_DATA end")
        return 1

    # Ensure comma after last entry
    before = data_text[:end_inner].rstrip()
    if not before.endswith(","):
        # last entry ends with `}` — add comma after that entry's closing brace
        # find last `}` before end_inner in before
        pass
    insert = (
        f"\n    // {MARKER_BEGIN} (gen_assemble_rarity_ladders.py)\n"
        + ",\n".join(new_data)
        + f",\n    // {MARKER_END}\n"
    )
    # If last char before whitespace+end is not comma, add one
    strip_end = before
    if not strip_end.endswith(","):
        insert = "," + insert
    data_text2 = before + insert + data_text[end_inner:]
    DATA_CC.write_text(data_text2, encoding="utf-8")

    append_string_array(I18N_INC, "PETAL_NAME_ZH", new_zh_names)
    append_string_array(I18N_INC, "PETAL_DESC_ZH", new_zh_descs)
    append_string_array(ZH_INC, "LIVE_PETAL_NAME_ZH", new_zh_names)

    print(f"Added {len(new_enums)} ladder petals across {len(bases)} assemble bases.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
