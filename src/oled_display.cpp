#include "oled_display.h"

void OledDisplay::begin() {
    Wire.begin(OLED_SDA, OLED_SCL);
    _u8g2.begin();
    _u8g2.setBusClock(400000);
    _u8g2.setContrast(255);
}

void OledDisplay::hline(int ly) {
    _u8g2.drawHLine(x(0), y(ly), OLED_WIDTH);
}

void OledDisplay::drawProgressBar(int lx, int ly, int w, int h, float pct) {
    _u8g2.drawFrame(x(lx), y(ly), w, h);
    int fill = (int)(pct * (w - 2));
    if (fill > 0)
        _u8g2.drawBox(x(lx) + 1, y(ly) + 1, fill, h - 2);
}

// ── Загрузочный экран ─────────────────────────────────────
void OledDisplay::drawBoot(uint8_t step, const char* msg) {
    _u8g2.clearBuffer();
    _u8g2.setFont(u8g2_font_5x7_tr);

    // Заголовок
    _u8g2.drawStr(x(2), y(8), "Hydroponics v1");
    hline(10);

    // Шаги загрузки (показываем последние 3)
    char line[32];
    snprintf(line, sizeof(line), "%d. %s", step, msg);
    _u8g2.drawStr(x(1), y(20), line);

    // Прогресс-бар загрузки
    float pct = (float)step / 4.0f;
    drawProgressBar(1, 28, 70, 6, pct);

    _u8g2.sendBuffer();
}

// ── Основной дашборд ─────────────────────────────────────
void OledDisplay::drawDashboard(NTPManager* ntp,
                                 RelayController* relay,
                                 WiFiManager* wifi) {
    _u8g2.clearBuffer();

    // ── Строка 1: Время (крупный шрифт 7x13) ─────────────
    _u8g2.setFont(u8g2_font_7x13_tr);
    if (ntp->isSynced()) {
        // Показываем HH:MM (8 символов * 7px = 56px, влезает)
        String t = ntp->getTimeString().substring(0, 5); // "HH:MM"
        _u8g2.drawStr(x(1), y(13), t.c_str());
    } else {
        _u8g2.drawStr(x(1), y(13), "--:--");
    }

    // WiFi-иконка справа (маленьким шрифтом)
    _u8g2.setFont(u8g2_font_5x7_tr);
    _u8g2.drawStr(x(55), y(9),  wifi->isConnected() ? "W+" : "W-");
    if (ntp->isSynced())
        _u8g2.drawStr(x(55), y(17), "NTP");

    hline(19); // разделитель

    // ── Строка 2: Состояние помпы ────────────────────────
    if (relay->isOn()) {
        _u8g2.drawStr(x(1), y(27), "PUMP ON");
        float pct = relay->progress();
        if (pct >= 0.0f)
            drawProgressBar(1, 29, 70, 5, pct);
    } else {
        _u8g2.drawStr(x(1), y(27), "pump off");
    }

    // ── Строка 3: Следующий полив ────────────────────────
    _u8g2.setFont(u8g2_font_4x6_tr); // самый мелкий — влезет больше текста
    if (ntp->isSynced() && !relay->isOn()) {
        uint8_t nH = 0, nM = 0; uint16_t nDur = 0;
        int cur = ntp->getHour() * 60 + ntp->getMinute();
        int best = 99999;
        for (uint8_t i = 0; i < SCHEDULE_COUNT; i++) {
            int slot = WATERING_SCHEDULE[i].hour * 60
                     + WATERING_SCHEDULE[i].minute;
            int d = slot - cur;
            if (d <= 0) d += 1440;
            if (d < best) {
                best = d;
                nH = WATERING_SCHEDULE[i].hour;
                nM = WATERING_SCHEDULE[i].minute;
                nDur = WATERING_SCHEDULE[i].duration_sec;
            }
        }
        char buf[22];
        snprintf(buf, sizeof(buf), "Next %02d:%02d in%dh%02dm",
            nH, nM, best / 60, best % 60);
        _u8g2.drawStr(x(0), y(38), buf);
    } else if (relay->isOn()) {
        char buf[18];
        uint16_t rem = relay->remainingSec();
        snprintf(buf, sizeof(buf), "Done in %ds", rem);
        _u8g2.drawStr(x(0), y(38), buf);
    }

    _u8g2.sendBuffer();
}
