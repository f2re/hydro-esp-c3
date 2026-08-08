#pragma once
#include <WiFi.h>
#include <DNSServer.h>
#include "config.h"

class WiFiManager {
public:
    bool connect(const char* ssid, const char* pass, uint32_t timeout_ms = WIFI_TIMEOUT_MS);
    bool isConnected();
    void ensureConnected(const char* ssid, const char* pass);
    String localIP();

    // Setup network is intentionally open: first installation must require
    // only selecting HydroESP-Setup and opening the local page.
    bool startAP(const char* ap_ssid);
    void updateDNS();
    bool isAPMode() const { return ap_mode; }

private:
    DNSServer dnsServer;
    bool ap_mode = false;
    uint32_t lastReconnectAttempt = 0;
};
