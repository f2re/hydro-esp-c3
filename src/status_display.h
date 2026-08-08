#pragma once
#include <Arduino.h>
#include "ntp_manager.h"
#include "relay_controller.h"
#include "wifi_manager.h"
#include "scheduler.h"

class StatusDisplay {
public:
    void printBoot();
    void printBootStep(const char* icon, const char* msg,
                       bool ok, const String& detail = "");
    void printSchedule(Scheduler* scheduler);
    void draw(NTPManager* ntp, RelayController* relay,
              WiFiManager* wifi, Scheduler* scheduler);

private:
    void cls();
    String fmtUptime();
    String progressBar(float pct, uint8_t width = 18);
};
