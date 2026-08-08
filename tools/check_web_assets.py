#!/usr/bin/env python3
"""Regression checks for tiny flash-resident Web identity assets."""

from __future__ import annotations

import re
import struct
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
ASSETS = ROOT / "src" / "web_assets.h"
DOC_SVG = ROOT / "docs" / "assets" / "hydroesp-favicon.svg"
SERVER = ROOT / "src" / "web_server.h"
MAX_FLASH_ASSET_BYTES = 4096


def fail(message: str) -> None:
    print(f"web-assets check: {message}", file=sys.stderr)
    raise SystemExit(1)


def require(text: str, needle: str, where: str) -> None:
    if needle not in text:
        fail(f"{where} missing {needle!r}")


def main() -> None:
    source = ASSETS.read_text(encoding="utf-8")
    match = re.search(r'R"svg\((.*?)\)svg";', source, flags=re.S)
    if not match:
        fail("embedded SVG literal not found")
    embedded_svg = match.group(1).strip()

    try:
        ET.fromstring(embedded_svg)
    except ET.ParseError as exc:
        fail(f"invalid embedded SVG: {exc}")

    documented_svg = DOC_SVG.read_text(encoding="utf-8").strip()
    if embedded_svg != documented_svg:
        fail("docs/assets/hydroesp-favicon.svg differs from firmware SVG")

    ico_section = source.split("HYDRO_FAVICON_ICO[]", 1)
    if len(ico_section) != 2:
        fail("ICO array not found")
    ico = bytes(int(value, 16) for value in re.findall(r"0x([0-9a-fA-F]{2})", ico_section[1]))
    if len(ico) < 22 or ico[:4] != b"\x00\x00\x01\x00":
        fail("ICO header is invalid")

    image_count = struct.unpack_from("<H", ico, 4)[0]
    if image_count != 2:
        fail(f"expected 2 ICO images, found {image_count}")
    dimensions = []
    for index in range(image_count):
        offset = 6 + index * 16
        width = ico[offset] or 256
        height = ico[offset + 1] or 256
        dimensions.append((width, height))
    if dimensions != [(16, 16), (32, 32)]:
        fail(f"unexpected ICO dimensions: {dimensions}")

    svg_bytes = len(embedded_svg.encode("utf-8"))
    total = svg_bytes + len(ico)
    if total > MAX_FLASH_ASSET_BYTES:
        fail(f"identity assets use {total} bytes; budget is {MAX_FLASH_ASSET_BYTES}")

    server = SERVER.read_text(encoding="utf-8")
    for route in ("/favicon.svg", "/favicon.ico"):
        require(server, route, "src/web_server.h")
    require(server, "beginResponse(", "src/web_server.h")
    require(server, "Cache-Control", "src/web_server.h")
    require(server, "sizeof(HYDRO_FAVICON_SVG) - 1", "src/web_server.h")
    require(server, "sizeof(HYDRO_FAVICON_ICO)", "src/web_server.h")
    if "String(HYDRO_FAVICON" in server:
        fail("favicon must not be copied into Arduino String/RAM")

    print(
        "web-assets check: OK "
        f"(SVG {svg_bytes} B, ICO {len(ico)} B, total {total} B flash payload)"
    )


if __name__ == "__main__":
    main()
