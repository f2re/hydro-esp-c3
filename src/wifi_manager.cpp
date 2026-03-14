// wifi_manager.cpp
#include "wifi_manager.h"
#include <Arduino.h>

bool WiFiManager::connect(const char* ssid, const char* pass, uint32_t timeout_ms) {
    if (ssid == nullptr || strlen(ssid) == 0) return false;
    Serial.printf("[WiFi] Connecting to %s", ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);
    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - start >= timeout_ms) {
            Serial.println("\n[WiFi] Timeout!");
            return false;
        }
        delay(500); Serial.print(".");
    }
    Serial.printf("\n[WiFi] IP: %s\n", WiFi.localIP().toString().c_str());
    ap_mode = false;
    return true;
}

bool WiFiManager::isConnected() { return WiFi.status() == WL_CONNECTED; }

void WiFiManager::ensureConnected(const char* ssid, const char* pass) {
    if (!ap_mode && !isConnected()) connect(ssid, pass);
}

String WiFiManager::localIP() { return WiFi.localIP().toString(); }

void WiFiManager::startAP(const char* ap_ssid, const char* ap_pass) {
    Serial.printf("[WiFi] Starting AP: %s\n", ap_ssid);
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ap_ssid, ap_pass);
    dnsServer.start(53, "*", WiFi.softAPIP());
    ap_mode = true;
    Serial.printf("[WiFi] AP IP: %s\n", WiFi.softAPIP().toString().c_str());
}

void WiFiManager::updateDNS() {
    if (ap_mode) {
        dnsServer.processNextRequest();
    }
}
