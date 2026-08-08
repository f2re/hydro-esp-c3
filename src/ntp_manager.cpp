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

uint32_t NTPManager::getEpochTime() const {
    return _client ? _client->getEpochTime() : 0;
}

void NTPManager::getSunriseSunset(float lat, float lon, float &sunrise, float &sunset) {
    if (!_synced || lat == 0.0) { 
        sunrise = 6.0; 
        sunset = 18.0; 
        return; 
    }
    
    time_t rawtime = _client->getEpochTime();
    struct tm * ti = localtime(&rawtime);
    
    int day = ti->tm_yday;
    float latRad = lat * PI / 180.0;
    float declination = 0.409 * sin(2.0 * PI * (day - 81) / 365.0);
    float arg = -tan(latRad) * tan(declination);
    
    if (arg > 1.0) { // Polar night
        sunrise = 0; sunset = 0; return;
    } else if (arg < -1.0) { // Polar day
        sunrise = 0; sunset = 24; return;
    }
    
    float hourAngle = acos(arg);
    float sunsetTime = 12.0 + hourAngle * 12.0 / PI;
    float sunriseTime = 12.0 - hourAngle * 12.0 / PI;
    
    float lonCorrection = (lon - (15.0 * (float)_offset)) / 15.0;
    
    sunrise = sunriseTime - lonCorrection;
    sunset = sunsetTime - lonCorrection;
    
    if (sunrise < 0) sunrise += 24;
    if (sunrise >= 24) sunrise -= 24;
    if (sunset < 0) sunset += 24;
    if (sunset >= 24) sunset -= 24;
}

void NTPManager::setTimeOffset(int offset_hours) {
    _offset = offset_hours;
    if (_client) _client->setTimeOffset(offset_hours * 3600);
}

