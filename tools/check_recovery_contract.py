#!/usr/bin/env python3
"""Static regression checks for deploy neutrality and reliable Wi-Fi OTA."""

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
require("wifi-flash.sh", "tools/wifi_flash.py")
require("wifi-flash.ps1", "tools\\wifi_flash.py")

require("tools/usb_deploy.py", '"WIFI_SEED_ID"')
require("tools/usb_deploy.py", "env.pop(key, None)")

# Normal Wi-Fi updates use the already-running HTTP server because it is the
# most interoperable path across macOS/Linux/Windows LANs. Port 3232 remains a
# Web-independent fallback when HTTP itself is unavailable.
require("tools/wifi_flash.py", "DEFAULT_OTA_PORT = 3232")
require("tools/wifi_flash.py", "DEFAULT_OTA_TIMEOUT = 3")
require("tools/wifi_flash.py", '"/ota/upload"')
require("tools/wifi_flash.py", '"/api/status"')
require("tools/wifi_flash.py", "OTA verified:")
require("tools/wifi_flash.py", "wait_for_reboot")
require("tools/wifi_flash.py", "--recovery-first")
require("tools/wifi_flash.py", "stdout=subprocess.PIPE")
require("tools/wifi_flash.py", "building OTA-safe firmware without Wi-Fi provisioning flags")
forbid("tools/wifi_flash.py", "recovery OTA unavailable:")

# Firmware keeps the independent ArduinoOTA channel and services it continuously.
require("src/main.cpp", "recoveryOTA.begin")
require("src/main.cpp", "recoveryOTA.handle")
require("src/recovery_ota.cpp", "ArduinoOTA.setMdnsEnabled(false)")
require("src/recovery_ota.cpp", "ArduinoOTA.begin()")
require("src/recovery_ota.cpp", "ArduinoOTA.handle()")

# Once flashing starts, the ordinary application loop must not redraw regular
# OLED pages, run the watering scheduler or accept the physical start button.
require("src/main.cpp", "if (otaManager.isUpdating())")
require("src/main.cpp", "OTA callbacks own the progress screen")
require("src/ota_manager.cpp", "Keep the OTA latch active until ESP.restart()")
require("src/ota_manager.cpp", "if (!updating || total == 0) return;")

print("recovery/deploy contract: OK (HTTP-first verified OTA + port-3232 fallback + stable OTA UI)")
