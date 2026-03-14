#include "config_storage.h"

void ConfigStorage::begin() {
    prefs.begin("hydro", false);
}

void ConfigStorage::load(Config &config) {
    config.wifi_ssid = prefs.getString("ssid", WIFI_SSID);
    config.wifi_pass = prefs.getString("pass", WIFI_PASSWORD);
    config.timezone_offset = prefs.getInt("tz", TIMEZONE_OFFSET);
    
    config.schedule_count = prefs.getUChar("sched_cnt", SCHEDULE_COUNT);
    if (config.schedule_count > 16) config.schedule_count = 16;

    size_t len = prefs.getBytesLength("sched");
    if (len > 0 && len == config.schedule_count * sizeof(WateringSlot)) {
        prefs.getBytes("sched", config.schedule, len);
    } else {
        config.schedule_count = SCHEDULE_COUNT;
        for (int i = 0; i < SCHEDULE_COUNT; i++) {
            config.schedule[i] = WATERING_SCHEDULE[i];
        }
    }
}

void ConfigStorage::save(const Config &config) {
    prefs.putString("ssid", config.wifi_ssid);
    prefs.putString("pass", config.wifi_pass);
    prefs.putInt("tz", config.timezone_offset);
    prefs.putUChar("sched_cnt", config.schedule_count);
    prefs.putBytes("sched", config.schedule, config.schedule_count * sizeof(WateringSlot));
}

void ConfigStorage::clear() {
    prefs.clear();
}

ConfigStorage configStorage;
