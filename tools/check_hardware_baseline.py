#!/usr/bin/env python3
"""Guard the ESP32-C3 hardware/boot contract that was proven on the real setup."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

platformio = (ROOT / "platformio.ini").read_text(encoding="utf-8")
config = (ROOT / "src/config.h").read_text(encoding="utf-8")
main = (ROOT / "src/main.cpp").read_text(encoding="utf-8")
oled = (ROOT / "src/oled_display.cpp").read_text(encoding="utf-8")

assert "53.03.10/platform-espressif32.zip" in platformio, "hardware baseline platform drifted"
for token in (
    "#define RELAY_PIN    4",
    "#define RELAY_ON     HIGH",
    "#define RELAY_OFF    LOW",
    "#define OLED_SDA     5",
    "#define OLED_SCL     6",
    "#define LED_PIN      8",
    "#define BUTTON_PIN   9",
):
    assert token in config, f"hardware contract drifted: {token}"

safe_pin = main.index("pinMode(RELAY_PIN, OUTPUT_OPEN_DRAIN);")
safe_off = main.index("digitalWrite(RELAY_PIN, RELAY_OFF);")
serial = main.index("Serial.begin(115200);")
config_load = main.index("configStorage.begin();")
oled_begin = main.index("oled.begin();")

assert safe_pin < safe_off < serial, "relay must be forced OFF before Serial/other initialization"
assert oled_begin < config_load, "OLED must start before NVS/config loading for boot visibility"
assert "Wire.begin(OLED_SDA, OLED_SCL);" in oled, "OLED I2C pins changed"

print("hardware baseline check: OK")
