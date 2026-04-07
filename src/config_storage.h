#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include "config.h"

struct Config {
    String wifi_ssid;
    String wifi_pass;
    int timezone_offset;
    WateringSlot schedule[48];
    uint8_t schedule_count;
};

class ConfigStorage {
public:
    void begin();
    void load(Config &config);
    void save(const Config &config);
    void clear();

private:
    Preferences prefs;
};

extern ConfigStorage configStorage;
