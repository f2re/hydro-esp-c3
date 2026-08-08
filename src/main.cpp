#include <Arduino.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "config.h"
#include "version.h"
#include "wifi_manager.h"
#include "ntp_manager.h"
#include "event_log.h"
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

namespace {
constexpr uint32_t OLED_INTERVAL_MS = 1000;
constexpr uint32_t SERIAL_INTERVAL_MS = 5000;
constexpr uint32_t BUTTON_DEBOUNCE_MS = 35;
constexpr uint32_t BUTTON_HOLD_TO_START_MS = 800;

uint32_t lastSerial = 0;
uint32_t lastOled = 0;
uint32_t buttonChangedAt = 0;
uint32_t buttonPressedAt = 0;
bool buttonRaw = HIGH;
bool buttonStable = HIGH;
bool buttonActionDone = false;
bool buttonArmed = false;
Config appConfig;

void updatePhysicalButton() {
    const bool raw = digitalRead(BUTTON_PIN);
    const uint32_t now = millis();

    if (raw != buttonRaw) {
        buttonRaw = raw;
        buttonChangedAt = now;
    }

    if (raw != buttonStable &&
        static_cast<uint32_t>(now - buttonChangedAt) >= BUTTON_DEBOUNCE_MS) {
        buttonStable = raw;

        if (buttonStable == LOW) {
            buttonPressedAt = now;
            buttonActionDone = !buttonArmed;

            if (buttonArmed && relay.isOn()) {
                relay.off(PumpStopReason::Manual);
                buttonActionDone = true;
                Serial.println("[BTN] Manual stop");
            }
        } else {
            buttonArmed = true;
            buttonPressedAt = 0;
            buttonActionDone = false;
        }
    }

    if (buttonArmed && buttonStable == LOW && !buttonActionDone && !relay.isOn() &&
        static_cast<uint32_t>(now - buttonPressedAt) >= BUTTON_HOLD_TO_START_MS) {
        relay.runFor(DEFAULT_MANUAL_SECONDS, PumpSource::ButtonManual);
        buttonActionDone = true;
        Serial.printf("[BTN] Manual start %u sec (hold)\n", DEFAULT_MANUAL_SECONDS);
    }
}
}

void setup() {
    // Hardware fail-safe first. This is intentionally the first GPIO action:
    // the pump must be OFF even if NVS, OLED, Wi-Fi or HTTP initialization fails.
    pinMode(RELAY_PIN, OUTPUT_OPEN_DRAIN);
    digitalWrite(RELAY_PIN, RELAY_OFF);

#if HYDRO_DISABLE_BROWNOUT_WORKAROUND
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
#endif

    Serial.begin(115200);
    delay(1000);
    Serial.printf("\n[HydroESP] %s (%s), API v%d\n",
                  HYDRO_VERSION, HYDRO_BUILD_SHA, HYDRO_API_VERSION);
    Serial.println("[BOOT] relay forced OFF");

    pinMode(LED_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    digitalWrite(LED_PIN, HIGH);
    buttonRaw = buttonStable = digitalRead(BUTTON_PIN);
    buttonArmed = buttonStable == HIGH;

    // Bring the local display up before touching NVS. A broken configuration
    // must not turn into a completely black/undebuggable device.
    oled.begin();
    oled.drawBoot(1, "Boot...");
    Serial.println("[BOOT] OLED initialized");

    configStorage.begin();
    configStorage.load(appConfig);
    Serial.println("[BOOT] config loaded");
    eventLog.begin(&ntpMgr);

    serial.printBoot();
    oled.drawBoot(1, "Init...");

    relay.begin(&eventLog);
    Serial.println("[BOOT] relay controller ready/OFF");
    scheduler.begin(&relay, &ntpMgr);
    scheduler.updateConfig(appConfig.schedule, appConfig.schedule_count);
    scheduler.setEnabled(appConfig.automation_enabled);

    oled.drawBoot(2, "WiFi...");
    serial.printBootStep("📡", "WiFi", false, "");

    const bool wifiOk = wifiMgr.connect(
        appConfig.wifi_ssid.c_str(),
        appConfig.wifi_pass.c_str(),
        WIFI_TIMEOUT_MS
    );

    if (!wifiOk) {
        Serial.printf("[SETUP] Wi-Fi: %s (open, no password)\n", AP_SSID);
        Serial.println("[SETUP] Open: http://192.168.4.1");

        if (wifiMgr.startAP(AP_SSID)) {
            serial.printBootStep("📡", "Setup AP", true, AP_SSID);
            oled.drawProvisioning(AP_SSID, wifiMgr.localIP());
        } else {
            serial.printBootStep("⚠️", "Setup AP", false, "start failed");
            oled.drawBoot(2, "AP failed");
        }
    } else {
        serial.printBootStep("📡", "WiFi", true, wifiMgr.localIP());
        oled.drawBoot(2, "WiFi OK");
    }

    // The web UI is the primary setup tool. Bring it up immediately after
    // networking is ready; internet access and NTP are not prerequisites.
    webSrv.begin(&relay, &scheduler, &ntpMgr, &wifiMgr, &oled);
    if (wifiOk || wifiMgr.isAPMode()) {
        Serial.printf("[WEB] Open: http://%s/\n", wifiMgr.localIP().c_str());
    }

    if (wifiOk) {
        if (MDNS.begin(MDNS_HOST)) {
            MDNS.addService("http", "tcp", 80);
            Serial.printf("[mDNS] http://%s.local\n", MDNS_HOST);
        } else {
            Serial.println("[mDNS] unavailable; use the numeric IP shown above/OLED");
        }

        oled.drawBoot(3, "NTP...");
        ntpMgr.begin(appConfig.timezone_offset);
        serial.printBootStep("🕐", "NTP", ntpMgr.isSynced(), ntpMgr.getTimeString());
        oled.drawBoot(3, ntpMgr.isSynced() ? "NTP OK" : "NTP wait");
    } else {
        Serial.println("[NTP] Skipped in setup AP mode");
    }

    eventLog.record(EventType::Boot, PumpSource::None, PumpStopReason::None,
                    appConfig.automation_enabled ? 1 : 0);

    if (wifiOk) oled.drawBoot(4, "Ready!");
    serial.printSchedule(&scheduler);
    delay(600);
}

void loop() {
    if (wifiMgr.isAPMode()) {
        wifiMgr.updateDNS();
    } else {
        wifiMgr.ensureConnected(appConfig.wifi_ssid.c_str(), appConfig.wifi_pass.c_str());
    }

    ntpMgr.update();
    relay.update();
    scheduler.update();
    updatePhysicalButton();

    if (relay.isOn()) {
        digitalWrite(LED_PIN, (millis() / 500) % 2 == 0 ? LOW : HIGH);
    } else {
        digitalWrite(LED_PIN, HIGH);
    }

    const uint32_t now = millis();
    if (static_cast<uint32_t>(now - lastOled) >= OLED_INTERVAL_MS) {
        lastOled = now;
        if (wifiMgr.isAPMode()) {
            oled.drawProvisioning(AP_SSID, wifiMgr.localIP());
        } else {
            oled.update(&ntpMgr, &relay, &wifiMgr, &scheduler);
        }
    }

    if (static_cast<uint32_t>(now - lastSerial) >= SERIAL_INTERVAL_MS) {
        lastSerial = now;
        serial.draw(&ntpMgr, &relay, &wifiMgr, &scheduler);
    }

    delay(10);
}
