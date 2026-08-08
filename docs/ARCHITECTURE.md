# Архитектура HydroESP-C3

## Принцип

HydroESP-C3 — local-first контроллер. Полив живёт на ESP32-C3 и не зависит от браузера или облака. Web UI — операторская панель над локальным состоянием.

Второй принцип: обслуживание — отдельный режим. Automation можно поставить на паузу без удаления расписания; calibration выполняется только в этом состоянии.

## Основные модули

```text
main.cpp
 ├─ ConfigStorage      NVS: Wi‑Fi, UTC, location, schedule, hydraulics
 ├─ SecurityManager    отдельный commissioning credential
 ├─ WiFiManager        STA / protected setup AP / captive DNS
 ├─ NTPManager         время + sunrise/sunset helper
 ├─ RelayController    timeout + pump source/reason
 ├─ Scheduler          timer automation + pause
 ├─ EventLog           короткий RAM-журнал сессии
 ├─ WebServerManager   API v3 + embedded UI + OTA
 ├─ OledDisplay        status / OTA / commissioning recovery
 └─ StatusDisplay      Serial dashboard
```

## Boot flow

1. GPIO насоса переводится в OFF.
2. Загружается и валидируется NVS.
3. Инициализируются Scheduler, RelayController и EventLog.
4. Выполняется попытка подключения к сохранённому Wi‑Fi.
5. Если STA недоступна, запускается **защищённый** `HydroESP-Setup`.
6. Device key берётся из отдельного `hydrosec` namespace и показывается на OLED/Serial.
7. Запускаются NTP, HTTP API и mDNS.
8. Основной loop обслуживает сеть, таймер, насос, кнопку, OLED и Serial.

## Долгоживущая конфигурация

`ConfigStorage` хранит:

- домашний Wi‑Fi и UTC offset;
- latitude/longitude;
- отметку последней замены раствора;
- `automation_enabled`;
- hydraulic Q/efficiency + calibration metadata;
- расписание до 48 слотов.

Commissioning key хранится **отдельно** в `SecurityManager`, чтобы не смешивать его с домашним Wi‑Fi и обычным backup.

## Инварианты управления

- любой pump start ограничен временем;
- boot/reboot/OTA не оставляют насос включённым;
- web и BOOT используют hold-to-start;
- stop выполняется одним действием;
- pause не удаляет schedule;
- resume не запускает пропущенный слот задним числом;
- Scheduler — единый runtime-источник расписания для Web/OLED/Serial.

## Гидравлическая калибровка

Калибровка измеряет реальный расход установки серией коротких тестов. Сохраняются mean Q, efficiency, sample count, CV, timestamp и protocol version.

Это **не** модель водопотребления растения. Adaptive mode должен появляться только после реальных level/flow/T-RH/light/root-zone измерений.

## Web UI

`src/web_ui_v2.h` self-contained: внешние CDN не нужны.

UX-инварианты:

- desktop sidebar / mobile bottom navigation;
- automation state виден сразу;
- dangerous start требует удержания;
- schedule import создаёт draft;
- calibration оформлена мастером;
- ошибки показываются inline/toast, без native `alert/confirm`;
- UI screenshots в README воспроизводимо рендерятся из этого же embedded HTML.

## OTA

OTA останавливает pump и показывает progress в Web/OLED. Release updater проверяет SHA-256 на стороне клиента. Device-side signed OTA и automatic rollback пока не реализованы.

## Осознанно не реализовано

- minimum-level/dry-run interlock;
- live flow/current confirmation;
- persistent sensor history;
- RTC holdover;
- полноценный adaptive irrigation;
- отдельная web-auth/TLS;
- device-side signed OTA/rollback.

Эти функции добавляются отдельными проверяемыми этапами, а не декоративными флагами в UI.
