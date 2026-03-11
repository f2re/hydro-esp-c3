#include <Arduino.h>

// ── Пины (по схеме платы) ──────────────────────────────
#define RELAY_PIN  4   // IN реле → GPIO4
#define LED_PIN    8   // встроенный индикатор GPIO8
#define BTN_PIN    9   // BOOT кнопка GPIO9

// HW-307: активный LOW (LOW = реле включено)
#define RELAY_ON   LOW
#define RELAY_OFF  HIGH

// ── Тайминги ──────────────────────────────────────────
const uint32_t ON_TIME  = 5000;   // 5 сек — реле ВКЛ
const uint32_t OFF_TIME = 10000;  // 10 сек — реле ВЫКЛ

// ── Состояние ─────────────────────────────────────────
bool          relayState = false;
uint32_t      lastSwitch = 0;

void relayOn() {
    relayState = true;
    digitalWrite(RELAY_PIN, RELAY_ON);
    digitalWrite(LED_PIN,   LOW);      // LED горит (active LOW)
    Serial.println("🟢 RELAY ON  — помпа включена");
}

void relayOff() {
    relayState = false;
    digitalWrite(RELAY_PIN, RELAY_OFF);
    digitalWrite(LED_PIN,   HIGH);     // LED выкл
    Serial.println("⚫ RELAY OFF — помпа выключена");
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    pinMode(RELAY_PIN, OUTPUT);
    pinMode(LED_PIN,   OUTPUT);
    pinMode(BTN_PIN,   INPUT_PULLUP);

    relayOff();  // безопасное начальное состояние

    Serial.println("=== RELAY TEST ===");
    Serial.println("Цикл: 5с ВКЛ → 10с ВЫКЛ");
    Serial.println("Кнопка BOOT: ручное переключение");
    Serial.println("==================");

    lastSwitch = millis();
    relayOn();   // стартуем с включения, чтобы сразу увидеть
}

void loop() {
    uint32_t now = millis();
    uint32_t elapsed = now - lastSwitch;

    // ── Авто-цикл ─────────────────────────────────────
    if (relayState && elapsed >= ON_TIME) {
        lastSwitch = now;
        relayOff();
        Serial.printf("   следующее включение через %d сек\n",
            OFF_TIME / 1000);
    }
    else if (!relayState && elapsed >= OFF_TIME) {
        lastSwitch = now;
        relayOn();
        Serial.printf("   включено на %d сек\n", ON_TIME / 1000);
    }

    // ── BOOT кнопка: ручной toggle ────────────────────
    static bool btnPrev = HIGH;
    bool btnNow = digitalRead(BTN_PIN);
    if (btnPrev == HIGH && btnNow == LOW) {
        Serial.println("[BTN] Ручное переключение");
        lastSwitch = now;          // сброс таймера
        if (relayState) relayOff();
        else            relayOn();
    }
    btnPrev = btnNow;

    // ── Статус каждую секунду ─────────────────────────
    static uint32_t lastLog = 0;
    if (now - lastLog >= 1000) {
        lastLog = now;
        uint32_t remaining = relayState
            ? (ON_TIME  - elapsed) / 1000
            : (OFF_TIME - elapsed) / 1000;
        Serial.printf("   [%s] осталось: %ds\n",
            relayState ? "ON " : "OFF", remaining);
    }

    delay(20);
}
