# Аудит HydroESP-C3

Актуализировано: **2026-08-08**

## Состояние

HydroESP-C3 — local-first контроллер с timer automation, ручным управлением, maintenance pause, гидравлической калибровкой, OLED/Serial, Web UI, backup/restore, OTA и CI.

Текущий UX-принцип: **обычная установка не должна требовать ключей, паролей setup-сети, erase flash или специальных recovery-шагов**.

## Что закрыто

- `install.sh` / `install.ps1` просто собирают и прошивают контроллер;
- USB-install не удаляет существующие Wi‑Fi/расписание/калибровку;
- новая ненастроенная плата автоматически создаёт открытую `HydroESP-Setup`;
- Web UI setup доступен на `http://192.168.4.1` без пароля;
- ранее настроенная плата использует сохранённую LAN, а полный IP виден на OLED/Serial;
- HTTP не зависит от NTP/интернета;
- `update` также сохраняет настройки;
- для ручного flasher создаётся единый install image;
- limit расписания 48 слотов и server-side validation;
- maximum pump runtime, hold-to-start и мгновенный stop;
- reboot/OTA выключают насос;
- maintenance pause не удаляет schedule;
- source/reason tracking и RAM-журнал;
- серийная калибровка Q;
- responsive Web UI без CDN;
- backup/restore, diagnostics и CI.

## Открытые реальные риски

### P0 — аппаратная безопасность

1. Нет minimum-level/dry-run interlock.
2. Нет live flow confirmation.
3. Нужно исправить силовой power path и вернуть штатный brown-out detector.
4. Для критичного применения нет независимого hardware fail-safe.

### P1 — время и telemetry

- абсолютное время после reboot зависит от NTP;
- нет реальных `T/RH / solution T / light / level / live flow` streams.

### P2 — сетевой периметр

Web UI/API рассчитаны на доверенную LAN. Отдельную систему логинов/ролей намеренно не добавляем в базовый сценарий. При необходимости изоляция делается на уровне домашней сети/VLAN.

## Научная граница

Сохранённый `Q` — измеренный расход установки, а не модель потребности растения. VPD остаётся диагностикой. Adaptive irrigation имеет смысл только после реальных датчиков и накопления измерений.

## Следующий порядок без усложнения

1. level + live flow safety loop;
2. питание + brown-out protection;
3. `T/RH + solution T + light`;
4. adaptive irrigation только при наличии обратной связи;
5. всё остальное — только если возникнет реальная эксплуатационная потребность.
