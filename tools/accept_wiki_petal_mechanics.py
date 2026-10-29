#!/usr/bin/env python3
"""Batch E: real server regressions + explicit, non-equivalent coverage levels.

Runs production combat code, never counts symbol references as behavior tests.
The registry smoke exercises all combat IDs but does not claim every wiki rule
is reproduced. Raw reference inventory is included so missing pages stay visible.
"""
from __future__ import annotations
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import sys
from datetime import datetime, timezone

import accept_wiki_strength_batches as strength

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT.parent / "temp/wiki_research/acceptance"
SCOPES = {
    "status_wire": ["health ratios", "fractional damage feedback", "serialization"],
    "hornet_precision": ["Hornet", "all rarity health subtraction", "fractional DOT"],
    "fang_poo": ["Fang", "Poo", "forced aggro"],
    "spawn_safety": ["every map and zone", "hub exclusion", "rarity caps"],
    "mimic": ["Mimic", "formation", "equipment", "missing rungs", "equipment edits"],
    "defense": ["Root", "Shovel", "Wax", "Nazar Amulet", "Sponge", "Bandage"],
    "combat_effects": ["Bur", "Glass", "Ankh", "Mana Orb", "Magic Eye"],
    "active_skills": ["Pollen Sandstorm", "World Tree Seed", "Chrono Vortex", "Electric Web",
                      "Sand Leaf Dart", "Pollen", "Grapes", "Peas", "Soldier Egg"],
}
RAW_ALIASES = {"Magic Leaf": "Leaf_Magic", "Magic Cactus": "Cactus_Magic",
               "Magic Cotton": "Cotton_Magic", "Magic Missile": "Missile_Magic",
               "Magic Bubble": "Bubble_Magic", "Champion's Crown": "Champion_s_Crown"}

def source_inventory():
    return [p for folder in ("Server", "Shared", "tools") for p in (ROOT / folder).rglob("*")
            if p.suffix in {".hh", ".cc", ".py", ".sh"}
            and not any(part.startswith("build") or part == "uWebSockets" for part in p.parts)]

def fingerprint(paths):
    digest = hashlib.sha256()
    for p in sorted(paths):
        digest.update(p.relative_to(ROOT).as_posix().encode())
        digest.update(p.read_bytes())
    return digest.hexdigest()

def main() -> int:
    OUT.mkdir(parents=True, exist_ok=True)
    before_source = fingerprint(source_inventory())
    script = ROOT / "tools/test_petal_mechanics.sh"
    if os.name == "nt":
        wsl_path = "/mnt/" + script.drive[0].lower() + script.as_posix()[2:]
        command = ["wsl", "-u", "root", "-e", "bash", wsl_path]
    else:
        command = ["bash", str(script)]
    run = subprocess.run(command, capture_output=True, text=True, encoding="utf-8", errors="replace")
    log = run.stdout + run.stderr
    (OUT / "batch_E_runtime.log").write_text(log, encoding="utf-8")
    print(log, end="", flush=True)
    results = {key: {"pass": bool(re.search(rf"^PASS {key}:", log, re.M)), "scope": scope}
               for key, scope in SCOPES.items()}
    source_files = source_inventory()
    after_source = fingerprint(source_files)
    code = {p.relative_to(ROOT).as_posix(): p.read_text(encoding="utf-8", errors="replace")
            for p in source_files if p.suffix in {".hh", ".cc"} and "Server" in p.parts}
    source = strength.STATIC.read_text(encoding="utf-8")
    families: dict[str, list[str]] = {}
    for b in strength.parse_petal_blocks(source):
        name = strength.get_str(b, "name")
        if name not in {None, "None", "Coin"}: families.setdefault(name, []).append(b)
    referenced = {}
    used = sorted(set(re.findall(r"\.(\w+)\s*=", "\n".join(strength.extract_attrs(b) for bs in families.values() for b in bs))))
    for field in used:
        hits = []
        for file, text in code.items():
            for line, content in enumerate(text.splitlines(), 1):
                if re.search(rf"\.{field}\b", content): hits.append(f"{file}:{line}")
        referenced[field] = {"level": "referenced" if hits else "not_referenced", "locations": hits[:8]}
    raw = ROOT.parent / "temp/wiki_research/raw/petals_en"
    inventory = []
    verified_names = {name for key, result in results.items() if result["pass"] for name in result["scope"]}
    for name, blocks in sorted(families.items()):
        path = raw / (RAW_ALIASES.get(name, name.replace(" ", "_")) + ".wikitext")
        inventory.append({"name": name, "rarities": len(blocks),
                          "coverage": "targeted_behavior_tests" if name in verified_names else "registry_dispatch_or_data_only",
                          "raw_reference": str(path) if path.is_file() else None,
                          "attributes": sorted(set(re.findall(r"\.(\w+)\s*=", strength.extract_attrs(blocks[0]))))})
    smoke = re.search(r"PASS registry_smoke: (\d+) combat", log)
    passed = run.returncode == 0 and all(r["pass"] for r in results.values()) and bool(smoke) and before_source == after_source
    report = {"generated_at": datetime.now(timezone.utc).isoformat(), "source_sha256": after_source,
              "source_changed_during_test": before_source != after_source,
              "pass": passed, "meaning": "Runtime regression gate; not a claim of full wiki equivalence.",
              "runtime_exit": run.returncode, "behavior_tests": results,
              "registry_smoke_ids": int(smoke[1]) if smoke else 0,
              "references_only": referenced, "families": inventory}
    (OUT / "batch_E_mechanics.json").write_text(json.dumps(report, ensure_ascii=False, indent=2), encoding="utf-8")
    lines = ["# 花瓣机制运行验收 E", "", f"回归闸门：{'PASS' if passed else 'FAIL'}", "",
             "覆盖层级分开记录：字段引用 ≠ 全表触发冒烟 ≠ 定向行为断言 ≠ 完整 Wiki 复刻。",
             f"全表触发冒烟：{report['registry_smoke_ids']} 个战斗花瓣 ID。", "",
             "| 测试组 | 结果 | 断言范围 |", "| --- | --- | --- |"]
    for key, r in results.items(): lines.append(f"| {key} | {'PASS' if r['pass'] else 'FAIL'} | {', '.join(r['scope'])} |")
    lines += ["", "全部家族及原文库存见 batch_E_mechanics.json；raw_reference=null 表示本地没有英文页，不能宣称已对照原文。",
              "", "运行命令：`python tools/accept_wiki_petal_mechanics.py`。"]
    (OUT / "MECHANICS.md").write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"Batch E: {'PASS' if passed else 'FAIL'}; wrote {OUT}")
    return 0 if passed else 1

if __name__ == "__main__":
    sys.exit(main())
