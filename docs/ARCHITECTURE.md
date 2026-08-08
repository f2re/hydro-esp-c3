# Архитектура HydroESP-C3

## Принцип

HydroESP-C3 — local-first контроллер. Полив работает на ESP32-C3 и не зависит от браузера или облака. Web UI — локальная операторская панель.

Установка намеренно простая: `hydroctl install` берёт Wi‑Fi из environment / `.env` либо спрашивает его в терминале. Если SSID пропущен или подключение не удалось, устройство поднимает открытую `HydroESP-Setup` и отдаёт Web UI на `192.168.4.1`.

Повторный `deploy` принципиально отделён от provisioning: он не передаёт `WIFI_SSID`, `WIFI_PASSWORD` и `WIFI_SEED_ID`, поэтому обычная перепрошивка не меняет сохранённую сеть/NVS. Для восстановления по Wi‑Fi есть независимый recovery OTA на порту `3232`, не зависящий от Web UI.

## Основные модули

```text
main.cpp
 ├─ ConfigStorage      NVS + one-shot Wi-Fi seed
 ├─ WiFiManager        STA / open setup AP / captive DNS
 ├─ NTPManager         время + sunrise/sunset helper
 ├─ RelayController    timeout + pump source/reason
 ├─ Scheduler          timer automation + pause
 ├─ EventLog           короткий RAM-журнал сессии
 ├─ RecoveryOTA        независимый ArduinoOTA recovery, порт 3232
 ├─ WebServerManager   API v3 + embedded UI + HTTP OTA + identity assets
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
8. В STA-режиме запускаются mDNS, NTP и recovery OTA.
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

Повторный `deploy` не использует provisioning flags вообще. `wifi-flash` также собирает recovery-safe application image без compile-time Wi‑Fi credentials.

## Инварианты управления

- любой pump start ограничен временем;
- boot/reboot/OTA не оставляют насос включённым;
- web и BOOT используют hold-to-start;
- stop выполняется одним действием;
- pause не удаляет schedule;
- resume не запускает пропущенный слот задним числом;
- Scheduler — единый runtime-источник расписания для Web/OLED/Serial.

## Web UI и статические ресурсы

`src/web_ui_v2.h` self-contained: внешние CDN не нужны. В AP-режиме wildcard DNS и `onNotFound` помогают открыть локальную страницу без отдельного backend.

Большой `WEB_UI_HTML` хранится в `PROGMEM` и отдаётся explicit-length `AsyncProgmemResponse`. Нельзя возвращать его через `request->send(..., const char*)`: такой overload сначала копирует весь HTML в Arduino `String` и на ESP32-C3 может закончиться пустым `200 OK` при неудачной крупной heap-аллокации.

Фирменные Web-ресурсы находятся в `src/web_assets.h`:

```text
/favicon.ico  -> настоящий ICO 16×16 + 32×32
/favicon.svg  -> компактный масштабируемый SVG
```

Они также хранятся в flash/`PROGMEM`, отдаются explicit-length ответом и кэшируются браузером на 7 суток. Суммарный image payload около 2.7 КБ; CI ограничивает его 4 КБ. NVS, LittleFS/SPIFFS и постоянный RAM-буфер для favicon не используются.

Подробно: [WEB_ASSETS.md](WEB_ASSETS.md).

## OTA / recovery

Есть два независимых сетевых пути обновления:

1. HTTP `/ota/upload` через `AsyncWebServer`;
2. recovery ArduinoOTA/espota на порту `3232`.

`tools/wifi_flash.py` сначала использует recovery OTA, а для старой прошивки умеет автоматически перейти на прямой `/ota/upload`. Поэтому сломанная HTML-страница не должна превращать устройство в обязательный USB-recovery.

## Flash / update

CI проверяет application image и merged install image. Технические bootloader/partition offsets скрыты от обычного пользователя: стандартный `install.sh` всё делает сам, а для ручного flasher публикуется один `hydro-esp-c3-install.bin`.

Статические UI-ресурсы должны иметь измеряемый flash-budget. Нельзя добавлять крупные PNG/JPEG «на всякий случай» без измерения итогового firmware size и явной эксплуатационной пользы.

## Следующие реальные улучшения

- minimum-level/dry-run interlock;
- live flow confirmation;
- T/RH и температура раствора;
- persistent sensor history только при необходимости;
- adaptive irrigation только после реальных измерений.
