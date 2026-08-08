#include "oled_display.h"

void OledDisplay::drawProvisioning(const String& ssid, const String& ip) {
    _u8g2.clearBuffer();
    _u8g2.setFont(u8g2_font_5x7_tr);
    _u8g2.drawStr(x(1), y(7), "SETUP WIFI");
    hline(10);

    _u8g2.setFont(u8g2_font_4x6_tr);
    _u8g2.drawStr(x(1), y(18), ssid.c_str());
    _u8g2.drawStr(x(1), y(26), "NO PASSWORD");

    String ipLine = "OPEN " + ip;
    _u8g2.drawStr(x(1), y(34), ipLine.c_str());
    _u8g2.sendBuffer();
}
