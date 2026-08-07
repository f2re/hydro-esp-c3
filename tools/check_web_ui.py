#!/usr/bin/env python3
from __future__ import annotations

import re
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
UI = ROOT / "src/web_ui_v2.h"


def fail(message: str) -> None:
    print(f"web-ui check: {message}", file=sys.stderr)
    raise SystemExit(1)


def main() -> None:
    text = UI.read_text(encoding="utf-8")
    match = re.search(r'R"rawliteral\((.*)\)rawliteral";', text, flags=re.S)
    if not match:
        fail("embedded raw HTML literal not found")
    html = match.group(1)

    for forbidden in ("alert(", "confirm("):
        if forbidden in html:
            fail(f"native blocking dialog found: {forbidden}")

    ids = re.findall(r'\bid="([^"]+)"', html)
    duplicate_ids = sorted({x for x in ids if ids.count(x) > 1})
    if duplicate_ids:
        fail("duplicate element ids: " + ", ".join(duplicate_ids))

    scripts = re.findall(r"<script>(.*?)</script>", html, flags=re.S | re.I)
    if len(scripts) != 1:
        fail(f"expected exactly one inline script, found {len(scripts)}")

    with tempfile.NamedTemporaryFile("w", suffix=".js", encoding="utf-8", delete=False) as handle:
        handle.write(scripts[0])
        js_path = Path(handle.name)
    try:
        result = subprocess.run(["node", "--check", str(js_path)], text=True, capture_output=True)
        if result.returncode:
            print(result.stdout)
            print(result.stderr, file=sys.stderr)
            fail("JavaScript syntax check failed")
    finally:
        js_path.unlink(missing_ok=True)

    print(f"web-ui check: OK ({len(html)} bytes, {len(ids)} ids)")


if __name__ == "__main__":
    main()
