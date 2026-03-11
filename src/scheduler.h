// scheduler.h
#pragma once
#include <Arduino.h>
#include "config.h"
#include "relay_controller.h"
#include "ntp_manager.h"

class Scheduler {
public:
    void begin(RelayController* relay, NTPManager* ntp);
    void update();
private:
    RelayController* _relay = nullptr;
    NTPManager* _ntp = nullptr;
    uint8_t _lastCheckedMinute = 255;
};
