# Hardware baseline

Для ESP32-C3 Super Mini восстановлен аппаратный baseline, существовавший до августовской модернизации.

- pioarduino: `53.03.10`;
- relay: GPIO 4, active HIGH (`ON=HIGH`, `OFF=LOW`);
- relay GPIO: `OUTPUT_OPEN_DRAIN`;
- OLED: SSD1306 72×40 logical window, SDA GPIO 5, SCL GPIO 6;
- LED: GPIO 8;
- BOOT: GPIO 9;
- USB CDC on boot enabled.

Критический boot-инвариант: GPIO реле переводится в `RELAY_OFF` первой операцией `setup()`, до Serial, NVS, OLED, Wi-Fi и HTTP. OLED инициализируется до чтения NVS, чтобы boot-ошибка была видна локально.

Обновлять platform/framework или менять GPIO/полярность можно только отдельным PR после стендового smoke-test: power-on, OLED boot screen, Wi-Fi STA/AP, HTTP `/ping`, relay OFF at boot, timed relay start/stop.
