#include "status_display.h"

// ── ANSI helpers ────────────────────────────────────────────
#define CLS      "\033[2J\033[H"
#define BOLD     "\033[1m"
#define DIM      "\033[2m"
#define RST      "\033[0m"
#define GREEN    "\033[32m"
#define YELLOW   "\033[33m"
#define CYAN     "\033[36m"
#define RED      "\033[31m"
#define BLUE     "\033[34m"

void StatusDisplay::cls() { Serial.print(CLS); }

// ── Прогресс-бар ────────────────────────────────────────────
String StatusDisplay::progressBar(float pct, uint8_t w) {
    if (pct < 0) pct = 0;
    if (pct > 1) pct = 1;
    uint8_t filled = (uint8_t)(pct * w);
    String s = "[";
    for (uint8_t i = 0; i < w; i++)
        s += (i < filled) ? "█" : "░";
    s += "] ";
    s += String((int)(pct * 100));
    s += "%";
    return s;
}

// ── Аптайм ──────────────────────────────────────────────────
String StatusDisplay::fmtUptime() {
    uint32_t s = millis() / 1000;
    uint32_t h = s / 3600; s %= 3600;
    uint32_t m = s / 60;   s %= 60;
    char buf[12];
    snprintf(buf, sizeof(buf), "%02lu:%02lu:%02lu", h, m, s);
    return String(buf);
}

// ── До следующего слота ─────────────────────────────────────
int StatusDisplay::minutesUntilNext(uint8_t curH, uint8_t curM,
                                     uint8_t* oH, uint8_t* oM,
                                     uint16_t* oDur) {
    int cur = curH * 60 + curM;
    int best = 99999;
    for (uint8_t i = 0; i < SCHEDULE_COUNT; i++) {
        int slot = WATERING_SCHEDULE[i].hour * 60
                 + WATERING_SCHEDULE[i].minute;
        int d = slot - cur;
        if (d <= 0) d += 1440;
        if (d < best) {
            best = d;
            *oH   = WATERING_SCHEDULE[i].hour;
            *oM   = WATERING_SCHEDULE[i].minute;
            *oDur = WATERING_SCHEDULE[i].duration_sec;
        }
    }
    return best;
}

// ── Загрузочный экран (один раз) ────────────────────────────
void StatusDisplay::printBoot() {
    _startMs = millis();
    cls();
    Serial.println();
    Serial.println(BOLD CYAN
        "  ╔══════════════════════════════════════════╗" RST);
    Serial.println(BOLD CYAN
        "  ║  🌿  Hydroponics Tower  v1.0             ║" RST);
    Serial.println(BOLD CYAN
        "  ║  💧  Система автополива  ESP32-C3        ║" RST);
    Serial.println(BOLD CYAN
        "  ╚══════════════════════════════════════════╝" RST);
    Serial.println();
}

void StatusDisplay::printBootStep(const char* icon, const char* msg,
                                   bool ok, const String& detail) {
    Serial.printf("  %s  %-28s", icon, msg);
    if (ok) Serial.print(GREEN "✅" RST);
    else    Serial.print(RED   "❌" RST);
    if (detail.length()) Serial.printf("  %s", detail.c_str());
    Serial.println();
}

// ── Основной дашборд ────────────────────────────────────────
void StatusDisplay::draw(NTPManager* ntp,
                          RelayController* relay,
                          WiFiManager* wifi) {
    cls();

    // ── Заголовок ──────────────────────────────────────────
    Serial.println(BOLD "  🌿  Hydroponics Tower  " DIM
                   "· " RST DIM + fmtUptime() + RST);
    Serial.println(DIM
        "  ──────────────────────────────────────────" RST);

    // ── Время ──────────────────────────────────────────────
    if (ntp->isSynced()) {
        Serial.printf("  🕐  " BOLD "%s" RST "  UTC+%d\n",
            ntp->getTimeString().c_str(), TIMEZONE_OFFSET);
    } else {
        Serial.println("  🕐  " YELLOW "⚠️  NTP не синхронизирован" RST);
    }

    // ── WiFi ───────────────────────────────────────────────
    if (wifi->isConnected()) {
        Serial.printf("  📡  " GREEN "WiFi ✅" RST "  %s\n",
            wifi->localIP().c_str());
    } else {
        Serial.println("  📡  " RED "WiFi ❌  нет соединения" RST);
    }

    Serial.println(DIM
        "  ──────────────────────────────────────────" RST);

    // ── Состояние помпы ────────────────────────────────────
    if (relay->isOn()) {
        Serial.println("  💧  " BOLD GREEN "ПОЛИВ ИДЁТ" RST);
        // прогресс-бар если знаем сколько осталось
        float pct = relay->progress();  // 0.0–1.0
        if (pct >= 0) {
            Serial.printf("      %s\n",
                progressBar(pct).c_str());
        }
    } else {
        Serial.println("  💤  " DIM "Помпа выключена" RST);
    }

    // ── Следующий полив ────────────────────────────────────
    if (ntp->isSynced() && !relay->isOn()) {
        uint8_t nH = 0, nM = 0;
        uint16_t nDur = 0;
        int delta = minutesUntilNext(
            ntp->getHour(), ntp->getMinute(), &nH, &nM, &nDur);
        Serial.printf(
            "  ⏭   Следующий полив: " BOLD "%02d:%02d" RST
            "  через " CYAN "%dч %02dмин" RST
            "  (%dс)\n",
            nH, nM, delta / 60, delta % 60, nDur);
    }

    Serial.println(DIM
        "  ──────────────────────────────────────────" RST);

    // ── Расписание ─────────────────────────────────────────
    Serial.println("  📋  " DIM "Расписание:" RST);
    uint8_t curH = ntp->isSynced() ? ntp->getHour()   : 255;
    uint8_t curM = ntp->isSynced() ? ntp->getMinute() : 255;
    for (uint8_t i = 0; i < SCHEDULE_COUNT; i++) {
        bool isCurrent = (WATERING_SCHEDULE[i].hour   == curH &&
                          WATERING_SCHEDULE[i].minute == curM);
        Serial.printf("      %s %02d:%02d  %ds\n",
            isCurrent ? "▶" : "·",
            WATERING_SCHEDULE[i].hour,
            WATERING_SCHEDULE[i].minute,
            WATERING_SCHEDULE[i].duration_sec);
    }

    Serial.println(DIM
        "  ──────────────────────────────────────────" RST);
    Serial.println(DIM "  обновление каждые 5 сек" RST);
}
