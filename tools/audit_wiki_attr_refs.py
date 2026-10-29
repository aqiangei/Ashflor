#!/usr/bin/env python3
"""Audit: every // wiki import PetalAttributes field that StaticData sets nonzero
must be referenced at least once under Server/."""
from __future__ import annotations

import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
DEFS = ROOT / "Shared" / "StaticDefinitions.hh"
STATIC = ROOT / "Shared" / "StaticData.cc"
SERVER = ROOT / "Server"


def wiki_fields() -> list[str]:
    text = DEFS.read_text(encoding="utf-8", errors="replace")
    fields = []
    for m in re.finditer(
        r"(?:float|uint8_t|int)\s+(\w+)\s*=\s*[^;]*;\s*//\s*wiki import",
        text,
    ):
        fields.append(m.group(1))
    return fields


def field_used_in_static(field: str) -> bool:
    # Nonzero / true assignment in designated initializers
    pat = re.compile(rf"\.{re.escape(field)}\s*=")
    return bool(pat.search(STATIC.read_text(encoding="utf-8", errors="replace")))


def server_blob() -> str:
    parts = []
    for p in SERVER.rglob("*"):
        if p.suffix.lower() in {".cc", ".hh", ".h", ".cpp", ".hpp"}:
            parts.append(p.read_text(encoding="utf-8", errors="replace"))
    return "\n".join(parts)


def main() -> int:
    blob = server_blob()
    missing = []
    skipped = []
    ok = []
    for f in wiki_fields():
        if not field_used_in_static(f):
            skipped.append(f)
            continue
        # Accept attrs.X, A.X, B.X, .X, or bare identifier in Server
        if re.search(rf"(?:attrs|A|B|Pd|D|L)\.{re.escape(f)}\b|\.{re.escape(f)}\s*[=,}}>]|\b{re.escape(f)}\b", blob):
            ok.append(f)
        else:
            missing.append(f)
    print(f"wiki import fields: {len(ok) + len(missing) + len(skipped)}")
    print(f"  referenced in Server: {len(ok)}")
    print(f"  StaticData unused (skip): {len(skipped)}")
    print(f"  MISSING Server refs: {len(missing)}")
    for f in missing:
        print(f"    - {f}")
    return 1 if missing else 0


if __name__ == "__main__":
    sys.exit(main())
