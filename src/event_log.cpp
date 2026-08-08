#include "event_log.h"

void EventLog::begin(NTPManager* ntp) {
    _ntp = ntp;
    clear();
}

void EventLog::clear() {
    _head = 0;
    _count = 0;
    _sequence = 0;
}

void EventLog::record(EventType type,
                      PumpSource source,
                      PumpStopReason reason,
                      int32_t value) {
    OperationEvent event;
    event.sequence = ++_sequence;
    event.uptime_sec = millis() / 1000UL;
    event.local_epoch = (_ntp && _ntp->isSynced()) ? _ntp->getLocalEpoch() : 0;
    event.type = type;
    event.source = source;
    event.reason = reason;
    event.value = value;

    _events[_head] = event;
    _head = (_head + 1) % EVENT_LOG_CAPACITY;
    if (_count < EVENT_LOG_CAPACITY) ++_count;
}

bool EventLog::getNewest(uint8_t index, OperationEvent& event) const {
    if (index >= _count) return false;
    int pos = static_cast<int>(_head) - 1 - static_cast<int>(index);
    while (pos < 0) pos += EVENT_LOG_CAPACITY;
    event = _events[pos];
    return true;
}

const char* EventLog::typeCode(EventType type) {
    switch (type) {
        case EventType::Boot: return "boot";
        case EventType::PumpStart: return "pump_start";
        case EventType::PumpStop: return "pump_stop";
        case EventType::AutomationEnabled: return "automation_enabled";
        case EventType::AutomationPaused: return "automation_paused";
        case EventType::ScheduleChanged: return "schedule_changed";
        case EventType::HydraulicsSaved: return "hydraulics_saved";
        case EventType::ConfigChanged: return "config_changed";
        case EventType::OtaStarted: return "ota_started";
        case EventType::RebootRequested: return "reboot_requested";
        default: return "unknown";
    }
}

const char* EventLog::sourceCode(PumpSource source) {
    switch (source) {
        case PumpSource::Schedule: return "schedule";
        case PumpSource::WebManual: return "web_manual";
        case PumpSource::ButtonManual: return "button_manual";
        case PumpSource::Calibration: return "calibration";
        default: return "none";
    }
}

const char* EventLog::stopReasonCode(PumpStopReason reason) {
    switch (reason) {
        case PumpStopReason::Manual: return "manual";
        case PumpStopReason::Timeout: return "timeout";
        case PumpStopReason::Reboot: return "reboot";
        case PumpStopReason::Ota: return "ota";
        case PumpStopReason::AutomationPaused: return "automation_paused";
        default: return "none";
    }
}

EventLog eventLog;
