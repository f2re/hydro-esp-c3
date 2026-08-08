#!/usr/bin/env python3
from __future__ import annotations

import re
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
UI = ROOT / "src/web_ui_v2.h"
SERVER = ROOT / "src/web_server.cpp"


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

    # The embedded page is much larger than a safe temporary Arduino String on
    # an ESP32-C3. ESPAsyncWebServer's const-char overload copies the whole body
    # into String and can silently degrade to HTTP 200 / Content-Length: 0 when
    # the allocation fails. Require the explicit-length progmem response path.
    server = SERVER.read_text(encoding="utf-8")
    broken_send = 'request->send(200, "text/html; charset=utf-8", WEB_UI_HTML);'
    if broken_send in server:
        fail("large WEB_UI_HTML is sent through allocating const-char overload")
    for required in (
        "void sendWebUi(AsyncWebServerRequest *request)",
        "reinterpret_cast<const uint8_t*>(WEB_UI_HTML)",
        "sizeof(WEB_UI_HTML) - 1",
    ):
        if required not in server:
            fail(f"flash-streamed UI transport missing: {required}")
    if server.count("sendWebUi(request);") < 2:
        fail("both root route and setup-AP fallback must use flash-streamed UI transport")

    print(f"web-ui check: OK ({len(html)} bytes, {len(ids)} ids, flash-streamed transport)")


if __name__ == "__main__":
    main()
