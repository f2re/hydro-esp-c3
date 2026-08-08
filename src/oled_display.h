#pragma once
#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include "config.h"
#include "ntp_manager.h"
#include "relay_controller.h"
#include "wifi_manager.h"
#include "scheduler.h"

enum DisplayPage : uint8_t {
    PAGE_CLOCK = 0,
    PAGE_NEXT = 1,
    PAGE_SCHEDULE = 2,
    PAGE_COUNT = 3
};

constexpr uint32_t PAGE_INTERVAL_MS = 5000;

class OledDisplay {
public:
    void begin();
    void drawBoot(uint8_t step, const char* msg);
    void drawOTA(uint8_t progress);
    void drawProvisioning(const String& ssid, const String& key, const String& ip);
    void update(NTPManager* ntp, RelayController* relay,
                WiFiManager* wifi, Scheduler* scheduler);
    void showPage(DisplayPage page);
    DisplayPage currentPage() const { return _page; }

private:
    U8G2_SSD1306_128X64_NONAME_F_HW_I2C _u8g2{
        U8G2_R0, U8X8_PIN_NONE, OLED_SCL, OLED_SDA};

    DisplayPage _page = PAGE_CLOCK;
    uint32_t _lastSwitch = 0;
    uint8_t _animFrame = 0;
    uint32_t _lastAnim = 0;

    inline int x(int localX) const { return localX + X_OFFSET; }
    inline int y(int localY) const { return localY + Y_OFFSET; }

    void hline(int localY);
    void drawProgressBar(int localX, int localY, int w, int h, float pct);
    void drawWifiIcon(int px, int py, bool connected, int rssi);

    void drawPageClock(NTPManager* ntp, WiFiManager* wifi);
    void drawPageNext(NTPManager* ntp, Scheduler* scheduler);
    void drawPageSchedule(NTPManager* ntp, Scheduler* scheduler);
    void drawPageWatering(RelayController* relay);
};
