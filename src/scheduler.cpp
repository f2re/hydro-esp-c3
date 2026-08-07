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

void Scheduler::update() {
    if (!_relay || !_ntp || !_ntp->isSynced()) return;

    const uint8_t minute = _ntp->getMinute();
    if (minute == _lastCheckedMinute) return;

    // Consume the minute even when the pump is already running. This prevents a
    // scheduled cycle from firing late in the same minute after a manual cycle
    // ends, which would otherwise produce an unexpected double watering.
    _lastCheckedMinute = minute;
    if (_relay->isOn()) return;

    const uint8_t hour = _ntp->getHour();
    for (uint8_t i = 0; i < _count; ++i) {
        if (hour == _schedule[i].hour && minute == _schedule[i].minute) {
            Serial.printf("[Scheduler] Slot %u: %02u:%02u -> %u sec\n",
                          i, hour, minute, _schedule[i].duration_sec);
            _relay->runFor(_schedule[i].duration_sec);
            break;
        }
    }
}

String Scheduler::getNextWateringString() {
    if (!_ntp || !_ntp->isSynced() || _count == 0) return "--:--";

    const uint8_t hour = _ntp->getHour();
    const uint8_t minute = _ntp->getMinute();
    int minDiff = 1440;
    int nextIdx = -1;

    for (uint8_t i = 0; i < _count; ++i) {
        int diff = (_schedule[i].hour * 60 + _schedule[i].minute) -
                   (hour * 60 + minute);
        if (diff <= 0) diff += 1440;
        if (diff < minDiff) {
            minDiff = diff;
            nextIdx = i;
        }
    }

    if (nextIdx < 0) return "--:--";

    char buf[6];
    snprintf(buf, sizeof(buf), "%02u:%02u",
             _schedule[nextIdx].hour, _schedule[nextIdx].minute);
    return String(buf);
}
