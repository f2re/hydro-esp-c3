from pathlib import Path

p = Path('tools/hydroctl.py')
s = p.read_text(encoding='utf-8')
old = '''    # `install` means a predictable first installation. Old NVS/Wi-Fi settings\n    # are removed so the next boot always opens HydroESP-Setup. Use\n    # --keep-settings only for an intentional USB reinstall.\n    if not args.keep_settings:\n        say("fresh install: clearing old controller settings")\n        erase = [pio, "run", "-t", "erase"]\n        if args.port:\n            erase += ["--upload-port", args.port]\n        run(erase, env=env)\n\n    cmd = [pio, "run", "-t", "upload"]\n    if args.port:\n        cmd += ["--upload-port", args.port]\n    run(cmd, env=env)\n\n    say("installation complete")\n'''
new = '''    cmd = [pio, "run", "-t", "upload"]\n    if args.port:\n        cmd += ["--upload-port", args.port]\n    run(cmd, env=env)\n\n    # A fresh install clears only the NVS partition, not the whole flash.\n    # The uploaded app/bootloader remain intact and the next boot predictably\n    # enters HydroESP-Setup with factory defaults.\n    if not args.keep_settings:\n        say("fresh install: clearing saved Wi-Fi/settings")\n        erase = [pio, "pkg", "exec", "--", "esptool", "--chip", "esp32c3"]\n        if args.port:\n            erase += ["--port", args.port]\n        erase += ["erase-region", "0x9000", "0x4000"]\n        run(erase, env=env)\n\n    say("installation complete")\n'''
if old not in s:
    raise SystemExit('install erase block not found')
p.write_text(s.replace(old, new, 1), encoding='utf-8')
