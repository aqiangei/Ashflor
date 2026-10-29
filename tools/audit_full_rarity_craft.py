#!/usr/bin/env python3
"""Full audit: every family has 9 rarities; craft ladders; assemble UI resolve."""
from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CC = (ROOT / "Shared" / "StaticData.cc").read_text(encoding="utf-8")
HH = (ROOT / "Shared" / "StaticDefinitions.hh").read_text(encoding="utf-8")
RECIPE = (ROOT / "Shared" / "Recipe.cc").read_text(encoding="utf-8")
PANELS = (ROOT / "Client" / "Ui" / "InGame" / "GamePanels.cc").read_text(encoding="utf-8")
CLIENT = (ROOT / "Server" / "Client.cc").read_text(encoding="utf-8")
NET = (ROOT / "Client" / "Network.cc").read_text(encoding="utf-8")
CRAFT = (ROOT / "Shared" / "Craft.cc").read_text(encoding="utf-8")

RARITY = [
    "Common", "Unusual", "Rare", "Epic", "Legendary",
    "Mythic", "Ultra", "Super", "Unique",
]


def parse_blocks(text: str) -> list[str]:
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
    return entries


def parse_enums(hh: str) -> list[str]:
    m = re.search(
        r"namespace PetalID\s*\{.*?enum\s*:\s*T\s*\{(.*?)\bkNumPetals", hh, re.S
    )
    if not m:
        raise SystemExit("PetalID enum missing")
    ids = []
    for line in m.group(1).splitlines():
        line = line.split("//")[0].strip().rstrip(",")
        if line.startswith("k"):
            ids.append(line)
    return ids


def main() -> int:
    blocks = parse_blocks(CC)
    enums = parse_enums(HH)
    n = min(len(blocks), len(enums))
    print(f"=== DATA === blocks={len(blocks)} enum={len(enums)} paired={n}")

    by_name: dict[str, set[int]] = {}
    name_of: dict[str, str] = {}
    enum_of_name_r: dict[tuple[str, int], str] = {}
    for i in range(n):
        b = blocks[i]
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
        by_name.setdefault(name, set()).add(ri)
        name_of[enums[i]] = name
        # highest enum ordinal wins (matches Craft::find_same_name_at_rarity)
        prev = enum_of_name_r.get((name, ri))
        if prev is None or i > enums.index(prev):
            enum_of_name_r[(name, ri)] = enums[i]

    incomplete = [
        (nm, [RARITY[i] for i in range(9) if i not in rs])
        for nm, rs in sorted(by_name.items())
        if len(rs) < 9
    ]
    print(f"families={len(by_name)} incomplete={len(incomplete)}")
    for nm, miss in incomplete:
        print(f"  INCOMPLETE {nm}: missing {', '.join(miss)}")

    # 5-merge: every non-Unique rung needs +1 same name
    upgrade_ok = 0
    upgrade_gap = []
    for nm, rs in by_name.items():
        for r in sorted(rs):
            if r >= 8:
                continue
            if (r + 1) in rs:
                upgrade_ok += 1
            else:
                upgrade_gap.append((nm, RARITY[r], RARITY[r + 1]))
    print(f"=== CRAFT 5-merge === ok_steps={upgrade_ok} gaps={len(upgrade_gap)}")
    for g in upgrade_gap:
        print(f"  NO_UPGRADE {g[0]} {g[1]} -> {g[2]}")

    # can_upgrade_petal excludes Coin and bag_only; check code still allows mats
    if "is_shop_material" in CRAFT and "can_upgrade_petal" in CRAFT:
        cu = re.search(
            r"uint8_t can_upgrade_petal\(PetalID::T id\)\s*\{(.*?)\n    \}",
            CRAFT,
            re.S,
        )
        body = cu.group(1) if cu else ""
        blocks_mats = "is_shop_material" in body
        print(f"can_upgrade blocks materials={blocks_mats} (want False)")

    # Recipes
    recipe_pat = re.compile(
        r"\{\s*PetalID::(k\w+)\s*,\s*(\d+)\s*,\s*(\d+)\s*,\s*"
        r"\{\s*"
        r"(?:\{?\s*PetalID::(k\w+)\s*,\s*(\d+)\s*\}?\s*,?\s*)?"
        r"(?:\{?\s*PetalID::(k\w+)\s*,\s*(\d+)\s*\}?\s*,?\s*)?"
        r"(?:\{?\s*PetalID::(k\w+)\s*,\s*(\d+)\s*\}?\s*,?\s*)?"
        r"(?:\{?\s*PetalID::(k\w+)\s*,\s*(\d+)\s*\}?\s*,?\s*)?"
    )
    # Simpler line-oriented parse of Def array
    recipes = []
    for m in re.finditer(
        r"\{\s*PetalID::(k\w+)\s*,\s*(\d+)\s*,\s*(\d+)\s*,\s*\{([^}]*(?:\{[^}]*\}[^}]*)*)\}\s*\}",
        RECIPE,
        re.S,
    ):
        res = m.group(1)
        tier = int(m.group(2))
        n_ing = int(m.group(3))
        body = m.group(4)
        ings = []
        for im in re.finditer(r"PetalID::(k\w+)\s*,\s*(\d+)", body):
            iid, cnt = im.group(1), int(im.group(2))
            if cnt > 0:
                ings.append((iid, cnt))
        recipes.append((res, tier, n_ing, ings))

    print(f"=== ASSEMBLE recipes={len(recipes)} ===")
    assemble_fail = []
    for res, tier, n_ing, ings in recipes:
        uses_coin = any(iid == "kCoin" for iid, _ in ings)
        rname = name_of.get(res)
        if not rname:
            assemble_fail.append((res, "result enum has no PETAL_DATA name"))
            continue
        if uses_coin:
            # Coin-shop: baked result only; Coin is currency (no ladder).
            continue
        for ri in range(9):
            if (rname, ri) not in enum_of_name_r:
                assemble_fail.append((res, rname, f"no result @{RARITY[ri]}"))
                continue
            for iid, cnt in ings:
                if iid == "kCoin":
                    continue
                iname = name_of.get(iid)
                if not iname:
                    assemble_fail.append((res, iid, "ingredient no name"))
                    break
                if (iname, ri) not in enum_of_name_r:
                    assemble_fail.append(
                        (res, rname, f"ing {iname} missing @{RARITY[ri]}")
                    )
                    break

    # Deduplicate fail messages
    fail_unique = sorted(set(str(x) for x in assemble_fail))
    print(f"assemble_resolve_problems={len(fail_unique)}")
    for x in fail_unique[:40]:
        print(f"  FAIL {x}")
    if len(fail_unique) > 40:
        print(f"  ... +{len(fail_unique) - 40} more")

    # UI / server wiring
    print("=== WIRING ===")
    checks = [
        ("UI recipe_resolves_at", "recipe_resolves_at" in PANELS),
        ("UI petal_at_rarity filter", "petal_at_rarity" in PANELS and "recipe_filter_rarity" in PANELS),
        ("UI craft_recipe(rarity)", "craft_recipe(recipe_index, recipe_filter_rarity)" in PANELS),
        ("Net writes rarity", "writer.write<uint8_t>(rarity)" in NET and "kCraftRecipe" in NET),
        ("Server reads rarity", "mode == CraftMode::kCraftRecipe" in CLIENT
         and "petal_at_rarity" in CLIENT),
        ("Craft same-name only", "find_same_name_at_rarity" in CRAFT),
    ]
    for label, ok in checks:
        print(f"  {'OK' if ok else 'MISSING'} {label}")

    # Coin must NOT have full ladder
    coin_rs = by_name.get("Coin", set())
    print(f"Coin rarities (want empty/excluded)={coin_rs}")

    # Summary verdict
    wiring_ok = all(ok for _, ok in checks)
    verdict = (
        len(incomplete) == 0
        and len(upgrade_gap) == 0
        and len(fail_unique) == 0
        and wiring_ok
    )
    print("=== VERDICT ===", "PASS" if verdict else "FAIL")
    return 0 if verdict else 1


if __name__ == "__main__":
    sys.exit(main())
