#!/usr/bin/env python3
"""Regression checks for the small on-device OLED user interface."""

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def text(path: str) -> str:
    return (ROOT / path).read_text(encoding="utf-8")


def require(path: str, needle: str) -> None:
    value = text(path)
    if needle not in value:
        raise SystemExit(f"display contract failed: {path} missing {needle!r}")


def forbid(path: str, needle: str) -> None:
    value = text(path)
    if needle in value:
        raise SystemExit(f"display contract failed: {path} contains obsolete {needle!r}")


# Russian human-facing labels use U8g2's built-in UTF-8 Cyrillic font. Technical
# tokens such as IP/Wi-Fi/NTP/hydro.local intentionally remain standard names.
require("src/oled_display.cpp", "u8g2_font_unifont_t_cyrillic")
require("src/oled_display.cpp", "drawUTF8")
for label in (
    "ЗАПУСК",
    "ГОТОВО",
    "ПРОШИВКА",
    "ОШИБКА",
    "САЙТ",
    "АВТОПОЛИВ",
    "ПАУЗА",
    "НЕТ ПЛАНА",
    "ПЛАН",
    "ПОЛИВ",
):
    require("src/oled_display.cpp", label)

# A paused scheduler must never look like an active countdown on the device.
require("src/oled_display.cpp", "if (!scheduler->isEnabled())")

# OTA owns the display and redraws are throttled instead of sending a full I2C
# framebuffer for every network packet.
require("src/oled_display.h", "_lastOtaProgress")
require("src/oled_display.h", "_lastOtaDraw")
require("src/oled_display.cpp", "delta < 2")
require("src/oled_display.cpp", "drawOTAComplete")
require("src/oled_display.cpp", "drawOTAError")
require("src/recovery_ota.cpp", "drawOTAComplete")
require("src/recovery_ota.cpp", "drawOTAError")

# Provisioning is also localized; SSID/IP remain literal network identifiers.
require("src/oled_provisioning.cpp", "НАСТРОЙКА")
for obsolete_literal in (
    '"SETUP WIFI"',
    '"NO PASSWORD"',
    '"WEB ADDRESS"',
    '"NEXT WATERING"',
    '"SCHEDULE"',
    '"WATERING"',
    '"OTA UPDATE..."',
):
    forbid("src/oled_display.cpp", obsolete_literal)
    forbid("src/oled_provisioning.cpp", obsolete_literal)

print("display contract: OK (Russian states + pause safety + throttled OTA progress)")
