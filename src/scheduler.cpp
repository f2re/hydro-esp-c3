#include "scheduler.h"

void Scheduler::begin(RelayController* relay, NTPManager* ntp) {
    _relay = relay; _ntp = ntp;
}

void Scheduler::updateConfig(const WateringSlot* schedule, uint8_t count) {
    _count = count > 16 ? 16 : count;
    for (int i = 0; i < _count; i++) {
        _schedule[i] = schedule[i];
    }
}

void Scheduler::update() {
    if (!_relay || !_ntp || !_ntp->isSynced()) return;
    
    uint8_t m = _ntp->getMinute();
    if (m == _lastCheckedMinute) return;
    _lastCheckedMinute = m;

    uint8_t h = _ntp->getHour();
    for (uint8_t i = 0; i < _count; i++) {
        if (h == _schedule[i].hour && m == _schedule[i].minute) {
            if (!_relay->isOn()) {
                Serial.printf("[Scheduler] Slot %d: %02d:%02d → %d sec\n",
                    i, h, m, _schedule[i].duration_sec);
                _relay->runFor(_schedule[i].duration_sec);
            }
        }
    }
}

String Scheduler::getNextWateringString() {
    if (!_ntp->isSynced()) return "--:--";
    uint8_t h = _ntp->getHour();
    uint8_t m = _ntp->getMinute();
    
    int minDiff = 1440;
    int nextIdx = -1;
    
    for (int i = 0; i < _count; i++) {
        int diff = (_schedule[i].hour * 60 + _schedule[i].minute) - (h * 60 + m);
        if (diff <= 0) diff += 1440;
        if (diff < minDiff) {
            minDiff = diff;
            nextIdx = i;
        }
    }
    
    if (nextIdx != -1) {
        char buf[6];
        snprintf(buf, 6, "%02d:%02d", _schedule[nextIdx].hour, _schedule[nextIdx].minute);
        return String(buf);
    }
    return "--:--";
}
