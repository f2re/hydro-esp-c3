#include "status_display.h"
#include "version.h"

#define CLS    "\033[2J\033[H"
#define BOLD   "\033[1m"
#define DIM    "\033[2m"
#define RST    "\033[0m"
#define GREEN  "\033[32m"
#define YELLOW "\033[33m"
#define CYAN   "\033[36m"
#define RED    "\033[31m"

void StatusDisplay::cls() {
    Serial.print(CLS);
}

String StatusDisplay::progressBar(float pct, uint8_t width) {
    if (pct < 0.0f) pct = 0.0f;
    if (pct > 1.0f) pct = 1.0f;
    const uint8_t filled = static_cast<uint8_t>(pct * width);
    String result = "[";
    for (uint8_t i = 0; i < width; ++i) result += i < filled ? "█" : "░";
    result += "] ";
    result += String(static_cast<int>(pct * 100));
    result += "%";
    return result;
}

String StatusDisplay::fmtUptime() {
    uint32_t seconds = millis() / 1000UL;
    const uint32_t days = seconds / 86400UL;
    seconds %= 86400UL;
    const uint32_t hours = seconds / 3600UL;
    seconds %= 3600UL;
    const uint32_t minutes = seconds / 60UL;

    char buf[24];
    if (days > 0) {
        snprintf(buf, sizeof(buf), "%lud %02lu:%02lu",
                 static_cast<unsigned long>(days),
                 static_cast<unsigned long>(hours),
                 static_cast<unsigned long>(minutes));
    } else {
        snprintf(buf, sizeof(buf), "%02lu:%02lu:%02lu",
                 static_cast<unsigned long>(hours),
                 static_cast<unsigned long>(minutes),
                 static_cast<unsigned long>(seconds % 60UL));
    }
    return String(buf);
}

void StatusDisplay::printBoot() {
    cls();
    Serial.println();
    Serial.println(BOLD CYAN "  HydroESP-C3" RST);
    Serial.printf("  firmware %s  build %s  API v%d\n",
                  HYDRO_VERSION, HYDRO_BUILD_SHA, HYDRO_API_VERSION);
    Serial.println(DIM "  local hydroponics controller" RST);
    Serial.println();
}

void StatusDisplay::printBootStep(const char* icon, const char* msg,
                                  bool ok, const String& detail) {
    Serial.printf("  %s  %-24s", icon, msg);
    Serial.print(ok ? GREEN "OK" RST : YELLOW "WAIT" RST);
    if (detail.length()) Serial.printf("  %s", detail.c_str());
    Serial.println();
}

void StatusDisplay::printSchedule(Scheduler* scheduler) {
    Serial.printf("\n  Schedule: %u slot(s)\n", scheduler ? scheduler->count() : 0);
    if (!scheduler) return;

    for (uint8_t i = 0; i < scheduler->count(); ++i) {
        WateringSlot slot {};
        if (!scheduler->getSlot(i, slot)) continue;
        Serial.printf("    %02u  %02u:%02u  %us\n",
                      i + 1, slot.hour, slot.minute, slot.duration_sec);
    }
    Serial.println();
}

void StatusDisplay::draw(NTPManager* ntp, RelayController* relay,
                         WiFiManager* wifi, Scheduler* scheduler) {
    cls();
    Serial.printf(BOLD "  HydroESP-C3 %s" RST DIM "  · %s  · %s\n" RST,
                  HYDRO_VERSION, HYDRO_BUILD_SHA, fmtUptime().c_str());
    Serial.println(DIM "  --------------------------------------------------" RST);

    if (ntp->isSynced()) {
        Serial.printf("  Time     " BOLD "%s  %s" RST "  UTC%+d\n",
                      ntp->getDateString().c_str(), ntp->getTimeString().c_str(),
                      ntp->getTimeOffsetHours());
    } else {
        Serial.println("  Time     " YELLOW "NOT SYNCED - schedule paused" RST);
    }

    if (wifi->isAPMode()) {
        Serial.printf("  Network  " YELLOW "%s" RST "  %s\n",
                      AP_SSID, wifi->localIP().c_str());
    } else if (wifi->isConnected()) {
        Serial.printf("  Network  " GREEN "WiFi OK" RST "  %s  RSSI %d dBm\n",
                      wifi->localIP().c_str(), WiFi.RSSI());
    } else {
        Serial.println("  Network  " YELLOW "reconnecting" RST);
    }

    Serial.println(DIM "  --------------------------------------------------" RST);

    if (relay->isOn()) {
        Serial.printf("  Pump     " BOLD GREEN "ON" RST "  %us remaining\n",
                      relay->remainingSec());
        const float progress = relay->progress();
        if (progress >= 0.0f) Serial.printf("           %s\n", progressBar(progress).c_str());
    } else {
        Serial.println("  Pump     OFF");
    }

    WateringSlot next {};
    int minutesUntil = 0;
    if (scheduler && scheduler->getNextSlot(next, minutesUntil)) {
        Serial.printf("  Next     " BOLD "%02u:%02u" RST "  in %dh %02dmin  duration %us\n",
                      next.hour, next.minute, minutesUntil / 60,
                      minutesUntil % 60, next.duration_sec);
    } else {
        Serial.println("  Next     --:--");
    }

    Serial.printf("  Slots    %u\n", scheduler ? scheduler->count() : 0);
    Serial.println(DIM "  --------------------------------------------------" RST);

    if (scheduler && scheduler->count()) {
        const uint8_t shown = scheduler->count() < 8 ? scheduler->count() : 8;
        Serial.println(DIM "  Upcoming/runtime schedule:" RST);

        uint8_t first = 0;
        if (ntp->isSynced() && scheduler->getNextSlot(next, minutesUntil)) {
            for (uint8_t i = 0; i < scheduler->count(); ++i) {
                WateringSlot candidate {};
                if (scheduler->getSlot(i, candidate) &&
                    candidate.hour == next.hour && candidate.minute == next.minute) {
                    first = i;
                    break;
                }
            }
        }

        for (uint8_t row = 0; row < shown; ++row) {
            const uint8_t index = (first + row) % scheduler->count();
            WateringSlot slot {};
            if (!scheduler->getSlot(index, slot)) continue;
            Serial.printf("    %s %02u:%02u  %us\n",
                          row == 0 && ntp->isSynced() ? ">" : " ",
                          slot.hour, slot.minute, slot.duration_sec);
        }
        if (scheduler->count() > shown) {
            Serial.printf(DIM "    ... +%u more\n" RST, scheduler->count() - shown);
        }
    }

    Serial.println(DIM "  refresh: 5 s" RST);
}
