from pathlib import Path
import re

i18n = Path(r"d:\ashflor\ashflor\Client\I18nData.inc").read_text(encoding="utf-8")
hh = Path(r"d:\ashflor\ashflor\Shared\StaticDefinitions.hh").read_text(encoding="utf-8")
m = re.search(r"namespace PetalID.*?enum\s*:\s*T\s*\{(.*?)\bkNumPetals", hh, re.S)
ids = [l.split("//")[0].strip().rstrip(",") for l in m.group(1).splitlines()]
ids = [x for x in ids if x.startswith("k")]
need = len(ids)
print("need", need)

for marker in ["PETAL_NAME_ZH", "PETAL_DESC_ZH"]:
    m = re.search(rf"{marker}\[PetalID::kNumPetals\] = \{{(.*?)\n\}};", i18n, re.S)
    body = m.group(1)
    strs = re.findall(r'"(?:\\.|[^"\\])*"', body)
    print(marker, len(strs), "delta", len(strs) - need)
    # show last 5
    print("  last:", [s[1:-1] for s in strs[-5:]])
    print("  first new-ish around 330:", [s[1:-1] for s in strs[328:335]])
