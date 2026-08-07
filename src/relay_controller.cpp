#include "relay_controller.h"

void RelayController::begin() {
    pinMode(RELAY_PIN, OUTPUT_OPEN_DRAIN);
    off();
}

void RelayController::on() {
    // Ручное включение без явной длительности всегда ограничено часом.
    runFor(3600);
    Serial.println("[Relay] ON (Manual 1h limit)");
}

void RelayController::off() {
    _active = false;
    _endTime = 0;
    _totalMs = 0;
    digitalWrite(RELAY_PIN, RELAY_OFF);
    Serial.println("[Relay] OFF");
}

bool RelayController::isOn() const {
    return _active;
}

void RelayController::runFor(uint16_t seconds) {
    if (seconds == 0) {
        off();
        return;
    }

    _totalMs = (uint32_t)seconds * 1000UL;
    _active = true;
    _endTime = millis() + _totalMs;
    digitalWrite(RELAY_PIN, RELAY_ON);
    Serial.printf("[Relay] ON for %u sec\n", seconds);
}

void RelayController::update() {
    if (_active && _endTime > 0) {
        // Знаковая разность корректно переживает переполнение millis(),
        // пока интервал существенно меньше 2^31 мс (у нас максимум 1 час).
        if ((int32_t)(millis() - _endTime) >= 0) off();
    }
}

float RelayController::progress() const {
    if (!_active || _endTime == 0 || _totalMs == 0) return -1.0f;

    const int32_t remaining = (int32_t)(_endTime - millis());
    if (remaining <= 0) return 1.0f;

    float value = 1.0f - (float)remaining / (float)_totalMs;
    if (value < 0.0f) value = 0.0f;
    if (value > 1.0f) value = 1.0f;
    return value;
}

uint16_t RelayController::remainingSec() const {
    if (!_active || _endTime == 0) return 0;

    const int32_t remaining = (int32_t)(_endTime - millis());
    if (remaining <= 0) return 0;

    return (uint16_t)(((uint32_t)remaining + 999UL) / 1000UL);
}
