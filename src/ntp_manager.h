// ntp_manager.h
#pragma once
#include <NTPClient.h>
#include <WiFiUdp.h>

class NTPManager {
public:
    void begin();
    void update();
    bool isSynced() const;
    uint8_t getHour() const;
    uint8_t getMinute() const;
    String getTimeString() const;
    String getDateString() const;
    void setTimeOffset(int offset_hours);
    uint32_t getEpochTime() const;
    void getSunriseSunset(float lat, float lon, float &sunrise, float &sunset);
private:
    WiFiUDP _udp;
    NTPClient* _client = nullptr;
    bool _synced = false;
    int _offset = 0;
};
