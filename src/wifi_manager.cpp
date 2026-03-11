// wifi_manager.cpp
#include "wifi_manager.h"
#include <Arduino.h>

bool WiFiManager::connect(const char* ssid, const char* pass, uint32_t timeout_ms) {
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
    return true;
}
bool WiFiManager::isConnected() { return WiFi.status() == WL_CONNECTED; }
void WiFiManager::ensureConnected(const char* ssid, const char* pass) {
    if (!isConnected()) connect(ssid, pass);
}
String WiFiManager::localIP() { return WiFi.localIP().toString(); }
