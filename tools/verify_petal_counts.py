from pathlib import Path
import re
ROOT = Path(__file__).resolve().parents[1]
hh = (ROOT/"Shared"/"StaticDefinitions.hh").read_text(encoding="utf-8")
m = re.search(r"namespace PetalID.*?enum\s*:\s*T\s*\{(.*?)\bkNumPetals", hh, re.S)
ids = [l.split("//")[0].strip().rstrip(",") for l in m.group(1).splitlines() if l.split("//")[0].strip().startswith("k")]
print("enum", len(ids))
cc = (ROOT/"Shared"/"StaticData.cc").read_text(encoding="utf-8")
a, b = cc.index("PETAL_DATA"), cc.index("MOB_DATA")
print("names", len(re.findall(r"\.name\s*=", cc[a:b])))
# brace sanity in last appended section
sec = cc[cc.rfind("Full rarity ladders"):b]
print("bad double close", len(re.findall(r"\}\n    \}\n    \},", sec)))
i18n = (ROOT/"Client"/"I18nData.inc").read_text(encoding="utf-8")
na, nb = i18n.index("PETAL_NAME_ZH"), i18n.index("PETAL_DESC_ZH")
nc = i18n.index("MOB_NAME_ZH")
print("NAME", len(re.findall(r'"(?:\\.|[^"\\])*"', i18n[na:nb])))
print("DESC", len(re.findall(r'"(?:\\.|[^"\\])*"', i18n[nb:nc])))
# sample Leaf Scrap rarities
print("Leaf Scrap blocks", cc[a:b].count('.name = "Leaf Scrap"'))
