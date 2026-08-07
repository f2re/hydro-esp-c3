#include "relay_controller.h"

void RelayController::begin(EventLog* log) {
    _log = log;
    pinMode(RELAY_PIN, OUTPUT_OPEN_DRAIN);
    _active = false;
    _endTime = 0;
    _totalMs = 0;
    _source = PumpSource::None;
    digitalWrite(RELAY_PIN, RELAY_OFF);
}

void RelayController::on() {
    runFor(MAX_WATERING_SECONDS, PumpSource::WebManual);
    Serial.printf("[Relay] ON (manual limit %u sec)\n", MAX_WATERING_SECONDS);
}

void RelayController::off(PumpStopReason reason) {
    if (!_active) {
        digitalWrite(RELAY_PIN, RELAY_OFF);
        return;
    }

    const PumpSource previousSource = _source;
    _active = false;
    _endTime = 0;
    _totalMs = 0;
    _source = PumpSource::None;
    digitalWrite(RELAY_PIN, RELAY_OFF);
    Serial.printf("[Relay] OFF reason=%s\n", EventLog::stopReasonCode(reason));

    if (_log) {
        _log->record(EventType::PumpStop, previousSource, reason, 0);
    }
}

bool RelayController::isOn() const {
    return _active;
}

void RelayController::runFor(uint16_t seconds, PumpSource source) {
    if (seconds == 0) {
        off(PumpStopReason::Manual);
        return;
    }
    if (seconds > MAX_WATERING_SECONDS) seconds = MAX_WATERING_SECONDS;

    _totalMs = static_cast<uint32_t>(seconds) * 1000UL;
    _active = true;
    _endTime = millis() + _totalMs;
    _source = source;
    digitalWrite(RELAY_PIN, RELAY_ON);
    Serial.printf("[Relay] ON for %u sec source=%s\n",
                  seconds, EventLog::sourceCode(source));

    if (_log) {
        _log->record(EventType::PumpStart, source, PumpStopReason::None, seconds);
    }
}

void RelayController::update() {
    if (_active && _endTime > 0 && static_cast<int32_t>(millis() - _endTime) >= 0) {
        off(PumpStopReason::Timeout);
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
