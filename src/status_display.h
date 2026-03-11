#pragma once
#include <Arduino.h>
#include "config.h"
#include "ntp_manager.h"
#include "relay_controller.h"
#include "wifi_manager.h"

class StatusDisplay {
public:
    void printBoot();                       // один раз при старте
    void printBootStep(const char* icon,
                       const char* msg,
                       bool ok,
                       const String& detail = "");
    void printSchedule();                   // напечатать расписание в Serial
    void draw(NTPManager* ntp,
              RelayController* relay,
              WiFiManager* wifi);           // полная перерисовка дашборда
private:
    uint32_t _startMs = 0;

    void    cls();
    void    bar(const char* label, float pct, uint8_t width = 20);
    int     minutesUntilNext(uint8_t h, uint8_t m,
                             uint8_t* oH, uint8_t* oM,
                             uint16_t* oDur);
    String  fmtUptime();
    String  progressBar(float pct, uint8_t w = 18);
};
