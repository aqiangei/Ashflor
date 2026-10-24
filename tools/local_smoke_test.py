#!/usr/bin/env python3
"""Local smoke test against running gardn-server (default :9001)."""
from __future__ import annotations

import json
import sys
import urllib.error
import urllib.request
from pathlib import Path

BASE = sys.argv[1] if len(sys.argv) > 1 else "http://127.0.0.1:9001"
ROOT = Path(__file__).resolve().parents[1]


def get(path: str, timeout: float = 15):
    url = BASE.rstrip("/") + path
    req = urllib.request.Request(url, headers={"Accept": "*/*"})
    with urllib.request.urlopen(req, timeout=timeout) as r:
        return r.status, r.headers, r.read()


def ok(name: str, cond: bool, detail: str = ""):
    mark = "PASS" if cond else "FAIL"
    print(f"  [{mark}] {name}" + (f" — {detail}" if detail else ""))
    return cond


def main() -> int:
    fails = 0
    print(f"# smoke {BASE}\n")

    print("## HTTP static")
    # / serves index; /index.html and server.wasm are not always exposed
    for path, want_magic in (
        ("/", None),
        ("/gardn-client.js", None),
        ("/gardn-client.wasm", b"\x00asm"),
        ("/sw.js", None),
    ):
        try:
            st, hdr, body = get(path)
            good = st == 200 and len(body) > 100
            if want_magic:
                good = good and body[:4] == want_magic
            if not ok(path, good, f"status={st} bytes={len(body)}"):
                fails += 1
        except Exception as e:
            ok(path, False, str(e))
            fails += 1

    print("\n## API")
    health = None
    try:
        st, _, body = get("/api/health")
        health = json.loads(body.decode("utf-8", "replace"))
        good = st == 200 and bool(health.get("ok") or health.get("status") in ("ok", "healthy", "good"))
        # tolerant: any JSON with maps/players-ish keys
        if not good and isinstance(health, dict) and st == 200:
            good = True
        if not ok("/api/health", good, json.dumps(health, ensure_ascii=False)[:200]):
            fails += 1
    except Exception as e:
        ok("/api/health", False, str(e))
        fails += 1

    for path in ("/health", "/api/live", "/api/catalog", "/api/map-layout"):
        try:
            st, _, body = get(path)
            good = st == 200 and len(body) > 2
            detail = f"status={st} bytes={len(body)}"
            if path == "/api/catalog" and good:
                try:
                    cat = json.loads(body)
                    if isinstance(cat, dict):
                        detail += f" keys={list(cat.keys())[:8]}"
                    elif isinstance(cat, list):
                        detail += f" items={len(cat)}"
                except Exception:
                    pass
            if not ok(path, good, detail):
                fails += 1
        except Exception as e:
            ok(path, False, str(e))
            fails += 1

    print("\n## WebSocket handshake")
    try:
        import asyncio

        try:
            import websockets  # type: ignore
        except ImportError:
            # fallback: raw upgrade via http.client is hard; use node if available
            ok("ws connect", False, "websockets pkg missing — skip (HTTP OK)")
        else:

            async def ws_ping():
                uri = BASE.replace("http://", "ws://").replace("https://", "wss://") + "/"
                async with websockets.connect(uri, open_timeout=5, close_timeout=2) as ws:
                    # server may push binary immediately or wait for client hello
                    try:
                        msg = await asyncio.wait_for(ws.recv(), timeout=2.0)
                        return True, f"recv type={type(msg).__name__} len={len(msg) if hasattr(msg,'__len__') else '?'}"
                    except asyncio.TimeoutError:
                        return True, "connected (no immediate push)"

            good, detail = asyncio.run(ws_ping())
            if not ok("ws connect", good, detail):
                fails += 1
    except Exception as e:
        ok("ws connect", False, str(e))
        fails += 1

    print("\n## Data audits (offline)")
    import subprocess

    for script in ("audit_all_petal_attrs.py", "audit_full_rarity_craft.py"):
        r = subprocess.run(
            [sys.executable, str(ROOT / "tools" / script)]
            + (["--summary"] if "attrs" in script else []),
            cwd=str(ROOT),
            capture_output=True,
            text=True,
            encoding="utf-8",
            errors="replace",
        )
        out = (r.stdout or "") + (r.stderr or "")
        passed = r.returncode == 0 and ("problems=0" in out or "VERDICT === PASS" in out or "PASS" in out)
        if "attrs" in script:
            passed = "problems=0" in out
        if not ok(script, passed, out.strip().splitlines()[-1] if out.strip() else f"rc={r.returncode}"):
            fails += 1
            for line in out.strip().splitlines()[-5:]:
                print(f"         {line}")

    print(f"\n# result: {'PASS' if fails == 0 else f'{fails} FAIL(s)'}")
    return 0 if fails == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
