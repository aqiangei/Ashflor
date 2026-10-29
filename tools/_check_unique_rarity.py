from pathlib import Path
import re

root = Path(__file__).resolve().parents[1]
hh = (root / "Shared/StaticDefinitions.hh").read_text(encoding="utf-8")
m = re.search(r"namespace PetalID \{(.*?)kNumPetals", hh, re.S)
ids = []
for line in m.group(1).splitlines():
    t = line.strip()
    if t.startswith("k") and "," in t:
        ids.append(t.split(",")[0].strip())

sd = (root / "Shared/StaticData.cc").read_text(encoding="utf-8")
# PETAL_DATA is a C array of structs in declaration order matching enum.
# Find start of array initializer after PetalData ... PETAL_DATA
start = sd.find("PETAL_DATA[]")
if start < 0:
    start = sd.find("PETAL_DATA[PetalID::kNumPetals]")
if start < 0:
    # fallback: first .name = after "PetalData"
    start = sd.find("PetalData")
print("start marker", start)
body = sd[start:]
# Collect successive rarity fields until we have len(ids)
rars = []
for rm in re.finditer(r"\.rarity = RarityID::(k\w+)", body):
    rars.append(rm.group(1))
    if len(rars) >= len(ids):
        break
print("parsed rarities", len(rars), "enums", len(ids))
for i in [40, 249, 255, 261, 329, 57]:
    if i < len(ids) and i < len(rars):
        print(i, ids[i], rars[i])
    else:
        print(i, "missing")

# Count how many *Unique enum with non-kUnique rarity (data bug)
bad = 0
ok = 0
for i, name in enumerate(ids):
    if not name.endswith("Unique"):
        continue
    if i >= len(rars):
        bad += 1
        continue
    if rars[i] == "kUnique":
        ok += 1
    else:
        bad += 1
        if bad <= 15:
            print("MISMATCH", i, name, rars[i])
print("Unique-named ok", ok, "mismatch", bad)
