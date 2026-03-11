#include <Arduino.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

#define RELAY_PIN  4
#define LED_PIN    8
#define BTN_PIN    9

const uint32_t ON_TIME  = 5000;
const uint32_t OFF_TIME = 10000;

bool     relayState = false;
uint32_t lastSwitch = 0;

void relayOn() {
    relayState = true;
    digitalWrite(RELAY_PIN, HIGH);   // тянем к GND → реле ВКЛ (в режиме Open Drain)
    digitalWrite(LED_PIN,   LOW);
    Serial.println(">>> НАСОС ВКЛ");
}

void relayOff() {
    relayState = false;
    digitalWrite(RELAY_PIN, LOW);  // отпускаем (высокий импеданс)
    digitalWrite(LED_PIN,   HIGH);
    Serial.println(">>> насос выкл");
}

void setup() {
    // Реле ВЫКЛ первым делом
    pinMode(RELAY_PIN, OUTPUT_OPEN_DRAIN); // Ключевое изменение для стабильности
    digitalWrite(RELAY_PIN, LOW);

    // Отключаем Brown-out детектор
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    pinMode(BTN_PIN, INPUT_PULLUP);
    digitalWrite(LED_PIN, HIGH);

    delay(1000);
    Serial.println("=== RELAY TEST (open drain) ===");

    lastSwitch = millis();
    relayOn();
}

void loop() {
    uint32_t now     = millis();
    uint32_t elapsed = now - lastSwitch;

    // ── Авто-цикл ─────────────────────────────────────
    if (relayState && elapsed >= ON_TIME) {
        lastSwitch = now;
        relayOff();
    } else if (!relayState && elapsed >= OFF_TIME) {
        lastSwitch = now;
        relayOn();
    }

    // ── BOOT кнопка: ручной toggle ────────────────────
    static bool btnPrev = HIGH;
    bool btnNow = digitalRead(BTN_PIN);
    if (btnPrev == HIGH && btnNow == LOW) {
        lastSwitch = now;
        if (relayState) relayOff();
        else            relayOn();
    }
    btnPrev = btnNow;

    // ── Статус каждую секунду ─────────────────────────
    static uint32_t lastLog = 0;
    if (now - lastLog >= 1000) {
        lastLog = now;
        uint32_t rem = relayState
            ? (ON_TIME  - elapsed) / 1000
            : (OFF_TIME - elapsed) / 1000;
        Serial.printf("  [%s] осталось %ds\n",
            relayState ? "ВКЛ " : "выкл", rem);
    }

    delay(20);
}
