# Безопасность HydroESP-C3

## Модель эксплуатации

HydroESP-C3 рассчитан на **доверенную локальную сеть** и физически контролируемую установку. HTTP API не должен публиковаться в интернет или пробрасываться через NAT.

## Что уже защищено программно

- Wi‑Fi password не возвращается через API и не попадает в backup;
- commissioning AP `HydroESP-Setup` защищён отдельным случайным device key;
- commissioning key хранится отдельно от основной конфигурации и переживает reboot/OTA;
- key показывается только локально: OLED + Serial recovery;
- state-changing операции используют `POST`;
- API валидирует schedule/config/hydraulics;
- любой pump start имеет maximum runtime;
- web и BOOT требуют hold-to-start;
- stop выполняется одним действием;
- reboot и OTA сначала выключают насос;
- timer automation имеет persisted pause;
- calibration endpoint работает только при paused automation и idle pump;
- restore по умолчанию оставляет automation paused;
- pump start получает source, stop — reason;
- текущая сессия имеет RAM-only operation journal;
- version/build SHA/API version доступны для диагностики.

Эти механизмы уменьшают операторский риск, но **не заменяют аппаратные interlocks**.

## Что пока НЕ защищено

### HTTP без TLS / web-auth

После подключения к LAN устройство использует локальный HTTP без пользовательской авторизации. Узел с доступом к этой LAN всё ещё может вызывать API управления.

Поэтому текущая версия:

- не предназначена для public/guest Wi‑Fi;
- не должна иметь WAN port-forward;
- желательно размещается в IoT VLAN или доверенной домашней LAN.

### OTA без device-side подписи

`hydroctl update` проверяет SHA-256 release asset на стороне клиента, но ESP пока не проверяет криптографическую подпись application image. Это контроль целостности updater-а, а не signed OTA/Secure Boot.

### Нет hardware dry-run / flow interlock

Сохранённый `pump_flow_lpm` — результат ручной калибровки. Он не подтверждает, что вода действительно течёт в каждом следующем цикле.

Пока отсутствуют:

- minimum-level sensor;
- realtime flow/current confirmation;
- авария `pump ON / no flow`;
- независимый hardware fail-safe.

### Brown-out workaround

Для совместимости со старой установкой может быть включён `HYDRO_DISABLE_BROWNOUT_WORKAROUND`. После исправления силового питания штатную brown-out protection нужно вернуть.

### Журнал не является persistent audit log

EventLog хранит до 32 событий только в RAM и очищается при reboot. Для долговременного security/audit нужен отдельный wear-aware ring log.

## Commissioning key

Первый/аварийный setup-flow:

```text
SSID: HydroESP-Setup
Password: <device key на OLED/Serial>
URL: http://192.168.4.1
```

Ключ генерируется из `esp_random()` и сохраняется в отдельном namespace `hydrosec`. Он не экспортируется в обычную резервную копию.

Подробности: [COMMISSIONING_SECURITY.md](COMMISSIONING_SECURITY.md).

## Рекомендованная сеть

- не публиковать TCP/80 в WAN;
- не использовать public/guest Wi‑Fi;
- отключить UPnP port-forward для устройства;
- по возможности выделить IoT VLAN;
- разрешить доступ только доверенным клиентам;
- разрешить DNS/NTP, если расписание работает без RTC.

## Безопасный maintenance workflow

```bash
python3 tools/hydroctl.py pause
python3 tools/hydroctl.py status
```

После обслуживания проверьте plumbing, schedule, calibration и events, затем:

```bash
python3 tools/hydroctl.py resume
```

Restore также оставляет automation paused без явного `--resume-automation`.

## Что делать дальше

Без лишнего усложнения приоритет такой:

1. **hardware level + flow interlock**;
2. вернуть brown-out protection после исправления питания;
3. простая авторизация опасных HTTP-действий;
4. signed OTA / rollback;
5. persistent telemetry/audit только когда она реально понадобится.

## Release integrity

GitHub Release workflow формирует `.sha256`; `hydroctl update` проверяет checksum при его наличии. До signed OTA доверие заканчивается на клиенте, выполняющем update.

## Сообщение об уязвимости

Не публикуйте Wi‑Fi credentials, commissioning key, приватные адреса или другие secrets. Укажите version/build SHA, затронутый component/endpoint и минимальные шаги воспроизведения.
