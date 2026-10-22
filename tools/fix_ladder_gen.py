"""Fix broken attrs closing from gen_petal_ladders.py and verify i18n counts."""
from pathlib import Path
import re

ROOT = Path(r"d:\ashflor\ashflor")
cc_path = ROOT / "Shared" / "StaticData.cc"
cc = cc_path.read_text(encoding="utf-8")

marker = "// ---- Auto-filled rarity ladders (gen_petal_ladders.py) ----"
idx = cc.find(marker)
if idx < 0:
    raise SystemExit("marker missing")
# Only fix inside auto-filled section through end of PETAL_DATA
end = cc.find("\n}};", idx)
section = cc[idx:end]
# Broken pattern: attributes close + extra petal close before comma
fixed = re.sub(r"(\n        \})\n    \}\n    \},", r"\1\n    },", section)
fixed = re.sub(r"(\n        \})\n    \}\n\}\};", r"\1\n    }\n}};", fixed)
# Also last entry may be `}\n    }\n}};` without comma
fixed2 = re.sub(r"(\n        \})\n    \}$", r"\1\n    }", fixed)
print("section fixes", section.count("}\n    }\n    },"), "->", fixed2.count("}\n    }\n    },"))
cc2 = cc[:idx] + fixed2 + cc[end:]
# Verify no remaining broken pattern in section
sec2 = cc2[idx:cc2.find("\n}};", idx)]
bad = len(re.findall(r"\}\n    \}\n    \},", sec2))
print("remaining bad", bad)
cc_path.write_text(cc2, encoding="utf-8")

# Count petals in data
start = cc2.index("PETAL_DATA")
endm = cc2.index("MOB_DATA")
names = len(re.findall(r"\.name\s*=", cc2[start:endm]))
print("PETAL_DATA names", names)

i18n = (ROOT / "Client" / "I18nData.inc").read_text(encoding="utf-8")
for marker in ["PETAL_NAME_ZH", "PETAL_DESC_ZH"]:
    m = re.search(rf"{marker}\[PetalID::kNumPetals\] = \{{(.*?)\n\}};", i18n, re.S)
    n = len(re.findall(r'"(?:\\.|[^"\\])*"', m.group(1)))
    print(marker, n)

hh = (ROOT / "Shared" / "StaticDefinitions.hh").read_text(encoding="utf-8")
m = re.search(r"namespace PetalID.*?enum\s*:\s*T\s*\{(.*?)\bkNumPetals", hh, re.S)
ids = [l.split("//")[0].strip().rstrip(",") for l in m.group(1).splitlines()]
ids = [x for x in ids if x.startswith("k")]
print("enum", len(ids))
