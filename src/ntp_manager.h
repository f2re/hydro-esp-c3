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
private:
    WiFiUDP _udp;
    NTPClient* _client = nullptr;
    bool _synced = false;
};
