#include <Arduino.h>
#include "config.h"
#include "wifi_manager.h"
#include "ntp_manager.h"
#include "relay_controller.h"
#include "scheduler.h"
#include "status_display.h"

WiFiManager  wifiMgr;
NTPManager   ntpMgr;
RelayController relay;
Scheduler    scheduler;
StatusDisplay display;
unsigned long lastStatus = 0;
const unsigned long DRAW_INTERVAL = 5000;

void setup() {
    Serial.begin(115200);
    delay(1000);

    display.printBoot();
    relay.begin();

    display.printBootStep("📡", "Подключение к WiFi...", false, "");
    bool wOk = wifiMgr.connect(WIFI_SSID, WIFI_PASSWORD, WIFI_TIMEOUT_MS);
    display.printBootStep("📡", "WiFi", wOk, wifiMgr.localIP());

    if (wOk) {
        ntpMgr.begin();
        display.printBootStep("🕐", "NTP синхронизация",
            ntpMgr.isSynced(), ntpMgr.getTimeString());
    }

    scheduler.begin(&relay, &ntpMgr);
    delay(2000); // показать итог загрузки 2 сек, потом дашборд
}

void loop() {
    wifiMgr.ensureConnected(WIFI_SSID, WIFI_PASSWORD);
    ntpMgr.update();
    relay.update();
    scheduler.update();

    if (millis() - lastStatus >= DRAW_INTERVAL) {
        lastStatus = millis();
        display.draw(&ntpMgr, &relay, &wifiMgr);
    }
    delay(100);
}
