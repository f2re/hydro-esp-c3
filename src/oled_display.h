#pragma once
#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include "config.h"
#include "ntp_manager.h"
#include "relay_controller.h"
#include "wifi_manager.h"

// Страницы дисплея
enum DisplayPage : uint8_t {
    PAGE_CLOCK    = 0,  // крупное время + дата + WiFi
    PAGE_NEXT     = 1,  // обратный отсчёт до полива
    PAGE_SCHEDULE = 2,  // список расписания
    PAGE_COUNT    = 3
};

#define PAGE_INTERVAL_MS  5000  // смена страницы каждые 5 сек

class OledDisplay {
public:
    void begin();
    void drawBoot(uint8_t step, const char* msg);

    // Главный метод — вызывать в loop(), сам управляет ротацией
    void update(NTPManager* ntp, RelayController* relay, WiFiManager* wifi);

    // Принудительно показать страницу (напр. при нажатии кнопки)
    void showPage(DisplayPage p);
    
    DisplayPage currentPage() const { return _page; }

private:
    // Используем драйвер SSD1306 для 72x40 с офсетами
    U8G2_SSD1306_128X64_NONAME_F_HW_I2C _u8g2{
        U8G2_R0, U8X8_PIN_NONE, OLED_SCL, OLED_SDA};

    DisplayPage _page          = PAGE_CLOCK;
    uint32_t    _lastSwitch    = 0;
    uint8_t     _animFrame     = 0;
    uint32_t    _lastAnim      = 0;

    inline int x(int lx) { return lx + X_OFFSET; }
    inline int y(int ly) { return ly + Y_OFFSET; }

    void hline(int ly);
    void drawProgressBar(int lx, int ly, int w, int h, float pct);
    void drawWifiIcon(int px, int py, bool connected, int rssi);

    // Страницы
    void drawPageClock   (NTPManager* ntp, WiFiManager* wifi);
    void drawPageNext    (NTPManager* ntp, RelayController* relay);
    void drawPageSchedule(NTPManager* ntp);
    void drawPageWatering(RelayController* relay); // при активном поливе
};
