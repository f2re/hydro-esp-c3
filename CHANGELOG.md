# Changelog

Значимые изменения HydroESP-C3 фиксируются здесь. Версии выпускаются тегами `vMAJOR.MINOR.PATCH`; release firmware и checksum формирует GitHub Actions.

## Unreleased

### Added

- self-contained responsive web UI: desktop sidebar + mobile bottom navigation;
- light/dark/system theme, keyboard focus и reduced-motion support;
- hold-to-start для web и физической BOOT-кнопки;
- визуальная 24-часовая шкала расписания и dirty-state;
- сохранённый режим timer automation `active / paused` без удаления графика;
- отдельный maintenance flow;
- pump start source и stop reason;
- RAM-only журнал 32 последних управляющих событий текущей сессии;
- API v3: automation, hydraulics, calibration start, events;
- серийная гидравлическая калибровка: mean Q, sample CV, timestamp и protocol version;
- инженерный калькулятор на сохранённом Q + диагностический VPD;
- safe backup v2 без Wi‑Fi password;
- `hydroctl` для install/doctor/status/events/pause/resume/backup/restore/update;
- OTA/release pipeline с SHA-256;
- **SecurityManager** с отдельным persistent commissioning key;
- WPA2-защищённый `HydroESP-Setup`;
- OLED recovery screen с SSID/key/IP;
- `tools/export_ui_preview.py` для воспроизводимого preview embedded UI;
- CI screenshots: desktop overview, mobile overview и hydraulic calibration;
- визуальный README с badges, screenshots и коротким quick start.

### Changed

- restore fail-safe: сначала pause, затем restore; resume только явно;
- pause/resume не создаёт догоняющий schedule-cycle;
- calibration test разрешён только при paused automation и idle pump;
- default build не встраивает обязательный домашний Wi‑Fi credential;
- fallback setup network теперь требует отдельный device key;
- commissioning key хранится отдельно от schedule/Wi‑Fi configuration и не входит в backup;
- в AP mode OLED постоянно показывает данные для восстановления доступа;
- README сокращён: эксплуатационные детали вынесены в `docs/`;
- CI artifact теперь содержит firmware, mock UI preview и screenshots.

### Fixed

- потеря последних циклов 38-slot factory schedule при старом web-limit 32;
- выдача сохранённого Wi‑Fi password через read API;
- блокирующий reconnect в основном loop;
- неверный IP в AP mode;
- rollover relay progress/timeout;
- догоняющий schedule-cycle после manual overlap/resume;
- рассинхронизация OLED/Serial с runtime schedule;
- compile-time timezone вместо runtime настройки;
- случайный BOOT-start и start после удержания BOOT во время reset.

### Security / Safety

- reboot/OTA сначала выключают насос;
- любой pump start имеет maximum runtime;
- pause automation безопасно отделён от maintenance/manual cycles;
- protected commissioning AP заменяет открытую setup-сеть;
- commissioning secret доступен локально через OLED/Serial и не выдаётся HTTP endpoint;
- backup не содержит Wi‑Fi password или commissioning key;
- остаются честно открытыми: web-auth/TLS, signed OTA и hardware level/flow interlocks.

## До первой стабильной версии

Первый stable tag следует ставить только после:

1. зелёного CI на `main`;
2. стендового smoke-test реальной платы;
3. проверки USB install → protected provisioning → schedule → pause/resume → manual pump → calibration → backup/restore → OTA;
4. проверки питания и повторяемости Q;
5. фиксации hardware/security ограничений в release notes.
