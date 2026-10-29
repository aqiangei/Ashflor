#!/usr/bin/env python3
"""Audit wiki petal art cases in Client/Assets/Petal.cc.

1) Presence: every EN name in todo/wiki-petal-gap.md must have a case.
2) Crude heuristic: case body with few draw calls that are only
   arc / round_rect / ellipse → flagged as placeholder-quality.
"""
from __future__ import annotations

import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
GAP = ROOT.parent / "todo" / "wiki-petal-gap.md"
PETAL_CC = ROOT / "Client" / "Assets" / "Petal.cc"

ALIASES = {
    "Talisman of Evasion": ["kTalismanOfEvasion", "kTalisman"],
    "Nazar Amulet": ["kNazarAmulet", "kNazar"],
    "Mysterious Powder": ["kMysteriousPowder", "kPowder"],
    "Mysterious Relic": ["kMysteriousRelic", "kRelic"],
    "Pharaoh's Crown": ["kPharaohCrown", "kPharaohsCrown", "kPharaoh"],
    "Champion's Crown": ["kChampionCrown", "kChampionsCrown", "kChampion"],
    "Paranormal Goggles": ["kParanormalGoggles", "kGoggles", "kParanormal"],
    "Soul Splitter": ["kSoulSplitter"],
    "Dark Mark": ["kDarkMark"],
    "Blood Sacrifice": ["kBloodSacrifice"],
    "Magic Leaf": ["kMagicLeaf"],
    "Magic Cactus": ["kMagicCactus"],
    "Magic Cotton": ["kMagicCotton"],
    "Magic Eye": ["kMagicEye"],
    "Magic Stinger": ["kMagicStinger"],
    "Magic Missile": ["kMagicMissile"],
    "Magic Stick": ["kMagicStick"],
    "Magic Bubble": ["kMagicBubble"],
    "Mana Orb": ["kManaOrb"],
    "Privet Berry": ["kPrivetBerry"],
    "Light Bulb": ["kLightBulb"],
    "Dead Leaf": ["kDeadLeaf"],
    "Electric Web": ["kElectricWeb"],
    "Poker Chip": ["kPokerChip"],
}

DRAW_RE = re.compile(
    r"ctx\.(?:begin_path|move_to|line_to|qcurve_to|bcurve_to|arc|ellipse|"
    r"round_rect|partial_arc|close_path|fill|stroke)\s*\("
)
SIMPLE_ONLY = re.compile(
    r"ctx\.(?:arc|ellipse|round_rect)\s*\("
)
RICH = re.compile(
    r"ctx\.(?:move_to|line_to|qcurve_to|bcurve_to|partial_arc)\s*\("
)


def en_names_from_gap() -> list[str]:
    if not GAP.is_file():
        return []
    text = GAP.read_text(encoding="utf-8", errors="replace")
    names = []
    for line in text.splitlines():
        if not line.startswith("|"):
            continue
        cols = [c.strip() for c in line.strip("|").split("|")]
        if len(cols) < 3:
            continue
        if cols[0] in {"Phase", "---"} or cols[0].startswith("-"):
            continue
        if not cols[0].isdigit():
            continue
        names.append(cols[1])
    return names


def name_to_candidates(name: str) -> list[str]:
    if name in ALIASES:
        return ALIASES[name]
    parts = re.findall(r"[A-Za-z0-9]+", name)
    pascal = "".join(p[:1].upper() + p[1:] for p in parts)
    return [f"k{pascal}"]


def split_cases(cc: str) -> dict[str, str]:
    """Map PetalID enum suffix → case body until next case/default/}."""
    out: dict[str, str] = {}
    # Find each case PetalID::kFoo: and capture until next case/default at same switch depth
    for m in re.finditer(r"case\s+PetalID::(k\w+)\s*:", cc):
        start = m.end()
        # Look ahead for next case PetalID or default:
        nxt = re.search(r"\n\s*(?:case\s+PetalID::|default\s*:)", cc[start:])
        end = start + nxt.start() if nxt else min(start + 2500, len(cc))
        out[m.group(1)] = cc[start:end]
    return out


def crude_score(body: str) -> tuple[bool, int, int]:
    draws = DRAW_RE.findall(body)
    simple = SIMPLE_ONLY.findall(body)
    rich = RICH.findall(body)
    n = len(draws)
    # Placeholder: ≤6 draw ops and no rich path commands, or ≤3 draws total
    is_crude = (n <= 3) or (n <= 8 and len(rich) == 0 and len(simple) >= 1)
    return is_crude, n, len(rich)


def main() -> int:
    cc = PETAL_CC.read_text(encoding="utf-8", errors="replace")
    cases = set(re.findall(r"case\s+PetalID::(k\w+)\s*:", cc))
    bodies = split_cases(cc)

    missing = []
    ok = []
    for name in en_names_from_gap():
        cands = name_to_candidates(name)
        if any(c in cases for c in cands):
            ok.append(name)
        else:
            missing.append((name, cands))

    # Focus crude scan on wiki-import style IDs + ALIASES targets
    focus = set()
    for name, cands in [(n, name_to_candidates(n)) for n in en_names_from_gap()] or []:
        focus.update(cands)
    for cands in ALIASES.values():
        focus.update(cands)
    # Also scan any kMagic* / recent stubs present
    focus.update(c for c in cases if c.startswith("kMagic") or c in {
        "kManaOrb", "kTalisman", "kNazar", "kCompass", "kMimic", "kPowder",
        "kRelic", "kDarkMark", "kSoulSplitter", "kGoggles", "kRubber",
        "kCotton", "kAir", "kDust", "kSoil", "kBean", "kClay",
    })

    crude = []
    for cid in sorted(focus):
        body = bodies.get(cid)
        if not body:
            continue
        bad, n, rich = crude_score(body)
        if bad:
            crude.append((cid, n, rich))

    print(f"gap petals: {len(ok) + len(missing)}")
    print(f"  have Petal.cc case: {len(ok)}")
    print(f"  MISSING art case: {len(missing)}")
    for name, cands in missing:
        print(f"    - {name} (tried {', '.join(cands)})")
    print(f"crude heuristic hits: {len(crude)}")
    for cid, n, rich in crude:
        print(f"    - {cid} draws≈{n} rich_paths={rich}")
    return 1 if missing or crude else 0


if __name__ == "__main__":
    sys.exit(main())
