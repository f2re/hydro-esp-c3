#pragma once

#define RELAY_PIN   5
#define RELAY_ON    LOW    // HW-307: активный LOW
#define RELAY_OFF   HIGH

struct WateringSlot { uint8_t hour, minute; uint16_t duration_sec; };

// Расписание полива — редактируй как нужно
static const WateringSlot WATERING_SCHEDULE[] = {
    {8,  0, 300},   // 08:00 — 5 минут
    {12, 0, 300},   // 12:00 — 5 минут
    {18, 0, 300},   // 18:00 — 5 минут
    {22, 0, 180},   // 22:00 — 3 минуты
};
static const uint8_t SCHEDULE_COUNT =
    sizeof(WATERING_SCHEDULE) / sizeof(WateringSlot);

#ifndef WIFI_SSID
#define WIFI_SSID     "your_ssid"
#endif
#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "your_password"
#endif
#ifndef TIMEZONE_OFFSET
#define TIMEZONE_OFFSET 3  // UTC+3 Москва
#endif

#define WIFI_TIMEOUT_MS    15000
#define NTP_SYNC_INTERVAL  3600000UL
