# Безопасность HydroESP-C3

## Модель эксплуатации

Текущая версия рассчитана на **доверенную локальную сеть** и физически контролируемую установку. HTTP API не должен публиковаться в интернет и не должен пробрасываться через роутер/NAT.

## Что уже защищено программно

- Wi‑Fi password не возвращается через `GET /api/config` и не попадает в backup;
- state-changing операции используют `POST`;
- API валидирует schedule/config/hydraulics;
- любой pump start имеет maximum runtime;
- web и physical BOOT требуют hold-to-start;
- stop выполняется одним действием;
- reboot и OTA сначала выключают pump;
- timer automation имеет отдельную persisted pause, не требующую удаления расписания;
- calibration endpoint работает только при paused automation и idle pump;
- calibration duration серверно ограничен 5–120 s;
- restore по умолчанию оставляет automation paused;
- pump start получает явный source, stop — reason;
- текущая сессия имеет RAM-only operation journal;
- version/build SHA/API version доступны для диагностики происхождения firmware.

Эти механизмы уменьшают риск операторской ошибки, но **не заменяют аппаратные interlocks**.

## Что пока НЕ защищено

### HTTP без TLS / web-auth

Устройство использует локальный HTTP. Узел с доступом к этой LAN может наблюдать/модифицировать трафик и вызывать API управления. До появления auth устройство нельзя публиковать в недоверенную сеть.

### OTA без device-side подписи

`hydroctl update` проверяет SHA-256 release asset на стороне клиента, но ESP пока не проверяет криптографическую подпись пользовательского application image. Это контроль целостности updater-а, а не Secure Boot/signed OTA.

### Provisioning AP открыт

`HydroESP-Setup` запускается без password для простого commissioning. Такой режим допустим только под физическим контролем.

### Нет hardware dry-run / flow interlock

Сохранённый `pump_flow_lpm` — результат ручной калибровки. Он **не подтверждает**, что вода реально течёт в каждом следующем цикле.

Пока нет:

- датчика минимального уровня;
- realtime flow/current confirmation;
- аварии `pump command ON, flow=0`;
- аппаратного fail-safe.

### Brown-out workaround

Для совместимости с существующей установкой может быть включён `HYDRO_DISABLE_BROWNOUT_WORKAROUND`. Это временная мера. После исправления силового питания штатную brown-out protection нужно вернуть.

### Журнал не является persistent audit log

Текущий EventLog хранит до 32 событий только в RAM и очищается при reboot. Это сделано намеренно, чтобы десятки pump events в сутки не писались в NVS.

Для security/audit задачи нужен отдельный persistent ring log с контролем ресурса flash/FRAM и политикой retention.

## Рекомендованная сеть

Минимально:

- не публиковать TCP/80 в WAN;
- не использовать public/guest Wi‑Fi;
- отключить UPnP-пробросы для устройства;
- по возможности выделить IoT VLAN;
- разрешить доступ только доверенным clients;
- разрешить исходящий DNS/NTP, если расписание работает без RTC.

## Безопасный maintenance workflow

Перед гидравлическими работами или калибровкой:

```bash
python3 tools/hydroctl.py pause
python3 tools/hydroctl.py status
```

После работы:

1. проверить schedule и plumbing;
2. при необходимости выполнить calibration;
3. проверить `events`;
4. только затем:

```bash
python3 tools/hydroctl.py resume
```

Restore также оставляет automation paused без явного `--resume-automation`.

## План hardening

Приоритет P0/P1:

1. low-level sensor + аппаратный запрет dry-run;
2. realtime flow/current confirmation;
3. авария `command ON / no flow`;
4. возврат brown-out detector после исправления питания;
5. защищённый provisioning credential;
6. session/web-auth для опасных операций;
7. CSRF policy после auth;
8. signed firmware / Secure Boot-compatible release process;
9. downgrade policy;
10. post-boot health confirmation + automatic rollback;
11. persistent audit/event log с ограниченным wear;
12. rate limiting управляющих endpoint.

## Release integrity

GitHub Release workflow формирует `.sha256`; `hydroctl update` проверяет checksum при его наличии. До signed OTA доверие всё ещё заканчивается на клиенте, выполняющем update.

## Сообщение об уязвимости

Не публикуйте рабочие Wi‑Fi credentials, приватные адреса или другие secrets.

Укажите version/build SHA, endpoint/component, требуемый уровень доступа, ожидаемое/фактическое поведение и минимальные шаги воспроизведения без секретов.
