#include "config_storage.h"

namespace {
bool validSlot(const WateringSlot &slot) {
    return slot.hour <= 23 && slot.minute <= 59 &&
           slot.duration_sec >= 1 && slot.duration_sec <= MAX_WATERING_SECONDS;
}
}

void ConfigStorage::begin() {
    prefs.begin("hydro", false);
}

void ConfigStorage::loadFactorySchedule(Config &config) {
    config.schedule_count = 0;
    for (uint8_t i = 0; i < SCHEDULE_COUNT && i < MAX_SCHEDULE_SLOTS; ++i) {
        config.schedule[config.schedule_count++] = WATERING_SCHEDULE[i];
    }
}

void ConfigStorage::load(Config &config) {
    config.wifi_ssid = prefs.getString("ssid", WIFI_SSID);
    config.wifi_pass = prefs.getString("pass", WIFI_PASSWORD);
    config.timezone_offset = prefs.getInt("tz", TIMEZONE_OFFSET);
    if (config.timezone_offset < -12 || config.timezone_offset > 14) {
        config.timezone_offset = TIMEZONE_OFFSET;
    }

    config.automation_enabled = prefs.getBool("auto", true);
    config.pump_flow_ml_min = prefs.getUInt("flow_ml", 0);
    if (config.pump_flow_ml_min > 100000UL) {
        config.pump_flow_ml_min = 0;
    }
    config.delivery_efficiency_pct = prefs.getUChar("eff_pct", 85);
    if (config.delivery_efficiency_pct < 10 || config.delivery_efficiency_pct > 100) {
        config.delivery_efficiency_pct = 85;
    }

    uint8_t count = prefs.getUChar("sched_cnt", SCHEDULE_COUNT);
    if (count > MAX_SCHEDULE_SLOTS) {
        loadFactorySchedule(config);
        return;
    }

    const size_t expected = static_cast<size_t>(count) * sizeof(WateringSlot);
    const size_t stored = prefs.getBytesLength("sched");
    if (count == 0) {
        config.schedule_count = 0;
        return;
    }
    if (stored != expected || expected == 0) {
        loadFactorySchedule(config);
        return;
    }

    prefs.getBytes("sched", config.schedule, expected);
    for (uint8_t i = 0; i < count; ++i) {
        if (!validSlot(config.schedule[i])) {
            loadFactorySchedule(config);
            return;
        }
    }
    config.schedule_count = count;
}

void ConfigStorage::save(const Config &config) {
    const uint8_t count = config.schedule_count > MAX_SCHEDULE_SLOTS
        ? MAX_SCHEDULE_SLOTS
        : config.schedule_count;

    prefs.putString("ssid", config.wifi_ssid);
    prefs.putString("pass", config.wifi_pass);
    prefs.putInt("tz", config.timezone_offset);
    prefs.putBool("auto", config.automation_enabled);
    prefs.putUInt("flow_ml", config.pump_flow_ml_min);
    prefs.putUChar("eff_pct", config.delivery_efficiency_pct);
    prefs.putUChar("sched_cnt", count);

    if (count == 0) {
        prefs.remove("sched");
    } else {
        prefs.putBytes("sched", config.schedule,
                       static_cast<size_t>(count) * sizeof(WateringSlot));
    }
}

void ConfigStorage::clear() {
    prefs.clear();
}

ConfigStorage configStorage;
