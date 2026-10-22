from pathlib import Path
import re

path = Path(r"d:\ashflor\ashflor\Client\I18nData.inc")
t = path.read_text(encoding="utf-8")
a = t.index("PETAL_NAME_ZH")
b = t.index("PETAL_DESC_ZH")
c = t.index("MOB_NAME_ZH")
name_body = t[a:b]
desc_body = t[b:c]
name_strs = re.findall(r'"(?:\\.|[^"\\])*"', name_body)
desc_strs = re.findall(r'"(?:\\.|[^"\\])*"', desc_body)
print("NAME", len(name_strs))
print("DESC", len(desc_strs))

hh = Path(r"d:\ashflor\ashflor\Shared\StaticDefinitions.hh").read_text(encoding="utf-8")
m = re.search(r"namespace PetalID.*?enum\s*:\s*T\s*\{(.*?)\bkNumPetals", hh, re.S)
ids = [l.split("//")[0].strip().rstrip(",") for l in m.group(1).splitlines()]
ids = [x for x in ids if x.startswith("k")]
print("enum", len(ids))

# Ensure closings have semicolons
# Rebuild arrays if counts wrong: keep first `need` strings only
need = len(ids)

def rebuild(marker, strs, following_marker):
    global t
    # find array
    m = re.search(
        rf"(static char const \*const {marker}\[PetalID::kNumPetals\] = \{{)\n",
        t,
    )
    if not m:
        raise SystemExit(f"no {marker}")
    start = m.end()
    # end at following marker or next static
    end = t.index(following_marker)
    # walk back to closing brace of this array
    close = t.rfind("}", start, end)
    kept = strs[:need]
    body = ",\n".join(f"    {s}" for s in kept) + "\n};\n"
    t = t[: m.start()] + m.group(1) + "\n" + body + t[end:]

if len(name_strs) != need or len(desc_strs) != need:
    print("trimming/rebuilding to", need)
    # Extract raw quoted tokens again from sections before rebuild
    rebuild("PETAL_NAME_ZH", name_strs, "static char const *const PETAL_DESC_ZH")
    # re-find desc after name rebuild
    b2 = t.index("PETAL_DESC_ZH")
    c2 = t.index("MOB_NAME_ZH")
    desc_strs2 = re.findall(r'"(?:\\.|[^"\\])*"', t[b2:c2])
    rebuild("PETAL_DESC_ZH", desc_strs2, "static char const *const MOB_NAME_ZH")
    path.write_text(t, encoding="utf-8")
    # verify
    t = path.read_text(encoding="utf-8")
    a = t.index("PETAL_NAME_ZH")
    b = t.index("PETAL_DESC_ZH")
    c = t.index("MOB_NAME_ZH")
    print("after NAME", len(re.findall(r'"(?:\\.|[^"\\])*"', t[a:b])))
    print("after DESC", len(re.findall(r'"(?:\\.|[^"\\])*"', t[b:c])))
else:
    print("counts already OK")
