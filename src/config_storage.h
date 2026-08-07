#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include "config.h"

struct Config {
    String wifi_ssid;
    String wifi_pass;
    int timezone_offset = TIMEZONE_OFFSET;
    bool automation_enabled = true;
    uint32_t pump_flow_ml_min = 0;
    uint8_t delivery_efficiency_pct = 85;
    WateringSlot schedule[MAX_SCHEDULE_SLOTS]{};
    uint8_t schedule_count = 0;
};

class ConfigStorage {
public:
    void begin();
    void load(Config &config);
    void save(const Config &config);
    void clear();

private:
    Preferences prefs;
    void loadFactorySchedule(Config &config);
};

extern ConfigStorage configStorage;
