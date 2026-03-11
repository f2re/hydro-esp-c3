// wifi_manager.h
#pragma once
#include <WiFi.h>

class WiFiManager {
public:
    bool connect(const char* ssid, const char* pass, uint32_t timeout_ms = 15000);
    bool isConnected();
    void ensureConnected(const char* ssid, const char* pass);
    String localIP();
};
