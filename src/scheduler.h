#pragma once
#include <Arduino.h>
#include "config.h"
#include "relay_controller.h"
#include "ntp_manager.h"

class Scheduler {
public:
    void begin(RelayController* relay, NTPManager* ntp);
    void update();
    void updateConfig(const WateringSlot* schedule, uint8_t count);

    String getNextWateringString() const;
    bool getNextSlot(WateringSlot& slot, int& minutesUntil) const;
    bool getSlot(uint8_t index, WateringSlot& slot) const;
    uint8_t count() const { return _count; }

private:
    RelayController* _relay = nullptr;
    NTPManager* _ntp = nullptr;
    uint8_t _lastCheckedMinute = 255;
    WateringSlot _schedule[MAX_SCHEDULE_SLOTS]{};
    uint8_t _count = 0;
};
