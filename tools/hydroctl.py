#!/usr/bin/env python3
"""HydroESP-C3 maintenance CLI.

Standard-library-only front end for PlatformIO build/flash and the local device
HTTP API. Works on macOS, Linux and Windows with Python 3.9+.
"""

from __future__ import annotations

import argparse
import datetime as dt
import getpass
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import urllib.error
import urllib.request
import uuid

ROOT = Path(__file__).resolve().parents[1]
VENV = ROOT / ".venv"
REPO = "f2re/hydro-esp-c3"
DEFAULT_HOST = "http://hydro.local"


class HydroError(RuntimeError):
    pass


def say(message: str) -> None:
    print(f"[hydroctl] {message}")


def run(cmd: list[str], *, env: dict[str, str] | None = None) -> None:
    say("$ " + " ".join(cmd))
    result = subprocess.run(cmd, cwd=ROOT, env=env)
    if result.returncode:
        raise HydroError(f"command failed with exit code {result.returncode}")


def venv_python() -> Path:
    return VENV / ("Scripts/python.exe" if os.name == "nt" else "bin/python")


def venv_pio() -> Path:
    return VENV / ("Scripts/pio.exe" if os.name == "nt" else "bin/pio")


def ensure_pio(install: bool = True) -> str:
    system = shutil.which("pio")
    if system:
        return system
    local = venv_pio()
    if local.exists():
        return str(local)
    if not install:
        raise HydroError("PlatformIO not found")

    say("PlatformIO not found; creating isolated .venv")
    run([sys.executable, "-m", "venv", str(VENV)])
    run([str(venv_python()), "-m", "pip", "install", "--upgrade", "pip", "platformio>=6.1,<7"])
    if not local.exists():
        raise HydroError("PlatformIO installation did not create pio executable")
    return str(local)


def base_url(value: str) -> str:
    value = value.strip().rstrip("/")
    if not value.startswith(("http://", "https://")):
        value = "http://" + value
    return value


def request(url: str, *, method: str = "GET", body: bytes | None = None,
            headers: dict[str, str] | None = None, timeout: float = 8.0) -> bytes:
    req = urllib.request.Request(url, data=body, method=method, headers=headers or {})
    try:
        with urllib.request.urlopen(req, timeout=timeout) as response:
            return response.read()
    except urllib.error.HTTPError as exc:
        detail = exc.read().decode("utf-8", "replace")
        raise HydroError(f"HTTP {exc.code}: {detail or exc.reason}") from exc
    except urllib.error.URLError as exc:
        raise HydroError(f"connection failed: {exc.reason}") from exc


def get_json(url: str, timeout: float = 8.0):
    return json.loads(request(url, timeout=timeout).decode("utf-8"))


def post_json(url: str, payload) -> object:
    data = json.dumps(payload, ensure_ascii=False).encode("utf-8")
    raw = request(url, method="POST", body=data, headers={"Content-Type": "application/json"})
    return json.loads(raw.decode("utf-8")) if raw.strip() else {}


def command_bootstrap(_args) -> None:
    pio = ensure_pio(True)
    run([pio, "--version"])
    say("toolchain ready")


def load_dotenv(env: dict[str, str]) -> None:
    path = ROOT / ".env"
    if not path.is_file():
        return
    allowed = {"WIFI_SSID", "WIFI_PASSWORD", "TIMEZONE_OFFSET"}
    for raw in path.read_text(encoding="utf-8").splitlines():
        line = raw.strip()
        if not line or line.startswith("#") or "=" not in line:
            continue
        key, value = line.split("=", 1)
        key = key.strip()
        if key not in allowed or key in env:
            continue
        value = value.strip()
        if len(value) >= 2 and value[0] == value[-1] and value[0] in {"\"", "'"}:
            value = value[1:-1]
        env[key] = value


def build_env(args, *, prompt_wifi: bool = False) -> dict[str, str]:
    env = os.environ.copy()
    load_dotenv(env)

    if getattr(args, "timezone", None) is not None:
        env["TIMEZONE_OFFSET"] = str(args.timezone)

    ssid = env.get("WIFI_SSID", "").strip()
    password_set = "WIFI_PASSWORD" in env

    if prompt_wifi and not ssid and sys.stdin.isatty():
        ssid = input("Home Wi-Fi SSID (Enter = configure later via HydroESP-Setup): ").strip()
        if ssid:
            env["WIFI_SSID"] = ssid

    if prompt_wifi and ssid and not password_set and sys.stdin.isatty():
        env["WIFI_PASSWORD"] = getpass.getpass(
            "Home Wi-Fi password (Enter if the network is open): "
        )
        password_set = True

    if ssid:
        env["WIFI_SSID"] = ssid
        env.setdefault("WIFI_PASSWORD", "")
        if prompt_wifi:
            # A new token makes installer-provided credentials apply once even
            # when the controller already has older Wi-Fi settings in NVS.
            env["WIFI_SEED_ID"] = uuid.uuid4().hex[:16]
        say(f"Wi-Fi for this build: {ssid}")
    elif prompt_wifi:
        env.pop("WIFI_SSID", None)
        env.pop("WIFI_PASSWORD", None)
        env.pop("WIFI_SEED_ID", None)
        say("Wi-Fi not supplied; first setup will use HydroESP-Setup")

    return env


def command_build(args) -> None:
    pio = ensure_pio(True)
    env = build_env(args)
    if args.clean:
        run([pio, "run", "-t", "clean"], env=env)
    run([pio, "run"], env=env)
    firmware = ROOT / ".pio/build/esp32c3_supermini/firmware.bin"
    if firmware.exists():
        say(f"firmware: {firmware} ({firmware.stat().st_size // 1024} KiB)")


def command_install(args) -> None:
    pio = ensure_pio(True)
    env = build_env(args, prompt_wifi=True)
    if args.clean:
        run([pio, "run", "-t", "clean"], env=env)

    # Build first: never erase a working controller if the source does not compile.
    run([pio, "run"], env=env)

    # Installation is intentionally non-destructive: no flash/NVS erase.
    # A new board enters HydroESP-Setup; a configured board keeps its settings.
    cmd = [pio, "run", "-t", "upload"]
    if args.port:
        cmd += ["--upload-port", args.port]
    run(cmd, env=env)

    say("installation complete")
    if env.get("WIFI_SSID"):
        say(f"Controller will try Wi-Fi '{env['WIFI_SSID']}'; open the IP shown on OLED")
    else:
        say("Connect to Wi-Fi 'HydroESP-Setup' (no password), then open http://192.168.4.1")


def command_monitor(args) -> None:
    pio = ensure_pio(True)
    cmd = [pio, "device", "monitor", "--baud", "115200"]
    if args.port:
        cmd += ["--port", args.port]
    run(cmd)


def command_status(args) -> None:
    host = base_url(args.host)
    data = get_json(host + "/api/status")
    print(json.dumps(data, ensure_ascii=False, indent=2))


def command_events(args) -> None:
    host = base_url(args.host)
    data = get_json(host + "/api/events")
    events = data.get("events", []) if isinstance(data, dict) else []
    if not events:
        say("no events in current session")
        return
    for event in events:
        stamp = event.get("timestamp") or f"+{event.get('uptime', 0)}s"
        kind = event.get("type", "unknown")
        source = event.get("source", "none")
        reason = event.get("reason", "none")
        value = event.get("value", 0)
        extra = []
        if source != "none":
            extra.append(source)
        if reason != "none":
            extra.append(reason)
        if value:
            extra.append(str(value))
        print(f"{stamp:19}  {kind:20} {' · '.join(extra)}")


def set_automation(args, enabled: bool) -> None:
    host = base_url(args.host)
    result = post_json(host + "/api/automation", {"enabled": enabled})
    say("automation resumed" if result.get("enabled") else "automation paused")


def command_pause(args) -> None:
    set_automation(args, False)


def command_resume(args) -> None:
    set_automation(args, True)


def command_doctor(args) -> None:
    print("HydroESP-C3 doctor")
    print(f"  Python:      {sys.version.split()[0]}")
    print(f"  Project:     {ROOT}")
    print(f"  platformio:  {shutil.which('pio') or (venv_pio() if venv_pio().exists() else 'not installed')}")
    try:
        pio = ensure_pio(False)
        proc = subprocess.run([pio, "device", "list", "--json-output"], cwd=ROOT,
                              text=True, capture_output=True, timeout=12)
        ports = json.loads(proc.stdout or "[]") if proc.returncode == 0 else []
        print(f"  Serial:      {len(ports)} device(s) detected")
        for item in ports[:8]:
            print(f"    - {item.get('port', '?')}  {item.get('description', '')}")
    except Exception as exc:
        print(f"  Serial:      unavailable ({exc})")

    host = base_url(args.host)
    try:
        status = get_json(host + "/api/status", timeout=2.5)
        print(f"  Controller:  online at {host}")
        print(f"  Firmware:    {status.get('version', '?')} ({status.get('build', '?')})")
        print(f"  API:         v{status.get('api_version', '?')}")
        print(f"  Time sync:   {'yes' if status.get('time_synced') else 'no'}")
        print(f"  Automation:  {'enabled' if status.get('automation_enabled') else 'paused'}")
        flow = float(status.get("pump_flow_lpm") or 0)
        if flow:
            protocol = int(status.get("calibration_protocol_version") or 0)
            samples = int(status.get("calibration_sample_count") or 0)
            cv = float(status.get("calibration_cv_pct") or 0)
            quality = f", protocol v{protocol or '?'}, {samples} sample(s), CV {cv:.1f}%" if samples else ""
            print(f"  Hydraulics:  {flow:.3f} L/min{quality}")
        else:
            print("  Hydraulics:  not calibrated")
        print(f"  Pump:        {'ON / ' + status.get('pump_source', '?') if status.get('relay') else 'off'}")
    except Exception as exc:
        print(f"  Controller:  not reachable at {host} ({exc})")


def multipart_file(field: str, path: Path) -> tuple[bytes, str]:
    boundary = "----HydroESP" + uuid.uuid4().hex
    head = (
        f"--{boundary}\r\n"
        f"Content-Disposition: form-data; name=\"{field}\"; filename=\"{path.name}\"\r\n"
        "Content-Type: application/octet-stream\r\n\r\n"
    ).encode()
    tail = f"\r\n--{boundary}--\r\n".encode()
    return head + path.read_bytes() + tail, boundary


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def latest_release_binary() -> tuple[Path, tempfile.TemporaryDirectory]:
    release = get_json(f"https://api.github.com/repos/{REPO}/releases/latest", timeout=15)
    assets = release.get("assets", [])
    bins = [a for a in assets if a.get("name", "").endswith(".bin") and "hydro-esp-c3" in a.get("name", "")]
    sums = [a for a in assets if a.get("name", "").endswith(".sha256")]
    if not bins:
        raise HydroError("latest GitHub release contains no HydroESP-C3 .bin asset")

    asset = sorted(bins, key=lambda a: ("latest" not in a.get("name", ""), len(a.get("name", ""))))[0]
    temp = tempfile.TemporaryDirectory(prefix="hydroesp-")
    target = Path(temp.name) / asset["name"]
    say(f"downloading {release.get('tag_name', 'latest')} / {asset['name']}")
    target.write_bytes(request(asset["browser_download_url"], timeout=60))

    if sums:
        checksum_path = Path(temp.name) / sums[0]["name"]
        checksum_path.write_bytes(request(sums[0]["browser_download_url"], timeout=20))
        expected = None
        for line in checksum_path.read_text(encoding="utf-8").splitlines():
            parts = line.strip().split()
            if len(parts) >= 2 and Path(parts[-1].lstrip("*")).name == target.name:
                expected = parts[0].lower()
                break
        if expected:
            actual = sha256(target)
            if actual != expected:
                temp.cleanup()
                raise HydroError("SHA-256 mismatch for downloaded firmware")
            say("SHA-256 verified")
        else:
            say("warning: checksum file did not contain selected firmware name")
    else:
        say("warning: release has no .sha256 asset")

    return target, temp


def command_update(args) -> None:
    host = base_url(args.host)
    temp = None
    try:
        if args.file:
            firmware = Path(args.file).expanduser().resolve()
            if not firmware.is_file():
                raise HydroError(f"firmware not found: {firmware}")
        else:
            firmware, temp = latest_release_binary()

        if firmware.suffix.lower() != ".bin":
            raise HydroError("OTA file must have .bin extension")
        if firmware.stat().st_size < 10_000:
            raise HydroError("firmware file is unexpectedly small")

        say(f"uploading {firmware.name} ({firmware.stat().st_size // 1024} KiB) to {host}")
        body, boundary = multipart_file("file", firmware)
        result = request(host + "/ota/upload", method="POST", body=body,
                         headers={"Content-Type": f"multipart/form-data; boundary={boundary}"},
                         timeout=90).decode("utf-8", "replace").strip()
        if result != "OK":
            raise HydroError(f"device rejected OTA: {result or 'empty response'}")
        say("OTA accepted; controller is rebooting")
    finally:
        if temp:
            temp.cleanup()


def command_backup(args) -> None:
    host = base_url(args.host)
    status = get_json(host + "/api/status")
    schedule = get_json(host + "/api/schedule")
    config = get_json(host + "/api/config")
    hydraulics = get_json(host + "/api/hydraulics")
    payload = {
        "format": "hydroesp-backup-v2",
        "created_utc": dt.datetime.now(dt.timezone.utc).isoformat(),
        "device": {
            "version": status.get("version"),
            "build": status.get("build"),
            "ssid": config.get("ssid"),
            "tz": config.get("tz"),
        },
        "automation_enabled": bool(status.get("automation_enabled")),
        "hydraulics": hydraulics,
        "schedule": schedule,
    }
    target = Path(args.output).expanduser().resolve()
    target.write_text(json.dumps(payload, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    say(f"backup written to {target} (Wi-Fi password is never exported)")


def command_restore(args) -> None:
    source = Path(args.file).expanduser().resolve()
    payload = json.loads(source.read_text(encoding="utf-8"))
    fmt = payload.get("format")
    if fmt not in {"hydroesp-backup-v1", "hydroesp-backup-v2"} or not isinstance(payload.get("schedule"), list):
        raise HydroError("unsupported or invalid backup format")

    slots = payload["schedule"]
    if not args.yes:
        answer = input(f"Pause automation and replace device schedule with {len(slots)} slots? [y/N] ").strip().lower()
        if answer not in {"y", "yes"}:
            say("restore cancelled")
            return

    host = base_url(args.host)
    post_json(host + "/api/automation", {"enabled": False})
    post_json(host + "/api/schedule", slots)

    hydraulics = payload.get("hydraulics") if fmt == "hydroesp-backup-v2" else None
    if isinstance(hydraulics, dict):
        restored = {
            "flow_lpm": float(hydraulics.get("flow_lpm") or 0),
            "efficiency_pct": int(hydraulics.get("efficiency_pct") or 85),
        }
        if "sample_count" in hydraulics:
            restored["protocol_version"] = int(hydraulics.get("protocol_version") or 0)
            restored["sample_count"] = int(hydraulics.get("sample_count") or 0)
            restored["cv_pct"] = float(hydraulics.get("cv_pct") or 0)
            restored["calibration_epoch"] = int(hydraulics.get("calibration_epoch") or 0)
        post_json(host + "/api/hydraulics", restored)
        say("hydraulic calibration, protocol and repeatability metadata restored")

    if args.resume_automation:
        post_json(host + "/api/automation", {"enabled": True})
        say("automation resumed by explicit request")
    else:
        say("automation left paused for verification")
    say("schedule restored")


def parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(description="Install, diagnose and update HydroESP-C3")
    sub = p.add_subparsers(dest="command", required=True)

    s = sub.add_parser("bootstrap", help="install PlatformIO into an isolated project venv")
    s.set_defaults(func=command_bootstrap)

    for name, help_text, func in [
        ("build", "build firmware", command_build),
        ("install", "build and flash over USB", command_install),
    ]:
        s = sub.add_parser(name, help=help_text)
        s.add_argument("--clean", action="store_true", help="clean PlatformIO build first")
        s.add_argument("--timezone", type=int, choices=range(-12, 15), metavar="UTC", help="UTC offset (-12..14)")
        if name == "install":
            s.add_argument("--port", help="serial port; auto-detected when omitted")
        s.set_defaults(func=func)

    s = sub.add_parser("monitor", help="open 115200 baud serial monitor")
    s.add_argument("--port")
    s.set_defaults(func=command_monitor)

    for name, help_text, func in [
        ("status", "show controller status", command_status),
        ("doctor", "check toolchain, serial and controller", command_doctor),
        ("events", "show current-session operation events", command_events),
        ("pause", "pause timer automation", command_pause),
        ("resume", "resume timer automation", command_resume),
        ("backup", "export safe device backup", command_backup),
        ("update", "OTA update from a file or latest GitHub release", command_update),
        ("restore", "restore schedule and calibration from backup", command_restore),
    ]:
        s = sub.add_parser(name, help=help_text)
        s.add_argument("--host", default=DEFAULT_HOST, help=f"controller URL (default: {DEFAULT_HOST})")
        if name == "backup":
            s.add_argument("--output", default="hydroesp-backup.json")
        elif name == "update":
            s.add_argument("--file", help="local firmware.bin; latest GitHub release is used when omitted")
        elif name == "restore":
            s.add_argument("file")
            s.add_argument("--yes", action="store_true")
            s.add_argument("--resume-automation", action="store_true",
                           help="resume timer automation after restore; default is to leave it paused")
        s.set_defaults(func=func)

    return p


def main() -> int:
    try:
        args = parser().parse_args()
        args.func(args)
        return 0
    except (HydroError, OSError, ValueError, json.JSONDecodeError) as exc:
        print(f"[hydroctl] ERROR: {exc}", file=sys.stderr)
        return 2
    except KeyboardInterrupt:
        print("\n[hydroctl] cancelled", file=sys.stderr)
        return 130


if __name__ == "__main__":
    raise SystemExit(main())
