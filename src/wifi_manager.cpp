#include "wifi_manager.h"
#include <Arduino.h>

bool WiFiManager::connect(const char* ssid, const char* pass, uint32_t timeout_ms) {
    if (ssid == nullptr || strlen(ssid) == 0) return false;

    Serial.printf("[WiFi] Connecting to %s", ssid);
    WiFi.mode(WIFI_STA);
    WiFi.setHostname(MDNS_HOST);
    WiFi.begin(ssid, pass);

    const uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (static_cast<uint32_t>(millis() - start) >= timeout_ms) {
            Serial.println("\n[WiFi] Timeout");
            WiFi.disconnect(false);
            return false;
        }
        delay(250);
        Serial.print(".");
    }

    Serial.printf("\n[WiFi] IP: %s\n", WiFi.localIP().toString().c_str());
    ap_mode = false;
    lastReconnectAttempt = millis();
    return true;
}

bool WiFiManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

void WiFiManager::ensureConnected(const char* ssid, const char* pass) {
    if (ap_mode || isConnected()) return;
    if (ssid == nullptr || strlen(ssid) == 0) return;

    const uint32_t now = millis();
    if (static_cast<uint32_t>(now - lastReconnectAttempt) < WIFI_RETRY_INTERVAL_MS) return;
    lastReconnectAttempt = now;

    Serial.printf("[WiFi] Background reconnect to %s\n", ssid);
    WiFi.mode(WIFI_STA);
    WiFi.setHostname(MDNS_HOST);
    WiFi.disconnect(false);
    WiFi.begin(ssid, pass);
}

String WiFiManager::localIP() {
    return ap_mode ? WiFi.softAPIP().toString() : WiFi.localIP().toString();
}

bool WiFiManager::startAP(const char* ap_ssid, const char* ap_pass) {
    if (ap_ssid == nullptr || strlen(ap_ssid) == 0 ||
        ap_pass == nullptr || strlen(ap_pass) < 8) {
        Serial.println("[WiFi] Refusing to start unprotected provisioning AP");
        ap_mode = false;
        return false;
    }

    Serial.printf("[WiFi] Starting protected provisioning AP: %s\n", ap_ssid);
    WiFi.mode(WIFI_AP);
    if (!WiFi.softAP(ap_ssid, ap_pass)) {
        Serial.println("[WiFi] Failed to start provisioning AP");
        ap_mode = false;
        return false;
    }

    dnsServer.start(53, "*", WiFi.softAPIP());
    ap_mode = true;
    Serial.printf("[WiFi] Provisioning URL: http://%s\n", WiFi.softAPIP().toString().c_str());
    return true;
}

void WiFiManager::updateDNS() {
    if (ap_mode) dnsServer.processNextRequest();
}
