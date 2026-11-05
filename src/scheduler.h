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
    void setEnabled(bool enabled);
    void configureSolar(const SolarPlanSettings &settings, float latitude, float longitude);
    void refreshSolarPlan();
    bool solarEnabled() const { return _solar.enabled; }
    bool solarReady() const { return _solar.enabled && _solarError == SolarPlanError::None; }
    SolarPlanError solarError() const { return _solarError; }
    bool isEnabled() const { return _enabled; }

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
    bool _enabled = true;
    SolarPlanSettings _solar{};
    float _solarLatitude = 0.0f;
    float _solarLongitude = 0.0f;
    uint32_t _solarDay = UINT32_MAX;
    SolarPlanError _solarError = SolarPlanError::None;
};
