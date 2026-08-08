# Безопасность HydroESP-C3

## Модель эксплуатации

HydroESP-C3 рассчитан на **доверенную локальную сеть** и физически контролируемую установку. HTTP API не должен публиковаться в интернет или пробрасываться через NAT.

## Что уже защищено программно

- Wi‑Fi password не возвращается через API и не попадает в backup;
- commissioning AP `HydroESP-Setup` защищён отдельным случайным device key;
- commissioning key хранится отдельно от основной конфигурации и переживает reboot/OTA;
- key показывается локально на OLED и в Serial recovery;
- state-changing операции используют `POST`;
- API валидирует schedule/config/hydraulics;
- любой pump start имеет maximum runtime;
- web и BOOT требуют hold-to-start;
- reboot и OTA сначала выключают насос;
- timer automation имеет persisted pause;
- calibration работает только при paused automation и idle pump;
- restore по умолчанию оставляет automation paused;
- pump start получает source, stop — reason;
- текущая сессия имеет RAM-only operation journal.

Эти механизмы уменьшают операторский риск, но **не заменяют аппаратные interlocks**.

## Что пока не закрыто

### HTTP без отдельной пользовательской авторизации

После подключения к LAN устройство использует локальный HTTP. Поэтому текущую версию не следует публиковать в WAN или использовать в public/guest Wi‑Fi.

### OTA без device-side подписи

`hydroctl update` проверяет SHA-256 release asset на стороне клиента, но ESP пока не проверяет криптографическую подпись application image.

### Нет hardware dry-run / flow interlock

Сохранённый `pump_flow_lpm` — результат ручной калибровки. Он не подтверждает наличие воды в каждом следующем цикле. Нужны minimum-level sensor и live flow/current confirmation.

### Brown-out workaround

После исправления силового питания штатную brown-out protection нужно вернуть.

## Commissioning key

```text
SSID: HydroESP-Setup
KEY:  показан на OLED/Serial
URL:  http://192.168.4.1
```

Ключ генерируется на устройстве и сохраняется в отдельном namespace `hydrosec`. Он не экспортируется обычной резервной копией.

Подробности: [COMMISSIONING_SECURITY.md](COMMISSIONING_SECURITY.md).

## Безопасный maintenance workflow

```bash
python3 tools/hydroctl.py pause
python3 tools/hydroctl.py status
```

После обслуживания проверьте plumbing, schedule, calibration и events, затем:

```bash
python3 tools/hydroctl.py resume
```

## Следующий разумный порядок

1. hardware level + live flow interlock;
2. исправление питания и возврат brown-out protection;
3. простая авторизация опасных HTTP-действий;
4. signed OTA / rollback при необходимости;
5. persistent telemetry/audit только когда она действительно нужна.
