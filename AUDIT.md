# Аудит HydroESP-C3

Актуализировано: **2026-08-07**

## Текущее состояние

HydroESP-C3 переведён из экспериментального ESP32-контроллера в цельный локальный эксплуатационный контур: единые ограничения firmware/API/NVS, responsive web UI, commissioning, безопасное ручное управление, отдельный maintenance mode, гидравлическая калибровка, диагностика, backup/restore, USB installer, OTA updater, CI и release pipeline.

Принципиальная граница остаётся прежней: автоматический режим — **суточный таймер**. Проект не выдаёт фиксированный график, VPD или ручную гидравлическую калибровку за универсальную физиологическую модель потребности растений.

## Закрыто программным контуром

### Управление и отказоустойчивость

- единый `MAX_SCHEDULE_SLOTS = 48` во frontend/API/NVS/Scheduler;
- серверная валидация schedule/config/hydraulics;
- NVS validation + fallback для повреждённого расписания;
- любой start ограничен `MAX_WATERING_SECONDS`;
- reboot/OTA останавливают pump;
- BOOT и web: hold-to-start, stop одним действием;
- BOOT, удерживаемый при reset/flashing, не превращается в start после загрузки;
- устранён догоняющий schedule-cycle после manual overlap;
- pause/resume также потребляет текущую минуту и не запускает пропущенный slot;
- rollover-safe relay timeout/progress;
- non-blocking Wi‑Fi reconnect;
- runtime Scheduler — единый источник графика для автоматики/OLED/Serial;
- runtime UTC offset используется всеми интерфейсами.

### Maintenance mode

Timer automation имеет отдельное persisted состояние `active / paused`.

Пауза:

- не удаляет расписание;
- запрещает новые plan-start;
- останавливает активный **schedule**-цикл;
- не скрыто прерывает manual/calibration cycle;
- сохраняется в NVS;
- является обязательным условием calibration endpoint.

Restore через CLI намеренно ставит automation на паузу и не включает её обратно без явного `--resume-automation`.

### Объяснимость действий

RelayController отслеживает источник текущего запуска:

- `schedule`;
- `web_manual`;
- `button_manual`;
- `calibration`.

Stop получает причину `manual / timeout / reboot / ota / automation_paused`.

Добавлен RAM-only EventLog на 32 события текущей сессии: boot, start/stop, pause/resume, schedule/hydraulics/config changes, OTA/reboot. Он намеренно не пишет каждый полив в NVS.

### Web UI / UX

- desktop sidebar + mobile bottom navigation;
- dark/light/system theme;
- safe-area layout;
- toast/custom modal вместо `alert/confirm`;
- reduced-motion и keyboard focus;
- первый экран показывает automation mode, readiness и pump source;
- schedule dirty-state, disabled Save, runtime/duty summary, 24-hour timeline;
- import создаёт только draft;
- гидравлика оформлена как пошаговый calibration wizard;
- system screen объединяет diagnostics, session event log и backup;
- OTA drag-and-drop с confirmation/progress;
- version/build/API отображаются в UI.

### Гидравлическая калибровка

Теперь `Q` не предлагается вводить как условное паспортное число. Реализован воспроизводимый мастер:

1. pause automation;
2. направить фактический output в мерную ёмкость;
3. выполнить короткий calibration-run известной длительности;
4. измерить `V` мл;
5. вычислить `Q = V × 60 / t / 1000` л/мин;
6. сохранить Q в NVS как мл/мин;
7. сохранить коэффициент эффективной доставки `η`;
8. использовать сохранённые Q/η в инженерном калькуляторе.

API ограничивает calibration-run диапазоном 5–120 s и запрещает его при active automation или занятом pump.

Это измеряет гидравлику, но **не водопотребление культуры**.

### Данные, установка и сопровождение

- Wi‑Fi password не возвращается API и не попадает в backup;
- backup v2 содержит schedule + automation + hydraulics;
- `hydroctl`: bootstrap/build/install/monitor/status/doctor/events/pause/resume/backup/restore/update;
- PlatformIO при необходимости ставится в project-local `.venv`;
- portable PlatformIO config без macOS-only serial port;
- latest-release updater проверяет SHA-256 asset;
- release pipeline выпускает versioned `.bin`, latest `.bin`, checksum;
- CI проверяет tooling, embedded JS/UI и реальную firmware build.

### Документация

Актуальные документы:

- `README.md` — точка входа;
- `docs/INSTALL.md` — install/commissioning;
- `docs/UPDATE.md` — OTA/backup/fail-safe restore;
- `docs/TROUBLESHOOTING.md` — полевой runbook;
- `docs/ARCHITECTURE.md` — runtime state и safety invariants;
- `docs/API.md` — HTTP API v3;
- `docs/SECURITY.md` — trust boundary/hardening;
- `CHANGELOG.md` — release discipline.

## Открытые риски

### P0 — аппаратная безопасность

1. Нет minimum-level sensor и dry-run interlock.
2. Нет realtime подтверждения фактического потока/тока pump. Сохранённый Q — калибровочная константа, а не live feedback.
3. Brown-out workaround ещё может отключать штатную защиту MCU; нужно исправить power path и вернуть detector.
4. Нет независимого hardware fail-safe для критичного применения.

### P0/P1 — security

- local HTTP/API без authentication/TLS;
- OTA без device-side cryptographic signature verification;
- commissioning AP открыт;
- нет post-boot health confirmation/automatic rollback;
- EventLog не является persistent audit log.

До hardening устройство должно работать только в доверенной LAN без WAN port-forward.

### P1 — время

После полного power loss абсолютное время зависит от NTP. RTC/holdover отсутствует; fixed UTC offset не реализует DST policy.

### P1 — telemetry/adaptive

Пока нет реальных streams:

- air T/RH;
- PAR/PPFD/radiation;
- solution temperature;
- level;
- realtime flow;
- root-zone moisture/mass/drainage;
- EC/pH;
- persistent sensor/event history.

## Научно корректный расчёт текущей версии

Решается измеримая инженерная задача:

```text
V_event = N × d / 1000

t_on = 60 × V_event / (Q × η)
```

где `N` — число растений, `d` — заданная подача на растение за цикл, `Q` — **измеренный фактический расход установки**, `η` — эффективная доставка, `t_on` — время работы pump.

VPD рассчитывается по введённым T/RH только как diagnostic indicator. Он не является автоматическим multiplier времени полива.

## Рекомендуемая adaptive-модель v2

Минимальный измерительный контур:

1. `T/RH`;
2. PAR/PPFD или radiation;
3. solution T;
4. minimum level;
5. realtime flow;
6. root-zone/drainage state;
7. EC/pH при необходимости.

Концептуально:

```text
T_hat = f(RAD, VPD, LAI, crop, stage)
Demand(t) = integral(T_hat dt)

irrigate if:
  accumulated_demand >= threshold
  OR root_zone_condition <= low_limit

stop/limit if:
  delivered_volume >= target
  OR root_zone/drainage condition reached
  OR safety interlock triggered
```

Коэффициенты требуют калибровки на конкретной установке и проверки по фактическому water balance.

## Научная основа

- FAO Irrigation and Drainage Paper 56, revised 2026, *Crop evapotranspiration: Guidelines for computing crop water requirements*, DOI `10.4060/cd6621en` — фундамент ET, но reference ET нельзя напрямую выдавать за water requirement небольшой гидропонной башни.
- Medrano E. et al. (2012), *A simplified P-M model for improving irrigation management of strawberries in a semi-closed hydroponic system*, Acta Horticulturae 927, DOI `10.17660/ActaHortic.2012.927.43` — климат, leaf area, radiation и VPD.
- Huy T.N. et al. (2014), *Analyses of Transpiration and Growth of Paprika as Affected by Moisture Content of Growing Medium in Rockwool Culture*, DOI `10.7235/hort.2014.13177` — важность root-zone state вместе с atmospheric demand.

## Следующий порядок работ

1. hardware `level + realtime flow + power/brownout` safety loop;
2. repeatability/uniformity test для текущей Q-калибровки;
3. protected commissioning + web-auth + signed OTA/rollback;
4. RTC holdover;
5. `T/RH + solution T + light + flow` telemetry;
6. persistent ring storage + CSV export;
7. root-zone/drainage feedback;
8. profiles/crop stage;
9. только после накопления и проверки данных — отдельный adaptive mode при сохранении timer fallback.
