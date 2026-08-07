#pragma once
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "config.h"

class NTPManager {
public:
    void begin(int offset_hours = TIMEZONE_OFFSET);
    void update();
    bool isSynced() const;
    uint8_t getHour() const;
    uint8_t getMinute() const;
    String getTimeString() const;
    String getDateString() const;
    uint32_t getLocalEpoch() const;
    void setTimeOffset(int offset_hours);
    int getTimeOffsetHours() const { return _offsetHours; }

private:
    WiFiUDP _udp;
    NTPClient* _client = nullptr;
    bool _synced = false;
    int _offsetHours = TIMEZONE_OFFSET;
};
