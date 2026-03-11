#include <Arduino.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "config.h"
#include "wifi_manager.h"
#include "ntp_manager.h"
#include "relay_controller.h"
#include "scheduler.h"
#include "status_display.h"   // ANSI Serial
#include "oled_display.h"     // OLED

WiFiManager     wifiMgr;
NTPManager      ntpMgr;
RelayController relay;
Scheduler       scheduler;
StatusDisplay   serial;
OledDisplay     oled;

unsigned long lastSerial = 0;
unsigned long lastOled   = 0;
bool          btnPrev    = HIGH;

void setup() {
    // Реле ВЫКЛ первым делом
    pinMode(RELAY_PIN, OUTPUT_OPEN_DRAIN);
    digitalWrite(RELAY_PIN, LOW);

    // Отключаем Brown-out детектор для стабильности
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

    Serial.begin(115200);
    delay(1500);

    pinMode(LED_PIN,    OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    digitalWrite(LED_PIN, HIGH);

    oled.begin();

    // ── Загрузочная последовательность ──
    serial.printBoot();
    oled.drawBoot(1, "Init...");

    relay.begin();

    oled.drawBoot(2, "WiFi...");
    serial.printBootStep("📡", "WiFi", false, "");
    bool wOk = wifiMgr.connect(WIFI_SSID, WIFI_PASSWORD, WIFI_TIMEOUT_MS);
    serial.printBootStep("📡", "WiFi", wOk, wifiMgr.localIP());
    oled.drawBoot(2, wOk ? "WiFi OK" : "WiFi FAIL");

    oled.drawBoot(3, "NTP...");
    if (wOk) {
        ntpMgr.begin();
        serial.printBootStep("🕐", "NTP",
            ntpMgr.isSynced(), ntpMgr.getTimeString());
        oled.drawBoot(3, ntpMgr.isSynced() ? "NTP OK" : "NTP wait");
    }

    scheduler.begin(&relay, &ntpMgr);
    oled.drawBoot(4, "Ready!");
    serial.printSchedule();
    delay(1500);
}

void loop() {
    wifiMgr.ensureConnected(WIFI_SSID, WIFI_PASSWORD);
    ntpMgr.update();
    relay.update();
    scheduler.update();

    // ── LED: мигает при поливе, выкл в покое ─────────────
    if (relay.isOn()) {
        digitalWrite(LED_PIN, (millis() / 500) % 2 == 0 ? LOW : HIGH);
    } else {
        digitalWrite(LED_PIN, HIGH);
    }

    // ── BOOT кнопка: ручное переключение страниц или ручной полив ──
    bool btnNow = digitalRead(BUTTON_PIN);
    if (btnPrev == HIGH && btnNow == LOW) { // нажата
        if (relay.isOn()) {
            // Если полив идет - выключаем его
            relay.off();
            Serial.println("[BTN] Manual stop");
        } else {
            // Если полив не идет - переключаем страницу OLED
            DisplayPage nextPage = (DisplayPage)((oled.currentPage() + 1) % PAGE_COUNT);
            oled.showPage(nextPage);
            Serial.printf("[BTN] Page switch to %d\n", nextPage);
        }
    }
    // Длинное нажатие или другая логика для ручного запуска полива может быть добавлена здесь,
    // но для простоты оставим текущую логику кнопки.
    btnPrev = btnNow;

    // ── OLED: обновление каждую секунду (внутри метода update своя логика ротации) ──
    if (millis() - lastOled >= 1000) {
        lastOled = millis();
        oled.update(&ntpMgr, &relay, &wifiMgr);
    }

    // ── Serial ANSI: каждые 5 сек ─────────────────────────
    if (millis() - lastSerial >= 5000) {
        lastSerial = millis();
        serial.draw(&ntpMgr, &relay, &wifiMgr);
    }

    delay(20);
}
