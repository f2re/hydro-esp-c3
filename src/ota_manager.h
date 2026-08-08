#pragma once
#include <Arduino.h>
#include <Update.h>

class OTAManager {
public:
    void begin();
    int getProgress() const { return progress; }
    bool isUpdating() const { return updating; }
    void setProgress(size_t current, size_t total);
    void end(bool success);

private:
    // HTTP OTA callbacks can run outside the main loop task. These are simple
    // aligned scalar state flags shared with the display/control loop.
    volatile int progress = 0;
    volatile bool updating = false;
};

extern OTAManager otaManager;
