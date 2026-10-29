#!/usr/bin/env python3
"""Map account l: petal IDs to name/rarity using local StaticDefinitions + StaticData."""
from pathlib import Path
import re
import subprocess
import os

root = Path(__file__).resolve().parents[1]
hh = (root / "Shared/StaticDefinitions.hh").read_text(encoding="utf-8", errors="replace")
m = re.search(r"namespace PetalID \{(.*?)kNumPetals", hh, re.S)
ids = []
for line in m.group(1).splitlines():
    line = line.strip()
    if line.startswith("k") and "," in line:
        ids.append(line.split(",")[0].strip())
print("kNumPetals", len(ids))

# rarity from PETAL_DATA order in StaticData.cc — entries in enum order
sd = (root / "Shared/StaticData.cc").read_text(encoding="utf-8", errors="replace")
# Find PETAL_DATA array body — each {.name=... .rarity=RarityID::kX}
# Simpler: count rarity assignments in order of petal structs after PETAL_DATA[
rarities = []
# Match blocks that look like petal defs with rarity
for rm in re.finditer(r"\.rarity = RarityID::(k\w+)", sd):
    rarities.append(rm.group(1))
# PETAL_DATA has one rarity per petal; also MOB_DATA etc. — take first kNumPetals after marker
idx = sd.find("PETAL_DATA[")
if idx < 0:
    idx = sd.find("PetalData const PETAL_DATA")
chunk = sd[idx:idx + 5000000] if idx >= 0 else sd
petal_rar = []
for rm in re.finditer(r"\.rarity = RarityID::(k\w+)", chunk):
    petal_rar.append(rm.group(1))
    if len(petal_rar) >= len(ids):
        break
print("petal rarity entries parsed", len(petal_rar))

pem = os.path.expandvars(r"%USERPROFILE%\.ssh\ASHFLOR.pem")
out = subprocess.check_output([
    "ssh", "-i", pem, "-o", "StrictHostKeyChecking=no",
    "root@154.8.157.208",
    "grep -E ' l:' /opt/ashflor/accounts.dat | head -20"
], text=True, errors="replace")

non_unique = 0
unique = 0
oob = 0
for ln in out.splitlines():
    m = re.search(r" l:([0-9=,]+)", ln)
    if not m:
        continue
    user = ln.split()[0] if ln.split() else "?"
    pairs = m.group(1).split(",")
    print("---", user, "n=", len(pairs))
    for p in pairs[:12]:
        if "=" not in p:
            continue
        a, b = p.split("=", 1)
        try:
            i, lv = int(a), int(b)
        except ValueError:
            continue
        if i >= len(ids):
            print(f"  id={i} lv={lv} OOB")
            oob += 1
            continue
        name = ids[i]
        rar = petal_rar[i] if i < len(petal_rar) else "?"
        mark = "OK" if rar == "kUnique" else "NOT_UNIQUE"
        if rar == "kUnique":
            unique += 1
        else:
            non_unique += 1
        print(f"  id={i} {name} rar={rar} lv={lv} [{mark}]")
print("summary unique_pairs", unique, "non_unique", non_unique, "oob", oob)
