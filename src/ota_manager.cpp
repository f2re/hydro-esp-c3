#include "ota_manager.h"

#include <esp_system.h>
#include <esp_timer.h>

namespace {
esp_timer_handle_t restartTimer = nullptr;

void restartTimerCallback(void*) {
    esp_restart();
}
}

void OTAManager::begin() {
    progress = 0;
    updating = true;
}

void OTAManager::setProgress(size_t current, size_t total) {
    if (!updating || total == 0) return;
    progress = static_cast<int>((static_cast<uint64_t>(current) * 100U) / total);
    if (progress > 100) progress = 100;
}

void OTAManager::end(bool success) {
    if (success) {
        // Keep the OTA latch active until the actual restart. This prevents the
        // normal scheduler/display loop from resuming between Update.end(true)
        // and the reboot.
        progress = 100;
        updating = true;
    } else {
        progress = 0;
        updating = false;
    }
}

void OTAManager::scheduleRestart(uint32_t delayMs) {
    if (delayMs < 250) delayMs = 250;

    // A pending reboot is a maintenance/safety state too. Stop the ordinary
    // application loop immediately so a schedule slot or button press cannot
    // start the pump during the short ACK -> reboot window.
    updating = true;

    if (!restartTimer) {
        esp_timer_create_args_t args{};
        args.callback = &restartTimerCallback;
        args.name = "hydro-restart";
        if (esp_timer_create(&args, &restartTimer) != ESP_OK) {
            Serial.println("[OTA] ERROR: cannot create deferred restart timer");
            updating = false;
            return;
        }
    }

    // Ignore INVALID_STATE when the timer is currently idle.
    esp_timer_stop(restartTimer);
    if (esp_timer_start_once(restartTimer, static_cast<uint64_t>(delayMs) * 1000ULL) != ESP_OK) {
        Serial.println("[OTA] ERROR: cannot schedule deferred restart");
        updating = false;
    }
}

OTAManager otaManager;
