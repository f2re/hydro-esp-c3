# HydroESP-C3 HTTP API

API предназначен для локальной сети. Текущая версия контракта: **v3**.

STA: `http://hydro.local`  
Provisioning AP: `http://192.168.4.1`

## Общие правила

- JSON — UTF-8;
- изменяющие состояние операции используют `POST`;
- Wi‑Fi password никогда не возвращается API;
- валидация возвращает HTTP 400 + `{ "error": "..." }`;
- конфликт режима/занятый насос может возвращать HTTP 409;
- OTA — multipart upload, не JSON;
- клиенты должны проверять `api_version`.

## `GET /ping`

Ответ: `pong`.

## `GET /api/status`

Основное оперативное состояние.

```json
{
  "api_version": 3,
  "version": "v1.3.0",
  "build": "a1b2c3d4",
  "time": "18:42:15",
  "date": "07.08.2026",
  "time_synced": true,
  "uptime": 7215,
  "relay": false,
  "relay_remaining": 0,
  "relay_progress": -1,
  "pump_source": "none",
  "ssid": "HomeWiFi",
  "rssi": -58,
  "ip": "192.168.1.50",
  "ap_mode": false,
  "next": "19:00",
  "schedule_count": 38,
  "automation_enabled": true,
  "pump_flow_lpm": 1.742,
  "delivery_efficiency_pct": 85,
  "hydraulics_calibrated": true,
  "event_count": 7
}
```

`pump_source`:

- `none`;
- `schedule`;
- `web_manual`;
- `button_manual`;
- `calibration`.

`rssi` может отсутствовать в AP-режиме.

## `GET /api/diagnostics`

```json
{
  "api_version": 3,
  "version": "v1.3.0",
  "build": "a1b2c3d4",
  "free_heap": 123456,
  "min_free_heap": 110000,
  "flash_size": 4194304,
  "sketch_size": 890000,
  "free_sketch_space": 650000,
  "reset_reason": 1,
  "reset_reason_text": "питание включено",
  "event_log_capacity": 32,
  "event_log_session_only": true
}
```

## Автоматика

### `GET /api/automation`

```json
{"enabled":true}
```

### `POST /api/automation`

```json
{"enabled":false}
```

Пауза:

- не удаляет расписание;
- запрещает новые плановые запуски;
- если в этот момент работает именно `schedule`-цикл, он останавливается с причиной `automation_paused`;
- ручные циклы не выключаются скрыто;
- при возобновлении Scheduler не «догоняет» слот, пропущенный в текущей минуте.

Состояние сохраняется в NVS.

## Ручное управление насосом

### `POST /api/relay/on?duration=N`

Запускает ручной web-цикл.

- минимум 1 с;
- максимум `MAX_WATERING_SECONDS` (3600 с);
- неверное значение заменяется безопасным default.

```json
{"status":"ok"}
```

### `POST /api/relay/off`

Немедленная ручная остановка.

```json
{"status":"ok"}
```

## Калибровка гидравлики

### `POST /api/calibration/start?duration=N`

Запускает отдельный calibration-цикл.

Условия:

- таймерная автоматика должна быть на паузе;
- насос должен быть выключен;
- длительность 5–120 с.

Успех:

```json
{"status":"ok"}
```

Конфликты:

```json
{"error":"automation_must_be_paused"}
{"error":"pump_busy"}
```

Некорректная длительность:

```json
{"error":"calibration_duration_5_120"}
```

### `GET /api/hydraulics`

```json
{
  "flow_lpm":1.742,
  "efficiency_pct":85,
  "calibrated":true
}
```

### `POST /api/hydraulics`

```json
{
  "flow_lpm":1.742,
  "efficiency_pct":85
}
```

Ограничения:

- `flow_lpm`: `0` для очистки либо 0.05…100 л/мин;
- `efficiency_pct`: 10…100.

Расход хранится в NVS как целое число мл/мин, чтобы не зависеть от бинарного представления float.

Калибровочный мастер web UI вычисляет расход из измерения:

```text
Q [л/мин] = measured_ml × 60 / duration_sec / 1000
```

## Журнал текущей сессии

### `GET /api/events`

```json
{
  "session_only": true,
  "events": [
    {
      "sequence": 7,
      "uptime": 412,
      "timestamp": "2026-08-07 18:42:15",
      "type": "pump_stop",
      "source": "calibration",
      "reason": "timeout",
      "value": 0
    }
  ]
}
```

Типы событий:

- `boot`;
- `pump_start`;
- `pump_stop`;
- `automation_enabled`;
- `automation_paused`;
- `schedule_changed`;
- `hydraulics_saved`;
- `config_changed`;
- `ota_started`;
- `reboot_requested`.

Причины остановки:

- `manual`;
- `timeout`;
- `reboot`;
- `ota`;
- `automation_paused`.

`timestamp` отсутствует, если на момент события NTP ещё не был синхронизирован; `uptime` присутствует всегда.

Журнал RAM-only и содержит максимум 32 записи. Он не является долговременным audit log.

### `POST /api/events/clear`

Очищает только RAM-журнал текущей сессии.

## Расписание

### `GET /api/schedule`

```json
[
  {"h":6,"m":0,"d":180},
  {"h":6,"m":30,"d":120}
]
```

Поля:

- `h`: 0–23;
- `m`: 0–59;
- `d`: 1–3600 секунд.

### `POST /api/schedule`

Полностью заменяет расписание.

Ограничения:

- не более 48 слотов;
- уникальное `HH:MM`;
- длительность 1–3600 секунд.

Возможные ошибки:

```json
{"error":"schedule_must_be_array"}
{"error":"too_many_slots"}
{"error":"invalid_slot"}
{"error":"duplicate_time"}
```

### `POST /api/schedule/reset`

Восстанавливает fallback-график из `src/config.h` и записывает его в NVS.

## Сеть и время

### `GET /api/config`

```json
{
  "ssid":"HomeWiFi",
  "has_pass":true,
  "tz":3
}
```

Пароль не выдаётся.

### `POST /api/config`

```json
{
  "ssid":"HomeWiFi",
  "pass":"new-password-or-empty",
  "tz":3
}
```

- пустой `pass` оставляет старый пароль;
- SSID 1–32 символа;
- password до 63 символов;
- UTC offset −12…+14.

После записи насос останавливается и устройство перезагружается.

## `POST /api/reboot`

Фиксирует событие, останавливает насос и перезагружает контроллер.

## `POST /ota/upload`

Multipart/form-data, поле `file`.

Перед записью:

1. фиксируется `ota_started`;
2. насос останавливается с причиной `ota`;
3. запускается Update API.

Успех:

```text
HTTP 200
OK
```

Ошибка:

```text
HTTP 500
FAIL
```

## `GET /manifest.webmanifest`

Manifest standalone/PWA-представления UI.

## Совместимость

API v3 добавляет операции, гидравлику и журнал сессии. Новые необязательные поля могут добавляться без bump версии, если существующая семантика не меняется. Breaking changes требуют увеличения `HYDRO_API_VERSION`.

## Примеры

```bash
curl http://hydro.local/api/status
curl http://hydro.local/api/events
curl -X POST -H 'Content-Type: application/json' \
  -d '{"enabled":false}' http://hydro.local/api/automation
curl -X POST 'http://hydro.local/api/calibration/start?duration=30'
curl -X POST -H 'Content-Type: application/json' \
  -d '{"flow_lpm":1.742,"efficiency_pct":85}' http://hydro.local/api/hydraulics
```

Для обычной эксплуатации предпочтительнее `tools/hydroctl.py`.
