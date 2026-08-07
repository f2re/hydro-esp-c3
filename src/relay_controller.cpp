#include "relay_controller.h"

void RelayController::begin() {
    pinMode(RELAY_PIN, OUTPUT_OPEN_DRAIN);
    off();
}

void RelayController::on() {
    runFor(MAX_WATERING_SECONDS);
    Serial.printf("[Relay] ON (manual limit %u sec)\n", MAX_WATERING_SECONDS);
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
    if (seconds > MAX_WATERING_SECONDS) seconds = MAX_WATERING_SECONDS;

    _totalMs = static_cast<uint32_t>(seconds) * 1000UL;
    _active = true;
    _endTime = millis() + _totalMs;
    digitalWrite(RELAY_PIN, RELAY_ON);
    Serial.printf("[Relay] ON for %u sec\n", seconds);
}

void RelayController::update() {
    if (_active && _endTime > 0 && static_cast<int32_t>(millis() - _endTime) >= 0) {
        off();
    }
}

float RelayController::progress() const {
    if (!_active || _endTime == 0 || _totalMs == 0) return -1.0f;

    const int32_t remaining = static_cast<int32_t>(_endTime - millis());
    if (remaining <= 0) return 1.0f;

    float value = 1.0f - static_cast<float>(remaining) / static_cast<float>(_totalMs);
    if (value < 0.0f) value = 0.0f;
    if (value > 1.0f) value = 1.0f;
    return value;
}

uint16_t RelayController::remainingSec() const {
    if (!_active || _endTime == 0) return 0;

    const int32_t remaining = static_cast<int32_t>(_endTime - millis());
    if (remaining <= 0) return 0;

    return static_cast<uint16_t>((static_cast<uint32_t>(remaining) + 999UL) / 1000UL);
}
