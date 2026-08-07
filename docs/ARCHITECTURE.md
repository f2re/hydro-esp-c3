# Архитектура HydroESP-C3

## Принцип

HydroESP-C3 — локальный контроллер. Критическая функция полива не должна зависеть от браузера, облака или постоянного Wi‑Fi. Web UI — операторская панель над состоянием, которое живёт на ESP32-C3.

Второй принцип: **обслуживание является отдельным режимом**, а не набором скрытых исключений. Таймерную автоматику можно поставить на паузу без удаления расписания; калибровка разрешена только в этом состоянии.

## Модули

```text
main.cpp
 ├─ ConfigStorage      NVS: Wi‑Fi, UTC, schedule, automation, hydraulics
 ├─ WiFiManager        STA / provisioning AP / captive DNS
 ├─ NTPManager         локальное абсолютное время
 ├─ EventLog           RAM-only журнал управляющих событий сессии
 ├─ RelayController    timeout + source/reason текущего цикла
 ├─ Scheduler          timer automation + explicit pause
 ├─ WebServerManager   HTTP API v3 + UI + OTA
 ├─ OledDisplay        локальная индикация
 └─ StatusDisplay      Serial dashboard
```

UI в `src/web_ui_v2.h` self-contained и не использует CDN.

## Загрузка

1. старт MCU;
2. GPIO насоса принудительно переводится в OFF;
3. читается/валидируется NVS;
4. создаётся RAM-журнал текущей сессии;
5. Scheduler получает сохранённый график и persisted `automation_enabled`;
6. выполняется Wi‑Fi STA attempt;
7. при отсутствии рабочего Wi‑Fi запускается `HydroESP-Setup`;
8. запускается NTP;
9. boot фиксируется в EventLog;
10. запускаются HTTP server и mDNS;
11. `loop()` обслуживает network/NTP/relay/Scheduler/BOOT/OLED/Serial.

## Runtime state

### ConfigStorage / NVS

Долгоживущая конфигурация:

- Wi‑Fi SSID/password;
- UTC offset;
- `automation_enabled`;
- `pump_flow_ml_min`;
- `delivery_efficiency_pct`;
- расписание.

Расход хранится целым числом **мл/мин**, а не float. Это даёт стабильное бинарное представление в NVS.

Старые устройства совместимы: отсутствующие новые keys получают безопасные defaults без сброса Wi‑Fi и расписания.

### Scheduler

Scheduler — единственный runtime-источник расписания для автоматики, OLED и Serial.

`setEnabled(false)`:

- сохраняет расписание без изменений;
- запрещает новые schedule-start;
- помечает текущую минуту обработанной.

`setEnabled(true)` также потребляет текущую минуту, поэтому возобновление режима не создаёт «догоняющий» полив.

### RelayController

RelayController знает не только ON/OFF, но и источник текущего цикла:

- `schedule`;
- `web_manual`;
- `button_manual`;
- `calibration`.

Все циклы идут через `runFor()` и ограничиваются `MAX_WATERING_SECONDS`.

Остановка получает явную причину:

- manual;
- timeout;
- reboot;
- OTA;
- automation paused.

Это позволяет объяснять оператору причину действия и формировать корректный журнал.

### EventLog

`EventLog` — кольцевой буфер на **32 события в RAM**.

Записываются boot, pump start/stop, pause/resume, schedule change, hydraulics save, config change, OTA и reboot request.

Каждая запись содержит:

- sequence;
- uptime;
- локальный epoch, если NTP уже есть;
- event type;
- pump source;
- stop reason;
- одно числовое value.

Почему RAM-only: типовая установка может выполнять десятки циклов в сутки. Записывать каждый start/stop в NVS — неправильный способ строить историю и лишний износ flash. Долговременный event/sensor log должен появиться как отдельный кольцевой storage-контур вместе с телеметрией.

## Инварианты безопасности

### Boot

GPIO насоса принудительно OFF до работы Scheduler. Удерживаемая во время reset BOOT-кнопка не интерпретируется как start до первого release.

### Ограниченное время работы

Любой программный start имеет timeout. Бессрочного relay-ON состояния нет.

### OTA / reboot

Перед reboot и OTA насос выключается с соответствующей причиной.

### Физическая кнопка

- active pump → stop немедленно;
- inactive pump → start только после hold;
- короткое случайное касание не запускает насос.

### Пауза автоматики

При переводе в maintenance mode:

- новый schedule-start запрещён;
- если прямо сейчас выполняется **schedule**-цикл, он останавливается;
- ручной/calibration цикл скрыто не прерывается;
- график остаётся в NVS.

### Калибровка

`/api/calibration/start` разрешён только если:

- automation paused;
- relay OFF;
- test duration 5–120 s.

Это не заменяет аппаратный level/flow interlock, но не позволяет калибровочному тесту пересекаться с активным таймером.

## Гидравлическая калибровка

Web flow:

1. pause automation;
2. подготовить мерную ёмкость;
3. включить pump на известное `t`;
4. измерить `V` мл;
5. вычислить `Q = V × 60 / t / 1000` л/мин;
6. сохранить Q и delivery efficiency;
7. использовать Q как вход инженерного расчёта.

Калибровка измеряет **фактическую гидравлику установки**, но не водопотребление культуры.

## NVS validation

Проверяются UTC offset, число/размер schedule slots, диапазоны времени/длительности, flow и efficiency. Повреждённое расписание заменяется factory fallback; отдельные новые параметры возвращаются к безопасным defaults.

## Сеть

### STA

Домашняя LAN, `hydro.local`, NTP и web UI.

### Provisioning AP

```text
SSID: HydroESP-Setup
URL:  http://192.168.4.1
```

После сохранения Wi‑Fi устройство reboot и повторяет STA flow.

## Web UI как операторская панель

Основные UX-инварианты:

- режим автоматики виден на первом экране;
- pausing не спрятан в settings;
- pump start требует hold, stop — одно действие;
- источник текущего запуска отображается;
- calibration оформлена как мастер, а не набор несвязанных полей;
- schedule import создаёт draft;
- restore через CLI оставляет automation paused по умолчанию;
- dangerous actions используют собственный modal;
- responsive desktop/mobile layouts и reduced-motion;
- runtime errors показываются toast/inline состояниями, не native alerts.

CI извлекает embedded HTML, выполняет `node --check`, ищет duplicate ids и запрещает `alert()`/`confirm()`.

## Расписание

Единый предел: `MAX_SCHEDULE_SLOTS = 48`.

```cpp
struct WateringSlot {
    uint8_t hour;
    uint8_t minute;
    uint16_t duration_sec;
};
```

API/UI запрещают duplicate `HH:MM`.

## Инженерный расчёт

Разделены:

1. гидравлика — фактический Q → время подачи требуемого объёма;
2. атмосферный спрос — VPD как diagnostic indicator.

VPD не масштабирует water duration автоматически. Adaptive v2 должен добавить реальные RAD/PPFD, level, flow, root-zone/drainage и safety interlocks.

## API versioning

`HYDRO_API_VERSION = 3` соответствует появлению operational mode, hydraulic calibration и current-session event log. Клиенты обязаны проверять версию контракта.

Build также получает `HYDRO_VERSION` и `HYDRO_BUILD_SHA` через `scripts/build_flags.py`.

## Release pipeline

Tag `v*` → GitHub Actions → PlatformIO build → versioned `.bin` + `latest.bin` + SHA-256 → GitHub Release.

PR CI проверяет maintenance tooling, embedded JS/UI и реальную firmware build.

## Осознанно не реализовано

- аппаратный dry-run/low-level interlock;
- подтверждение фактического потока в реальном времени;
- persistent sensor/event history;
- RTC holdover;
- adaptive irrigation;
- signed OTA;
- automatic post-boot rollback;
- web authentication.

Эти функции должны вводиться как отдельные проверяемые контуры, а не декоративные элементы UI.
