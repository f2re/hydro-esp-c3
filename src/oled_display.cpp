#include "oled_display.h"
#include "version.h"
#include <WiFi.h>

void OledDisplay::begin() {
    Wire.begin(OLED_SDA, OLED_SCL);
    _u8g2.begin();
    _u8g2.setBusClock(400000);
    _u8g2.setContrast(255);
}

void OledDisplay::showPage(DisplayPage page) {
    _page = page;
    _lastSwitch = millis();
}

void OledDisplay::hline(int localY) {
    _u8g2.drawHLine(x(0), y(localY), OLED_WIDTH);
}

void OledDisplay::drawProgressBar(int localX, int localY, int w, int h, float pct) {
    if (pct < 0.0f) pct = 0.0f;
    if (pct > 1.0f) pct = 1.0f;
    _u8g2.drawFrame(x(localX), y(localY), w, h);
    const int fill = static_cast<int>(pct * (w - 2));
    if (fill > 0) _u8g2.drawBox(x(localX) + 1, y(localY) + 1, fill, h - 2);
}

void OledDisplay::drawWifiIcon(int px, int py, bool connected, int rssi) {
    if (!connected) {
        _u8g2.drawLine(px, py + 1, px + 5, py + 6);
        _u8g2.drawLine(px + 5, py + 1, px, py + 6);
        return;
    }

    const int bars = rssi >= -60 ? 3 : (rssi >= -75 ? 2 : 1);
    _u8g2.drawPixel(px + 3, py + 7);
    _u8g2.drawHLine(px + 2, py + 5, 3);
    if (bars >= 2) _u8g2.drawHLine(px + 1, py + 3, 5);
    if (bars >= 3) _u8g2.drawHLine(px, py + 1, 7);
}

void OledDisplay::drawBoot(uint8_t step, const char* msg) {
    _u8g2.clearBuffer();
    _u8g2.setFont(u8g2_font_5x7_tr);
    _u8g2.drawStr(x(1), y(7), "HydroESP-C3");

    String version = HYDRO_VERSION;
    if (version.length() > 11) version = version.substring(0, 11);
    _u8g2.setFont(u8g2_font_4x6_tr);
    _u8g2.drawStr(x(45), y(7), version.c_str());
    hline(10);

    char line[24];
    snprintf(line, sizeof(line), "%u/4 %s", step, msg);
    _u8g2.setFont(u8g2_font_5x7_tr);
    _u8g2.drawStr(x(1), y(21), line);
    drawProgressBar(1, 29, 70, 6, static_cast<float>(step) / 4.0f);
    _u8g2.sendBuffer();
}

void OledDisplay::drawOTA(uint8_t progress) {
    _u8g2.clearBuffer();
    _u8g2.setFont(u8g2_font_5x7_tr);
    _u8g2.drawStr(x(2), y(8), "OTA UPDATE...");
    hline(10);

    char line[20];
    snprintf(line, sizeof(line), "Flashing: %u%%", progress);
    _u8g2.drawStr(x(1), y(20), line);
    drawProgressBar(1, 28, 70, 6, static_cast<float>(progress) / 100.0f);
    _u8g2.sendBuffer();
}

void OledDisplay::update(NTPManager* ntp, RelayController* relay,
                         WiFiManager* wifi, Scheduler* scheduler) {
    const uint32_t now = millis();

    if (static_cast<uint32_t>(now - _lastAnim) >= 250) {
        _animFrame = (_animFrame + 1) % 4;
        _lastAnim = now;
    }

    if (relay->isOn()) {
        drawPageWatering(relay);
        _page = PAGE_CLOCK;
        _lastSwitch = now;
        return;
    }

    if (static_cast<uint32_t>(now - _lastSwitch) >= PAGE_INTERVAL_MS) {
        _page = static_cast<DisplayPage>((_page + 1) % PAGE_COUNT);
        _lastSwitch = now;
    }

    switch (_page) {
        case PAGE_CLOCK: drawPageClock(ntp, wifi); break;
        case PAGE_NETWORK: drawPageNetwork(wifi); break;
        case PAGE_NEXT: drawPageNext(ntp, scheduler); break;
        case PAGE_SCHEDULE: drawPageSchedule(ntp, scheduler); break;
        default: _page = PAGE_CLOCK; break;
    }
}

void OledDisplay::drawPageClock(NTPManager* ntp, WiFiManager* wifi) {
    _u8g2.clearBuffer();

    _u8g2.setFont(u8g2_font_9x18_tr);
    const String time = ntp->isSynced() ? ntp->getTimeString().substring(0, 5) : "--:--";
    _u8g2.drawStr(x(13), y(19), time.c_str());

    hline(22);
    _u8g2.setFont(u8g2_font_4x6_tr);

    if (wifi->isAPMode()) {
        _u8g2.drawStr(x(1), y(31), "AP 192.168.4.1");
    } else if (wifi->isConnected()) {
        const int rssi = WiFi.RSSI();
        drawWifiIcon(x(1), y(24), true, rssi);
        String ip = wifi->localIP();
        const int lastDot = ip.lastIndexOf('.');
        const String tail = lastDot >= 0 ? ip.substring(lastDot + 1) : ip;
        char line[22];
        snprintf(line, sizeof(line), "IP *.%s  %ddB", tail.c_str(), rssi);
        _u8g2.drawStr(x(10), y(31), line);
    } else {
        drawWifiIcon(x(1), y(24), false, -100);
        _u8g2.drawStr(x(10), y(31), "WiFi reconnect");
    }

    if (!ntp->isSynced() && (_animFrame % 2 == 0)) {
        _u8g2.drawStr(x(52), y(7), "NTP!");
    }

    _u8g2.sendBuffer();
}

void OledDisplay::drawPageNetwork(WiFiManager* wifi) {
    _u8g2.clearBuffer();
    _u8g2.setFont(u8g2_font_5x7_tr);
    _u8g2.drawStr(x(1), y(7), "WEB ADDRESS");
    hline(10);

    _u8g2.setFont(u8g2_font_4x6_tr);
    if (wifi->isConnected()) {
        const String ip = wifi->localIP();
        _u8g2.drawStr(x(1), y(19), "OPEN IN BROWSER");
        _u8g2.drawStr(x(1), y(27), ip.c_str());
        _u8g2.drawStr(x(1), y(35), "or hydro.local");
    } else {
        _u8g2.drawStr(x(1), y(21), "WiFi offline");
        _u8g2.drawStr(x(1), y(31), "check Serial");
    }
    _u8g2.sendBuffer();
}

void OledDisplay::drawPageNext(NTPManager* ntp, Scheduler* scheduler) {
    _u8g2.clearBuffer();
    _u8g2.setFont(u8g2_font_5x7_tr);
    _u8g2.drawStr(x(1), y(7), "NEXT WATERING");
    hline(10);

    if (!ntp->isSynced()) {
        _u8g2.drawStr(x(11), y(25), "WAIT FOR NTP");
        _u8g2.sendBuffer();
        return;
    }

    WateringSlot slot {};
    int minutes = 0;
    if (!scheduler->getNextSlot(slot, minutes)) {
        _u8g2.drawStr(x(9), y(25), "NO SCHEDULE");
        _u8g2.sendBuffer();
        return;
    }

    char timeStr[6];
    snprintf(timeStr, sizeof(timeStr), "%02u:%02u", slot.hour, slot.minute);
    _u8g2.setFont(u8g2_font_9x18_tr);
    _u8g2.drawStr(x(13), y(28), timeStr);

    const String fullTime = ntp->getTimeString();
    const int seconds = fullTime.length() >= 8 ? fullTime.substring(6, 8).toInt() : 0;
    long remaining = static_cast<long>(minutes) * 60L - seconds;
    if (remaining < 0) remaining += 86400L;

    const int hours = remaining / 3600;
    const int mins = (remaining % 3600) / 60;
    char bottom[24];
    if (hours > 0) {
        snprintf(bottom, sizeof(bottom), "in %dh%02dm  %us", hours, mins, slot.duration_sec);
    } else {
        snprintf(bottom, sizeof(bottom), "in %dm  dur %us", mins, slot.duration_sec);
    }
    _u8g2.setFont(u8g2_font_4x6_tr);
    _u8g2.drawStr(x(1), y(38), bottom);
    _u8g2.sendBuffer();
}

void OledDisplay::drawPageSchedule(NTPManager* ntp, Scheduler* scheduler) {
    _u8g2.clearBuffer();
    _u8g2.setFont(u8g2_font_4x6_tr);
    _u8g2.drawStr(x(1), y(6), "SCHEDULE");

    char countLine[10];
    snprintf(countLine, sizeof(countLine), "%u slots", scheduler->count());
    _u8g2.drawStr(x(43), y(6), countLine);
    hline(8);

    if (scheduler->count() == 0) {
        _u8g2.drawStr(x(12), y(24), "NO SCHEDULE");
        _u8g2.sendBuffer();
        return;
    }

    WateringSlot next {};
    int minutes = 0;
    uint8_t firstIndex = 0;
    if (scheduler->getNextSlot(next, minutes)) {
        for (uint8_t i = 0; i < scheduler->count(); ++i) {
            WateringSlot candidate {};
            if (scheduler->getSlot(i, candidate) &&
                candidate.hour == next.hour && candidate.minute == next.minute) {
                firstIndex = i;
                break;
            }
        }
    }

    const uint8_t rows = scheduler->count() < 4 ? scheduler->count() : 4;
    for (uint8_t rowIndex = 0; rowIndex < rows; ++rowIndex) {
        const uint8_t index = (firstIndex + rowIndex) % scheduler->count();
        WateringSlot slot {};
        if (!scheduler->getSlot(index, slot)) continue;

        char line[18];
        snprintf(line, sizeof(line), "%c%02u:%02u %3us",
                 rowIndex == 0 ? '>' : ' ',
                 slot.hour, slot.minute, slot.duration_sec);

        const int row = 15 + rowIndex * 7;
        if (rowIndex == 0 && ntp->isSynced()) {
            _u8g2.setDrawColor(1);
            _u8g2.drawBox(x(0), y(row - 6), OLED_WIDTH, 7);
            _u8g2.setDrawColor(0);
            _u8g2.drawStr(x(1), y(row), line);
            _u8g2.setDrawColor(1);
        } else {
            _u8g2.drawStr(x(1), y(row), line);
        }
    }

    _u8g2.sendBuffer();
}

void OledDisplay::drawPageWatering(RelayController* relay) {
    _u8g2.clearBuffer();
    _u8g2.setFont(u8g2_font_7x13_tr);
    _u8g2.drawStr(x(1), y(13), "WATERING");
    if (_animFrame % 2 == 0) hline(14);

    const uint16_t remaining = relay->remainingSec();
    char remStr[8];
    snprintf(remStr, sizeof(remStr), "%us", remaining);
    _u8g2.setFont(u8g2_font_9x18_tr);
    const int width = strlen(remStr) * 9;
    _u8g2.drawStr(x((72 - width) / 2), y(30), remStr);

    const float pct = relay->progress();
    if (pct >= 0.0f) drawProgressBar(1, 33, 70, 5, pct);

    const int dropY = y(18) + (_animFrame * 2);
    _u8g2.drawCircle(x(63), dropY, 2);
    _u8g2.sendBuffer();
}
