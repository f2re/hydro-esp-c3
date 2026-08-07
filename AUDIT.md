# Аудит HydroESP-C3

Актуализировано: **2026-08-07**

## Текущее состояние

Проект переведён из экспериментального ESP32-контроллера с встроенной HTML-страницей в цельный локальный продуктовый контур: единые ограничения прошивки/API/NVS, responsive web UI, commissioning, диагностика, backup/restore, USB installer, OTA updater, воспроизводимый build и release pipeline.

При этом принципиальная граница сохранена: текущий автоматический режим — **суточный таймер**, а не универсальная модель физиологической потребности растений. Adaptive irrigation остаётся отдельным этапом с датчиками и калибровкой.

## Закрыто в ветке модернизации

### Управление и надёжность

- единый лимит расписания `MAX_SCHEDULE_SLOTS = 48` во frontend/API/NVS/Scheduler;
- серверная валидация часов, минут, длительности и дублей времени;
- валидация NVS при загрузке, fallback при повреждённых данных;
- любой программный запуск насоса ограничен `MAX_WATERING_SECONDS`;
- reboot и OTA принудительно выключают насос;
- физическая кнопка: удержание для старта, немедленный stop при активном насосе;
- web-кнопка: удержание для старта, stop одним действием;
- исправлен сценарий «ручной цикл закончился в минуту планового слота → Scheduler запускает второй цикл»;
- relay timeout/progress устойчив к переполнению `millis()`;
- Wi‑Fi reconnect не блокирует основной `loop()`;
- AP IP берётся из `softAPIP()`;
- пустой factory SSID приводит сразу к понятному commissioning `HydroESP-Setup`.

### Web UI / UX

- отдельная desktop sidebar и mobile bottom navigation;
- light / dark / system theme;
- safe-area layout для мобильных устройств;
- toast вместо `alert()`;
- custom modal вместо `confirm()`;
- hold-to-start для опасного действия;
- явные online/offline, AP, NTP и firmware states;
- skeleton loading;
- reduced-motion accessibility;
- расписание с dirty-state и disabled Save до изменения;
- 24-часовая визуальная шкала циклов;
- summary: количество циклов / runtime / duty cycle;
- генерация рассчитанного расписания только как **черновика**;
- backup/export JSON без пароля;
- import создаёт черновик, а не молча пишет NVS;
- OTA drag-and-drop, confirmation и progress;
- diagnostics: heap, minimum heap, flash, sketch size, reset reason, firmware version/build SHA.

### Безопасность данных

- `/api/config` больше не возвращает пароль Wi‑Fi;
- пустой пароль при изменении SSID/UTC означает «не менять существующий»;
- backup не содержит пароль;
- version/build/API contract доступны через HTTP;
- опасные серверные операции валидируют входные данные.

### Установка и сопровождение

- создан `tools/hydroctl.py` без сторонних Python runtime-зависимостей;
- команды `bootstrap/build/install/monitor/status/doctor/backup/restore/update`;
- PlatformIO при необходимости устанавливается в project-local `.venv`;
- убран macOS-only hardcoded serial port;
- `install.sh/.ps1` и `update.sh/.ps1`;
- normal build не требует зашивать Wi‑Fi credentials;
- latest-release updater проверяет SHA-256 asset;
- release workflow публикует versioned `.bin`, `latest.bin` и checksum;
- build metadata генерируется из tag/git SHA;
- CI проверяет Python/shell, embedded JS, duplicate ids, отсутствие blocking browser dialogs и PlatformIO build.

### Документация

Разделены эксплуатационные документы:

- `README.md` — точка входа;
- `docs/INSTALL.md` — установка/commissioning;
- `docs/UPDATE.md` — OTA/recovery/backup;
- `docs/TROUBLESHOOTING.md` — полевой runbook;
- `docs/ARCHITECTURE.md` — модули и safety invariants;
- `docs/API.md` — HTTP API v2;
- `docs/SECURITY.md` — текущая trust boundary и hardening;
- `AUDIT.md` — инженерная/научная граница проекта.

## Открытые риски

### P0 — аппаратная безопасность

1. **Нет датчика минимального уровня раствора.** Насос не знает, есть ли вода; нужен hardware/software interlock от сухого хода.
2. **Нет подтверждения фактического потока.** GPIO ON не означает, что жидкость реально подаётся. Нужен flow sensor либо другой контроль работы.
3. **Brown-out detector временно отключён.** Это compatibility workaround существующей установки. Нужно исправить питание/развязку насоса, измерить просадку и вернуть штатную защиту.
4. **Нет независимого аварийного ограничения силовой части.** Для критичного применения software timeout желательно дополнять аппаратным fail-safe.

### P0/P1 — security

1. локальный HTTP/API без authentication;
2. OTA endpoint без device-side cryptographic signature verification;
3. commissioning AP открыт;
4. нет полноценного post-boot image health check/automatic rollback;
5. нет audit log опасных операций.

До закрытия этих пунктов устройство должно работать только в доверенной LAN без WAN port-forward.

### P1 — автономность времени

1. после полного сброса питания абсолютное время зависит от NTP;
2. без интернета/локального NTP расписание после reboot не может гарантированно восстановить время;
3. UTC offset не учитывает DST автоматически.

Для полностью автономной установки следует добавить RTC и определить policy time validity/holdover.

### P1 — телеметрия и adaptive irrigation

Пока нет:

- температуры/RH воздуха;
- PAR/PPFD или радиации;
- температуры раствора;
- уровня;
- расхода;
- состояния субстрата/корневой зоны или дренажа;
- EC/pH;
- локального журнала поливов/аварий/перезапусков.

Без этих данных adaptive mode не должен имитироваться подбором красивых коэффициентов.

## Что считать научно корректным расчётом сейчас

Фиксированный интервал сам по себе научной моделью не является. Реализованный UI решает измеримую инженерную задачу гидравлики:

```text
V_event = N × d / 1000

t_on = 60 × V_event / (Q × η)
```

где:

- `N` — число растений;
- `d` — заданная подача на растение за цикл, мл;
- `Q` — **измеренный фактический** расход всей установки, л/мин;
- `η` — коэффициент эффективной доставки;
- `t_on` — время работы насоса, с.

VPD рассчитывается по температуре/RH только как диагностический показатель. Он не является автоматическим множителем длительности.

## Рекомендуемая adaptive-модель v2

Минимальный измерительный контур:

1. `T/RH` — SHT31/SHT4x или эквивалент;
2. PAR/PPFD или калиброванная радиация/освещённость;
3. температура раствора;
4. минимальный уровень;
5. фактический расход;
6. влажность/масса корневой зоны либо измерение дренажа;
7. EC/pH — если режим питания должен входить в автоматику.

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

Коэффициенты нельзя универсально зашить для всех культур. Их нужно калибровать на конкретной башне и проверять по фактическому водному балансу.

## Научная основа

- FAO Irrigation and Drainage Paper 56, revised 2026, *Crop evapotranspiration: Guidelines for computing crop water requirements*, DOI `10.4060/cd6621en`. Методология ET полезна как фундамент, но reference ET нельзя напрямую выдавать за расход небольшой гидропонной башни.
- Medrano E. et al. (2012), *A simplified P-M model for improving irrigation management of strawberries in a semi-closed hydroponic system*, Acta Horticulturae 927, DOI `10.17660/ActaHortic.2012.927.43` — связь водопотребления с климатом, листовой площадью, радиацией и VPD.
- Huy T.N. et al. (2014), *Analyses of Transpiration and Growth of Paprika as Affected by Moisture Content of Growing Medium in Rockwool Culture*, DOI `10.7235/hort.2014.13177` — важность состояния корневой зоны вместе с атмосферным спросом.

## Следующий порядок работ

1. аппаратный `level + flow + power/brownout` safety loop;
2. защищённый commissioning + web-auth + signed OTA/rollback;
3. RTC для автономного времени;
4. `T/RH + solution T + light + flow` телеметрия;
5. локальный кольцевой журнал событий и измерений;
6. мастер калибровки расхода;
7. профили культуры/стадии роста;
8. только после накопления данных — отдельный `adaptive` mode при сохранении простого timer fallback.
