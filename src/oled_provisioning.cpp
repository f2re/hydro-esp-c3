#include "oled_display.h"

void OledDisplay::drawProvisioning(const String& ssid, const String& key, const String& ip) {
    _u8g2.clearBuffer();
    _u8g2.setFont(u8g2_font_5x7_tr);
    _u8g2.drawStr(x(1), y(7), "SETUP WIFI");
    hline(10);

    _u8g2.setFont(u8g2_font_4x6_tr);
    _u8g2.drawStr(x(1), y(17), ssid.c_str());

    String keyLine = "KEY " + key;
    _u8g2.drawStr(x(1), y(25), keyLine.c_str());

    String ipLine = "OPEN " + ip;
    _u8g2.drawStr(x(1), y(33), ipLine.c_str());
    _u8g2.sendBuffer();
}
