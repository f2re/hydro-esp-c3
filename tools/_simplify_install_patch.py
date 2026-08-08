from pathlib import Path

p = Path('tools/hydroctl.py')
s = p.read_text(encoding='utf-8')

old = '''def command_install(args) -> None:\n    pio = ensure_pio(True)\n    env = build_env(args)\n    if args.clean:\n        run([pio, "run", "-t", "clean"], env=env)\n    run([pio, "run"], env=env)\n    cmd = [pio, "run", "-t", "upload"]\n    if args.port:\n        cmd += ["--upload-port", args.port]\n    run(cmd, env=env)\n    say("flash completed")\n    if args.factory_wifi:\n        say("Open http://hydro.local after the controller joins Wi-Fi")\n    else:\n        say("First boot: HydroESP-Setup is protected; read the KEY on OLED or Serial")\n        say("Then connect to HydroESP-Setup and open http://192.168.4.1")\n'''
new = '''def command_install(args) -> None:\n    pio = ensure_pio(True)\n    env = build_env(args)\n    if args.clean:\n        run([pio, "run", "-t", "clean"], env=env)\n\n    # Build first: never erase a working controller if the source does not compile.\n    run([pio, "run"], env=env)\n\n    # `install` means a predictable first installation. Old NVS/Wi-Fi settings\n    # are removed so the next boot always opens HydroESP-Setup. Use\n    # --keep-settings only for an intentional USB reinstall.\n    if not args.keep_settings:\n        say("fresh install: clearing old controller settings")\n        erase = [pio, "run", "-t", "erase"]\n        if args.port:\n            erase += ["--upload-port", args.port]\n        run(erase, env=env)\n\n    cmd = [pio, "run", "-t", "upload"]\n    if args.port:\n        cmd += ["--upload-port", args.port]\n    run(cmd, env=env)\n\n    say("installation complete")\n    if args.factory_wifi:\n        say("Open http://hydro.local or the numeric IP shown on OLED")\n    else:\n        say("Connect to Wi-Fi 'HydroESP-Setup' (no password)")\n        say("Open http://192.168.4.1")\n'''
if old not in s:
    raise SystemExit('command_install block not found')
s = s.replace(old, new, 1)

old = '''        if name == "install":\n            s.add_argument("--port", help="serial port; auto-detected when omitted")\n        s.set_defaults(func=func)\n'''
new = '''        if name == "install":\n            s.add_argument("--port", help="serial port; auto-detected when omitted")\n            s.add_argument("--keep-settings", action="store_true",\n                           help="USB reinstall without clearing saved Wi-Fi/settings")\n        s.set_defaults(func=func)\n'''
if old not in s:
    raise SystemExit('install parser block not found')
s = s.replace(old, new, 1)

p.write_text(s, encoding='utf-8')
