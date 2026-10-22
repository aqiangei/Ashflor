from pathlib import Path
import re

hh = Path(r"d:\ashflor\ashflor\Shared\StaticDefinitions.hh").read_text(encoding="utf-8")
m = re.search(r"namespace PetalID.*?enum\s*:\s*T\s*\{(.*?)\bkNumPetals", hh, re.S)
ids = [l.split("//")[0].strip().rstrip(",") for l in m.group(1).splitlines()]
ids = [x for x in ids if x.startswith("k")]
print("enum", len(ids))

cc = Path(r"d:\ashflor\ashflor\Shared\StaticData.cc").read_text(encoding="utf-8")
start = cc.index("PETAL_DATA")
end = cc.index("MOB_DATA")
print("names", len(re.findall(r"\.name\s*=", cc[start:end])))

i18n = Path(r"d:\ashflor\ashflor\Client\I18nData.inc").read_text(encoding="utf-8")
for marker in ["PETAL_NAME_ZH", "PETAL_DESC_ZH"]:
    m = re.search(rf"{marker}\[PetalID::kNumPetals\] = \{{(.*?)\n\}};", i18n, re.S)
    n = len(re.findall(r'"(?:\\.|[^"\\])*"', m.group(1)))
    print(marker, n)

# Spot-check: Peas should have Legendary+ after fill
peas = [i for i, e in enumerate(ids) if "Peas" in e]
print("Peas enum ids", [ids[i] for i in peas[:20]], "count", len(peas))
