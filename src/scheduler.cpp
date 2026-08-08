#include "scheduler.h"

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

void Scheduler::setEnabled(bool enabled) {
    _enabled = enabled;
    // Consume the current minute when changing mode so resuming automation does
    // not immediately fire a slot that was intentionally skipped while paused.
    if (_ntp && _ntp->isSynced()) {
        _lastCheckedMinute = _ntp->getMinute();
    }
}

void Scheduler::update() {
    if (!_relay || !_ntp || !_ntp->isSynced()) return;

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
        if (diff <= 0) diff += 1440;
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
