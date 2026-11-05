#include "scheduler.h"

static_assert(SOLAR_PLAN_MAX_SLOTS == MAX_SCHEDULE_SLOTS,
              "Solar plan and controller schedule limits must match");

void Scheduler::begin(RelayController* relay, NTPManager* ntp) {
    _relay = relay;
    _ntp = ntp;
}

void Scheduler::updateConfig(const WateringSlot* schedule, uint8_t count) {
    _count = count > MAX_SCHEDULE_SLOTS ? MAX_SCHEDULE_SLOTS : count;
    for (uint8_t i = 0; i < _count; ++i) {
        _schedule[i] = schedule[i];
    }
}

void Scheduler::configureSolar(const SolarPlanSettings &settings, float latitude, float longitude) {
    _solar = settings;
    _solarLatitude = latitude;
    _solarLongitude = longitude;
    _solarDay = UINT32_MAX;
    _solarError = _solar.enabled ? SolarPlanError::TimeUnavailable : SolarPlanError::None;
    if (_solar.enabled) {
        // A cached manual plan must never run in solar mode.
        _count = 0;
        refreshSolarPlan();
    }
}

void Scheduler::refreshSolarPlan() {
    if (!_solar.enabled) return;
    if (!_ntp || !_ntp->isSynced()) {
        _count = 0;
        _solarError = SolarPlanError::TimeUnavailable;
        return;
    }
    const uint32_t today = _ntp->getLocalEpoch() / 86400UL;
    if (today == _solarDay) return;
    _solarDay = today;
    _count = 0; // Invalid or polar daylight => no scheduled pump activation.

    float sunrise = 0.0f, sunset = 0.0f;
    _ntp->getSunriseSunset(_solarLatitude, _solarLongitude, sunrise, sunset);
    const SolarPlan plan = buildSolarPlan(sunrise, sunset, _solar);
    _solarError = plan.error;
    if (_solarError != SolarPlanError::None) {
        Serial.printf("[Solar] No safe plan today: %s\n", solarPlanErrorCode(_solarError));
        return;
    }
    for (uint8_t i = 0; i < plan.count; ++i) {
        const uint16_t minute = plan.start_minutes[i];
        _schedule[i] = {static_cast<uint8_t>(minute / 60),
                        static_cast<uint8_t>(minute % 60),
                        _solar.duration_sec};
    }
    _count = plan.count;
    Serial.printf("[Solar] Local day %lu: %u runs, sunrise %.2f, sunset %.2f\n",
                  static_cast<unsigned long>(today), _count, sunrise, sunset);
}

void Scheduler::setEnabled(bool enabled) {
    _enabled = enabled;
    // Consume the current minute when changing mode so resuming automation does
    // not immediately fire a slot that was intentionally skipped while paused.
    if (_ntp && _ntp->isSynced()) {
        _lastCheckedMinute = _ntp->getMinute();
    }
}

void Scheduler::update() {
    if (!_relay || !_ntp) return;
    refreshSolarPlan(); // Recalculate at local midnight, before pump decisions.
    if (!_ntp->isSynced()) return;

    const uint8_t minute = _ntp->getMinute();
    if (minute == _lastCheckedMinute) return;

    _lastCheckedMinute = minute;
    if (!_enabled || _relay->isOn()) return;

    const uint8_t hour = _ntp->getHour();
    for (uint8_t i = 0; i < _count; ++i) {
        if (hour == _schedule[i].hour && minute == _schedule[i].minute) {
            Serial.printf("[Scheduler] Slot %u: %02u:%02u -> %u sec\n",
                          i, hour, minute, _schedule[i].duration_sec);
            _relay->runFor(_schedule[i].duration_sec, PumpSource::Schedule);
            break;
        }
    }
}

bool Scheduler::getSlot(uint8_t index, WateringSlot& slot) const {
    if (index >= _count) return false;
    slot = _schedule[index];
    return true;
}

bool Scheduler::getNextSlot(WateringSlot& slot, int& minutesUntil) const {
    if (!_ntp || !_ntp->isSynced() || _count == 0) return false;

    const int current = _ntp->getHour() * 60 + _ntp->getMinute();
    int best = 1441;
    int nextIndex = -1;

    for (uint8_t i = 0; i < _count; ++i) {
        int diff = (_schedule[i].hour * 60 + _schedule[i].minute) - current;
        if (diff <= 0) {
            if (_solar.enabled) continue; // Tomorrow's solar plan will differ.
            diff += 1440;
        }
        if (diff < best) {
            best = diff;
            nextIndex = i;
        }
    }

    if (nextIndex < 0) return false;
    slot = _schedule[nextIndex];
    minutesUntil = best;
    return true;
}

String Scheduler::getNextWateringString() const {
    WateringSlot slot {};
    int minutesUntil = 0;
    if (!getNextSlot(slot, minutesUntil)) return "--:--";

    char buf[6];
    snprintf(buf, sizeof(buf), "%02u:%02u", slot.hour, slot.minute);
    return String(buf);
}
