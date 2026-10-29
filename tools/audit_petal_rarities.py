"""Audit which petal families are missing rarity rungs (excl. Coin)."""
from __future__ import annotations
from pathlib import Path
import re

ROOT = Path(r"d:\ashflor\ashflor")
RARITY = ["Common", "Unusual", "Rare", "Epic", "Legendary", "Mythic", "Ultra", "Super", "Unique"]

def parse_enum():
    hh = (ROOT / "Shared" / "StaticDefinitions.hh").read_text(encoding="utf-8")
    m = re.search(r"namespace PetalID.*?enum\s*:\s*T\s*\{(.*?)\bkNumPetals", hh, re.S)
    ids = []
    for line in m.group(1).splitlines():
        line = line.split("//")[0].strip().rstrip(",")
        if line.startswith("k"):
            ids.append(line)
    return ids

def parse_petals():
    cc = (ROOT / "Shared" / "StaticData.cc").read_text(encoding="utf-8")
    start = cc.index("PETAL_DATA = {{")
    end = cc.index("\n}};", start)
    body = cc[start:end]
    entries = []
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
    out = []
    for idx, block in enumerate(entries):
        nm = re.search(r'\.name\s*=\s*"([^"]*)"', block)
        rar = re.search(r"\.rarity\s*=\s*RarityID::k(\w+)", block)
        if not nm or not rar:
            continue
        try:
            ri = RARITY.index(rar.group(1))
        except ValueError:
            continue
        out.append({"idx": idx, "name": nm.group(1), "rarity": ri})
    return out

def main():
    ids = parse_enum()
    petals = parse_petals()
    n = min(len(ids), len(petals))
    print(f"enum={len(ids)} petals={len(petals)}")

    # Materials: kMat* enum ids
    mat_idx = {i for i, e in enumerate(ids[:n]) if e.startswith("kMat")}
    skip = {"None", "Coin"}

    families = {}
    for i in range(n):
        p = petals[i]
        if p["name"] in skip:
            continue
        is_mat = i in mat_idx
        fam = families.setdefault(p["name"], {"rarities": set(), "mat": is_mat, "enums": []})
        fam["rarities"].add(p["rarity"])
        fam["enums"].append(ids[i])
        fam["mat"] = fam["mat"] or is_mat

    incomplete = []
    complete = 0
    mat_complete = 0
    mat_incomplete = []
    for name, info in sorted(families.items()):
        rs = info["rarities"]
        missing = [RARITY[r] for r in range(9) if r not in rs]
        # User wants ALL rarities Common..Unique for every petal except Coin.
        # Materials also should have all rarities (as craft mats, no special stats).
        if missing:
            incomplete.append((name, missing, info["mat"], min(rs) if rs else -1, max(rs) if rs else -1))
            if info["mat"]:
                mat_incomplete.append((name, missing))
        else:
            complete += 1
            if info["mat"]:
                mat_complete += 1

    print(f"families={len(families)} complete={complete} incomplete={len(incomplete)}")
    print(f"materials: complete={mat_complete} incomplete={len(mat_incomplete)}")
    print("\n--- incomplete (first 40) ---")
    for name, missing, is_mat, rmin, rmax in incomplete[:40]:
        tag = "MAT" if is_mat else "PET"
        print(f"  [{tag}] {name}: have {RARITY[rmin]}..{RARITY[rmax]} missing {missing}")
    if len(incomplete) > 40:
        print(f"  ... +{len(incomplete)-40} more")
    print("\n--- material incomplete ---")
    for name, missing in mat_incomplete:
        print(f"  {name}: missing {missing}")

if __name__ == "__main__":
    main()
