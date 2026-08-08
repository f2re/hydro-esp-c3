# Changelog

Значимые изменения HydroESP-C3 фиксируются здесь. Версии выпускаются тегами `vMAJOR.MINOR.PATCH`.

## Unreleased

### Added

- responsive desktop/mobile Web UI без CDN;
- timer automation с persisted maintenance pause;
- source/reason tracking и RAM session log;
- серийная hydraulic calibration;
- `hydroctl` для install/doctor/status/pause/resume/backup/restore/update;
- install Wi‑Fi resolution: environment → `.env` → interactive prompt;
- one-shot `WIFI_SEED_ID`, который применяет установочный Wi‑Fi без очистки расписания/калибровки;
- regression-check для install env/.env;
- numeric Web IP на OLED/Serial;
- единый `hydro-esp-c3-install.bin` для ручной recovery/factory прошивки;
- visual README и воспроизводимые UI screenshots.

### Changed

- `install.sh` / `install.ps1` теперь автоматически используют `WIFI_SSID/WIFI_PASSWORD`, читают `.env` или спрашивают сеть и скрытый пароль;
- пустой SSID не блокирует установку: используется fallback `HydroESP-Setup`;
- install Wi‑Fi одноразово заменяет только сохранённые сетевые данные; schedule/calibration не стираются;
- `HydroESP-Setup` остаётся открытой fallback-сетью без device key и дополнительных паролей;
- обычный USB `install` не выполняет erase flash/NVS;
- `update` сохраняет Wi‑Fi, расписание и калибровку;
- HTTP стартует до NTP и не зависит от доступа к Интернету;
- release/build проверяют корректный flash layout, но технические детали скрыты от обычной установки.

### Removed

- `SecurityManager`;
- commissioning/operator key;
- отдельный `hydrosec` credential flow;
- необходимость искать key на OLED/Serial при первой установке;
- автоматический erase/reset настроек из обычного `install`.

### Fixed

- install с новым SSID теперь работает и на ранее настроенной плате, где старый NVS раньше имел приоритет над compile-time Wi‑Fi;
- конфликт OTA metadata / `boot_app0` в старом flash layout;
- возможность принять application `firmware.bin` за полный install image;
- неоднозначность адреса Web UI: полный IP теперь виден на OLED/Serial;
- зависимость setup Web UI от NTP;
- рассинхронизация OLED/Serial с runtime schedule;
- догоняющий schedule-cycle после manual overlap/resume;
- случайный pump start коротким BOOT press.

### Known limits

- нет hardware minimum-level/live-flow interlock;
- brown-out workaround ещё требует аппаратного устранения причины;
- Web UI/API рассчитаны на доверенную LAN;
- adaptive irrigation не включается без реальных sensor streams.

## Перед стабильным release

1. зелёный CI на `main`;
2. USB install с env/.env Wi‑Fi → прямое подключение к LAN;
3. USB install с пропуском SSID → `HydroESP-Setup` → Wi‑Fi setup;
4. reinstall настроенной платы с новым install Wi‑Fi → смена только сети, сохранение schedule/calibration;
5. проверка реальной силовой части насоса.
