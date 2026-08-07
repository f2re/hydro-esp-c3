#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include "config.h"

struct Config {
    String wifi_ssid;
    String wifi_pass;
    int timezone_offset = TIMEZONE_OFFSET;
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
