#pragma once
#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include "config.h"
#include "ntp_manager.h"
#include "relay_controller.h"
#include "wifi_manager.h"

class OledDisplay {
public:
    void begin();
    void drawBoot(uint8_t step, const char* msg);
    void drawDashboard(NTPManager* ntp,
                       RelayController* relay,
                       WiFiManager* wifi);
    void drawWatering(uint16_t remaining_sec, uint16_t total_sec);
private:
    // Виртуальный буфер 128x64, физически используем 72x40 с офсетом
    U8G2_SSD1306_128X64_NONAME_F_HW_I2C _u8g2{
        U8G2_R0, U8X8_PIN_NONE, OLED_SCL, OLED_SDA};

    // Все draw-вызовы: координаты в локальной системе 72x40
    // Добавляем X_OFFSET / Y_OFFSET внутри методов
    inline int x(int lx) { return lx + X_OFFSET; }
    inline int y(int ly) { return ly + Y_OFFSET; }

    void hline(int ly);                    // горизонтальная черта
    void drawProgressBar(int lx, int ly,
                         int w, int h,
                         float pct);
};
