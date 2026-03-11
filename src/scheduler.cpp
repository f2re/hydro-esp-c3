// scheduler.cpp
#include "scheduler.h"

void Scheduler::begin(RelayController* relay, NTPManager* ntp) {
    _relay = relay; _ntp = ntp;
}
void Scheduler::update() {
    if (!_relay || !_ntp || !_ntp->isSynced()) return;
    uint8_t m = _ntp->getMinute();
    if (m == _lastCheckedMinute) return;  // эту минуту уже проверяли
    _lastCheckedMinute = m;

    uint8_t h = _ntp->getHour();
    for (uint8_t i = 0; i < SCHEDULE_COUNT; i++) {
        if (h == WATERING_SCHEDULE[i].hour && m == WATERING_SCHEDULE[i].minute) {
            if (!_relay->isOn()) {
                Serial.printf("[Scheduler] Slot %d: %02d:%02d → %d sec\n",
                    i, h, m, WATERING_SCHEDULE[i].duration_sec);
                _relay->runFor(WATERING_SCHEDULE[i].duration_sec);
            }
        }
    }
}
