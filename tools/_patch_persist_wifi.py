from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
path = ROOT / 'tools' / 'hydroctl.py'
s = path.read_text(encoding='utf-8')

old = '''def load_dotenv(env: dict[str, str]) -> None:\n    path = ROOT / ".env"\n    if not path.is_file():\n        return\n    allowed = {"WIFI_SSID", "WIFI_PASSWORD", "TIMEZONE_OFFSET"}\n    for raw in path.read_text(encoding="utf-8").splitlines():\n        line = raw.strip()\n        if not line or line.startswith("#") or "=" not in line:\n            continue\n        key, value = line.split("=", 1)\n        key = key.strip()\n        if key not in allowed or key in env:\n            continue\n        value = value.strip()\n        if len(value) >= 2 and value[0] == value[-1] and value[0] in {"\\\"", "'"}:\n            value = value[1:-1]\n        env[key] = value\n\n\ndef build_env(args, *, prompt_wifi: bool = False) -> dict[str, str]:\n'''
new = '''def load_dotenv(env: dict[str, str]) -> None:\n    path = ROOT / ".env"\n    if not path.is_file():\n        return\n    allowed = {"WIFI_SSID", "WIFI_PASSWORD", "TIMEZONE_OFFSET"}\n    for raw in path.read_text(encoding="utf-8").splitlines():\n        line = raw.strip()\n        if not line or line.startswith("#") or "=" not in line:\n            continue\n        key, value = line.split("=", 1)\n        key = key.strip()\n        if key not in allowed or key in env:\n            continue\n        value = value.strip()\n        if len(value) >= 2 and value[0] == value[-1] == '\"':\n            try:\n                value = json.loads(value)\n            except json.JSONDecodeError:\n                value = value[1:-1]\n        elif len(value) >= 2 and value[0] == value[-1] == "'":\n            value = value[1:-1]\n        env[key] = value\n\n\ndef save_wifi_dotenv(ssid: str, password: str) -> None:\n    path = ROOT / ".env"\n    values = {\n        "WIFI_SSID": json.dumps(ssid, ensure_ascii=False),\n        "WIFI_PASSWORD": json.dumps(password, ensure_ascii=False),\n    }\n    lines = path.read_text(encoding="utf-8").splitlines() if path.is_file() else [\n        "# Local HydroESP-C3 settings. This file is ignored by Git.",\n    ]\n    seen: set[str] = set()\n    result: list[str] = []\n    for line in lines:\n        stripped = line.strip()\n        if "=" in stripped and not stripped.startswith("#"):\n            key = stripped.split("=", 1)[0].strip()\n            if key in values:\n                result.append(f"{key}={values[key]}")\n                seen.add(key)\n                continue\n        result.append(line)\n    for key in ("WIFI_SSID", "WIFI_PASSWORD"):\n        if key not in seen:\n            result.append(f"{key}={values[key]}")\n    path.write_text("\\n".join(result).rstrip() + "\\n", encoding="utf-8")\n    if os.name != "nt":\n        path.chmod(0o600)\n    say("Wi-Fi saved to local .env for future installs")\n\n\ndef build_env(args, *, prompt_wifi: bool = False) -> dict[str, str]:\n'''
if old not in s:
    raise SystemExit('dotenv block not found')
s = s.replace(old, new, 1)

old2 = '''    if prompt_wifi and not ssid and sys.stdin.isatty():\n        ssid = input("Home Wi-Fi SSID (Enter = configure later via HydroESP-Setup): ").strip()\n        if ssid:\n            env["WIFI_SSID"] = ssid\n\n    if prompt_wifi and ssid and not password_set and sys.stdin.isatty():\n        env["WIFI_PASSWORD"] = getpass.getpass(\n            "Home Wi-Fi password (Enter if the network is open): "\n        )\n        password_set = True\n\n    if ssid:\n'''
new2 = '''    prompted_ssid = False\n    prompted_password = False\n    if prompt_wifi and not ssid and sys.stdin.isatty():\n        ssid = input("Home Wi-Fi SSID (Enter = configure later via HydroESP-Setup): ").strip()\n        if ssid:\n            env["WIFI_SSID"] = ssid\n            prompted_ssid = True\n\n    if prompt_wifi and ssid and not password_set and sys.stdin.isatty():\n        env["WIFI_PASSWORD"] = getpass.getpass(\n            "Home Wi-Fi password (Enter if the network is open): "\n        )\n        password_set = True\n        prompted_password = True\n\n    if prompt_wifi and ssid and (prompted_ssid or prompted_password):\n        save_wifi_dotenv(ssid, env.get("WIFI_PASSWORD", ""))\n\n    if ssid:\n'''
if old2 not in s:
    raise SystemExit('prompt block not found')
s = s.replace(old2, new2, 1)
path.write_text(s, encoding='utf-8')

test = ROOT / 'tools' / 'check_install_wifi.py'
t = test.read_text(encoding='utf-8')
marker = 'print("install-wifi check: OK")\n'
extra = '''# Interactive credentials are persisted locally and round-trip safely.\norig_root = hydroctl.ROOT\ntry:\n    hydroctl.ROOT = tmp\n    hydroctl.save_wifi_dotenv("saved net", "p@ss # with spaces")\n    persisted = {}\n    hydroctl.load_dotenv(persisted)\n    assert persisted["WIFI_SSID"] == "saved net"\n    assert persisted["WIFI_PASSWORD"] == "p@ss # with spaces"\nfinally:\n    hydroctl.ROOT = orig_root\n\n'''
if marker not in t:
    raise SystemExit('test marker not found')
t = t.replace(marker, extra + marker, 1)
test.write_text(t, encoding='utf-8')

doc = ROOT / 'docs' / 'INSTALL.md'
d = doc.read_text(encoding='utf-8')
needle = '`install` просто собирает и прошивает контроллер. Он **не стирает flash и не удаляет сохранённые настройки**.\n'
addition = '\nПри первом интерактивном вводе SSID и пароля установщик сохраняет их в локальный `.env` (он исключён из Git). Следующие установки используют их автоматически и больше не спрашивают.\n'
if needle in d and 'Следующие установки используют их автоматически' not in d:
    d = d.replace(needle, needle + addition, 1)
doc.write_text(d, encoding='utf-8')
