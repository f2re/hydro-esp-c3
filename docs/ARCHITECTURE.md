# Архитектура HydroESP-C3

## Принцип

HydroESP-C3 — локальный контроллер: критическая функция полива живёт на ESP32-C3 и не зависит от браузера, облака или постоянного соединения с сервером.

Второй принцип — **обслуживание является отдельным режимом**. Таймер можно поставить на паузу без удаления расписания; calibration-run разрешён только в этом состоянии.

## Модули

```text
main.cpp
 ├─ ConfigStorage       NVS: Wi‑Fi, UTC, schedule, automation, hydraulics
 ├─ SecurityManager     отдельный commissioning key / namespace hydrosec
 ├─ WiFiManager         STA / protected setup AP / captive DNS
 ├─ NTPManager          локальное абсолютное время
 ├─ EventLog            RAM-only журнал текущей сессии
 ├─ RelayController     timeout + source/reason
 ├─ Scheduler           timer automation + explicit pause
 ├─ WebServerManager    HTTP API v3 + UI + OTA
 ├─ OledDisplay         local state + setup recovery screen
 └─ StatusDisplay       Serial dashboard/recovery
```

UI в `src/web_ui_v2.h` полностью self-contained: runtime CDN/облако не нужны.

## Boot flow

1. MCU стартует, pump GPIO принудительно OFF;
2. загружается и валидируется основная NVS-конфигурация;
3. SecurityManager открывает отдельный `hydrosec` namespace;
4. Scheduler получает runtime schedule и `automation_enabled`;
5. выполняется Wi‑Fi STA attempt;
6. если STA не поднялся — запускается `HydroESP-Setup` с persistent device key;
7. OLED постоянно показывает setup SSID/key/IP;
8. Serial дублирует credential как физический recovery channel;
9. запускаются NTP, HTTP server и при STA — mDNS;
10. `loop()` обслуживает network/NTP/relay/Scheduler/BOOT/OLED/Serial.

## Runtime state

### ConfigStorage

Хранит Wi‑Fi, UTC offset, automation state, calibration metadata и schedule. Старые устройства получают безопасные defaults без сброса существующих Wi‑Fi/слотов.

### SecurityManager

Commissioning credential намеренно отделён от основной конфигурации:

- namespace: `hydrosec`;
- key создаётся из `esp_random()`;
- 10 символов из читаемого 32-символьного алфавита;
- используется как WPA2 password setup AP;
- переживает reboot/OTA;
- не входит в `/api/config` и обычный backup;
- constant-time `verify()` оставлен как простой building block для будущей auth, но HTTP auth в текущем этапе не включена.

### Scheduler

Единственный runtime-источник расписания для автоматики, OLED и Serial. Pause/resume помечает текущую минуту обработанной, поэтому пропущенный slot не запускается задним числом.

### RelayController

Каждый start имеет timeout и источник `schedule / web_manual / button_manual / calibration`. Stop получает явную причину `manual / timeout / reboot / ota / automation_paused`.

### EventLog

Кольцевой буфер на 32 события **в RAM**. Persistent журнал намеренно не строится поверх NVS; долговременная telemetry должна иметь отдельный wear-aware storage.

## Safety invariants

- pump OFF после boot;
- любой software-start ограничен временем;
- BOOT: hold для start, одно действие для stop;
- reboot/OTA сначала останавливают pump;
- maintenance pause не удаляет schedule;
- calibration не запускается поверх timer automation;
- restore оставляет automation paused по умолчанию;
- сохранённый Q — calibration constant, **не live flow feedback**.

## Гидравлическая калибровка

Web wizard делает серию коротких измерений через мерную ёмкость:

```text
Qi = Vml × 60 / tsec / 1000
Qmean = ΣQi / n
CV = sample_stddev(Qi) / Qmean × 100%
```

Сохраняются mean Q, efficiency, sample count, CV, timestamp и protocol version. CV характеризует только repeatability серии.

## Сеть

### STA

Домашняя LAN → `hydro.local` + NTP + Web UI.

### Protected commissioning AP

```text
SSID: HydroESP-Setup
KEY:  OLED / Serial
URL:  http://192.168.4.1
```

После сохранения домашнего Wi‑Fi устройство reboot и снова пробует STA.

## Web UI

UX-инварианты:

- automation state виден сразу;
- start требует hold, stop — одно действие;
- schedule import создаёт draft;
- calibration оформлена как мастер;
- ошибки — toast/inline, без native `alert/confirm`;
- desktop/mobile layouts и reduced-motion;
- screenshots README строятся автоматически из **этого же embedded HTML** с mock API.

CI проверяет embedded JS, duplicate ids, собирает firmware и рендерит воспроизводимые UI screenshots.

## Что намеренно не реализовано

- minimum-level/dry-run interlock;
- live flow/current confirmation;
- persistent sensor history;
- RTC holdover;
- adaptive irrigation;
- web authentication/TLS;
- device-side signed OTA;
- automatic rollback.

Приоритет следующего аппаратного этапа: `level + live flow + исправленное питание/brown-out`.
