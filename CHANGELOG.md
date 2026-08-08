# Changelog

Значимые изменения HydroESP-C3 фиксируются здесь. Версии выпускаются тегами `vMAJOR.MINOR.PATCH`.

## Unreleased

### Added

- responsive desktop/mobile Web UI без CDN;
- timer automation с persisted maintenance pause;
- source/reason tracking и RAM session log;
- серийная hydraulic calibration;
- `hydroctl` для install/doctor/status/pause/resume/backup/restore/update;
- numeric Web IP на OLED/Serial;
- единый `hydro-esp-c3-install.bin` для ручной чистой прошивки;
- visual README и воспроизводимые UI screenshots.

### Changed

- первичная установка упрощена до `install → HydroESP-Setup → 192.168.4.1 → домашний Wi‑Fi`;
- `HydroESP-Setup` снова открытая сеть без device key и дополнительных паролей;
- `install` по умолчанию очищает старую NVS-конфигурацию для предсказуемого first boot;
- `install --keep-settings` оставлен только как явный advanced сценарий;
- `update` сохраняет Wi‑Fi, расписание и калибровку;
- HTTP стартует до NTP и не зависит от доступа к Интернету;
- release/build по-прежнему проверяют корректный flash layout, но технические детали скрыты от обычной установки.

### Removed

- `SecurityManager`;
- commissioning/operator key;
- отдельный `hydrosec` credential flow;
- необходимость искать key на OLED/Serial при первой установке.

### Fixed

- конфликт OTA metadata / `boot_app0` в старом flash layout;
- возможность принять application `firmware.bin` за полный install image;
- потеря Web UI из-за сохранённого старого SSID после «первичной» установки;
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
2. USB install → открытая `HydroESP-Setup` → Wi‑Fi setup → schedule → calibration → OTA smoke-test;
3. проверка реальной силовой части насоса.
