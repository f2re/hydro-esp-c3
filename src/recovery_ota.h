#pragma once

#include <Arduino.h>

#include "relay_controller.h"
#include "wifi_manager.h"
#include "oled_display.h"

constexpr uint16_t RECOVERY_OTA_PORT = 3232;

class RecoveryOTA {
public:
    void begin(RelayController* relay, WiFiManager* wifi, OledDisplay* oled);
    void handle();
    bool isReady() const { return ready; }

private:
    RelayController* relay = nullptr;
    WiFiManager* wifi = nullptr;
    OledDisplay* oled = nullptr;
    bool ready = false;
};

extern RecoveryOTA recoveryOTA;
