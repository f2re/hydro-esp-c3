Import("env")

import hashlib
import os
import subprocess


def git(*args, fallback="unknown"):
    try:
        return subprocess.check_output(
            ["git", *args],
            cwd=env.subst("$PROJECT_DIR"),
            stderr=subprocess.DEVNULL,
            text=True,
        ).strip()
    except Exception:
        return fallback


def c_string(value: str) -> str:
    value = value.replace("\\", "\\\\").replace('"', '\\"')
    return f'\\"{value}\\"'


version = os.getenv("HYDRO_VERSION") or git("describe", "--tags", "--always", "--dirty")
sha = os.getenv("GITHUB_SHA") or git("rev-parse", "HEAD")
sha = sha[:8] if sha and sha != "unknown" else "unknown"

flags = [
    f"-DHYDRO_VERSION={c_string(version)}",
    f"-DHYDRO_BUILD_SHA={c_string(sha)}",
]

# Wi-Fi can be injected by hydroctl/install.sh from environment variables or a
# local .env file. WIFI_SEED_ID makes those credentials a one-shot provisioning
# seed: they are copied into NVS once, then later Web UI edits are respected.
ssid = os.getenv("WIFI_SSID", "")
password = os.getenv("WIFI_PASSWORD", "")
if ssid:
    seed_id = os.getenv("WIFI_SEED_ID", "").strip()
    if not seed_id:
        seed_id = hashlib.sha256((ssid + "\0" + password).encode("utf-8")).hexdigest()[:16]
    flags.append(f"-DWIFI_SSID={c_string(ssid)}")
    flags.append(f"-DWIFI_PASSWORD={c_string(password)}")
    flags.append(f"-DWIFI_SEED_ID={c_string(seed_id)}")

tz = os.getenv("TIMEZONE_OFFSET")
if tz:
    try:
        offset = int(tz)
        if -12 <= offset <= 14:
            flags.append(f"-DTIMEZONE_OFFSET={offset}")
    except ValueError:
        print("[build] Ignoring invalid TIMEZONE_OFFSET; expected integer -12..14")

env.Append(BUILD_FLAGS=flags)
print(f"[build] HydroESP-C3 {version} ({sha})")
