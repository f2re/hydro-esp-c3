from pathlib import Path

p = Path('tools/hydroctl.py')
s = p.read_text(encoding='utf-8')

old = '''    # A fresh install clears only the NVS partition, not the whole flash.
    # Do it before upload so the serial port is still in the same predictable
    # state; the following upload then boots with factory defaults.
    if not args.keep_settings:
        say("fresh install: clearing saved Wi-Fi/settings")
        erase = [pio, "pkg", "exec", "--", "esptool", "--chip", "esp32c3"]
        if args.port:
            erase += ["--port", args.port]
        erase += ["erase-region", "0x9000", "0x4000"]
        run(erase, env=env)

    cmd = [pio, "run", "-t", "upload"]
'''
new = '''    # Installation is intentionally non-destructive: no flash/NVS erase.
    # A new board enters HydroESP-Setup; a configured board keeps its settings.
    cmd = [pio, "run", "-t", "upload"]
'''
if old not in s:
    raise SystemExit('NVS reset block not found')
s = s.replace(old, new, 1)

old = '''    say("installation complete")
    if args.factory_wifi:
        say("Open http://hydro.local or the numeric IP shown on OLED")
    else:
        say("Connect to Wi-Fi 'HydroESP-Setup' (no password)")
        say("Open http://192.168.4.1")
'''
new = '''    say("installation complete")
    say("Open the address shown on OLED")
    if not args.factory_wifi:
        say("New/unconfigured board: Wi-Fi 'HydroESP-Setup' (no password), http://192.168.4.1")
'''
if old not in s:
    raise SystemExit('install final message block not found')
s = s.replace(old, new, 1)

old = '''        if name == "install":
            s.add_argument("--port", help="serial port; auto-detected when omitted")
            s.add_argument("--keep-settings", action="store_true",
                           help="USB reinstall without clearing saved Wi-Fi/settings")
'''
new = '''        if name == "install":
            s.add_argument("--port", help="serial port; auto-detected when omitted")
'''
if old not in s:
    raise SystemExit('keep-settings parser block not found')
s = s.replace(old, new, 1)

p.write_text(s, encoding='utf-8')
