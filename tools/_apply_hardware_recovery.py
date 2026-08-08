from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

pio = ROOT / 'platformio.ini'
s = pio.read_text(encoding='utf-8')
s = s.replace('55.03.37/platform-espressif32.zip', '53.03.10/platform-espressif32.zip')
s = s.replace('; Pinned pioarduino platform: Arduino-ESP32 3.x / ESP-IDF 5.x.\n; Do not switch to an unpinned "stable" URL in production builds.', '; Known-working hardware baseline from the pre-August controller.\n; Bump only after a real ESP32-C3/OLED/relay/Wi-Fi smoke-test.')
pio.write_text(s, encoding='utf-8')

main = ROOT / 'src/main.cpp'
s = main.read_text(encoding='utf-8')
old = '''void setup() {\n#if HYDRO_DISABLE_BROWNOUT_WORKAROUND\n    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);\n#endif\n\n    Serial.begin(115200);\n    delay(1000);\n    Serial.printf("\\n[HydroESP] %s (%s), API v%d\\n",\n                  HYDRO_VERSION, HYDRO_BUILD_SHA, HYDRO_API_VERSION);\n\n    pinMode(LED_PIN, OUTPUT);\n    pinMode(BUTTON_PIN, INPUT_PULLUP);\n    digitalWrite(LED_PIN, HIGH);\n    buttonRaw = buttonStable = digitalRead(BUTTON_PIN);\n    buttonArmed = buttonStable == HIGH;\n\n    configStorage.begin();\n    configStorage.load(appConfig);\n    eventLog.begin(&ntpMgr);\n\n    oled.begin();\n    serial.printBoot();\n    oled.drawBoot(1, "Init...");\n\n    relay.begin(&eventLog);\n'''
new = '''void setup() {\n    // Hardware fail-safe first. This is intentionally the first GPIO action:\n    // the pump must be OFF even if NVS, OLED, Wi-Fi or HTTP initialization fails.\n    pinMode(RELAY_PIN, OUTPUT_OPEN_DRAIN);\n    digitalWrite(RELAY_PIN, RELAY_OFF);\n\n#if HYDRO_DISABLE_BROWNOUT_WORKAROUND\n    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);\n#endif\n\n    Serial.begin(115200);\n    delay(1000);\n    Serial.printf("\\n[HydroESP] %s (%s), API v%d\\n",\n                  HYDRO_VERSION, HYDRO_BUILD_SHA, HYDRO_API_VERSION);\n    Serial.println("[BOOT] relay forced OFF");\n\n    pinMode(LED_PIN, OUTPUT);\n    pinMode(BUTTON_PIN, INPUT_PULLUP);\n    digitalWrite(LED_PIN, HIGH);\n    buttonRaw = buttonStable = digitalRead(BUTTON_PIN);\n    buttonArmed = buttonStable == HIGH;\n\n    // Bring the local display up before touching NVS. A broken configuration\n    // must not turn into a completely black/undebuggable device.\n    oled.begin();\n    oled.drawBoot(1, "Boot...");\n    Serial.println("[BOOT] OLED initialized");\n\n    configStorage.begin();\n    configStorage.load(appConfig);\n    Serial.println("[BOOT] config loaded");\n    eventLog.begin(&ntpMgr);\n\n    serial.printBoot();\n    oled.drawBoot(1, "Init...");\n\n    relay.begin(&eventLog);\n    Serial.println("[BOOT] relay controller ready/OFF");\n'''
if old not in s:
    raise SystemExit('main setup baseline block not found')
main.write_text(s.replace(old, new, 1), encoding='utf-8')

wf = ROOT / '.github/workflows/build.yml'
s = wf.read_text(encoding='utf-8')
s = s.replace('python -m py_compile tools/hydroctl.py tools/check_web_ui.py tools/export_ui_preview.py tools/make_factory_image.py scripts/build_flags.py', 'python -m py_compile tools/hydroctl.py tools/check_web_ui.py tools/check_install_wifi.py tools/check_hardware_baseline.py tools/export_ui_preview.py tools/make_factory_image.py scripts/build_flags.py')
s = s.replace('python tools/hydroctl.py install --help >/dev/null', 'python tools/hydroctl.py install --help >/dev/null\n          python tools/check_install_wifi.py\n          python tools/check_hardware_baseline.py')
wf.write_text(s, encoding='utf-8')
