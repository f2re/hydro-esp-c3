#include "oled_display.h"

void OledDisplay::drawProvisioning(const String& ssid, const String& ip) {
    _u8g2.clearBuffer();
    drawRuCentered("НАСТРОЙКА", 13);

    _u8g2.setFont(u8g2_font_4x6_tr);
    String shownSsid = ssid;
    if (shownSsid.length() > 17) shownSsid = shownSsid.substring(0, 17);
    _u8g2.drawStr(x(1), y(22), shownSsid.c_str());

    String ipLine = "IP " + ip;
    _u8g2.drawStr(x(1), y(30), ipLine.c_str());

    // Open setup AP: no password. A compact unlocked padlock avoids another
    // English label on the 72x40 logical display.
    const int lx = x(2);
    const int ly = y(33);
    _u8g2.drawFrame(lx, ly + 2, 7, 5);
    _u8g2.drawArc(lx + 5, ly + 2, 3, 3, 0, 2);
    _u8g2.drawStr(x(12), y(39), "OPEN Wi-Fi");
    _u8g2.sendBuffer();
}
