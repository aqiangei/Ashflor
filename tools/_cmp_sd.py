from pathlib import Path
import re
from collections import defaultdict

R = ["Common", "Unusual", "Rare", "Epic", "Legendary", "Mythic", "Ultra", "Super", "Unique"]


def count_fam(path: str):
    t = Path(path).read_text(encoding="utf-8")
    s = t.index("PETAL_DATA")
    e = t.index("\n}};", s)
    body = t[s:e]
    entries, d, b = [], 0, None
    for j, ch in enumerate(body):
        if ch == "{":
            d += 1
            if d == 3 and b is None:
                b = j
        elif ch == "}":
            if d == 3 and b is not None:
                entries.append(body[b : j + 1])
                b = None
            d -= 1
    fam = defaultdict(set)
    for bl in entries:
        nm = re.search(r'\.name\s*=\s*"([^"]*)"', bl)
        rar = re.search(r"\.rarity\s*=\s*RarityID::k(\w+)", bl)
        if nm and rar and nm.group(1) not in ("None", "Coin"):
            if rar.group(1) in R:
                fam[nm.group(1)].add(R.index(rar.group(1)))
    full = sum(1 for v in fam.values() if len(v) == 9)
    return len(entries), len(fam), full, len(t)


for p in ["Shared/StaticData.cc", "Shared/StaticData.cc.bak-fixall"]:
    print(p, count_fam(p))
