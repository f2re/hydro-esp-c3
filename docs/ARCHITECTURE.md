# Архитектура HydroESP-C3

## Принцип

HydroESP-C3 — local-first контроллер. Полив работает на ESP32-C3 и не зависит от браузера или облака. Web UI — локальная операторская панель.

Установка намеренно простая: если домашний Wi‑Fi не настроен, устройство поднимает открытую `HydroESP-Setup` и сразу отдаёт Web UI на `192.168.4.1`.

## Основные модули

```text
main.cpp
 ├─ ConfigStorage      NVS: Wi‑Fi, UTC, location, schedule, hydraulics
 ├─ WiFiManager        STA / open setup AP / captive DNS
 ├─ NTPManager         время + sunrise/sunset helper
 ├─ RelayController    timeout + pump source/reason
 ├─ Scheduler          timer automation + pause
 ├─ EventLog           короткий RAM-журнал сессии
 ├─ WebServerManager   API v3 + embedded UI + OTA
 ├─ OledDisplay        status / IP / OTA / setup
 └─ StatusDisplay      Serial dashboard
```

## Boot flow

1. GPIO насоса переводится в OFF.
2. Загружается и валидируется NVS.
3. Инициализируются Scheduler, RelayController и EventLog.
4. Выполняется попытка подключения к сохранённому Wi‑Fi.
5. Если STA недоступна, запускается открытая `HydroESP-Setup`.
6. HTTP сервер запускается сразу после готовности сети.
7. В STA-режиме запускаются mDNS и NTP.
8. OLED/Serial показывают прямой numeric IP.
9. Основной loop обслуживает сеть, таймер, насос, кнопку, OLED и Serial.

HTTP специально не зависит от NTP: страница настройки должна открываться даже без интернета.

## Конфигурация

`ConfigStorage` хранит:

- домашний Wi‑Fi и UTC offset;
- latitude/longitude;
- отметку последней замены раствора;
- `automation_enabled`;
- hydraulic Q/efficiency + calibration metadata;
- расписание до 48 слотов.

Обычный `install` очищает NVS, чтобы первичная установка всегда была предсказуемой. `update` NVS не стирает.

## Инварианты управления

- любой pump start ограничен временем;
- boot/reboot/OTA не оставляют насос включённым;
- web и BOOT используют hold-to-start;
- stop выполняется одним действием;
- pause не удаляет schedule;
- resume не запускает пропущенный слот задним числом;
- Scheduler — единый runtime-источник расписания для Web/OLED/Serial.

## Web UI

`src/web_ui_v2.h` self-contained: внешние CDN не нужны. В AP-режиме wildcard DNS и `onNotFound` помогают открыть локальную страницу без отдельного backend.

## Flash / update

CI проверяет application image и merged install image. Технические bootloader/partition offsets скрыты от обычного пользователя: стандартный `install.sh` всё делает сам, а для ручного flasher публикуется один `hydro-esp-c3-install.bin`.

## Следующие реальные улучшения

- minimum-level/dry-run interlock;
- live flow confirmation;
- T/RH и температура раствора;
- persistent sensor history только при необходимости;
- adaptive irrigation только после реальных измерений.
