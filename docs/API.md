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
  "calibration_protocol_version": 1,
  "calibration_sample_count": 3,
  "calibration_cv_pct": 2.7,
  "calibration_epoch": 1786137735,
  "calibrated_at": "2026-08-07 18:42:15",
  "event_count": 7
}
```

`pump_source`: `none`, `schedule`, `web_manual`, `button_manual`, `calibration`.

`calibration_protocol_version = 0` означает legacy/unknown procedure. Новые серии, созданные текущим мастером, получают `HYDRO_CALIBRATION_PROTOCOL_VERSION = 1`.

`rssi` может отсутствовать в AP-режиме.

## `GET /api/diagnostics`

Возвращает version/build/API, free/min heap, flash/sketch sizes, reset reason и параметры RAM event log.

## Автоматика

### `GET /api/automation`

```json
{"enabled":true}
```

### `POST /api/automation`

```json
{"enabled":false}
```

Пауза не удаляет расписание, запрещает новые schedule-start, останавливает активный schedule-cycle, но не скрыто прерывает manual/calibration cycle. При resume Scheduler не «догоняет» слот текущей минуты. Состояние сохраняется в NVS.

## Ручное управление насосом

### `POST /api/relay/on?duration=N`

Ручной web-start, 1…3600 s.

### `POST /api/relay/off`

Немедленная manual stop.

## Калибровка гидравлики

### `POST /api/calibration/start?duration=N`

Запускает calibration-cycle только если automation paused и pump idle. Duration: 5…120 s.

Возможные конфликты:

```json
{"error":"automation_must_be_paused"}
{"error":"pump_busy"}
```

### `GET /api/hydraulics`

```json
{
  "flow_lpm": 1.742,
  "efficiency_pct": 85,
  "calibrated": true,
  "protocol_version": 1,
  "sample_count": 3,
  "cv_pct": 2.7,
  "calibration_epoch": 1786137735,
  "calibrated_at": "2026-08-07 18:42:15"
}
```

`sample_count` и `cv_pct` описывают **повторяемость серии**, а не точность агрономической нормы. `protocol_version` описывает методику, которой получена серия.

### `POST /api/hydraulics`

Минимальный вариант — изменение Q/η без замены metadata существующей серии:

```json
{
  "flow_lpm": 1.742,
  "efficiency_pct": 85
}
```

Новая серия:

```json
{
  "flow_lpm": 1.742,
  "efficiency_pct": 85,
  "sample_count": 3,
  "cv_pct": 2.7
}
```

Если `protocol_version` отсутствует при передаче новой серии, firmware ставит текущий `HYDRO_CALIBRATION_PROTOCOL_VERSION`.

Для restore допускается точное сохранение metadata старой серии:

```json
{
  "flow_lpm": 1.742,
  "efficiency_pct": 85,
  "protocol_version": 1,
  "sample_count": 3,
  "cv_pct": 2.7,
  "calibration_epoch": 1786137735
}
```

Ограничения:

- `flow_lpm`: `0` для очистки либо 0.05…100 л/мин;
- `efficiency_pct`: 10…100;
- `protocol_version`: 0…текущая версия протокола;
- `sample_count`: 0…9;
- `cv_pct`: 0…500.

Если `sample_count` не передан, существующие metadata серии сохраняются. Если `flow_lpm=0`, вся calibration metadata, включая protocol version, очищается. Если новая серия передана без `calibration_epoch`, сервер использует текущее синхронизированное локальное NTP-время либо 0 при отсутствии NTP.

Будущая/неизвестная protocol version не принимается как текущая. Это не позволяет прошивке молча интерпретировать измерение, выполненное по новой неизвестной методике.

Q хранится в NVS целым числом мл/мин; CV — в сотых долях процента.

Для отдельного измерения:

```text
Qi [л/мин] = measured_ml × 60 / duration_sec / 1000
```

Для серии web UI использует среднее Q и sample coefficient of variation:

```text
mean(Q) = sum(Qi) / n
CV = sample_stddev(Qi) / mean(Q) × 100%
```

UI требует минимум 2 повтора и рекомендует 3+. CV характеризует только разброс повторных измерений.

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

Типы: `boot`, `pump_start`, `pump_stop`, `automation_enabled`, `automation_paused`, `schedule_changed`, `hydraulics_saved`, `config_changed`, `ota_started`, `reboot_requested`.

Stop reasons: `manual`, `timeout`, `reboot`, `ota`, `automation_paused`.

`timestamp` отсутствует, если NTP ещё не синхронизирован; `uptime` присутствует всегда. Журнал RAM-only, максимум 32 записи.

### `POST /api/events/clear`

Очищает RAM-журнал текущей сессии.

## Расписание

### `GET /api/schedule`

```json
[
  {"h":6,"m":0,"d":180},
  {"h":6,"m":30,"d":120}
]
```

### `POST /api/schedule`

Полностью заменяет расписание. До 48 slots, уникальное `HH:MM`, duration 1…3600 s.

Ошибки: `schedule_must_be_array`, `too_many_slots`, `invalid_slot`, `duplicate_time`.

### `POST /api/schedule/reset`

Восстанавливает fallback-график `src/config.h`.

## Сеть и время

### `GET /api/config`

```json
{"ssid":"HomeWiFi","has_pass":true,"tz":3}
```

Password не выдаётся.

### `POST /api/config`

```json
{"ssid":"HomeWiFi","pass":"new-password-or-empty","tz":3}
```

Пустой `pass` оставляет старый пароль. После записи pump stop + reboot.

## `POST /api/reboot`

Фиксирует событие, останавливает pump и reboot.

## `POST /ota/upload`

Multipart/form-data, поле `file`. Перед записью фиксируется `ota_started` и pump stop с reason `ota`.

Успех: HTTP 200 + `OK`; ошибка: HTTP 500 + `FAIL`.

## `GET /manifest.webmanifest`

Manifest standalone/PWA-представления UI.

## Совместимость

API v3 добавляет operational mode, hydraulic calibration и current-session event log. Новые необязательные поля могут добавляться без bump, если существующая семантика не меняется. Breaking changes требуют увеличения `HYDRO_API_VERSION`.

Hydraulic calibration имеет отдельную `protocol_version`, потому что изменение методики измерения не обязательно требует breaking-change всего HTTP API.

## Примеры

```bash
curl http://hydro.local/api/status
curl http://hydro.local/api/events
curl -X POST -H 'Content-Type: application/json' \
  -d '{"enabled":false}' http://hydro.local/api/automation
curl -X POST 'http://hydro.local/api/calibration/start?duration=30'
curl -X POST -H 'Content-Type: application/json' \
  -d '{"flow_lpm":1.742,"efficiency_pct":85,"sample_count":3,"cv_pct":2.7}' \
  http://hydro.local/api/hydraulics
```

Для обычной эксплуатации предпочтительнее `tools/hydroctl.py`.
