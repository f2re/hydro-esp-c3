#pragma once
#include <Arduino.h>
#include "config.h"

class RelayController {
public:
    void begin();
    void on();
    void off();
    bool isOn() const;
    void runFor(uint16_t seconds);  // запуск на N секунд (non-blocking)
    void update();                  // вызывать в loop()
    float progress() const;         // 0.0 = только начал, 1.0 = конец, -1 = неизвестно
    uint16_t remainingSec() const;  // секунд до конца
private:
    bool _active = false;
    unsigned long _endTime = 0;
    uint32_t _totalMs = 0;
};
