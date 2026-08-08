# Аудит HydroESP-C3

Актуализировано: **2026-08-08**

## Состояние проекта

HydroESP-C3 уже не экспериментальная HTML-страница вокруг реле. Сейчас это цельный local-first контур:

- timer automation + отдельная maintenance pause;
- ограниченное ручное управление;
- source/reason tracking для насоса;
- серийная гидравлическая калибровка;
- desktop/mobile Web UI;
- OLED + Serial recovery;
- backup/restore;
- USB install + OTA updater;
- diagnostics, CI и release pipeline;
- защищённый commissioning AP.

Принципиальная граница сохранена: автоматический режим — **суточный таймер**, а сохранённый Q — калибровочная константа. Проект не изображает adaptive irrigation без реальных сенсоров.

## Что закрыто

### Управление

- единый limit 48 slots;
- server-side validation;
- NVS fallback при повреждённом schedule;
- maximum pump runtime;
- hold-to-start, immediate stop;
- reboot/OTA → pump OFF;
- без догоняющего schedule-cycle после manual overlap или resume;
- non-blocking Wi‑Fi reconnect;
- Scheduler — единый runtime source для Web/OLED/Serial.

### Maintenance mode

Automation имеет persisted `active / paused`. Pause не удаляет schedule и является обязательным условием calibration. CLI restore также оставляет automation paused до явной проверки.

### Гидравлика

Calibration wizard использует серию измерений:

```text
Qi = Vml × 60 / tsec / 1000
Qmean = ΣQi / n
CV = sample_stddev(Qi) / Qmean × 100%
```

Сохраняются mean Q, efficiency, sample count, CV, timestamp и protocol version. CV означает **repeatability**, не agronomic confidence.

### UI / UX

- desktop sidebar + mobile bottom navigation;
- dark/light/system theme;
- responsive safe areas;
- toast/modal вместо blocking browser dialogs;
- automation readiness на первом экране;
- 24h schedule timeline;
- calibration wizard;
- session event log;
- diagnostics;
- OTA drag-and-drop;
- README screenshots автоматически рендерятся из реального embedded HTML с mock API.

### Commissioning security

- `HydroESP-Setup` больше не открытая сеть;
- device key генерируется через `esp_random()`;
- отдельный NVS namespace `hydrosec`;
- key переживает reboot/OTA;
- key не входит в `/api/config` или backup;
- OLED показывает SSID/key/IP, Serial остаётся recovery channel.

Это защищает commissioning Wi‑Fi, но **не является web-auth/TLS**.

## Открытые риски

### P0 — аппаратная безопасность

1. Нет minimum-level/dry-run interlock.
2. Нет live flow/current confirmation: GPIO ON не доказывает наличие воды.
3. Нужно исправить power path и вернуть штатный brown-out detector.
4. Для критичного применения нет независимого hardware fail-safe.

### P1 — сетевой/security слой

- local HTTP API пока без пользовательской auth/TLS;
- application OTA без device-side signature verification;
- нет post-boot health confirmation/automatic rollback;
- EventLog RAM-only, не persistent audit trail.

### P1 — время и телеметрия

- после полного power loss абсолютное время зависит от NTP;
- нет RTC holdover/DST policy;
- нет real sensor streams `T/RH / PAR / solution T / level / flow / root-zone / EC/pH`.

## Научно корректный расчёт текущей версии

```text
V_event = N × d / 1000
t_on = 60 × V_event / (Q × η)
```

`Q` — измеренный фактический расход установки, `η` — effective delivery. VPD остаётся diagnostic indicator и не масштабирует полив автоматически.

## Следующий разумный порядок работ

Без избыточного усложнения:

1. hardware **level + live flow** safety loop;
2. исправить питание и вернуть brown-out protection;
3. простая auth для опасных HTTP-действий;
4. `T/RH + solution T + light` telemetry;
5. RTC при реальной необходимости автономной работы без NTP;
6. только после появления данных — adaptive irrigation с timer fallback.

## Научная основа

- FAO Irrigation and Drainage Paper 56, revised 2026 — фундамент ET, но reference ET нельзя напрямую выдавать за расход небольшой гидропонной установки;
- Medrano E. et al. (2012), DOI `10.17660/ActaHortic.2012.927.43` — radiation/VPD/leaf area для hydroponic irrigation modelling;
- Huy T.N. et al. (2014), DOI `10.7235/hort.2014.13177` — важность root-zone state в rockwool culture.

Подробные operational contracts: `README.md`, `docs/ARCHITECTURE.md`, `docs/API.md`, `docs/SECURITY.md`.
