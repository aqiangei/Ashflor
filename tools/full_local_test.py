#!/usr/bin/env python3
"""Full local verification suite (data audits + live :9001 smoke)."""
from __future__ import annotations

import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def run(label: str, args: list[str]) -> int:
    print(f"\n{'=' * 60}\n# {label}\n{'=' * 60}")
    r = subprocess.run(
        [sys.executable, *args],
        cwd=str(ROOT),
        encoding="utf-8",
        errors="replace",
    )
    print(f"# {label} exit={r.returncode}")
    return r.returncode


def main() -> int:
    fails = 0
    steps = [
        ("verify_seven_petals", ["tools/verify_seven_petals.py"]),
        ("spotcheck_final", ["tools/spotcheck_final.py"]),
        ("audit_all_petal_attrs --summary", ["tools/audit_all_petal_attrs.py", "--summary"]),
        ("audit_full_rarity_craft", ["tools/audit_full_rarity_craft.py"]),
        ("verify_i18n", ["Scripts/verify_i18n.py"]),
        ("verify_petal_counts", ["tools/verify_petal_counts.py"]),
        ("local_smoke_test :9001", ["tools/local_smoke_test.py", "http://127.0.0.1:9001"]),
    ]
    for label, args in steps:
        if run(label, args) != 0:
            fails += 1

    print(f"\n{'=' * 60}")
    print(f"# FULL RESULT: {'PASS' if fails == 0 else f'{fails}/{len(steps)} step(s) FAILED'}")
    return 0 if fails == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
