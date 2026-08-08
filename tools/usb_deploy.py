#!/usr/bin/env python3
"""Repeat USB deployment that never provisions or changes Wi-Fi settings."""

from __future__ import annotations

import argparse
import os
from pathlib import Path
import sys

TOOLS = Path(__file__).resolve().parent
if str(TOOLS) not in sys.path:
    sys.path.insert(0, str(TOOLS))

import hydroctl


def deployment_env() -> dict[str, str]:
    env = os.environ.copy()
    # A repeat deploy must be configuration-neutral. In particular it must not
    # embed a new Wi-Fi seed from shell variables or the project's .env file.
    for key in ("WIFI_SSID", "WIFI_PASSWORD", "WIFI_SEED_ID"):
        env.pop(key, None)
    return env


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Build and flash HydroESP-C3 over USB without changing Wi-Fi/NVS configuration"
    )
    parser.add_argument("--port", help="serial port; PlatformIO auto-detects when omitted")
    parser.add_argument("--clean", action="store_true", help="clean project build artifacts first")
    args = parser.parse_args()

    try:
        pio = hydroctl.ensure_pio(True)
        env = deployment_env()
        if args.clean:
            hydroctl.run([pio, "run", "-t", "clean"], env=env)

        hydroctl.say("repeat deploy: Wi-Fi provisioning disabled; stored NVS settings will be preserved")
        hydroctl.run([pio, "run"], env=env)

        cmd = [pio, "run", "-t", "upload"]
        if args.port:
            cmd += ["--upload-port", args.port]
        hydroctl.run(cmd, env=env)
        hydroctl.say("USB deploy complete; Wi-Fi credentials were not embedded or reseeded")
        return 0
    except (hydroctl.HydroError, OSError, ValueError) as exc:
        print(f"[deploy] ERROR: {exc}", file=sys.stderr)
        return 2
    except KeyboardInterrupt:
        print("\n[deploy] cancelled", file=sys.stderr)
        return 130


if __name__ == "__main__":
    raise SystemExit(main())
