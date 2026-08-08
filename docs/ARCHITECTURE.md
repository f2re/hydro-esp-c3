# Архитектура HydroESP-C3

## Принцип

HydroESP-C3 — local-first контроллер. Полив работает на ESP32-C3 и не зависит от браузера или облака. Web UI — локальная операторская панель.

Установка намеренно простая: `hydroctl install` берёт Wi‑Fi из environment / `.env` либо спрашивает его в терминале. Если SSID пропущен или подключение не удалось, устройство поднимает открытую `HydroESP-Setup` и отдаёт Web UI на `192.168.4.1`.

## Основные модули

```text
main.cpp
 ├─ ConfigStorage      NVS + one-shot Wi-Fi seed
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
3. Если install-сборка содержит новый `WIFI_SEED_ID`, встроенные SSID/password один раз записываются в NVS.
4. Инициализируются Scheduler, RelayController и EventLog.
5. Выполняется попытка подключения к Wi‑Fi.
6. Если STA недоступна, запускается открытая `HydroESP-Setup`.
7. HTTP сервер запускается сразу после готовности сети.
8. В STA-режиме запускаются mDNS и NTP.
9. OLED/Serial показывают прямой numeric IP.

HTTP специально не зависит от NTP: страница настройки должна открываться даже без интернета.

## Конфигурация

`ConfigStorage` хранит:

- домашний Wi‑Fi и UTC offset;
- latitude/longitude;
- отметку последней замены раствора;
- `automation_enabled`;
- hydraulic Q/efficiency + calibration metadata;
- расписание до 48 слотов.

`install` и `update` не стирают NVS. Install Wi‑Fi seed меняет только сетевые данные и применяется один раз. После этого Wi‑Fi можно менять из Web UI; reboot не возвращает старые compile-time credentials.

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
