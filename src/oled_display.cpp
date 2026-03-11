#include "oled_display.h"
#include <WiFi.h>

// ─────────────────────────────────────────────────────────────
void OledDisplay::begin() {
    Wire.begin(OLED_SDA, OLED_SCL);
    _u8g2.begin();
    _u8g2.setBusClock(400000);
    _u8g2.setContrast(255);
}

void OledDisplay::showPage(DisplayPage p) {
    _page = p;
    _lastSwitch = millis();
}

// ── Вспомогательные ──────────────────────────────────────────
void OledDisplay::hline(int ly) {
    _u8g2.drawHLine(x(0), y(ly), OLED_WIDTH);
}

void OledDisplay::drawProgressBar(int lx, int ly, int w, int h, float pct) {
    if (pct < 0) pct = 0;
    if (pct > 1) pct = 1;
    _u8g2.drawFrame(x(lx), y(ly), w, h);
    int fill = (int)(pct * (w - 2));
    if (fill > 0)
        _u8g2.drawBox(x(lx) + 1, y(ly) + 1, fill, h - 2);
}

// WiFi-иконка 7x8: 3 дуги или крест
void OledDisplay::drawWifiIcon(int px, int py, bool connected, int rssi) {
    if (!connected) {
        _u8g2.drawLine(px, py+1, px+5, py+6);
        _u8g2.drawLine(px+5, py+1, px, py+6);
        return;
    }
    int bars = (rssi >= -60) ? 3 : (rssi >= -75) ? 2 : 1;
    _u8g2.drawPixel(px+3, py+7);           // точка
    _u8g2.drawHLine(px+2, py+5, 3);        // малая дуга (всегда)
    if (bars >= 2) _u8g2.drawHLine(px+1, py+3, 5);
    if (bars >= 3) _u8g2.drawHLine(px,   py+1, 7);
}

// ── Загрузочный экран ─────────────────────────────────────────
void OledDisplay::drawBoot(uint8_t step, const char* msg) {
    _u8g2.clearBuffer();
    _u8g2.setFont(u8g2_font_5x7_tr);
    _u8g2.drawStr(x(2), y(8), "Hydroponics v1");
    hline(10);
    char line[32];
    snprintf(line, sizeof(line), "%d. %s", step, msg);
    _u8g2.drawStr(x(1), y(20), line);
    drawProgressBar(1, 28, 70, 6, (float)step / 4.0f);
    _u8g2.sendBuffer();
}

// ── Главный метод ротации ─────────────────────────────────────
void OledDisplay::update(NTPManager* ntp,
                          RelayController* relay,
                          WiFiManager* wifi) {
    uint32_t now = millis();

    // Анимация кадров
    if (now - _lastAnim >= 250) {
        _animFrame = (_animFrame + 1) % 4;
        _lastAnim  = now;
    }

    // Во время полива — принудительно страница полива
    if (relay->isOn()) {
        drawPageWatering(relay);
        _lastSwitch = now; // сброс таймера, чтобы после полива начать с PAGE_CLOCK
        _page = PAGE_CLOCK;
        return;
    }

    // Смена страницы по таймеру
    if (now - _lastSwitch >= PAGE_INTERVAL_MS) {
        _page = (DisplayPage)((_page + 1) % PAGE_COUNT);
        _lastSwitch = now;
    }

    switch (_page) {
        case PAGE_CLOCK:    drawPageClock   (ntp, wifi); break;
        case PAGE_NEXT:     drawPageNext    (ntp, relay); break;
        case PAGE_SCHEDULE: drawPageSchedule(ntp);       break;
        default: break;
    }
}

// ════════════════════════════════════════════════════════════
// СТРАНИЦА 0: ЧАСЫ — крупное время, дата, WiFi-иконка
// ════════════════════════════════════════════════════════════
void OledDisplay::drawPageClock(NTPManager* ntp, WiFiManager* wifi) {
    _u8g2.clearBuffer();

    // ── Крупное время по центру ────────────────────────────
    _u8g2.setFont(u8g2_font_9x18_tr); // ~9px широкий, 18px высокий
    if (ntp->isSynced()) {
        String t = ntp->getTimeString().substring(0, 5); // "HH:MM"
        // Центрирование: 5 символов × 9px = 45px → отступ (72-45)/2 = 13
        _u8g2.drawStr(x(13), y(20), t.c_str());
    } else {
        _u8g2.drawStr(x(13), y(20), "--:--");
    }

    // ── Секунды ─
    _u8g2.setFont(u8g2_font_5x7_tr);
    if (ntp->isSynced()) {
        String full = ntp->getTimeString(); // "HH:MM:SS"
        if (full.length() >= 8) {
            String sec = full.substring(6, 8);
            _u8g2.drawStr(x(57), y(20), sec.c_str());
        }
    }

    hline(22);

    // ── WiFi-иконка + IP ───────────────────────────────────
    _u8g2.setFont(u8g2_font_4x6_tr);
    bool wc = wifi->isConnected();
    int  rssi = wc ? WiFi.RSSI() : -100;
    drawWifiIcon(x(1), y(25), wc, rssi);

    if (wc) {
        // IP-адрес последний октет
        String ip = wifi->localIP();
        int dot = ip.lastIndexOf('.');
        char buf[18];
        snprintf(buf, sizeof(buf), "*.%s", ip.substring(dot+1).c_str());
        _u8g2.drawStr(x(10), y(31), buf);
        // RSSI
        char rssiStr[8];
        snprintf(rssiStr, sizeof(rssiStr), "%ddB", rssi);
        _u8g2.drawStr(x(40), y(31), rssiStr);
    } else {
        _u8g2.drawStr(x(10), y(31), "no WiFi");
    }

    // ── Мигающий индикатор NTP-синхронизации ──────────────
    if (!ntp->isSynced() && (_animFrame % 2 == 0)) {
        _u8g2.drawStr(x(50), y(9), "!NTP");
    }

    _u8g2.sendBuffer();
}

// ════════════════════════════════════════════════════════════
// СТРАНИЦА 1: СЛЕДУЮЩИЙ ПОЛИВ — большой обратный отсчёт
// ════════════════════════════════════════════════════════════
void OledDisplay::drawPageNext(NTPManager* ntp, RelayController* relay) {
    _u8g2.clearBuffer();

    // Заголовок
    _u8g2.setFont(u8g2_font_5x7_tr);
    _u8g2.drawStr(x(1), y(8), "Next watering:");
    hline(10);

    if (!ntp->isSynced()) {
        _u8g2.drawStr(x(5), y(25), "NTP wait...");
        _u8g2.sendBuffer();
        return;
    }

    // Поиск ближайшего слота
    int cur  = ntp->getHour() * 60 + ntp->getMinute();
    int best = 99999;
    uint8_t  nH = 0, nM = 0;
    uint16_t nDur = 0;
    for (uint8_t i = 0; i < SCHEDULE_COUNT; i++) {
        int slot = WATERING_SCHEDULE[i].hour * 60
                 + WATERING_SCHEDULE[i].minute;
        int d = slot - cur;
        if (d <= 0) d += 1440;
        if (d < best) {
            best = d; // минуты до полива
            nH   = WATERING_SCHEDULE[i].hour;
            nM   = WATERING_SCHEDULE[i].minute;
            nDur = WATERING_SCHEDULE[i].duration_sec;
        }
    }

    // ── Время следующего полива — крупно ──────────────────
    _u8g2.setFont(u8g2_font_9x18_tr);
    char timeStr[6];
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d", nH, nM);
    _u8g2.drawStr(x(13), y(28), timeStr);

    // ── Обратный отсчёт ───────────────────────────────────
    _u8g2.setFont(u8g2_font_5x7_tr);
    // Учитываем реальные секунды
    int  curSec    = ntp->isSynced() ? (ntp->getTimeString().substring(6,8).toInt()) : 0;
    long totalSec  = (long)best * 60 - curSec;
    if (totalSec < 0) totalSec += 86400;
    int h = totalSec / 3600;
    int m = (totalSec % 3600) / 60;
    int s = totalSec % 60;

    char countdown[16];
    if (h > 0)
        snprintf(countdown, sizeof(countdown), "in %dh %02dm %02ds", h, m, s);
    else
        snprintf(countdown, sizeof(countdown), "in %dm %02ds", m, s);
    _u8g2.drawStr(x(1), y(38), countdown);

    // ── Длительность полива ───────────────────────────────
    _u8g2.setFont(u8g2_font_4x6_tr);
    char durStr[14];
    snprintf(durStr, sizeof(durStr), "dur: %ds", nDur);
    _u8g2.drawStr(x(45), y(38), durStr);

    // Анимация: мигающая капля слева
    if (_animFrame < 2) {
        _u8g2.drawStr(x(1), y(28), "~");
    }

    _u8g2.sendBuffer();
}

// ════════════════════════════════════════════════════════════
// СТРАНИЦА 2: РАСПИСАНИЕ — список всех слотов
// ════════════════════════════════════════════════════════════
void OledDisplay::drawPageSchedule(NTPManager* ntp) {
    _u8g2.clearBuffer();
    _u8g2.setFont(u8g2_font_4x6_tr);
    _u8g2.drawStr(x(1), y(6), "Schedule:");
    hline(8);

    // Текущий момент для подсветки активного слота
    int curH = ntp->isSynced() ? ntp->getHour()   : 255;
    int curM = ntp->isSynced() ? ntp->getMinute() : 255;
    int cur  = curH * 60 + curM;

    // Вмещаем до 4 строк
    uint8_t shown = 0;
    int bestIdx = 0, bestD = 99999;
    for (uint8_t i = 0; i < SCHEDULE_COUNT; i++) {
        int d = WATERING_SCHEDULE[i].hour * 60
              + WATERING_SCHEDULE[i].minute - cur;
        if (d <= 0) d += 1440;
        if (d < bestD) { bestD = d; bestIdx = i; }
    }

    for (uint8_t j = 0; j < min((int)SCHEDULE_COUNT, 4); j++) {
        uint8_t i = (bestIdx + j) % SCHEDULE_COUNT;
        auto& sl = WATERING_SCHEDULE[i];
        bool isNext = (j == 0);

        char line[18];
        snprintf(line, sizeof(line), "%s%02d:%02d %3ds",
            isNext ? ">" : " ",
            sl.hour, sl.minute, sl.duration_sec);

        int row = 15 + j * 7;
        if (isNext) {
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

// ════════════════════════════════════════════════════════════
// СТРАНИЦА ПОЛИВА: активный полив
// ════════════════════════════════════════════════════════════
void OledDisplay::drawPageWatering(RelayController* relay) {
    _u8g2.clearBuffer();

    _u8g2.setFont(u8g2_font_7x13_tr);
    _u8g2.drawStr(x(1), y(13), "WATERING");
    if (_animFrame % 2 == 0) {
        hline(14);
    }

    uint16_t rem = relay->remainingSec();
    char remStr[8];
    snprintf(remStr, sizeof(remStr), "%ds", rem);
    _u8g2.setFont(u8g2_font_9x18_tr);
    int remLen = strlen(remStr) * 9;
    _u8g2.drawStr(x((72 - remLen) / 2), y(30), remStr);

    float pct = relay->progress();
    if (pct >= 0.0f) {
        drawProgressBar(1, 33, 70, 5, pct);
    }

    // Анимация капли справа
    int dropY = y(18) + (_animFrame * 2);
    _u8g2.drawCircle(x(63), dropY, 2);

    _u8g2.sendBuffer();
}
