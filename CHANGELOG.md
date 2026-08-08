# Changelog

Значимые изменения HydroESP-C3 фиксируются здесь. Версии выпускаются тегами `vMAJOR.MINOR.PATCH`; release firmware и checksum формирует GitHub Actions.

## Unreleased

### Added

- responsive desktop/mobile Web UI без CDN;
- timer automation с persisted maintenance pause;
- source/reason tracking для pump cycles и RAM session log;
- серийная hydraulic calibration: mean Q, CV, timestamp, protocol version;
- `hydroctl` для install/doctor/status/pause/resume/backup/restore/update;
- diagnostics, OTA progress в Web/OLED и release pipeline;
- координаты установки, sunrise/sunset helper и отметка обслуживания раствора;
- защищённый commissioning AP `HydroESP-Setup` с отдельным device key;
- OLED/Serial recovery для commissioning key;
- визуальный README с desktop/mobile screenshots;
- CI-render screenshots из реального embedded UI с mock API.

### Changed

- restore остаётся fail-safe: automation paused до явного resume;
- `WiFiManager::startAP()` не запускает commissioning без валидного ключа;
- commissioning credential хранится отдельно от основной NVS-конфигурации;
- README/INSTALL/SECURITY/ARCHITECTURE/AUDIT синхронизированы с текущим runtime;
- полезные функции `enhance hydro controls` сохранены при интеграции security/UI изменений.

### Fixed

- потеря schedule slots из-за старого frontend limit;
- выдача Wi‑Fi password через read API;
- blocking Wi‑Fi reconnect;
- rollover relay timeout/progress;
- догоняющий schedule-cycle после manual overlap/resume;
- рассинхронизация OLED/Serial с runtime schedule;
- случайный pump start коротким BOOT press;
- открытый fallback commissioning AP.

### Known limits

- нет hardware minimum-level/live-flow interlock;
- brown-out workaround ещё требует аппаратного устранения причины;
- локальный HTTP пока без отдельной пользовательской auth;
- OTA пока без device-side signature verification/automatic rollback;
- adaptive irrigation не включается без реальных sensor streams.

## Перед стабильным release

1. зелёный CI на `main`;
2. USB install → protected commissioning → schedule → pause/resume → calibration → backup/restore → OTA smoke-test;
3. проверка реальной силовой части насоса;
4. фиксация известных hardware/security ограничений в release notes.
