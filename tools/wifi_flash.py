#!/usr/bin/env python3
"""Build and push HydroESP-C3 firmware over Wi-Fi with verified completion."""

from __future__ import annotations

import argparse
import http.client
import os
from pathlib import Path
import socket
import subprocess
import sys
import time
import uuid
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
CHUNK_SIZE = 16 * 1024
FIRMWARE = ROOT / ".pio" / "build" / "esp32c3_supermini" / "firmware.bin"


class UploadProgress:
    def __init__(self, total: int) -> None:
        self.total = max(1, total)
        self.started = time.monotonic()
        self.last_percent = -1
        self.tty = sys.stdout.isatty()

    def update(self, sent: int, *, force: bool = False) -> None:
        sent = max(0, min(sent, self.total))
        percent = int(sent * 100 / self.total)
        if not force:
            if self.tty and percent == self.last_percent:
                return
            if not self.tty and percent not in {0, 25, 50, 75, 100}:
                return
            if not self.tty and percent == self.last_percent:
                return
        self.last_percent = percent
        elapsed = max(0.001, time.monotonic() - self.started)
        rate_kib = sent / 1024.0 / elapsed
        width = 24
        filled = int(width * percent / 100)
        bar = "#" * filled + "-" * (width - filled)
        line = (
            f"[OTA] [{bar}] {percent:3d}%  "
            f"{sent // 1024:4d}/{self.total // 1024:4d} KiB  {rate_kib:5.1f} KiB/s"
        )
        if self.tty:
            print("\r" + line, end="", flush=True)
            if percent >= 100:
                print()
        else:
            print(line, flush=True)

    def finish(self) -> None:
        self.update(self.total, force=True)


def clean_host(value: str) -> str:
    value = value.strip()
    if not value:
        return DEFAULT_HOST
    if "://" in value:
        parsed = urlsplit(value)
        if not parsed.hostname:
            raise ValueError(f"invalid host: {value}")
        return parsed.hostname
    if value.count(":") == 1:
        host, maybe_port = value.rsplit(":", 1)
        if maybe_port.isdigit():
            return host
    return value


def build_env() -> dict[str, str]:
    env = os.environ.copy()
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
    timeout: int,
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


def multipart_parts(firmware: Path, boundary: str) -> tuple[bytes, bytes]:
    prefix = (
        f"--{boundary}\r\n"
        f'Content-Disposition: form-data; name="file"; filename="{firmware.name}"\r\n'
        "Content-Type: application/octet-stream\r\n\r\n"
    ).encode("utf-8")
    suffix = f"\r\n--{boundary}--\r\n".encode("ascii")
    return prefix, suffix


def http_upload(
    host: str,
    firmware: Path,
    *,
    expected_build: str | None,
    before_status: dict | None,
    verify_timeout: int,
) -> dict:
    """Stream firmware through HTTP, show progress, then verify the reboot."""
    boundary = "----HydroESP" + uuid.uuid4().hex
    prefix, suffix = multipart_parts(firmware, boundary)
    file_size = firmware.stat().st_size
    content_length = len(prefix) + file_size + len(suffix)
    progress = UploadProgress(file_size)
    file_sent = 0
    response_error: Exception | None = None

    hydroctl.say(
        f"OTA по HTTP: {firmware.name} -> {host}/ota/upload; Web UI для этого не нужен"
    )

    connection = http.client.HTTPConnection(host, 80, timeout=90)
    try:
        connection.putrequest("POST", "/ota/upload")
        connection.putheader("Content-Type", f"multipart/form-data; boundary={boundary}")
        connection.putheader("Content-Length", str(content_length))
        connection.putheader("Connection", "close")
        connection.endheaders()
        connection.send(prefix)

        progress.update(0, force=True)
        with firmware.open("rb") as source:
            while True:
                chunk = source.read(CHUNK_SIZE)
                if not chunk:
                    break
                connection.send(chunk)
                file_sent += len(chunk)
                progress.update(file_sent)
        connection.send(suffix)
        progress.finish()

        response = connection.getresponse()
        result = response.read().decode("utf-8", "replace").strip()
        if response.status != 200 or result != "OK":
            raise hydroctl.HydroError(
                f"HTTP OTA rejected firmware: HTTP {response.status}, {result or response.reason}"
            )
        hydroctl.say("контроллер подтвердил приём прошивки; ожидаю перезагрузку")
    except (
        hydroctl.HydroError,
        OSError,
        ConnectionError,
        socket.timeout,
        http.client.HTTPException,
    ) as exc:
        response_error = exc
        if file_sent < file_size:
            raise hydroctl.HydroError(
                f"соединение оборвалось до передачи файла: {file_sent}/{file_size} bytes ({exc})"
            ) from exc
        # Old firmware can reset before the final HTTP response is flushed.
        # The post-reboot build check below remains authoritative.
        hydroctl.say("финальный HTTP-ответ не получен; файл передан полностью, проверяю перезагрузку")
    finally:
        connection.close()

    status = wait_for_reboot(
        host,
        expected_build=expected_build,
        before_status=before_status,
        timeout=verify_timeout,
    )
    if response_error is not None:
        hydroctl.say("перезагрузка подтвердила успешную запись несмотря на обрыв старого HTTP-сеанса")
    hydroctl.say(
        f"OTA ПРОВЕРЕН: build {status.get('build', '?')}, uptime {status.get('uptime', '?')} s"
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
    verify_timeout: int,
    show_output: bool,
) -> dict:
    """Use independent ArduinoOTA/espota and verify the reboot afterwards."""
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

    hydroctl.say(f"ArduinoOTA: {firmware.name} -> {host}:{port}")
    if show_output:
        result = subprocess.run(cmd, cwd=ROOT, env=env)
        if result.returncode:
            raise hydroctl.HydroError(
                f"ArduinoOTA channel {host}:{port} failed with exit code {result.returncode}"
            )
    else:
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
            raise hydroctl.HydroError(
                f"ArduinoOTA channel {host}:{port} did not accept OTA ({detail})"
            )

    hydroctl.say("ArduinoOTA передал прошивку; ожидаю перезагрузку")
    status = wait_for_reboot(
        host,
        expected_build=expected_build,
        before_status=before_status,
        timeout=verify_timeout,
    )
    hydroctl.say(
        f"OTA ПРОВЕРЕН: build {status.get('build', '?')}, uptime {status.get('uptime', '?')} s"
    )
    return status


def main() -> int:
    parser = argparse.ArgumentParser(
        description=(
            "Flash HydroESP-C3 over Wi-Fi with progress and post-reboot verification. "
            "HTTP is the normal path; ArduinoOTA:3232 remains an independent recovery path."
        )
    )
    parser.add_argument(
        "host",
        nargs="?",
        default=DEFAULT_HOST,
        help="controller IP/hostname; numeric IP from OLED is preferred for recovery",
    )
    parser.add_argument(
        "--transport",
        choices=("auto", "http", "arduino"),
        default="auto",
        help="OTA transport: auto (default), http, or arduino (port 3232)",
    )
    parser.add_argument("--port", type=int, default=DEFAULT_OTA_PORT, help="ArduinoOTA port (default: 3232)")
    parser.add_argument(
        "--timeout",
        type=int,
        default=DEFAULT_OTA_TIMEOUT,
        help="ArduinoOTA invitation timeout in seconds (default: 3)",
    )
    parser.add_argument(
        "--verify-timeout",
        type=int,
        default=VERIFY_TIMEOUT,
        help="seconds to wait for controller after reboot (default: 45)",
    )
    parser.add_argument("--file", help="upload an existing firmware.bin instead of building current source")
    parser.add_argument("--clean", action="store_true", help="clean project build artifacts before building")
    # Backward-compatible aliases from the first recovery implementation.
    parser.add_argument("--recovery-first", action="store_true", help=argparse.SUPPRESS)
    parser.add_argument("--no-http-fallback", action="store_true", help=argparse.SUPPRESS)
    parser.add_argument("--debug", action="store_true", help="show espota.py diagnostic output")
    args = parser.parse_args()

    try:
        if args.timeout < 1 or args.timeout > 30:
            raise ValueError("--timeout must be in range 1..30 seconds")
        if args.verify_timeout < 5 or args.verify_timeout > 180:
            raise ValueError("--verify-timeout must be in range 5..180 seconds")

        transport = args.transport
        if args.recovery_first or args.no_http_fallback:
            transport = "arduino"

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
            hydroctl.say("[1/4] сборка OTA-safe firmware без Wi-Fi provisioning flags")
            hydroctl.run([pio, "run"], env=env)
            firmware = FIRMWARE
            expected_build = current_build_sha()
            if not firmware.is_file():
                raise hydroctl.HydroError(f"build did not produce {firmware}")

        if firmware.suffix.lower() != ".bin" or firmware.stat().st_size < 10_000:
            raise hydroctl.HydroError("firmware file is invalid or unexpectedly small")

        hydroctl.say("[2/4] проверка контроллера")
        before_status = get_status(host)
        if before_status is not None:
            hydroctl.say(
                f"контроллер online: build {before_status.get('build', '?')}, "
                f"uptime {before_status.get('uptime', '?')} s"
            )
        else:
            hydroctl.say("HTTP status не отвечает; попробую доступный recovery-канал")

        if transport == "http":
            hydroctl.say("[3/4] выбран HTTP OTA")
            http_upload(
                host,
                firmware,
                expected_build=expected_build,
                before_status=before_status,
                verify_timeout=args.verify_timeout,
            )
            hydroctl.say("[4/4] готово")
            return 0

        if transport == "arduino":
            hydroctl.say("[3/4] выбран ArduinoOTA / espota.py; прогресс показан ниже")
            recovery_upload(
                host,
                firmware,
                port=args.port,
                timeout=args.timeout,
                env=env,
                debug=args.debug,
                expected_build=expected_build,
                before_status=before_status,
                verify_timeout=args.verify_timeout,
                show_output=True,
            )
            hydroctl.say("[4/4] готово")
            return 0

        # Auto: HTTP is preferred when the local API is reachable because it
        # gives deterministic request semantics. ArduinoOTA remains independent
        # of the web stack and is used as the recovery path.
        errors: list[str] = []
        if before_status is not None:
            try:
                hydroctl.say("[3/4] auto: HTTP OTA (ArduinoOTA остаётся резервом)")
                http_upload(
                    host,
                    firmware,
                    expected_build=expected_build,
                    before_status=before_status,
                    verify_timeout=args.verify_timeout,
                )
                hydroctl.say("[4/4] готово")
                return 0
            except hydroctl.HydroError as exc:
                errors.append(str(exc))
                hydroctl.say("HTTP OTA не подтверждён; пробую независимый ArduinoOTA:3232")

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
                verify_timeout=args.verify_timeout,
                show_output=False,
            )
            hydroctl.say("[4/4] готово")
            return 0
        except hydroctl.HydroError as exc:
            errors.append(str(exc))
            hydroctl.say("ArduinoOTA не ответил; последняя попытка — прямой HTTP OTA")

        http_upload(
            host,
            firmware,
            expected_build=expected_build,
            before_status=before_status,
            verify_timeout=args.verify_timeout,
        )
        hydroctl.say("[4/4] готово")
        return 0

    except (hydroctl.HydroError, OSError, ValueError, http.client.HTTPException) as exc:
        print(f"[wifi-flash] ERROR: {exc}", file=sys.stderr)
        print(
            "[wifi-flash] Обновление НЕ подтверждено. Проверь IP на OLED. "
            "Если HTTP и ArduinoOTA:3232 недоступны, один раз используй USB deploy.",
            file=sys.stderr,
        )
        return 2
    except KeyboardInterrupt:
        print("\n[wifi-flash] отменено", file=sys.stderr)
        return 130


if __name__ == "__main__":
    raise SystemExit(main())
