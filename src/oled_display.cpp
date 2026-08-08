#include "oled_display.h"
#include "version.h"
#include <WiFi.h>

namespace {
const char* bootMessageRu(const char* msg) {
    if (!msg) return "ЗАПУСК";
    if (strcmp(msg, "Boot...") == 0) return "ЗАПУСК";
    if (strcmp(msg, "Init...") == 0) return "ИНИЦ.";
    if (strcmp(msg, "WiFi...") == 0) return "Wi-Fi";
    if (strcmp(msg, "WiFi OK") == 0) return "Wi-Fi OK";
    if (strcmp(msg, "AP failed") == 0) return "ОШИБКА AP";
    if (strcmp(msg, "NTP...") == 0) return "ВРЕМЯ";
    if (strcmp(msg, "NTP OK") == 0) return "ВРЕМЯ OK";
    if (strcmp(msg, "NTP wait") == 0) return "ЖДЕМ NTP";
    if (strcmp(msg, "Ready!") == 0) return "ГОТОВО";
    return msg;
}
}

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

void OledDisplay::drawRuCentered(const char* text, int localBaseline) {
    _u8g2.setFont(u8g2_font_unifont_t_cyrillic);
    const int width = _u8g2.getUTF8Width(text);
    const int localX = width < OLED_WIDTH ? (OLED_WIDTH - width) / 2 : 0;
    _u8g2.drawUTF8(x(localX), y(localBaseline), text);
}

void OledDisplay::drawBoot(uint8_t step, const char* msg) {
    _u8g2.clearBuffer();
    _u8g2.setFont(u8g2_font_4x6_tr);
    _u8g2.drawStr(x(1), y(6), "HydroESP");

    String version = HYDRO_VERSION;
    if (version.length() > 8) version = version.substring(0, 8);
    _u8g2.drawStr(x(39), y(6), version.c_str());
    hline(8);

    drawRuCentered(bootMessageRu(msg), 24);
    drawProgressBar(1, 32, 70, 6, static_cast<float>(step) / 4.0f);
    _u8g2.sendBuffer();
}

void OledDisplay::drawOTA(uint8_t progress) {
    const uint32_t now = millis();
    if (progress > 100) progress = 100;

    // OTA callbacks can arrive much faster than a 400 kHz I2C full-buffer OLED
    // can usefully refresh. Redraw only on visible progress change or at a
    // modest time cadence; 0 and 100 are always rendered immediately.
    if (progress != 0 && progress != 100 && _lastOtaProgress >= 0) {
        const int delta = abs(static_cast<int>(progress) - _lastOtaProgress);
        if ((delta < 2 && static_cast<uint32_t>(now - _lastOtaDraw) < 250U) ||
            (delta == 0 && static_cast<uint32_t>(now - _lastOtaDraw) < 500U)) {
            return;
        }
    }
    _lastOtaProgress = progress;
    _lastOtaDraw = now;

    _u8g2.clearBuffer();
    drawRuCentered("ПРОШИВКА", 14);

    char line[8];
    snprintf(line, sizeof(line), "%u%%", progress);
    _u8g2.setFont(u8g2_font_9x18_tr);
    const int width = _u8g2.getStrWidth(line);
    _u8g2.drawStr(x((OLED_WIDTH - width) / 2), y(31), line);
    drawProgressBar(1, 35, 70, 5, static_cast<float>(progress) / 100.0f);
    _u8g2.sendBuffer();
}

void OledDisplay::drawOTAComplete() {
    _lastOtaProgress = 100;
    _lastOtaDraw = millis();
    _u8g2.clearBuffer();
    drawRuCentered("ГОТОВО", 14);
    _u8g2.setFont(u8g2_font_9x18_tr);
    const char* done = "100%";
    const int width = _u8g2.getStrWidth(done);
    _u8g2.drawStr(x((OLED_WIDTH - width) / 2), y(34), done);
    _u8g2.sendBuffer();
}

void OledDisplay::drawOTAError() {
    _lastOtaProgress = -1;
    _lastOtaDraw = millis();
    _u8g2.clearBuffer();
    drawRuCentered("ОШИБКА", 14);
    drawRuCentered("ПОВТОРИТЕ", 34);
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
        _u8g2.drawStr(x(10), y(31), "Wi-Fi --");
    }

    if (!ntp->isSynced() && (_animFrame % 2 == 0)) {
        _u8g2.drawStr(x(52), y(7), "NTP!");
    }

    _u8g2.sendBuffer();
}

void OledDisplay::drawPageNetwork(WiFiManager* wifi) {
    _u8g2.clearBuffer();
    drawRuCentered("САЙТ", 13);
    hline(16);

    if (wifi->isConnected()) {
        _u8g2.setFont(u8g2_font_5x7_tr);
        const String ip = wifi->localIP();
        const int ipWidth = _u8g2.getStrWidth(ip.c_str());
        _u8g2.drawStr(x((OLED_WIDTH - ipWidth) / 2), y(27), ip.c_str());
        _u8g2.setFont(u8g2_font_4x6_tr);
        const char* host = "hydro.local";
        const int hostWidth = _u8g2.getStrWidth(host);
        _u8g2.drawStr(x((OLED_WIDTH - hostWidth) / 2), y(37), host);
    } else {
        drawRuCentered("НЕТ Wi-Fi", 34);
    }
    _u8g2.sendBuffer();
}

void OledDisplay::drawPageNext(NTPManager* ntp, Scheduler* scheduler) {
    _u8g2.clearBuffer();

    if (!scheduler->isEnabled()) {
        drawRuCentered("АВТОПОЛИВ", 14);
        drawRuCentered("ПАУЗА", 34);
        _u8g2.sendBuffer();
        return;
    }

    if (!ntp->isSynced()) {
        drawRuCentered("СЛЕД.", 14);
        drawRuCentered("НЕТ NTP", 34);
        _u8g2.sendBuffer();
        return;
    }

    WateringSlot slot {};
    int minutes = 0;
    if (!scheduler->getNextSlot(slot, minutes)) {
        drawRuCentered("СЛЕД.", 14);
        drawRuCentered("НЕТ ПЛАНА", 34);
        _u8g2.sendBuffer();
        return;
    }

    drawRuCentered("СЛЕД.", 12);

    char timeStr[6];
    snprintf(timeStr, sizeof(timeStr), "%02u:%02u", slot.hour, slot.minute);
    _u8g2.setFont(u8g2_font_9x18_tr);
    _u8g2.drawStr(x(13), y(30), timeStr);

    char bottom[24];
    if (minutes >= 60) {
        snprintf(bottom, sizeof(bottom), "+%d:%02d   %us",
                 minutes / 60, minutes % 60, slot.duration_sec);
    } else {
        snprintf(bottom, sizeof(bottom), "+%dmin   %us", minutes, slot.duration_sec);
    }
    _u8g2.setFont(u8g2_font_4x6_tr);
    _u8g2.drawStr(x(1), y(39), bottom);
    _u8g2.sendBuffer();
}

void OledDisplay::drawPageSchedule(NTPManager* ntp, Scheduler* scheduler) {
    _u8g2.clearBuffer();
    drawRuCentered("ПЛАН", 10);

    char countLine[5];
    snprintf(countLine, sizeof(countLine), "%u", scheduler->count());
    _u8g2.setFont(u8g2_font_4x6_tr);
    _u8g2.drawStr(x(61), y(6), countLine);
    hline(12);

    if (scheduler->count() == 0) {
        drawRuCentered("НЕТ ПЛАНА", 34);
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

    _u8g2.setFont(u8g2_font_4x6_tr);
    const uint8_t rows = scheduler->count() < 4 ? scheduler->count() : 4;
    for (uint8_t rowIndex = 0; rowIndex < rows; ++rowIndex) {
        const uint8_t index = (firstIndex + rowIndex) % scheduler->count();
        WateringSlot slot {};
        if (!scheduler->getSlot(index, slot)) continue;

        char line[18];
        snprintf(line, sizeof(line), "%c%02u:%02u %3us",
                 rowIndex == 0 ? '>' : ' ',
                 slot.hour, slot.minute, slot.duration_sec);

        const int row = 19 + rowIndex * 7;
        if (rowIndex == 0 && ntp->isSynced() && scheduler->isEnabled()) {
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
    drawRuCentered("ПОЛИВ", 13);

    const uint16_t remaining = relay->remainingSec();
    char remStr[8];
    snprintf(remStr, sizeof(remStr), "%u c", remaining);
    _u8g2.setFont(u8g2_font_9x18_tr);
    const int width = _u8g2.getStrWidth(remStr);
    _u8g2.drawStr(x((OLED_WIDTH - width) / 2), y(31), remStr);

    const float pct = relay->progress();
    if (pct >= 0.0f) drawProgressBar(1, 35, 70, 5, pct);
    _u8g2.sendBuffer();
}
