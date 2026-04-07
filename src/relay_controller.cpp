#include "relay_controller.h"

void RelayController::begin() {
    pinMode(RELAY_PIN, OUTPUT_OPEN_DRAIN);
    off();
}
void RelayController::on() {
    // Если включаем вручную без времени, ставим защитный таймаут 1 час,
    // чтобы работали таймеры и индикаторы на экране.
    runFor(3600);
    Serial.println("[Relay] ON (Manual 1h limit)");
}
void RelayController::off() {
    _active = false; _endTime = 0;
    digitalWrite(RELAY_PIN, RELAY_OFF);
    Serial.println("[Relay] OFF");
}
bool RelayController::isOn() const { return _active; }

void RelayController::runFor(uint16_t seconds) {
    _totalMs = (uint32_t)seconds * 1000;
    _active = true;
    _endTime = millis() + _totalMs;
    digitalWrite(RELAY_PIN, RELAY_ON);
    Serial.printf("[Relay] ON for %d sec\n", seconds);
}
void RelayController::update() {
    if (_active && _endTime > 0) {
        if ((long)(millis() - _endTime) >= 0) off();
    }
}
float RelayController::progress() const {
    if (!_active || _endTime == 0) return -1.0f;
    uint32_t now = millis();
    if (now >= _endTime) return 1.0f;
    return 1.0f - (float)(_endTime - now) / (float)_totalMs;
}
uint16_t RelayController::remainingSec() const {
    if (!_active || _endTime == 0) return 0;
    uint32_t now = millis();
    if (now >= _endTime) return 0;
    return (uint16_t)((_endTime - now) / 1000);
}
