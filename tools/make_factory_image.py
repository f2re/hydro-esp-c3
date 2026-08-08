#!/usr/bin/env python3
"""Build a single flashable ESP32-C3 install image from PlatformIO outputs."""

from __future__ import annotations

import argparse
from pathlib import Path
import os
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
BUILD = ROOT / ".pio" / "build" / "esp32c3_supermini"


def first_existing(paths: list[Path]) -> Path | None:
    for path in paths:
        if path.is_file():
            return path
    return None


def locate_esptool_command() -> list[str]:
    """Return a runnable esptool command for old and new PlatformIO layouts."""
    explicit = os.environ.get("ESPTOOL")
    candidates: list[Path] = []
    if explicit:
        candidates.append(Path(explicit).expanduser())

    for name in ("esptool", "esptool.py"):
        executable = shutil.which(name)
        if executable:
            candidates.append(Path(executable))

    home = Path.home()
    candidates.extend([
        home / ".platformio" / "penv" / "bin" / "esptool",
        home / ".platformio" / "penv" / "Scripts" / "esptool.exe",
        # pioarduino 53.x / esptool 4.x installs the script here.
        home / ".platformio" / "packages" / "tool-esptoolpy" / "esptool.py",
        # Some package variants expose a native entry point instead.
        home / ".platformio" / "packages" / "tool-esptoolpy" / "esptool",
        home / ".platformio" / "packages" / "tool-esptoolpy" / "esptool.exe",
    ])

    found = first_existing(candidates)
    if not found:
        raise SystemExit("esptool not found; run PlatformIO build/bootstrap first")
    if found.suffix.lower() == ".py":
        return [sys.executable, str(found)]
    return [str(found)]


def locate_boot_app0() -> Path:
    root = Path.home() / ".platformio" / "packages" / "framework-arduinoespressif32"
    direct = root / "tools" / "partitions" / "boot_app0.bin"
    if direct.is_file():
        return direct
    matches = list((Path.home() / ".platformio").glob("**/tools/partitions/boot_app0.bin"))
    if matches:
        return matches[0]
    raise SystemExit("boot_app0.bin not found in PlatformIO Arduino-ESP32 package")


def require(path: Path) -> Path:
    if not path.is_file():
        raise SystemExit(f"required build artifact not found: {path}")
    return path


def build_factory(output: Path) -> None:
    esptool = locate_esptool_command()
    bootloader = require(BUILD / "bootloader.bin")
    partitions = require(BUILD / "partitions.bin")
    firmware = require(BUILD / "firmware.bin")
    boot_app0 = locate_boot_app0()

    output.parent.mkdir(parents=True, exist_ok=True)
    cmd = esptool + [
        "--chip", "esp32c3", "merge_bin",
        "-o", str(output),
        "--flash_mode", "dio",
        "--flash_freq", "80m",
        "--flash_size", "4MB",
        "0x0000", str(bootloader),
        "0x8000", str(partitions),
        "0xE000", str(boot_app0),
        "0x10000", str(firmware),
    ]
    print("[factory] " + " ".join(cmd))
    subprocess.run(cmd, cwd=ROOT, check=True)
    print(f"[factory] image: {output} ({output.stat().st_size} bytes)")
    print("[factory] flash this merged image at address 0x0; it resets NVS/config")


def main() -> int:
    parser = argparse.ArgumentParser(description="Create merged ESP32-C3 install image")
    parser.add_argument("output", nargs="?", default="dist/hydro-esp-c3-install.bin")
    args = parser.parse_args()
    build_factory(Path(args.output).expanduser().resolve())
    return 0


if __name__ == "__main__":
    sys.exit(main())
