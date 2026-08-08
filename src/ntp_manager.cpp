#include "ntp_manager.h"
#include <Arduino.h>
#include <time.h>

void NTPManager::begin(int offset_hours) {
    setTimeOffset(offset_hours);
    _client = new NTPClient(_udp, "pool.ntp.org",
                            static_cast<long>(_offsetHours) * 3600L,
                            NTP_SYNC_INTERVAL);
    _client->begin();
    _synced = _client->update();
    if (_synced) {
        Serial.printf("[NTP] Synced: %s UTC%+d\n",
                      _client->getFormattedTime().c_str(), _offsetHours);
    } else {
        Serial.println("[NTP] Sync failed, will retry");
    }
}

void NTPManager::update() {
    if (!_client) return;
    if (_client->update()) {
        if (!_synced) {
            Serial.printf("[NTP] Synced: %s UTC%+d\n",
                          _client->getFormattedTime().c_str(), _offsetHours);
        }
        _synced = true;
    }
}

bool NTPManager::isSynced() const {
    return _synced;
}

uint8_t NTPManager::getHour() const {
    return _client ? _client->getHours() : 0;
}

uint8_t NTPManager::getMinute() const {
    return _client ? _client->getMinutes() : 0;
}

String NTPManager::getTimeString() const {
    return (_synced && _client) ? _client->getFormattedTime() : "--:--:--";
}

uint32_t NTPManager::getLocalEpoch() const {
    return (_synced && _client) ? static_cast<uint32_t>(_client->getEpochTime()) : 0;
}

void NTPManager::getSunriseSunset(float latitude, float longitude,
                                  float &sunrise, float &sunset) const {
    sunrise = 6.0f;
    sunset = 18.0f;
    if (!_synced || !_client || latitude < -90.0f || latitude > 90.0f ||
        longitude < -180.0f || longitude > 180.0f) {
        return;
    }

    const time_t localEpoch = static_cast<time_t>(_client->getEpochTime());
    struct tm dayInfo {};
    gmtime_r(&localEpoch, &dayInfo);

    const float latitudeRad = latitude * PI / 180.0f;
    const float declination = 0.409f * sinf(
        2.0f * PI * (static_cast<float>(dayInfo.tm_yday) - 81.0f) / 365.0f);
    const float argument = -tanf(latitudeRad) * tanf(declination);
    if (argument >= 1.0f) {
        sunrise = 0.0f;
        sunset = 0.0f;
        return;
    }
    if (argument <= -1.0f) {
        sunrise = 0.0f;
        sunset = 24.0f;
        return;
    }

    const float hourAngle = acosf(argument);
    const float solarSunrise = 12.0f - hourAngle * 12.0f / PI;
    const float solarSunset = 12.0f + hourAngle * 12.0f / PI;
    const float correction = (longitude - 15.0f * _offsetHours) / 15.0f;
    sunrise = fmodf(solarSunrise - correction + 24.0f, 24.0f);
    sunset = fmodf(solarSunset - correction + 24.0f, 24.0f);
}

String NTPManager::getDateString() const {
    if (!_synced || !_client) return "--.--.----";

    // NTPClient::getEpochTime() already includes the configured offset. Use
    // UTC calendar conversion so host/process TZ cannot shift the date again.
    const time_t localEpoch = static_cast<time_t>(_client->getEpochTime());
    struct tm value {};
    gmtime_r(&localEpoch, &value);

    char buf[12];
    snprintf(buf, sizeof(buf), "%02d.%02d.%04d",
             value.tm_mday, value.tm_mon + 1, value.tm_year + 1900);
    return String(buf);
}

void NTPManager::setTimeOffset(int offset_hours) {
    if (offset_hours < -12) offset_hours = -12;
    if (offset_hours > 14) offset_hours = 14;
    _offsetHours = offset_hours;
    if (_client) _client->setTimeOffset(static_cast<long>(_offsetHours) * 3600L);
}
