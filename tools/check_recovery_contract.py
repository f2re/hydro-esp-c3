#!/usr/bin/env python3
"""Static regression checks for deploy neutrality and Web-independent recovery OTA."""

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def require(path: str, needle: str) -> None:
    text = (ROOT / path).read_text(encoding="utf-8")
    if needle not in text:
        raise SystemExit(f"recovery contract failed: {path} missing {needle!r}")


def forbid(path: str, needle: str) -> None:
    text = (ROOT / path).read_text(encoding="utf-8")
    if needle in text:
        raise SystemExit(f"recovery contract failed: {path} contains forbidden {needle!r}")


require("deploy.sh", "tools/usb_deploy.py")
forbid("deploy.sh", 'hydroctl.py\" install')
require("deploy.ps1", "tools\\usb_deploy.py")
forbid("deploy.ps1", 'hydroctl.py\" install')

require("tools/usb_deploy.py", '"WIFI_SEED_ID"')
require("tools/usb_deploy.py", "env.pop(key, None)")
require("tools/wifi_flash.py", "DEFAULT_OTA_PORT = 3232")
require("tools/wifi_flash.py", '"/ota/upload"')
require("src/main.cpp", "recoveryOTA.begin")
require("src/main.cpp", "recoveryOTA.handle")
require("src/recovery_ota.cpp", "ArduinoOTA.setMdnsEnabled(false)")
require("src/recovery_ota.cpp", "ArduinoOTA.begin()")
require("src/recovery_ota.cpp", "ArduinoOTA.handle()")

print("recovery/deploy contract: OK")
