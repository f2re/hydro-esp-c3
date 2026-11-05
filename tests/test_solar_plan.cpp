#include "../src/solar_plan.h"
#include <assert.h>
#include <stdio.h>

static void expected(SolarPlanError error, float dawn, float dusk, const SolarPlanSettings& s) {
    SolarPlan p = buildSolarPlan(dawn, dusk, s);
    assert(p.error == error);
    if (error != SolarPlanError::None) assert(p.count == 0);
}

int main() {
    SolarPlanSettings settings;
    settings.enabled = true;
    settings.by_count = true;
    settings.count = 4;
    settings.duration_sec = 120;
    SolarPlan spring = buildSolarPlan(7.0f, 19.0f, settings);
    assert(spring.error == SolarPlanError::None && spring.count == 4);
    assert(spring.start_minutes[0] == 420 && spring.start_minutes[3] == 1138);
    assert(spring.start_minutes[3] * 60 + settings.duration_sec <= 19 * 3600);

    // Same persistent settings produce different hours as daylight changes.
    SolarPlan summer = buildSolarPlan(4.0f, 20.0f, settings);
    assert(summer.error == SolarPlanError::None && summer.count == 4);
    assert(summer.start_minutes[0] == 240 && summer.start_minutes[3] == 1198);
    assert(summer.start_minutes[0] != spring.start_minutes[0]);

    settings.count = 1;
    SolarPlan single = buildSolarPlan(6.0f, 18.0f, settings);
    assert(single.error == SolarPlanError::None && single.count == 1);
    assert(single.start_minutes[0] == 360);

    settings.by_count = false;
    settings.interval_min = 120;
    SolarPlan periodic = buildSolarPlan(7.0f, 19.0f, settings);
    assert(periodic.error == SolarPlanError::None && periodic.count == 6);
    assert(periodic.start_minutes[1] - periodic.start_minutes[0] == 120);

    settings.interval_min = 5;
    expected(SolarPlanError::TooManySlots, 4.0f, 20.0f, settings);
    settings.interval_min = 120;
    expected(SolarPlanError::NoDaylight, 0.0f, 0.0f, settings);
    expected(SolarPlanError::NoDaylight, 0.0f, 24.0f, settings);
    expected(SolarPlanError::NoDaylight, 19.0f, 7.0f, settings);

    settings.by_count = true;
    settings.count = 24;
    settings.duration_sec = 1200;
    expected(SolarPlanError::OverlappingSlots, 6.0f, 7.0f, settings);
    settings.duration_sec = 3600;
    expected(SolarPlanError::WindowTooShort, 6.0f, 6.5f, settings);

    settings.count = 0;
    expected(SolarPlanError::InvalidParameters, 7.0f, 19.0f, settings);
    puts("solar plan tests: OK");
    return 0;
}
