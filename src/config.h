#pragma once

// ── Дисплей ─────────────────────────────────────────────
#define OLED_SCL    6
#define OLED_SDA    5
#define OLED_WIDTH  72
#define OLED_HEIGHT 40
#define X_OFFSET    28   // (128-72)/2 = 28
#define Y_OFFSET    24   // (64-40)/2  = 12, но на этой плате 24

// ── Периферия ────────────────────────────────────────────
#define RELAY_PIN   4
#define RELAY_ON    LOW
#define RELAY_OFF   HIGH
#define LED_PIN     8    // встроенный LED
#define BUTTON_PIN  9    // BOOT-кнопка (INPUT_PULLUP, активна LOW)

// ── Расписание ───────────────────────────────────────────
struct WateringSlot { uint8_t hour, minute; uint16_t duration_sec; };

// Расширенное расписание для башенной гидропоники
static const WateringSlot WATERING_SCHEDULE[] = {
//  {час, мин, длительность_сек}
    { 6,  0,  180},  // рассвет — мягкий старт 3 мин
    { 8,  0,  300},  // утро — 5 мин (активный рост)
    {10,  0,  300},
    {12,  0,  300},  // полдень — поддержание влажности
    {14,  0,  300},
    {16,  0,  300},  // вечер — до захода
    {18,  0,  300},
    {20,  0,  240},  // сумерки — 4 мин
    {22,  0,  180},  // ночь — минимум, 3 мин
};
static const uint8_t SCHEDULE_COUNT =
    sizeof(WATERING_SCHEDULE) / sizeof(WateringSlot);

// ── WiFi / NTP ───────────────────────────────────────────
#ifndef WIFI_SSID
#define WIFI_SSID     "your_ssid"
#endif
#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "your_password"
#endif
#ifndef TIMEZONE_OFFSET
#define TIMEZONE_OFFSET 3
#endif

#define WIFI_TIMEOUT_MS    15000
#define NTP_SYNC_INTERVAL  3600000UL
