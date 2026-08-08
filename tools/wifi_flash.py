#!/usr/bin/env python3
"""Build and push HydroESP-C3 firmware over Wi-Fi with verified completion."""

from __future__ import annotations

import argparse
import http.client
import os
from pathlib import Path
import subprocess
import sys
import time
from urllib.parse import urlsplit

TOOLS = Path(__file__).resolve().parent
ROOT = TOOLS.parent
if str(TOOLS) not in sys.path:
    sys.path.insert(0, str(TOOLS))

import hydroctl

DEFAULT_OTA_PORT = 3232
DEFAULT_OTA_TIMEOUT = 3
DEFAULT_HOST = "hydro.local"
VERIFY_TIMEOUT = 45
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


def current_build_sha() -> str | None:
    try:
        value = subprocess.check_output(
            ["git", "rev-parse", "HEAD"],
            cwd=ROOT,
            stderr=subprocess.DEVNULL,
            text=True,
        ).strip()
    except (OSError, subprocess.CalledProcessError):
        return None
    return value[:8] if value else None


def get_status(host: str, timeout: float = 1.5) -> dict | None:
    try:
        data = hydroctl.get_json(hydroctl.base_url(host) + "/api/status", timeout=timeout)
    except (hydroctl.HydroError, OSError, ValueError, http.client.HTTPException):
        return None
    return data if isinstance(data, dict) else None


def wait_for_reboot(
    host: str,
    *,
    expected_build: str | None,
    before_status: dict | None,
    timeout: int = VERIFY_TIMEOUT,
) -> dict:
    """Wait until the controller returns after reboot and verify the new build when possible."""
    deadline = time.monotonic() + timeout
    saw_offline = False
    before_uptime = int((before_status or {}).get("uptime", -1))
    before_build = str((before_status or {}).get("build", ""))
    last_status: dict | None = None

    while time.monotonic() < deadline:
        status = get_status(host)
        if status is None:
            saw_offline = True
            time.sleep(0.7)
            continue

        last_status = status
        build = str(status.get("build", ""))
        try:
            uptime = int(status.get("uptime", -1))
        except (TypeError, ValueError):
            uptime = -1

        build_ok = not expected_build or build == expected_build
        reboot_ok = (
            before_status is None
            or saw_offline
            or (before_uptime >= 0 and uptime >= 0 and uptime < before_uptime)
            or (expected_build and before_build and expected_build != before_build and build == expected_build)
        )
        if build_ok and reboot_ok:
            return status

        time.sleep(0.7)

    if last_status is not None:
        raise hydroctl.HydroError(
            "controller returned, but the expected firmware/reboot was not confirmed "
            f"(build={last_status.get('build', '?')}, expected={expected_build or 'unknown'})"
        )
    raise hydroctl.HydroError(
        f"controller did not return at {host} within {timeout} seconds after OTA"
    )


def http_upload(
    host: str,
    firmware: Path,
    *,
    expected_build: str | None,
    before_status: dict | None,
) -> dict:
    """Upload through the HTTP endpoint and verify the reboot.

    A connection reset after the body was sent is not treated as an immediate
    failure: ESP.restart() can close the socket before urllib receives the small
    final response. In that case the post-reboot build check is authoritative.
    """
    url = hydroctl.base_url(host)
    hydroctl.say(
        f"Wi-Fi OTA: {firmware.name} -> {host} via HTTP /ota/upload; Web UI page is not required"
    )
    body, boundary = hydroctl.multipart_file("file", firmware)
    response_error: Exception | None = None

    try:
        result = hydroctl.request(
            url + "/ota/upload",
            method="POST",
            body=body,
            headers={"Content-Type": f"multipart/form-data; boundary={boundary}"},
            timeout=90,
        ).decode("utf-8", "replace").strip()
        if result != "OK":
            raise hydroctl.HydroError(f"HTTP OTA rejected firmware: {result or 'empty response'}")
        hydroctl.say("firmware accepted; waiting for controller reboot")
    except (hydroctl.HydroError, OSError, ConnectionError, http.client.HTTPException) as exc:
        # The current firmware restarts shortly after sending the final response.
        # On some macOS/network combinations that manifests as EOF/reset locally,
        # even though Update.end(true) has already succeeded. Verify the device
        # before deciding that the flash failed.
        response_error = exc
        hydroctl.say("OTA connection closed before final acknowledgement; verifying reboot")

    try:
        status = wait_for_reboot(
            host,
            expected_build=expected_build,
            before_status=before_status,
        )
    except hydroctl.HydroError:
        if response_error is not None:
            raise hydroctl.HydroError(f"HTTP OTA was not confirmed: {response_error}") from response_error
        raise

    hydroctl.say(
        f"OTA verified: controller is online, build {status.get('build', '?')}, "
        f"uptime {status.get('uptime', '?')} s"
    )
    return status


def recovery_upload(
    host: str,
    firmware: Path,
    *,
    port: int,
    timeout: int,
    env: dict[str, str],
    debug: bool,
    expected_build: str | None,
    before_status: dict | None,
) -> dict:
    """Use ArduinoOTA/espota without leaking a scary subprocess error on fallback."""
    espota = find_espota()
    cmd = [
        sys.executable,
        str(espota),
        "-i",
        host,
        "-p",
        str(port),
        "-t",
        str(timeout),
        "-r",
        "-f",
        str(firmware),
    ]
    if debug:
        cmd.append("-d")

    hydroctl.say(f"recovery OTA: {firmware.name} -> {host}:{port}")
    result = subprocess.run(
        cmd,
        cwd=ROOT,
        env=env,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    output = "\n".join(part.strip() for part in (result.stdout, result.stderr) if part.strip())
    if debug and output:
        print(output)
    if result.returncode:
        detail = output.splitlines()[-1] if output else f"exit code {result.returncode}"
        raise hydroctl.HydroError(f"recovery channel {host}:{port} did not accept OTA ({detail})")

    hydroctl.say("recovery OTA transfer complete; waiting for controller reboot")
    status = wait_for_reboot(
        host,
        expected_build=expected_build,
        before_status=before_status,
    )
    hydroctl.say(
        f"OTA verified: controller is online, build {status.get('build', '?')}, "
        f"uptime {status.get('uptime', '?')} s"
    )
    return status


def main() -> int:
    parser = argparse.ArgumentParser(
        description=(
            "Flash HydroESP-C3 over Wi-Fi. Normal updates use the HTTP OTA endpoint "
            "and verify the reboot; port 3232 remains an independent recovery fallback."
        )
    )
    parser.add_argument(
        "host",
        nargs="?",
        default=DEFAULT_HOST,
        help="controller IP/hostname; use the numeric IP from OLED if hydro.local is unavailable",
    )
    parser.add_argument("--port", type=int, default=DEFAULT_OTA_PORT, help="recovery OTA port (default: 3232)")
    parser.add_argument(
        "--timeout",
        type=int,
        default=DEFAULT_OTA_TIMEOUT,
        help="espota invitation timeout per attempt in seconds (default: 3)",
    )
    parser.add_argument("--file", help="upload an existing firmware.bin instead of building current source")
    parser.add_argument("--clean", action="store_true", help="clean project build artifacts before building")
    parser.add_argument(
        "--recovery-first",
        action="store_true",
        help="try independent ArduinoOTA port 3232 before the normal HTTP OTA path",
    )
    parser.add_argument(
        "--no-http-fallback",
        action="store_true",
        help="recovery-only mode: do not use HTTP OTA (implies --recovery-first)",
    )
    parser.add_argument("--debug", action="store_true", help="show espota.py diagnostic output")
    args = parser.parse_args()

    try:
        if args.timeout < 1 or args.timeout > 30:
            raise ValueError("--timeout must be in range 1..30 seconds")

        host = clean_host(args.host)
        env = build_env()
        pio = hydroctl.ensure_pio(True)

        if args.file:
            firmware = Path(args.file).expanduser().resolve()
            expected_build = None
            if not firmware.is_file():
                raise hydroctl.HydroError(f"firmware not found: {firmware}")
        else:
            if args.clean:
                hydroctl.run([pio, "run", "-t", "clean"], env=env)
            hydroctl.say("building OTA-safe firmware without Wi-Fi provisioning flags")
            hydroctl.run([pio, "run"], env=env)
            firmware = FIRMWARE
            expected_build = current_build_sha()
            if not firmware.is_file():
                raise hydroctl.HydroError(f"build did not produce {firmware}")

        if firmware.suffix.lower() != ".bin" or firmware.stat().st_size < 10_000:
            raise hydroctl.HydroError("firmware file is invalid or unexpectedly small")

        before_status = get_status(host)
        if before_status is not None:
            hydroctl.say(
                f"controller online: build {before_status.get('build', '?')}, "
                f"uptime {before_status.get('uptime', '?')} s"
            )

        recovery_first = args.recovery_first or args.no_http_fallback
        errors: list[str] = []

        if not recovery_first and before_status is not None:
            try:
                http_upload(
                    host,
                    firmware,
                    expected_build=expected_build,
                    before_status=before_status,
                )
                return 0
            except hydroctl.HydroError as exc:
                errors.append(str(exc))
                hydroctl.say("HTTP OTA was not confirmed; trying independent recovery channel 3232")

        try:
            recovery_upload(
                host,
                firmware,
                port=args.port,
                timeout=args.timeout,
                env=env,
                debug=args.debug,
                expected_build=expected_build,
                before_status=before_status,
            )
            return 0
        except hydroctl.HydroError as exc:
            errors.append(str(exc))
            if args.no_http_fallback:
                raise hydroctl.HydroError("; ".join(errors)) from exc
            hydroctl.say("recovery channel did not answer; trying HTTP OTA")

        # HTTP may still be alive even when the initial status probe failed.
        http_upload(
            host,
            firmware,
            expected_build=expected_build,
            before_status=before_status,
        )
        return 0

    except (hydroctl.HydroError, OSError, ValueError, http.client.HTTPException) as exc:
        print(f"[wifi-flash] ERROR: {exc}", file=sys.stderr)
        print(
            "[wifi-flash] Update was not verified. Check the IP on OLED and retry; "
            "if both HTTP and recovery OTA are unreachable, connect USB once.",
            file=sys.stderr,
        )
        return 2
    except KeyboardInterrupt:
        print("\n[wifi-flash] cancelled", file=sys.stderr)
        return 130


if __name__ == "__main__":
    raise SystemExit(main())
