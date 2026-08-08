#!/usr/bin/env python3
"""Build a single flashable ESP32-C3 factory image from PlatformIO outputs."""

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


def locate_esptool() -> Path:
    explicit = os.environ.get("ESPTOOL")
    candidates: list[Path] = []
    if explicit:
        candidates.append(Path(explicit).expanduser())
    executable = shutil.which("esptool")
    if executable:
        candidates.append(Path(executable))
    home = Path.home()
    candidates.extend([
        home / ".platformio" / "penv" / "bin" / "esptool",
        home / ".platformio" / "penv" / "Scripts" / "esptool.exe",
    ])
    found = first_existing(candidates)
    if found:
        return found
    raise SystemExit("esptool not found; run PlatformIO build/bootstrap first")


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
    esptool = locate_esptool()
    bootloader = require(BUILD / "bootloader.bin")
    partitions = require(BUILD / "partitions.bin")
    firmware = require(BUILD / "firmware.bin")
    boot_app0 = locate_boot_app0()

    output.parent.mkdir(parents=True, exist_ok=True)
    cmd = [
        str(esptool), "--chip", "esp32c3", "merge-bin",
        "-o", str(output),
        "--flash-mode", "dio",
        "--flash-freq", "80m",
        "--flash-size", "4MB",
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
    parser = argparse.ArgumentParser(description="Create merged ESP32-C3 factory image")
    parser.add_argument("output", nargs="?", default="dist/hydro-esp-c3-factory.bin")
    args = parser.parse_args()
    build_factory(Path(args.output).expanduser().resolve())
    return 0


if __name__ == "__main__":
    sys.exit(main())
