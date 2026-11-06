#pragma once
// Pure arithmetic, independent of Arduino/ESP32; also used by host-side tests.
#include <stdint.h>
#include <math.h>

constexpr uint8_t SOLAR_PLAN_MAX_SLOTS = 48;

struct SolarPlanSettings {
    bool enabled = false;
    bool by_count = true;
    uint8_t count = 16;
    uint16_t interval_min = 60;
    uint16_t start_offset_min = 0;
    uint16_t end_offset_min = 0;
    uint16_t duration_sec = 60;
};

enum class SolarPlanError : uint8_t {
    None, TimeUnavailable, InvalidParameters, NoDaylight,
    WindowTooShort, TooManySlots, OverlappingSlots
};

inline const char* solarPlanErrorCode(SolarPlanError error) {
    switch (error) {
        case SolarPlanError::None: return "ok";
        case SolarPlanError::TimeUnavailable: return "time_not_synced";
        case SolarPlanError::InvalidParameters: return "invalid_parameters";
        case SolarPlanError::NoDaylight: return "daylight_unavailable";
        case SolarPlanError::WindowTooShort: return "window_too_short";
        case SolarPlanError::TooManySlots: return "too_many_slots";
        case SolarPlanError::OverlappingSlots: return "overlapping_slots";
    }
    return "unknown";
}

struct SolarPlan {
    uint16_t start_minutes[SOLAR_PLAN_MAX_SLOTS]{};
    uint8_t count = 0;
    SolarPlanError error = SolarPlanError::None;
    uint16_t window_start = 0;
    uint16_t window_end = 0;
};

inline bool solarSettingsValid(const SolarPlanSettings &s) {
    return s.count >= 1 && s.count <= SOLAR_PLAN_MAX_SLOTS &&
           s.interval_min >= 5 && s.interval_min <= 720 &&
           s.start_offset_min <= 360 && s.end_offset_min <= 360 &&
           s.duration_sec >= 1 && s.duration_sec <= 3600;
}

// Input: sunrise/sunset as local civil hours, like /api/status.
// Last pump run must finish before sunset minus the configured margin.
inline SolarPlan buildSolarPlan(float sunrise, float sunset, const SolarPlanSettings &s) {
    SolarPlan plan;
    if (!solarSettingsValid(s)) {
        plan.error = SolarPlanError::InvalidParameters;
        return plan;
    }
    if (!isfinite(sunrise) || !isfinite(sunset) || sunrise < 0.0f ||
        sunset >= 24.0f || sunset <= sunrise) {
        plan.error = SolarPlanError::NoDaylight;
        return plan;
    }

    const int first = static_cast<int>(lroundf(sunrise * 60.0f)) + s.start_offset_min;
    const int end = static_cast<int>(lroundf(sunset * 60.0f)) - s.end_offset_min;
    const int latest = end - (s.duration_sec + 59) / 60;
    if (first < 0 || end > 1440 || latest < first) {
        plan.error = SolarPlanError::WindowTooShort;
        return plan;
    }
    plan.window_start = static_cast<uint16_t>(first);
    plan.window_end = static_cast<uint16_t>(end);

    if (s.by_count) {
        if (s.count > 1 && (latest - first) < (s.count - 1)) {
            plan.error = SolarPlanError::WindowTooShort;
            return plan;
        }
        for (uint8_t i = 0; i < s.count; ++i) {
            const float value = s.count == 1 ? first :
                first + static_cast<float>(latest - first) * i / (s.count - 1);
            plan.start_minutes[plan.count++] = static_cast<uint16_t>(lroundf(value));
        }
    } else {
        for (int minute = first; minute <= latest; minute += s.interval_min) {
            if (plan.count >= SOLAR_PLAN_MAX_SLOTS) {
                plan.error = SolarPlanError::TooManySlots;
                plan.count = 0;
                return plan;
            }
            plan.start_minutes[plan.count++] = static_cast<uint16_t>(minute);
        }
    }

    for (uint8_t i = 1; i < plan.count; ++i) {
        const int gap = plan.start_minutes[i] - plan.start_minutes[i - 1];
        if (gap <= 0 || static_cast<int>(s.duration_sec) > gap * 60) {
            plan.error = SolarPlanError::OverlappingSlots;
            plan.count = 0;
            return plan;
        }
    }
    return plan;
}
