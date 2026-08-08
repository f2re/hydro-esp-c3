#!/usr/bin/env python3
"""Build and push HydroESP-C3 firmware over Wi-Fi without depending on Web UI."""

from __future__ import annotations

import argparse
import os
from pathlib import Path
import sys
from urllib.parse import urlsplit

TOOLS = Path(__file__).resolve().parent
ROOT = TOOLS.parent
if str(TOOLS) not in sys.path:
    sys.path.insert(0, str(TOOLS))

import hydroctl

DEFAULT_OTA_PORT = 3232
DEFAULT_HOST = "hydro.local"
FIRMWARE = ROOT / ".pio" / "build" / "esp32c3_supermini" / "firmware.bin"


def clean_host(value: str) -> str:
    value = value.strip()
    if not value:
        return DEFAULT_HOST
    if "://" in value:
        parsed = urlsplit(value)
        if not parsed.hostname:
            raise ValueError(f"invalid host: {value}")
        return parsed.hostname
    # Keep ordinary hostnames/IPs unchanged. A trailing HTTP port is ignored.
    if value.count(":") == 1:
        host, maybe_port = value.rsplit(":", 1)
        if maybe_port.isdigit():
            return host
    return value


def build_env() -> dict[str, str]:
    env = os.environ.copy()
    # Recovery/repeat updates must never rewrite controller Wi-Fi credentials.
    for key in ("WIFI_SSID", "WIFI_PASSWORD", "WIFI_SEED_ID"):
        env.pop(key, None)
    return env


def find_espota() -> Path:
    roots: list[Path] = []
    configured = os.environ.get("PLATFORMIO_CORE_DIR", "").strip()
    if configured:
        roots.append(Path(configured).expanduser())
    roots.append(Path.home() / ".platformio")

    for root in roots:
        candidate = root / "packages" / "framework-arduinoespressif32" / "tools" / "espota.py"
        if candidate.is_file():
            return candidate
    raise hydroctl.HydroError(
        "espota.py not found in PlatformIO framework package; run a normal build/bootstrap first"
    )


def http_fallback(host: str, firmware: Path) -> None:
    url = hydroctl.base_url(host)
    hydroctl.say(
        "recovery OTA port did not answer; trying legacy HTTP /ota/upload directly "
        "(this does not require the Web UI page to render)"
    )
    body, boundary = hydroctl.multipart_file("file", firmware)
    result = hydroctl.request(
        url + "/ota/upload",
        method="POST",
        body=body,
        headers={"Content-Type": f"multipart/form-data; boundary={boundary}"},
        timeout=90,
    ).decode("utf-8", "replace").strip()
    if result != "OK":
        raise hydroctl.HydroError(f"legacy HTTP OTA rejected firmware: {result or 'empty response'}")
    hydroctl.say("legacy HTTP OTA accepted; controller is rebooting")


def main() -> int:
    parser = argparse.ArgumentParser(
        description=(
            "Flash HydroESP-C3 over Wi-Fi through the independent recovery OTA service; "
            "falls back to /ota/upload for older firmware"
        )
    )
    parser.add_argument(
        "host",
        nargs="?",
        default=DEFAULT_HOST,
        help="controller IP/hostname; use the numeric IP from OLED if hydro.local is unavailable",
    )
    parser.add_argument("--port", type=int, default=DEFAULT_OTA_PORT, help="recovery OTA port (default: 3232)")
    parser.add_argument("--file", help="upload an existing firmware.bin instead of building current source")
    parser.add_argument("--clean", action="store_true", help="clean project build artifacts before building")
    parser.add_argument(
        "--no-http-fallback",
        action="store_true",
        help="do not try the legacy HTTP OTA endpoint when recovery OTA does not answer",
    )
    parser.add_argument("--debug", action="store_true", help="enable espota.py debug output")
    args = parser.parse_args()

    try:
        host = clean_host(args.host)
        env = build_env()
        pio = hydroctl.ensure_pio(True)

        if args.file:
            firmware = Path(args.file).expanduser().resolve()
            if not firmware.is_file():
                raise hydroctl.HydroError(f"firmware not found: {firmware}")
        else:
            if args.clean:
                hydroctl.run([pio, "run", "-t", "clean"], env=env)
            hydroctl.say("building recovery-safe firmware without Wi-Fi provisioning flags")
            hydroctl.run([pio, "run"], env=env)
            firmware = FIRMWARE
            if not firmware.is_file():
                raise hydroctl.HydroError(f"build did not produce {firmware}")

        if firmware.suffix.lower() != ".bin" or firmware.stat().st_size < 10_000:
            raise hydroctl.HydroError("firmware file is invalid or unexpectedly small")

        espota = find_espota()
        cmd = [
            sys.executable,
            str(espota),
            "-i",
            host,
            "-p",
            str(args.port),
            "-r",
            "-f",
            str(firmware),
        ]
        if args.debug:
            cmd.append("-d")

        hydroctl.say(
            f"Wi-Fi recovery upload: {firmware.name} -> {host}:{args.port}; Web UI is not used"
        )
        try:
            hydroctl.run(cmd, env=env)
            hydroctl.say("recovery OTA upload complete; controller is rebooting")
            return 0
        except hydroctl.HydroError as ota_exc:
            if args.no_http_fallback:
                raise
            hydroctl.say(f"recovery OTA unavailable: {ota_exc}")
            http_fallback(host, firmware)
            return 0

    except (hydroctl.HydroError, OSError, ValueError) as exc:
        print(f"[wifi-flash] ERROR: {exc}", file=sys.stderr)
        print(
            "[wifi-flash] If both recovery OTA and HTTP are unreachable, connect USB once; "
            "future firmware will keep the independent port-3232 recovery channel.",
            file=sys.stderr,
        )
        return 2
    except KeyboardInterrupt:
        print("\n[wifi-flash] cancelled", file=sys.stderr)
        return 130


if __name__ == "__main__":
    raise SystemExit(main())
