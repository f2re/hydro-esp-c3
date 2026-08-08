#!/usr/bin/env python3
"""Regression checks for browser OTA progress and post-reboot verification."""

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "src" / "web_ui_v2.h"
text = SOURCE.read_text(encoding="utf-8")


def require(needle: str) -> None:
    if needle not in text:
        raise SystemExit(f"web-ota contract failed: missing {needle!r}")


def forbid(needle: str) -> None:
    if needle in text:
        raise SystemExit(f"web-ota contract failed: obsolete {needle!r} returned")


# Transfer is only stage one. The browser must distinguish accepted firmware,
# reboot in progress and the controller becoming reachable again.
for needle in (
    "xhr.upload.onprogress",
    "Math.min(99",
    "async function otaStatus",
    "async function waitForOtaReboot",
    "beforeUptime",
    "sawOffline",
    "uptimeReset",
    "buildChanged",
    "Прошивка принята. Перезагрузка",
    "Ожидаю возвращения контроллера после перезагрузки",
    "Готово. Контроллер снова доступен.",
    "Контроллер снова доступен.",
    "не вернулся в сеть за 45 секунд",
    "Передача не подтверждена",
    "xhr.ontimeout",
):
    require(needle)

# The previous UI treated server OK as the final success before reboot.
forbid("toast('Прошивка записана','Контроллер перезагружается.'")

print("web-ota contract: OK (transfer -> accepted -> reboot -> online verification)")
