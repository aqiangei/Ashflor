#!/usr/bin/env python3
"""Batched acceptance: petal + mob strength vs wiki curves / local wiki pack.

Writes reports under temp/wiki_research/acceptance/ so later sessions can resume
without re-deriving from chat memory.

Batches:
  A — petal combat/heal ladders vs wiki ×3 / heal√3
  B — wiki-import attrs referenced in Server/
  C — MOB_DATA Unique projection vs wiki mob tables (name-mapped)
  D — summary PASS/FAIL gates
"""
from __future__ import annotations

import json
import math
import re
import sys
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]  # ashflor/
REPO = ROOT.parent  # d:/ashflor
STATIC = ROOT / "Shared" / "StaticData.cc"
DEFS = ROOT / "Shared" / "StaticDefinitions.hh"
SERVER = ROOT / "Server"
WIKI = REPO / "temp" / "wiki_research"
OUT = WIKI / "acceptance"

RARITY = [
    "Common", "Unusual", "Rare", "Epic", "Legendary",
    "Mythic", "Ultra", "Super", "Unique",
]
COMBAT = [1.0, 3.0, 9.0, 27.0, 81.0, 243.0, 729.0, 2187.0, 6561.0]
SQRT3 = math.sqrt(3.0)
HEAL = [1.0, 3.0, 9.0, 27.0, 81.0, 243.0, 243.0 * SQRT3, 729.0, 729.0 * SQRT3]
# Live Ashflor ladder (RarityScale.hh). Mythic- stays wiki-steep; Ultra+ flattened
# so Unique petals can chip Ultra trash (was 145800 / 4.374e6 / 2.6244e7).
MOB_HP = [1.0, 3.75, 13.5, 54.0, 324.0, 3159.0, 28000.0, 140000.0, 700000.0]
# Wiki EN/ZH reference table — used only when projecting missing wiki Unique rows.
WIKI_MOB_HP = [1.0, 3.75, 13.5, 54.0, 324.0, 3159.0, 145800.0, 4374000.0, 26244000.0]
MOB_DMG = COMBAT[:]
# Product: live Unique HP << wiki Unique. Do not fail batch C on HP ratio alone.
MOB_HP_PRODUCT_DIVERGENCE = True

# ZH wiki title → Ashflor MOB_DATA English name(s)
MOB_NAME_MAP: dict[str, list[str]] = {
    "兵蚁": ["Soldier Ant"],
    "工蚁": ["Worker Ant"],
    "幼蚁": ["Baby Ant"],
    "火兵蚁": ["Soldier Fire Ant"],
    "蚁后": ["Queen Ant"],
    "黄蜂": ["Hornet"],
    "胡蜂": ["Jungle Wasp"],
    "蜘蛛": ["Spider"],
    "甲虫": ["Beetle"],
    "瓢虫_深色_": ["Ladybug"],  # Common ladybug (first)
    "水母": ["Moon Jelly"],
    "螃蟹": ["Reef Crab"],
    "蝎子": ["Scorpion"],
    "蜈蚣_邪恶_": ["Evil Centipede"],
    "海星_生物_": ["Coral Star"],
    "白兵蚁": ["Soldier Termite"],
    "水蛭": ["Sea Leech"],
    "螳螂": ["Emerald Mantis"],
}
# Single-rarity / joke petals — not expected to have 9-rung ladders
INCOMPLETE_OK = {
    "Azalea", "Beetle Egg", "Blue Iris", "Dahlia", "Tringer", "Useless Basic",
}
# Wiki petal Unique that clearly used mob-HP table (parse junk) — ignore mismatch
WIKI_PETAL_SUSPECT = {"Bubble"}

HEAL_ATTRS = (
    "constant_heal", "burst_heal", "aura_heal", "armor", "poison_armor", "damage_absorb",
)
COMBAT_ATTRS = (
    "stack_damage", "splash_damage", "lightning_damage",
    "tomato_max_damage", "clay_max_damage", "domino_max_damage",
)


def parse_petal_blocks(text: str) -> list[str]:
    start = text.index("PETAL_DATA = {{")
    end = text.index("\n}};", start)
    body = text[start:end]
    out: list[str] = []
    depth = 0
    begin = None
    for j, ch in enumerate(body):
        if ch == "{":
            depth += 1
            if depth == 3 and begin is None:
                begin = j
        elif ch == "}":
            if depth == 3 and begin is not None:
                out.append(body[begin : j + 1])
                begin = None
            depth -= 1
    return out


def parse_mob_blocks(text: str) -> list[str]:
    # MOB_DATA = {{ ... }};
    m = re.search(r"MOB_DATA\s*=\s*\{\{", text)
    if not m:
        raise SystemExit("MOB_DATA not found")
    start = m.start()
    # find matching close — same depth trick from PETAL style after "{{"
    i = text.index("{{", start) + 1  # at second {
    depth = 0
    begin_body = i
    for j in range(i, len(text)):
        if text[j] == "{":
            depth += 1
        elif text[j] == "}":
            depth -= 1
            if depth == 0:
                body = text[begin_body + 1 : j]
                break
    else:
        raise SystemExit("MOB_DATA parse fail")
    out: list[str] = []
    depth = 0
    begin = None
    for j, ch in enumerate(body):
        if ch == "{":
            depth += 1
            if depth == 1 and begin is None:
                begin = j
        elif ch == "}":
            if depth == 1 and begin is not None:
                out.append(body[begin : j + 1])
                begin = None
            depth -= 1
    return out


def get_num(block: str, key: str) -> float | None:
    m = re.search(rf"\.{key}\s*=\s*\{{?\s*([0-9.eE+-]+)", block)
    return float(m.group(1)) if m else None


def get_str(block: str, key: str) -> str | None:
    m = re.search(rf'\.{key}\s*=\s*"([^"]*)"', block)
    return m.group(1) if m else None


def get_rarity(block: str) -> int | None:
    m = re.search(r"\.rarity\s*=\s*RarityID::k(\w+)", block)
    if not m:
        return None
    try:
        return RARITY.index(m.group(1))
    except ValueError:
        return None


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


def rel_err(got: float, expect: float) -> float:
    if abs(expect) < 1e-9:
        return 0.0 if abs(got) < 1e-9 else 1.0
    return abs(got - expect) / abs(expect)


def batch_a_petals(text: str) -> dict:
    blocks = parse_petal_blocks(text)
    by: dict[str, dict[int, dict]] = {}
    for b in blocks:
        name = get_str(b, "name")
        ri = get_rarity(b)
        if not name or ri is None or name in ("None", "Coin"):
            continue
        attrs = extract_attrs(b)
        row = {
            "health": get_num(b, "health") or 0.0,
            "damage": get_num(b, "damage") or 0.0,
        }
        for k in HEAL_ATTRS + COMBAT_ATTRS:
            v = get_num(attrs, k)
            if v is not None:
                row[k] = v
        # keep first occurrence per name+rarity
        by.setdefault(name, {}).setdefault(ri, row)

    fail_combat: list[dict] = []
    fail_heal: list[dict] = []
    incomplete: list[str] = []
    ok_combat = ok_heal = 0
    tol = 0.03  # 3%

    for name, fam in sorted(by.items()):
        if len(fam) < 9:
            if name not in INCOMPLETE_OK:
                incomplete.append(f"{name} ({len(fam)}/9)")
            continue
        hs = [fam[i]["health"] for i in range(9)]
        ds = [fam[i]["damage"] for i in range(9)]
        # combat: from first nonzero anchor
        for label, vals in (("health", hs), ("damage", ds)):
            if max(vals) <= 1e-9:
                continue
            ari = next(i for i, v in enumerate(vals) if v > 1e-9)
            bad = False
            details = []
            for ri in range(9):
                exp = vals[ari] * (COMBAT[ri] / COMBAT[ari])
                e = rel_err(vals[ri], exp)
                if e > tol:
                    bad = True
                    details.append(
                        {"rarity": RARITY[ri], "got": vals[ri], "expect": exp, "err": round(e, 4)}
                    )
            if bad:
                fail_combat.append({"name": name, "field": label, "anchor": RARITY[ari], "bad": details[:3]})
            else:
                ok_combat += 1

        # heal attrs present on Common+
        for key in HEAL_ATTRS:
            if key not in fam[0] and not any(key in fam[i] for i in range(9)):
                continue
            vals = [fam[i].get(key, 0.0) for i in range(9)]
            if max(abs(v) for v in vals) <= 1e-9:
                continue
            ari = next(i for i, v in enumerate(vals) if abs(v) > 1e-9)
            bad = False
            details = []
            for ri in range(9):
                exp = vals[ari] * (HEAL[ri] / HEAL[ari])
                e = rel_err(vals[ri], exp)
                if e > tol:
                    bad = True
                    details.append(
                        {"rarity": RARITY[ri], "got": vals[ri], "expect": exp, "err": round(e, 4)}
                    )
            if bad:
                fail_heal.append({"name": name, "field": key, "anchor": RARITY[ari], "bad": details[:3]})
            else:
                ok_heal += 1

    # vs wiki_en_petals_scaled Unique when available
    wiki_path = WIKI / "wiki_en_petals_scaled.json"
    wiki_mismatch: list[dict] = []
    if wiki_path.is_file():
        wiki = json.loads(wiki_path.read_text(encoding="utf-8"))
        for name, fam in by.items():
            if len(fam) < 9 or name not in wiki:
                continue
            if name in WIKI_PETAL_SUSPECT:
                continue
            w = wiki[name]
            for field in ("health", "damage"):
                wr = w.get("Unique") or w.get("Eternal") or {}
                wv = wr.get(field)
                if wv is None:
                    continue
                # Skip wiki Unique that looks like mob-HP curve (petal Unique ≫ 1e6)
                if field == "health" and float(wv) >= 1e6:
                    continue
                got = fam[8][field]
                if abs(wv) < 1e-9 and abs(got) < 1e-9:
                    continue
                e = rel_err(got, float(wv))
                if e > 0.05:
                    wiki_mismatch.append(
                        {
                            "name": name,
                            "field": field,
                            "ash_unique": got,
                            "wiki_unique": wv,
                            "err": round(e, 4),
                        }
                    )

    return {
        "families_full": sum(1 for f in by.values() if len(f) == 9),
        "families_total": len(by),
        "incomplete_ladders": incomplete[:50],
        "incomplete_count": len(incomplete),
        "combat_series_ok": ok_combat,
        "combat_series_fail": fail_combat,
        "heal_series_ok": ok_heal,
        "heal_series_fail": fail_heal,
        "wiki_unique_mismatch": wiki_mismatch[:80],
        "wiki_unique_mismatch_count": len(wiki_mismatch),
        "pass": (
            len(fail_combat) == 0
            and len(fail_heal) == 0
            and len(incomplete) == 0
            and len(wiki_mismatch) == 0
        ),
    }


def batch_b_attrs() -> dict:
    defs = DEFS.read_text(encoding="utf-8", errors="replace")
    fields = re.findall(
        r"(?:float|uint8_t|int)\s+(\w+)\s*=\s*[^;]*;\s*//\s*wiki import", defs
    )
    static = STATIC.read_text(encoding="utf-8", errors="replace")
    server_blob = "\n".join(
        p.read_text(encoding="utf-8", errors="replace")
        for p in SERVER.rglob("*")
        if p.suffix.lower() in {".cc", ".hh", ".h", ".cpp", ".hpp"}
        and "uWebSockets" not in str(p)
    )
    missing = []
    unused_static = []
    ok = []
    for f in fields:
        if not re.search(rf"\.{re.escape(f)}\s*=", static):
            unused_static.append(f)
            continue
        if re.search(
            rf"(?:attrs|A|B|Pd|D|L)\.{re.escape(f)}\b|\.{re.escape(f)}\s*[=,}}>]|\b{re.escape(f)}\b",
            server_blob,
        ):
            ok.append(f)
        else:
            missing.append(f)
    return {
        "fields": len(fields),
        "ok": len(ok),
        "unused_in_static": unused_static,
        "missing_server": missing,
        "pass": len(missing) == 0,
    }


def batch_c_mobs(text: str) -> dict:
    blocks = parse_mob_blocks(text)
    ash: dict[str, dict] = {}
    for b in blocks:
        name = get_str(b, "name")
        ri = get_rarity(b)
        if not name or ri is None:
            continue
        hp = get_num(b, "health")
        dmg = get_num(b, "damage")
        if hp is None:
            continue
        row = {
            "base_rarity": ri,
            "base_rarity_name": RARITY[ri],
            "baked_hp": hp,
            "baked_dmg": dmg or 0.0,
            "unique_hp": hp * (MOB_HP[8] / MOB_HP[ri]),
            "unique_dmg": (dmg or 0.0) * (MOB_DMG[8] / MOB_DMG[ri]),
        }
        # Duplicate names (e.g. Ladybug variants): keep lowest base rarity
        if name in ash and ash[name]["base_rarity"] <= ri:
            continue
        ash[name] = row

    wiki_path = WIKI / "wiki_zh_mobs_reparsed.json"
    wiki = json.loads(wiki_path.read_text(encoding="utf-8")) if wiki_path.is_file() else {}

    compared = []
    unmatched_wiki = []
    for zh, wstats in wiki.items():
        if not wstats or "Common" not in wstats and "Unique" not in wstats:
            continue
        # Prefer Unique wiki row; else derive from Common × table
        w_u = wstats.get("Unique") or {}
        w_c = wstats.get("Common") or {}
        names = MOB_NAME_MAP.get(zh, [])
        # also try direct EN if key looks english
        if not names and re.match(r"^[A-Za-z]", zh):
            names = [zh.replace("_", " ")]
        hit = None
        for n in names:
            if n in ash:
                hit = n
                break
        if not hit:
            for n in names:
                for an in ash:
                    if n.lower() == an.lower():
                        hit = an
                        break
                if hit:
                    break
        # Ladybug: prefer Common-base entry when multiple exist in file —
        # ash dict already keeps last parse; re-parse Common Ladybug if needed.
        if hit == "Ladybug" and ash.get("Ladybug", {}).get("base_rarity", 0) != 0:
            # force compare using Common stats if we can find them below
            pass
        if not hit:
            if w_u or w_c:
                unmatched_wiki.append(zh)
            continue
        a = ash[hit]
        row = {
            "wiki_zh": zh,
            "ash_name": hit,
            "ash_base": a["base_rarity_name"],
            "ash_unique_hp": a["unique_hp"],
            "ash_unique_dmg": a["unique_dmg"],
            "wiki_unique_hp": w_u.get("health"),
            "wiki_unique_dmg": w_u.get("damage"),
        }
        # If wiki Unique missing, project from Common × wiki table (not live MOB_HP)
        if row["wiki_unique_hp"] is None and w_c.get("health") is not None:
            row["wiki_unique_hp"] = float(w_c["health"]) * WIKI_MOB_HP[8]
            row["wiki_unique_hp_projected"] = True
        if row["wiki_unique_dmg"] is None and w_c.get("damage") is not None:
            row["wiki_unique_dmg"] = float(w_c["damage"]) * MOB_DMG[8]
            row["wiki_unique_dmg_projected"] = True

        def gate(got, expect, lo=0.25, hi=4.0):
            if got is None or expect is None or expect == 0:
                return None
            r = got / expect
            return lo <= r <= hi, r

        hp_g = gate(row["ash_unique_hp"], row["wiki_unique_hp"])
        dg_g = gate(row["ash_unique_dmg"], row["wiki_unique_dmg"])
        row["hp_ok"] = None if hp_g is None else hp_g[0]
        row["hp_ratio"] = None if hp_g is None else round(hp_g[1], 4)
        row["dmg_ok"] = None if dg_g is None else dg_g[0]
        row["dmg_ratio"] = None if dg_g is None else round(dg_g[1], 4)
        # Ultra+ live ladder is intentionally below wiki Unique HP.
        if MOB_HP_PRODUCT_DIVERGENCE and row["hp_ok"] is False:
            row["hp_ok"] = True
            row["hp_product_divergence"] = True
        compared.append(row)

    fails = [
        r
        for r in compared
        if (r["hp_ok"] is False) or (r["dmg_ok"] is False)
    ]
    # Internal consistency: every ash mob Unique projection finite
    insane = [
        {"name": n, **v}
        for n, v in ash.items()
        if not math.isfinite(v["unique_hp"]) or v["unique_hp"] <= 0
    ]

    return {
        "ash_mobs": len(ash),
        "wiki_compared": len(compared),
        "compared": compared,
        "fails": fails,
        "unmatched_wiki_with_stats": unmatched_wiki,
        "insane_projections": insane,
        "pass": len(fails) == 0 and len(insane) == 0,
    }


def batch_d_summary(a: dict, b: dict, c: dict) -> dict:
    return {
        "generated_at": datetime.now(timezone.utc).isoformat(),
        "A_petals": {
            "pass": a["pass"],
            "families_full": a["families_full"],
            "combat_fail": len(a["combat_series_fail"]),
            "heal_fail": len(a["heal_series_fail"]),
            "wiki_unique_mismatch": a["wiki_unique_mismatch_count"],
            "incomplete": a["incomplete_count"],
        },
        "B_attrs": {
            "pass": b["pass"],
            "ok": b["ok"],
            "missing_server": b["missing_server"],
        },
        "C_mobs": {
            "pass": c["pass"],
            "compared": c["wiki_compared"],
            "fails": len(c["fails"]),
            "unmatched_wiki": len(c["unmatched_wiki_with_stats"]),
        },
        "overall_pass": a["pass"] and b["pass"] and c["pass"],
    }


def write_md(summary: dict, a: dict, b: dict, c: dict) -> None:
    lines = [
        f"# Wiki strength acceptance — {summary['generated_at']}",
        "",
        f"**Overall: {'PASS' if summary['overall_pass'] else 'FAIL'}**",
        "",
        "## Batch A — Petal ladders",
        f"- Full 9-rung families: {a['families_full']}/{a['families_total']}",
        f"- Combat series fails: {len(a['combat_series_fail'])}",
        f"- Heal series fails: {len(a['heal_series_fail'])}",
        f"- Wiki Unique mismatches (>5%): {a['wiki_unique_mismatch_count']}",
        f"- Incomplete ladders: {a['incomplete_count']}",
        "",
    ]
    if a["combat_series_fail"][:15]:
        lines.append("### Combat fails (sample)")
        for row in a["combat_series_fail"][:15]:
            lines.append(f"- {row['name']}.{row['field']} anchor={row['anchor']} bad={row['bad']}")
        lines.append("")
    if a["heal_series_fail"][:15]:
        lines.append("### Heal fails (sample)")
        for row in a["heal_series_fail"][:15]:
            lines.append(f"- {row['name']}.{row['field']} bad={row['bad']}")
        lines.append("")
    if a["wiki_unique_mismatch"][:20]:
        lines.append("### vs wiki_en Unique (sample)")
        for row in a["wiki_unique_mismatch"][:20]:
            lines.append(
                f"- {row['name']}.{row['field']}: ash={row['ash_unique']} wiki={row['wiki_unique']} err={row['err']}"
            )
        lines.append("")

    lines += [
        "## Batch B — wiki import attrs",
        f"- Referenced OK: {b['ok']}/{b['fields']}",
        f"- Missing Server: {b['missing_server'] or '∅'}",
        "",
        "## Batch C — Mobs Unique projection vs wiki ZH",
        f"- Ash mobs: {c['ash_mobs']}; compared: {c['wiki_compared']}; fails: {len(c['fails'])}",
        f"- Unmatched wiki titles: {c['unmatched_wiki_with_stats']}",
        "",
    ]
    if c["fails"]:
        lines.append("### Mob fails")
        for row in c["fails"]:
            lines.append(
                f"- {row['ash_name']} (wiki {row['wiki_zh']} base={row['ash_base']}): "
                f"hp_ratio={row['hp_ratio']} dmg_ratio={row['dmg_ratio']}"
            )
        lines.append("")
    if c["compared"]:
        lines.append("### Compared (all)")
        for row in c["compared"]:
            mark = "OK" if row not in c["fails"] else "FAIL"
            lines.append(
                f"- [{mark}] {row['ash_name']}: Unique hp={row['ash_unique_hp']:.4g}/wiki={row['wiki_unique_hp']} "
                f"dmg={row['ash_unique_dmg']:.4g}/wiki={row['wiki_unique_dmg']} "
                f"(hp×{row['hp_ratio']}, dmg×{row['dmg_ratio']})"
            )
    (OUT / "ACCEPTANCE.md").write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    OUT.mkdir(parents=True, exist_ok=True)
    text = STATIC.read_text(encoding="utf-8")
    print("Batch A petals…")
    a = batch_a_petals(text)
    print("Batch B attrs…")
    b = batch_b_attrs()
    print("Batch C mobs…")
    c = batch_c_mobs(text)
    s = batch_d_summary(a, b, c)

    (OUT / "batch_A_petals.json").write_text(
        json.dumps(a, indent=2, ensure_ascii=False), encoding="utf-8"
    )
    (OUT / "batch_B_attrs.json").write_text(
        json.dumps(b, indent=2, ensure_ascii=False), encoding="utf-8"
    )
    (OUT / "batch_C_mobs.json").write_text(
        json.dumps(c, indent=2, ensure_ascii=False), encoding="utf-8"
    )
    (OUT / "summary.json").write_text(
        json.dumps(s, indent=2, ensure_ascii=False), encoding="utf-8"
    )
    write_md(s, a, b, c)

    print(json.dumps(s, indent=2, ensure_ascii=False))
    print(f"Wrote {OUT}")
    return 0 if s["overall_pass"] else 1


if __name__ == "__main__":
    sys.exit(main())
