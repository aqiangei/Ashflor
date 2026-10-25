#!/usr/bin/env python3
"""High-water merge Unique levels (l:…) from a good accounts backup into current.

Usage (on the game host, server stopped or right before restart):
  python3 restore_unique_levels.py \\
    --good /opt/ashflor/backups/accounts-20261024T095557Z-pre-deploy.dat \\
    --current /opt/ashflor/accounts.dat \\
    [--dry-run]
"""
from __future__ import annotations

import argparse
import shutil
import time
from pathlib import Path


def parse_levels(line: str) -> dict[int, int]:
    out: dict[int, int] = {}
    for tok in line.split():
        if not tok.startswith("l:"):
            continue
        body = tok[2:]
        if not body or body == "-":
            continue
        for pair in body.split(","):
            if "=" not in pair:
                continue
            a, b = pair.split("=", 1)
            try:
                i, lv = int(a), int(b)
            except ValueError:
                continue
            if i > 0 and lv > 0:
                out[i] = max(out.get(i, 0), lv)
    return out


def strip_l(tokens: list[str]) -> list[str]:
    return [t for t in tokens if not t.startswith("l:")]


def format_l(levels: dict[int, int]) -> str | None:
    if not levels:
        return None
    parts = [f"{i}={levels[i]}" for i in sorted(levels)]
    return "l:" + ",".join(parts)


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--good", required=True, type=Path)
    ap.add_argument("--current", required=True, type=Path)
    ap.add_argument("--dry-run", action="store_true")
    args = ap.parse_args()

    good_map: dict[str, dict[int, int]] = {}
    for line in args.good.read_text(encoding="utf-8", errors="replace").splitlines():
        if not line.strip():
            continue
        user = line.split()[0]
        good_map[user] = parse_levels(line)

    lines_out: list[str] = []
    restored = 0
    bumped = 0
    for line in args.current.read_text(encoding="utf-8", errors="replace").splitlines():
        if not line.strip():
            lines_out.append(line)
            continue
        toks = line.split()
        user = toks[0]
        cur = parse_levels(line)
        src = good_map.get(user, {})
        merged = dict(cur)
        changed = False
        for i, lv in src.items():
            if lv > merged.get(i, 0):
                merged[i] = lv
                changed = True
                bumped += 1
        if not changed:
            lines_out.append(line)
            continue
        restored += 1
        base = strip_l(toks)
        lit = format_l(merged)
        if lit:
            base.append(lit)
        lines_out.append(" ".join(base))
        print(f"restore {user}: +from_backup ids={[i for i in sorted(src) if src[i] > cur.get(i, 0)]}")

    print(f"accounts_updated={restored} level_entries_bumped={bumped}")
    if args.dry_run:
        print("dry-run: no write")
        return
    bak = args.current.with_suffix(
        args.current.suffix + f".pre-lv-restore-{time.strftime('%Y%m%dT%H%M%SZ')}"
    )
    shutil.copy2(args.current, bak)
    args.current.write_text("\n".join(lines_out) + ("\n" if lines_out else ""), encoding="utf-8")
    print(f"wrote {args.current} (backup {bak})")


if __name__ == "__main__":
    main()
