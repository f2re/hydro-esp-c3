from pathlib import Path

p = Path('tools/hydroctl.py')
s = p.read_text(encoding='utf-8')
old = '''    cmd = [pio, "run", "-t", "upload"]
    if args.port:
        cmd += ["--upload-port", args.port]
    run(cmd, env=env)

    # A fresh install clears only the NVS partition, not the whole flash.
    # The uploaded app/bootloader remain intact and the next boot predictably
    # enters HydroESP-Setup with factory defaults.
    if not args.keep_settings:
        say("fresh install: clearing saved Wi-Fi/settings")
        erase = [pio, "pkg", "exec", "--", "esptool", "--chip", "esp32c3"]
        if args.port:
            erase += ["--port", args.port]
        erase += ["erase-region", "0x9000", "0x4000"]
        run(erase, env=env)

    say("installation complete")
'''
new = '''    # A fresh install clears only the NVS partition, not the whole flash.
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
    if args.port:
        cmd += ["--upload-port", args.port]
    run(cmd, env=env)

    say("installation complete")
'''
if old not in s:
    raise SystemExit('NVS install block not found')
p.write_text(s.replace(old, new, 1), encoding='utf-8')
