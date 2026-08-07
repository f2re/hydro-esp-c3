// wifi_manager.h
#pragma once
#include <WiFi.h>
#include <DNSServer.h>

class WiFiManager {
public:
    bool connect(const char* ssid, const char* pass, uint32_t timeout_ms = 15000);
    bool isConnected();
    void ensureConnected(const char* ssid, const char* pass);
    String localIP();

    void startAP(const char* ap_ssid = "HydroESP-Setup", const char* ap_pass = NULL);
    void updateDNS();
    bool isAPMode() { return ap_mode; }

private:
    DNSServer dnsServer;
    bool ap_mode = false;
    uint32_t lastReconnectAttempt = 0;
    static constexpr uint32_t RECONNECT_INTERVAL_MS = 10000;
};
