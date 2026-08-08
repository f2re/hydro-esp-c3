# Аудит HydroESP-C3

Актуализировано: **2026-08-08**

## Состояние проекта

HydroESP-C3 сейчас представляет собой цельный local-first контроллер: timer automation, ручное управление, maintenance pause, серийная гидравлическая калибровка, OLED/Serial, responsive Web UI, backup/restore, OTA, diagnostics и CI.

После последней интеграции сохранены и дополнительные функции актуального `main`: координаты установки, расчёт восхода/заката, отметка обслуживания раствора и OTA-progress на OLED.

Автоматический режим по-прежнему является **таймером**, а сохранённый `Q` — измеренной гидравлической константой. Проект не изображает adaptive irrigation без реальных сенсоров.

## Что закрыто

- единый лимит расписания 48 слотов и server-side validation;
- maximum pump runtime, hold-to-start и мгновенный stop;
- reboot/OTA выключают насос;
- schedule не «догоняется» после manual overlap или maintenance pause;
- persisted `automation active / paused`;
- source/reason tracking каждого pump cycle;
- RAM-журнал последних управляющих событий;
- серийная калибровка `Q` с mean/CV/timestamp/protocol version;
- desktop/mobile Web UI без CDN и native `alert/confirm`;
- backup/restore и OTA через `hydroctl`;
- защищённый `HydroESP-Setup` с отдельным device key;
- commissioning key показывается на OLED/Serial и не попадает в backup;
- README содержит реальные headless-снимки embedded UI;
- CI проверяет tooling, embedded JS, screenshots и PlatformIO build.

## Открытые риски

### P0 — аппаратная безопасность

1. Нет minimum-level/dry-run interlock.
2. Нет live flow/current confirmation: GPIO ON ещё не означает, что вода действительно течёт.
3. Нужно исправить силовой power path и вернуть штатный brown-out detector.
4. Для критичного применения нет независимого hardware fail-safe.

### P1 — сеть и обновление

- локальный HTTP пока без отдельной пользовательской авторизации;
- application OTA без device-side signature verification;
- automatic rollback после неудачной версии не реализован.

### P1 — время и telemetry

- после полного power loss абсолютное время зависит от NTP;
- RTC holdover отсутствует;
- нет реальных `T/RH / PAR / solution T / level / live flow / root-zone` streams.

## Научно корректный расчёт текущей версии

```text
V_event = N × d / 1000
t_on = 60 × V_event / (Q × η)
```

`Q` — фактически измеренный расход установки, `η` — эффективная доставка. VPD остаётся диагностическим показателем и не масштабирует полив автоматически.

## Следующий разумный порядок

Без лишнего усложнения:

1. hardware **level + live flow** safety loop;
2. исправить питание и вернуть brown-out protection;
3. простая auth для опасных HTTP-действий;
4. `T/RH + solution T + light` telemetry;
5. RTC только если реально нужна автономная работа без NTP;
6. adaptive irrigation — только после накопления измерений, с timer fallback.

## Научная основа

- FAO Irrigation and Drainage Paper 56, revised 2026, DOI `10.4060/cd6621en`;
- Medrano E. et al. (2012), Acta Horticulturae 927, DOI `10.17660/ActaHortic.2012.927.43`;
- Huy T.N. et al. (2014), DOI `10.7235/hort.2014.13177`.

Подробные эксплуатационные контракты: `README.md`, `docs/ARCHITECTURE.md`, `docs/API.md`, `docs/SECURITY.md`.
