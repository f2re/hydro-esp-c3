#include <Arduino.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "config.h"
#include "wifi_manager.h"
#include "ntp_manager.h"
#include "relay_controller.h"
#include "scheduler.h"
#include "status_display.h"
#include "oled_display.h"
#include "config_storage.h"
#include <ESPmDNS.h>
#include "web_server.h"

WiFiManager      wifiMgr;
NTPManager       ntpMgr;
RelayController  relay;
Scheduler        scheduler;
StatusDisplay    serial;
OledDisplay      oled;
WebServerManager webSrv;

unsigned long lastSerial = 0;
unsigned long lastOled   = 0;
bool          btnPrev    = HIGH;
Config        appConfig;

void setup() {
    // Реле ВЫКЛ первым делом
    pinMode(RELAY_PIN, OUTPUT_OPEN_DRAIN);
    digitalWrite(RELAY_PIN, RELAY_OFF);

    // Отключаем Brown-out детектор
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

    Serial.begin(115200);
    delay(1500);

    pinMode(LED_PIN,    OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    digitalWrite(LED_PIN, HIGH);

    configStorage.begin();
    configStorage.load(appConfig);

    oled.begin();
    serial.printBoot();
    oled.drawBoot(1, "Init...");

    relay.begin();
    scheduler.begin(&relay, &ntpMgr);
    scheduler.updateConfig(appConfig.schedule, appConfig.schedule_count);

    oled.drawBoot(2, "WiFi...");
    serial.printBootStep("📡", "WiFi", false, "");
    
    bool wOk = wifiMgr.connect(appConfig.wifi_ssid.c_str(), appConfig.wifi_pass.c_str(), WIFI_TIMEOUT_MS);
    if (!wOk) {
        serial.printBootStep("📡", "WiFi", false, "Starting AP...");
        oled.drawBoot(2, "AP Mode");
        wifiMgr.startAP();
    } else {
        serial.printBootStep("📡", "WiFi", true, wifiMgr.localIP());
        oled.drawBoot(2, "WiFi OK");
    }

    oled.drawBoot(3, "NTP...");
    ntpMgr.begin();
    ntpMgr.setTimeOffset(appConfig.timezone_offset);
    
    if (wOk) {
        serial.printBootStep("🕐", "NTP", ntpMgr.isSynced(), ntpMgr.getTimeString());
        oled.drawBoot(3, ntpMgr.isSynced() ? "NTP OK" : "NTP wait");
    }

    webSrv.begin(&relay, &scheduler, &ntpMgr, &wifiMgr);
    if (MDNS.begin("hydro")) {
        Serial.println("[mDNS] Responding at http://hydro.local");
    }

    oled.drawBoot(4, "Ready!");
    serial.printSchedule();
    delay(1500);
}

void loop() {
    if (!wifiMgr.isAPMode()) {
        wifiMgr.ensureConnected(appConfig.wifi_ssid.c_str(), appConfig.wifi_pass.c_str());
    } else {
        wifiMgr.updateDNS();
    }
    
    ntpMgr.update();
    relay.update();
    scheduler.update();

    // LED
    if (relay.isOn()) {
        digitalWrite(LED_PIN, (millis() / 500) % 2 == 0 ? LOW : HIGH);
    } else {
        digitalWrite(LED_PIN, HIGH);
    }

    // Button
    bool btnNow = digitalRead(BUTTON_PIN);
    if (btnPrev == HIGH && btnNow == LOW) {
        if (relay.isOn()) {
            relay.off();
            Serial.println("[BTN] Manual stop");
        } else {
            DisplayPage nextPage = (DisplayPage)((oled.currentPage() + 1) % PAGE_COUNT);
            oled.showPage(nextPage);
            Serial.printf("[BTN] Page switch to %d\n", nextPage);
        }
    }
    btnPrev = btnNow;

    // OLED
    if (millis() - lastOled >= 1000) {
        lastOled = millis();
        oled.update(&ntpMgr, &relay, &wifiMgr);
    }

    // Serial
    if (millis() - lastSerial >= 5000) {
        lastSerial = millis();
        serial.draw(&ntpMgr, &relay, &wifiMgr);
    }

    delay(20);
}
