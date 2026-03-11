#pragma once

// ── Дисплей ─────────────────────────────────────────────
#define OLED_SCL    6
#define OLED_SDA    5
#define OLED_WIDTH  72
#define OLED_HEIGHT 40
#define X_OFFSET    28   // (128-72)/2 = 28
#define Y_OFFSET    24   // (64-40)/2  = 12, но на этой плате 24

// ── Периферия ────────────────────────────────────────────
#define RELAY_PIN   4    // ← было GPIO5 (конфликт с SDA!) → GPIO4
#define RELAY_ON    LOW
#define RELAY_OFF   HIGH
#define LED_PIN     8    // встроенный LED
#define BUTTON_PIN  9    // BOOT-кнопка (INPUT_PULLUP, активна LOW)

// ── Тайминги ──────────────────────────────────────────
const uint32_t ON_TIME  = 5000;   // 5 сек — реле ВКЛ
const uint32_t OFF_TIME = 10000;  // 10 сек — реле ВЫКЛ

// ── Расписание ───────────────────────────────────────────
struct WateringSlot { uint8_t hour, minute; uint16_t duration_sec; };

static const WateringSlot WATERING_SCHEDULE[] = {
    {8,  0, 300},
    {12, 0, 300},
    {18, 0, 300},
    {22, 0, 180},
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
