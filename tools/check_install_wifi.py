#!/usr/bin/env python3
"""Regression checks for install Wi-Fi env/.env resolution."""

from __future__ import annotations

import importlib.util
import os
from pathlib import Path
import tempfile


ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("hydroctl", ROOT / "tools/hydroctl.py")
assert SPEC and SPEC.loader
hydroctl = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(hydroctl)


class Args:
    timezone = None


saved_root = hydroctl.ROOT
saved = {key: os.environ.get(key) for key in ("WIFI_SSID", "WIFI_PASSWORD", "TIMEZONE_OFFSET", "WIFI_SEED_ID")}
try:
    for key in saved:
        os.environ.pop(key, None)

    with tempfile.TemporaryDirectory(prefix="hydro-wifi-test-") as tmp:
        hydroctl.ROOT = Path(tmp)
        (hydroctl.ROOT / ".env").write_text(
            "WIFI_SSID=dotenv-net\nWIFI_PASSWORD=dotenv-pass\nTIMEZONE_OFFSET=2\n",
            encoding="utf-8",
        )

        env = hydroctl.build_env(Args(), prompt_wifi=True)
        assert env["WIFI_SSID"] == "dotenv-net"
        assert env["WIFI_PASSWORD"] == "dotenv-pass"
        assert env["TIMEZONE_OFFSET"] == "2"
        assert len(env["WIFI_SEED_ID"]) == 16

        os.environ["WIFI_SSID"] = "shell-net"
        os.environ["WIFI_PASSWORD"] = "shell-pass"
        env = hydroctl.build_env(Args(), prompt_wifi=True)
        assert env["WIFI_SSID"] == "shell-net"
        assert env["WIFI_PASSWORD"] == "shell-pass"
        assert len(env["WIFI_SEED_ID"]) == 16

    print("install-wifi check: OK")
finally:
    hydroctl.ROOT = saved_root
    for key, value in saved.items():
        if value is None:
            os.environ.pop(key, None)
        else:
            os.environ[key] = value
