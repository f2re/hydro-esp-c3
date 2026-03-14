#pragma once
#include <Arduino.h>
#include <Update.h>

class OTAManager {
public:
    void begin();
    int getProgress() { return progress; }
    bool isUpdating() { return updating; }
    void setProgress(size_t current, size_t total);
    void end(bool success);

private:
    int progress = 0;
    bool updating = false;
};

extern OTAManager otaManager;
