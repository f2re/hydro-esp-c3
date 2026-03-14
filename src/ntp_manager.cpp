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
String NTPManager::getTimeString() const {
    return _synced ? _client->getFormattedTime() : "--:--:--";
}

String NTPManager::getDateString() const {
    if (!_synced) return "--.--.----";
    time_t rawtime = _client->getEpochTime();
    struct tm * ti;
    ti = localtime(&rawtime);
    char buf[12];
    snprintf(buf, 12, "%02d.%02d.%04d", ti->tm_mday, ti->tm_mon + 1, ti->tm_year + 1900);
    return String(buf);
}

void NTPManager::setTimeOffset(int offset_hours) {
    if (_client) _client->setTimeOffset(offset_hours * 3600);
}

