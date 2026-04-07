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
#define RELAY_ON    HIGH
#define RELAY_OFF   LOW
#define LED_PIN     8    // встроенный LED
#define BUTTON_PIN  9    // BOOT-кнопка (INPUT_PULLUP, активна LOW)

// ── Расписание ───────────────────────────────────────────
struct WateringSlot { uint8_t hour, minute; uint16_t duration_sec; };

// Расписание для минваты (20-мин режим, без ночных поливов)
static const WateringSlot WATERING_SCHEDULE[] = {
//  {час, мин, длительность_сек}
    // УТРО: первый полив длиннее — восстанавливаем ночную сушку
    { 6,  0,  180},   // рассвет — 3 мин (восстановительный)
    { 6, 30,  120},
    { 7,  0,  120},
    { 7, 30,  120},
    // ДЕНЬ: каждые 20 мин, 2 мин включения
    { 8,  0,  120}, { 8, 20,  120}, { 8, 40,  120},
    { 9,  0,  120}, { 9, 20,  120}, { 9, 40,  120},
    {10,  0,  120}, {10, 20,  120}, {10, 40,  120},
    {11,  0,  120}, {11, 20,  120}, {11, 40,  120},
    {12,  0,  120}, {12, 20,  120}, {12, 40,  120},
    {13,  0,  120}, {13, 20,  120}, {13, 40,  120},
    {14,  0,  120}, {14, 20,  120}, {14, 40,  120},
    {15,  0,  120}, {15, 20,  120}, {15, 40,  120},
    {16,  0,  120}, {16, 20,  120}, {16, 40,  120},
    {17,  0,  120}, {17, 20,  120}, {17, 40,  120},
    {18,  0,  120}, {18, 20,  120}, {18, 40,  120},
    // ВЕЧЕР: последний полив до темноты
    {19,  0,  150},   // 2.5 мин — финальное увлажнение
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
