#include "ota_manager.h"

void OTAManager::begin() {
    progress = 0;
    updating = true;
}

void OTAManager::setProgress(size_t current, size_t total) {
    if (total > 0) {
        progress = (current * 100) / total;
    }
}

void OTAManager::end(bool success) {
    updating = false;
    if (success) {
        progress = 100;
    }
}

OTAManager otaManager;
