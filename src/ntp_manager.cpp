// ntp_manager.cpp
#include "ntp_manager.h"
#include "config.h"
#include <Arduino.h>

void NTPManager::begin() {
    _client = new NTPClient(_udp, "pool.ntp.org",
        (long)TIMEZONE_OFFSET * 3600, NTP_SYNC_INTERVAL);
    _client->begin();
    _synced = _client->update();
    if (_synced)
        Serial.printf("[NTP] Synced: %s\n", _client->getFormattedTime().c_str());
    else
        Serial.println("[NTP] Sync failed, will retry...");
}
void NTPManager::update() {
    if (!_client) return;
    if (_client->update() && !_synced) {
        _synced = true;
        Serial.printf("[NTP] Synced: %s\n", _client->getFormattedTime().c_str());
    }
}
bool NTPManager::isSynced() const { return _synced; }
uint8_t NTPManager::getHour()   const { return _client ? _client->getHours()   : 0; }
uint8_t NTPManager::getMinute() const { return _client ? _client->getMinutes() : 0; }
String  NTPManager::getTimeString() const {
    return _client ? _client->getFormattedTime() : "--:--:--";
}
