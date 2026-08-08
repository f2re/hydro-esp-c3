#pragma once
#include <Arduino.h>

// ── Hardware profile: ESP32-C3 Super Mini ────────────────────────────────
#define OLED_SCL     6
#define OLED_SDA     5
#define OLED_WIDTH   72
#define OLED_HEIGHT  40
#define X_OFFSET     28
#define Y_OFFSET     24

#define RELAY_PIN    4
#define RELAY_ON     HIGH
#define RELAY_OFF    LOW
#define LED_PIN      8
#define BUTTON_PIN   9

// Temporary compatibility switch for installations where pump startup causes
// a supply dip. Keep enabled only until power wiring/decoupling is corrected.
// Can be overridden with -DHYDRO_DISABLE_BROWNOUT_WORKAROUND=0.
#ifndef HYDRO_DISABLE_BROWNOUT_WORKAROUND
#define HYDRO_DISABLE_BROWNOUT_WORKAROUND 1
#endif

// ── Product limits / contracts ───────────────────────────────────────────
#define MAX_SCHEDULE_SLOTS                48
#define MAX_WATERING_SECONDS              3600
#define DEFAULT_MANUAL_SECONDS            60
#define HYDRO_CALIBRATION_PROTOCOL_VERSION 1
#define WIFI_TIMEOUT_MS                   15000UL
#define WIFI_RETRY_INTERVAL_MS            30000UL
#define NTP_SYNC_INTERVAL                 3600000UL

#define AP_SSID                   "HydroESP-Setup"
#define MDNS_HOST                 "hydro"

// ── Irrigation schedule ──────────────────────────────────────────────────
struct WateringSlot {
    uint8_t hour;
    uint8_t minute;
    uint16_t duration_sec;
};

// Factory/example preset for the current experimental mineral-wool setup.
// This is a fallback timer profile, not a universal agronomic recommendation.
// Calibrate pump flow and edit the schedule in the web UI before unattended use.
static const WateringSlot WATERING_SCHEDULE[] = {
    { 6,  0, 180},
    { 6, 30, 120},
    { 7,  0, 120},
    { 7, 30, 120},

    { 8,  0, 120}, { 8, 20, 120}, { 8, 40, 120},
    { 9,  0, 120}, { 9, 20, 120}, { 9, 40, 120},
    {10,  0, 120}, {10, 20, 120}, {10, 40, 120},
    {11,  0, 120}, {11, 20, 120}, {11, 40, 120},
    {12,  0, 120}, {12, 20, 120}, {12, 40, 120},
    {13,  0, 120}, {13, 20, 120}, {13, 40, 120},
    {14,  0, 120}, {14, 20, 120}, {14, 40, 120},
    {15,  0, 120}, {15, 20, 120}, {15, 40, 120},
    {16,  0, 120}, {16, 20, 120}, {16, 40, 120},
    {17,  0, 120}, {17, 20, 120}, {17, 40, 120},
    {18,  0, 120}, {18, 20, 120}, {18, 40, 120},

    {19,  0, 150},
};

static const uint8_t SCHEDULE_COUNT =
    sizeof(WATERING_SCHEDULE) / sizeof(WateringSlot);

static_assert(SCHEDULE_COUNT <= MAX_SCHEDULE_SLOTS,
              "Factory schedule exceeds MAX_SCHEDULE_SLOTS");

// ── Wi-Fi / time ─────────────────────────────────────────────────────────
// Zero-config builds intentionally embed no Wi-Fi secret. First boot therefore
// enters AP provisioning immediately. Optional factory credentials can be
// injected by scripts/build_flags.py through WIFI_SSID/WIFI_PASSWORD env vars.
#ifndef WIFI_SSID
#define WIFI_SSID ""
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD ""
#endif

#ifndef TIMEZONE_OFFSET
#define TIMEZONE_OFFSET 3
#endif
