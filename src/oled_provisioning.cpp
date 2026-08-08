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

    // Open setup AP: an unlocked padlock communicates "без пароля" without
    // spending another text row on the very small 72x40 logical viewport.
    // Keep it to basic line/frame primitives so it is portable across the
    // pinned U8g2 API and costs essentially nothing in flash.
    const int lx = x(32);
    const int ly = y(33);
    _u8g2.drawFrame(lx, ly + 2, 8, 5);
    _u8g2.drawLine(lx + 5, ly + 2, lx + 5, ly);
    _u8g2.drawLine(lx + 5, ly, lx + 8, ly);
    _u8g2.sendBuffer();
}
