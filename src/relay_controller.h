#pragma once
#include <Arduino.h>
#include "config.h"
#include "event_log.h"

class RelayController {
public:
    void begin(EventLog* log = nullptr);
    void on();
    void off(PumpStopReason reason = PumpStopReason::Manual);
    bool isOn() const;
    void runFor(uint16_t seconds, PumpSource source = PumpSource::WebManual);
    void update();
    float progress() const;
    uint16_t remainingSec() const;
    PumpSource source() const { return _source; }

private:
    bool _active = false;
    unsigned long _endTime = 0;
    uint32_t _totalMs = 0;
    PumpSource _source = PumpSource::None;
    EventLog* _log = nullptr;
};
