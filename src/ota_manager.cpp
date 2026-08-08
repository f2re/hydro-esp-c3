#include "ota_manager.h"

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
        // Keep the OTA latch active until ESP.restart(). This prevents the main
        // loop from drawing normal pages or running scheduled actions in the
        // short interval between Update.end(true) and the actual reboot.
        progress = 100;
        updating = true;
    } else {
        progress = 0;
        updating = false;
    }
}

OTAManager otaManager;
