from pathlib import Path

p = Path('tools/hydroctl.py')
s = p.read_text(encoding='utf-8')

old = '''def build_env(args) -> dict[str, str]:\n    env = os.environ.copy()\n    if getattr(args, "timezone", None) is not None:\n        env["TIMEZONE_OFFSET"] = str(args.timezone)\n    if getattr(args, "factory_wifi", False):\n        ssid = input("Factory Wi-Fi SSID: ").strip()\n        if not ssid:\n            raise HydroError("SSID cannot be empty when --factory-wifi is used")\n        password = getpass.getpass("Factory Wi-Fi password (may be empty): ")\n        env["WIFI_SSID"] = ssid\n        env["WIFI_PASSWORD"] = password\n    return env\n'''
new = '''def load_dotenv(env: dict[str, str]) -> None:\n    path = ROOT / ".env"\n    if not path.is_file():\n        return\n    allowed = {"WIFI_SSID", "WIFI_PASSWORD", "TIMEZONE_OFFSET"}\n    for raw in path.read_text(encoding="utf-8").splitlines():\n        line = raw.strip()\n        if not line or line.startswith("#") or "=" not in line:\n            continue\n        key, value = line.split("=", 1)\n        key = key.strip()\n        if key not in allowed or key in env:\n            continue\n        value = value.strip()\n        if len(value) >= 2 and value[0] == value[-1] and value[0] in {"\\\"", "'"}:\n            value = value[1:-1]\n        env[key] = value\n\n\ndef build_env(args, *, prompt_wifi: bool = False) -> dict[str, str]:\n    env = os.environ.copy()\n    load_dotenv(env)\n\n    if getattr(args, "timezone", None) is not None:\n        env["TIMEZONE_OFFSET"] = str(args.timezone)\n\n    ssid = env.get("WIFI_SSID", "").strip()\n    password_set = "WIFI_PASSWORD" in env\n\n    if prompt_wifi and not ssid and sys.stdin.isatty():\n        ssid = input("Home Wi-Fi SSID (Enter = configure later via HydroESP-Setup): ").strip()\n        if ssid:\n            env["WIFI_SSID"] = ssid\n\n    if prompt_wifi and ssid and not password_set and sys.stdin.isatty():\n        env["WIFI_PASSWORD"] = getpass.getpass(\n            "Home Wi-Fi password (Enter if the network is open): "\n        )\n        password_set = True\n\n    if ssid:\n        env["WIFI_SSID"] = ssid\n        env.setdefault("WIFI_PASSWORD", "")\n        if prompt_wifi:\n            # A new token makes installer-provided credentials apply once even\n            # when the controller already has older Wi-Fi settings in NVS.\n            env["WIFI_SEED_ID"] = uuid.uuid4().hex[:16]\n        say(f"Wi-Fi for this build: {ssid}")\n    elif prompt_wifi:\n        env.pop("WIFI_SSID", None)\n        env.pop("WIFI_PASSWORD", None)\n        env.pop("WIFI_SEED_ID", None)\n        say("Wi-Fi not supplied; first setup will use HydroESP-Setup")\n\n    return env\n'''
if old not in s:
    raise SystemExit('build_env block not found')
s = s.replace(old, new, 1)

old = '''def command_install(args) -> None:\n    pio = ensure_pio(True)\n    env = build_env(args)\n'''
new = '''def command_install(args) -> None:\n    pio = ensure_pio(True)\n    env = build_env(args, prompt_wifi=True)\n'''
if old not in s:
    raise SystemExit('command_install header not found')
s = s.replace(old, new, 1)

old = '''    say("installation complete")\n    say("Open the address shown on OLED")\n    if not args.factory_wifi:\n        say("New/unconfigured board: Wi-Fi 'HydroESP-Setup' (no password), http://192.168.4.1")\n'''
new = '''    say("installation complete")\n    if env.get("WIFI_SSID"):\n        say(f"Controller will try Wi-Fi '{env['WIFI_SSID']}'; open the IP shown on OLED")\n    else:\n        say("Connect to Wi-Fi 'HydroESP-Setup' (no password), then open http://192.168.4.1")\n'''
if old not in s:
    raise SystemExit('install final message block not found')
s = s.replace(old, new, 1)

old = '''        s.add_argument("--clean", action="store_true", help="clean PlatformIO build first")\n        s.add_argument("--factory-wifi", action="store_true", help="prompt for Wi-Fi credentials to embed as factory defaults")\n        s.add_argument("--timezone", type=int, choices=range(-12, 15), metavar="UTC", help="factory UTC offset (-12..14)")\n'''
new = '''        s.add_argument("--clean", action="store_true", help="clean PlatformIO build first")\n        s.add_argument("--timezone", type=int, choices=range(-12, 15), metavar="UTC", help="UTC offset (-12..14)")\n'''
if old not in s:
    raise SystemExit('parser Wi-Fi block not found')
s = s.replace(old, new, 1)

p.write_text(s, encoding='utf-8')
