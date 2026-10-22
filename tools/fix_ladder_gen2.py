from pathlib import Path
import re

cc_path = Path(r"d:\ashflor\ashflor\Shared\StaticData.cc")
cc = cc_path.read_text(encoding="utf-8")
marker = "// ---- Auto-filled rarity ladders (gen_petal_ladders.py) ----"
idx = cc.find(marker)
end = cc.find("\n}};", idx)
section = cc[idx:end]

# Case A: multi-line attrs already fixed mostly.
# Case B: single-line attrs / empty attrs still have extra `    }`
before = len(re.findall(r"\}\n    \}\n    \},", section))
section2 = re.sub(
    r"(\.attributes = \{[^\n]*\})\n    \}\n    \},",
    r"\1\n    },",
    section,
)
# Case C: multi-line attrs that still have extra close
section2 = re.sub(
    r"(\n        \})\n    \}\n    \},",
    r"\1\n    },",
    section2,
)
# Last entry before `}};`
tail = cc[end:]
section2 = re.sub(
    r"(\.attributes = \{[^\n]*\})\n    \}$",
    r"\1\n    }",
    section2,
)
section2 = re.sub(
    r"(\n        \})\n    \}$",
    r"\1\n    }",
    section2,
)
after = len(re.findall(r"\}\n    \}\n    \},", section2))
print("bad", before, "->", after)
cc_path.write_text(cc[:idx] + section2 + tail, encoding="utf-8")

# Spot-check empty attrs entries
m = re.search(r"\.attributes = \{\}\n    \},", section2)
print("empty attrs ok", bool(m))
m2 = re.search(r"\.attributes = \{\}\n    \}\n    \},", section2)
print("empty attrs still bad", bool(m2))
