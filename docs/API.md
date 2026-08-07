# HydroESP-C3 HTTP API

API предназначен для локальной сети. Текущая версия контракта: **v2**.

Базовый адрес в STA-режиме:

```text
http://hydro.local
```

В provisioning AP:

```text
http://192.168.4.1
```

## Общие правила

- JSON кодируется UTF-8;
- изменяющие состояние операции используют `POST`;
- пароль Wi‑Fi никогда не возвращается API;
- ошибки валидации возвращают HTTP 400 и JSON с полем `error`;
- OTA использует multipart upload и не является JSON endpoint.

## `GET /ping`

Проверка HTTP-сервера.

Ответ:

```text
pong
```

## `GET /api/status`

Основное оперативное состояние.

Пример:

```json
{
  "api_version": 2,
  "version": "v1.2.0",
  "build": "a1b2c3d4",
  "time": "18:42:15",
  "date": "07.08.2026",
  "time_synced": true,
  "uptime": 7215,
  "relay": false,
  "relay_remaining": 0,
  "relay_progress": -1,
  "ssid": "HomeWiFi",
  "rssi": -58,
  "ip": "192.168.1.50",
  "ap_mode": false,
  "next": "19:00",
  "schedule_count": 38
}
```

`rssi` может отсутствовать в AP-режиме.

## `GET /api/diagnostics`

Низкоуровневое состояние для UI/поддержки.

```json
{
  "api_version": 2,
  "version": "v1.2.0",
  "build": "a1b2c3d4",
  "free_heap": 123456,
  "min_free_heap": 110000,
  "flash_size": 4194304,
  "sketch_size": 890000,
  "free_sketch_space": 650000,
  "reset_reason": 1,
  "reset_reason_text": "питание включено"
}
```

## `POST /api/relay/on?duration=N`

Запускает насос на ограниченное время.

- `N` — секунды;
- минимум: 1;
- максимум: `MAX_WATERING_SECONDS` (сейчас 3600);
- некорректное значение заменяется безопасным ручным default.

Ответ:

```json
{"status":"ok"}
```

## `POST /api/relay/off`

Немедленно выключает насос.

```json
{"status":"ok"}
```

## `GET /api/schedule`

Возвращает массив суточных циклов:

```json
[
  {"h":6,"m":0,"d":180},
  {"h":6,"m":30,"d":120}
]
```

Поля:

- `h`: 0–23;
- `m`: 0–59;
- `d`: длительность в секундах.

## `POST /api/schedule`

Полностью заменяет расписание.

Тело — массив слотов. Ограничения:

- не более 48;
- уникальное `HH:MM`;
- длительность 1–3600 секунд.

Успех:

```json
{"status":"ok"}
```

Возможные ошибки:

```json
{"error":"schedule_must_be_array"}
{"error":"too_many_slots"}
{"error":"invalid_slot"}
{"error":"duplicate_time"}
```

## `POST /api/schedule/reset`

Восстанавливает заводской fallback-график из `src/config.h` и сразу записывает его в NVS.

## `GET /api/config`

Возвращает безопасную часть конфигурации:

```json
{
  "ssid":"HomeWiFi",
  "has_pass":true,
  "tz":3
}
```

`has_pass` сообщает только наличие сохранённого пароля. Сам пароль не выдаётся.

## `POST /api/config`

```json
{
  "ssid":"HomeWiFi",
  "pass":"new-password-or-empty",
  "tz":3
}
```

- пустой `pass` сохраняет старый пароль;
- SSID: 1–32 символа;
- пароль: до 63 символов;
- UTC offset: целое −12…+14.

После успешной записи насос останавливается, контроллер отвечает и перезагружается.

## `POST /api/reboot`

Останавливает насос и перезагружает контроллер.

## `POST /ota/upload`

Multipart/form-data upload application `.bin`.

Поле файла:

```text
file
```

Перед записью насос останавливается.

Успех:

```text
HTTP 200
OK
```

Ошибка Update API:

```text
HTTP 500
FAIL
```

После успешной записи контроллер перезагружается.

## `GET /manifest.webmanifest`

Минимальный manifest для standalone/PWA-представления встроенного UI.

## Совместимость клиентов

Клиенты должны проверять `api_version`. Новые поля могут добавляться без изменения версии, если существующая семантика не меняется. Breaking changes требуют увеличения `HYDRO_API_VERSION` и обновления этой страницы.

## Примеры

```bash
curl http://hydro.local/api/status
curl -X POST 'http://hydro.local/api/relay/on?duration=30'
curl -X POST http://hydro.local/api/relay/off
```

Для автоматизации предпочтительнее `tools/hydroctl.py`, поскольку он уже содержит проверку ошибок, backup и OTA-поток.
