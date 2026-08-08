#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include "config.h"

struct Config {
    String wifi_ssid;
    String wifi_pass;
    int timezone_offset = TIMEZONE_OFFSET;
    float latitude = 0.0f;
    float longitude = 0.0f;
    uint32_t last_solution_change = 0;
    bool automation_enabled = true;
    uint32_t pump_flow_ml_min = 0;
    uint8_t delivery_efficiency_pct = 85;
    uint8_t calibration_protocol_version = 0;
    uint8_t calibration_sample_count = 0;
    uint16_t calibration_cv_x100 = 0;
    uint32_t calibration_local_epoch = 0;
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
