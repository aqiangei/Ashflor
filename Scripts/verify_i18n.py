# -*- coding: utf-8 -*-
from pathlib import Path
import re
root = Path(__file__).resolve().parents[1]
inc = (root / "Client/I18nData.inc").read_text(encoding="utf-8")
hh = (root / "Client/I18n.hh").read_text(encoding="utf-8")
cc = (root / "Client/I18n.cc").read_text(encoding="utf-8")
t = (root / "Client/Ui/InGame/Tooltip.cc").read_text(encoding="utf-8")

def count_strings(block):
    return len(re.findall(r'^\s+"', block, re.M))

petal_names = count_strings(inc.split("PETAL_NAME_ZH")[1].split("};")[0])
mob_names = count_strings(inc.split("MOB_NAME_ZH")[1].split("};")[0])
enums = re.findall(r"\bk[A-Za-z0-9]+\b", hh.split("enum Str")[1].split("kCount")[0])
en = count_strings(cc.split("EN[I18n::kCount]")[1].split("};")[0])
zh = count_strings(cc.split("ZH[I18n::kCount]")[1].split("};")[0])
print("petal_names", petal_names)
print("mob_names", mob_names)
print("enum", len(enums), "EN", en, "ZH", zh)
print("tooltip I18n", "I18n::" in t, "Health left", '"Health:"' in t)
print("StaticData Basic", '"Basic"' in (root / "Shared/StaticData.cc").read_text(encoding="utf-8"))
