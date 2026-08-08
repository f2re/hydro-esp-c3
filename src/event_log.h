#pragma once
#include <Arduino.h>
#include "ntp_manager.h"

constexpr uint8_t EVENT_LOG_CAPACITY = 32;

enum class EventType : uint8_t {
    Boot = 0,
    PumpStart,
    PumpStop,
    AutomationEnabled,
    AutomationPaused,
    ScheduleChanged,
    HydraulicsSaved,
    ConfigChanged,
    OtaStarted,
    RebootRequested
};

enum class PumpSource : uint8_t {
    None = 0,
    Schedule,
    WebManual,
    ButtonManual,
    Calibration
};

enum class PumpStopReason : uint8_t {
    None = 0,
    Manual,
    Timeout,
    Reboot,
    Ota,
    AutomationPaused
};

struct OperationEvent {
    uint32_t sequence = 0;
    uint32_t uptime_sec = 0;
    uint32_t local_epoch = 0;
    EventType type = EventType::Boot;
    PumpSource source = PumpSource::None;
    PumpStopReason reason = PumpStopReason::None;
    int32_t value = 0;
};

class EventLog {
public:
    void begin(NTPManager* ntp);
    void clear();
    void record(EventType type,
                PumpSource source = PumpSource::None,
                PumpStopReason reason = PumpStopReason::None,
                int32_t value = 0);
    uint8_t count() const { return _count; }
    bool getNewest(uint8_t index, OperationEvent& event) const;

    static const char* typeCode(EventType type);
    static const char* sourceCode(PumpSource source);
    static const char* stopReasonCode(PumpStopReason reason);

private:
    NTPManager* _ntp = nullptr;
    OperationEvent _events[EVENT_LOG_CAPACITY]{};
    uint8_t _head = 0;
    uint8_t _count = 0;
    uint32_t _sequence = 0;
};

extern EventLog eventLog;
