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

# Human-facing Wi-Fi updater: visible streaming progress, explicit transport
# choice, and post-reboot verification are all part of the product contract.
require("tools/wifi_flash.py", "DEFAULT_OTA_PORT = 3232")
require("tools/wifi_flash.py", "DEFAULT_OTA_TIMEOUT = 3")
require("tools/wifi_flash.py", 'choices=("auto", "http", "arduino")')
require("tools/wifi_flash.py", '"--transport"')
require("tools/wifi_flash.py", "class UploadProgress")
require("tools/wifi_flash.py", "http.client.HTTPConnection")
require("tools/wifi_flash.py", "CHUNK_SIZE = 16 * 1024")
require("tools/wifi_flash.py", '"/api/status"')
require("tools/wifi_flash.py", "wait_for_reboot")
require("tools/wifi_flash.py", "OTA ПРОВЕРЕН")
require("tools/wifi_flash.py", "show_output=True")
require("tools/wifi_flash.py", "stdout=subprocess.PIPE")
forbid("tools/wifi_flash.py", "hydroctl.multipart_file")

# HTTP OTA must acknowledge the completed upload before reset. Reset is deferred
# from the AsyncWebServer callback through OTAManager/esp_timer.
require("src/web_server.cpp", 'server.on("/ota/upload"')
require("src/web_server.cpp", 'failed ? "FAIL" : "OK"')
require("src/web_server.cpp", 'response->addHeader("Connection", "close")')
require("src/web_server.cpp", "otaManager.scheduleRestart(1500)")
require("src/web_server.cpp", "HTTP acknowledgement queued")
require("src/web_server.cpp", "if (!final)")
require("src/web_server.cpp", "if (progress > 99) progress = 99")
require("src/web_server.cpp", "oled->drawOTAComplete()")
require("src/web_server.cpp", "oled->drawOTAError()")
forbid("src/web_server.cpp", "delay(450)")
forbid("src/web_server.cpp", "ESP.restart();")

require("src/ota_manager.h", "scheduleRestart")
require("src/ota_manager.cpp", "esp_timer_start_once")
require("src/ota_manager.cpp", "Keep the OTA latch active until the actual restart")
require("src/ota_manager.cpp", "if (!updating || total == 0) return;")

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

print(
    "recovery/deploy contract: OK "
    "(streaming progress + final HTTP ack + verified reboot + ArduinoOTA fallback)"
)
