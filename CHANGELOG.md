# Changelog

Значимые изменения HydroESP-C3 фиксируются здесь. Версии выпускаются тегами `vMAJOR.MINOR.PATCH`; release firmware и checksum формирует GitHub Actions.

## Unreleased

### Added

- self-contained responsive web UI: desktop sidebar + mobile bottom navigation;
- light/dark/system theme, keyboard focus и reduced-motion support;
- hold-to-start для web и физической BOOT-кнопки;
- визуальная 24-часовая шкала расписания и dirty-state;
- **сохранённый режим timer automation: active / paused** без удаления графика;
- отдельный maintenance flow для обслуживания и калибровки;
- явный источник каждого pump start: schedule / web / BOOT / calibration;
- явная причина pump stop: manual / timeout / reboot / OTA / automation pause;
- RAM-only кольцевой журнал 32 последних управляющих событий текущей сессии;
- API v3: automation, hydraulics, calibration start, events;
- пошаговый мастер измерения фактического расхода через мерную ёмкость;
- сохранение фактического `Q` в NVS как мл/мин;
- сохранение `delivery_efficiency_pct`;
- инженерный калькулятор, использующий сохранённую калибровку, и диагностический VPD;
- безопасный JSON backup v2: schedule + automation + hydraulics без Wi‑Fi password;
- device diagnostics endpoint/UI;
- firmware version, build SHA и API version;
- PWA manifest;
- `tools/hydroctl.py` для bootstrap/build/install/doctor/status/monitor/events/pause/resume/backup/restore/update;
- macOS/Linux и Windows installer/updater wrappers;
- SHA-256 verified latest-release updater;
- release workflow для versioned firmware assets;
- embedded web UI quality gate;
- эксплуатационная документация `docs/*`.

### Changed

- restore теперь fail-safe: сначала pause automation, затем schedule/hydraulics restore; автоматическое возобновление возможно только через явный `--resume-automation`;
- pause/resume потребляет текущую минуту Scheduler, поэтому пропущенный слот не запускается задним числом;
- calibration test разрешён только при paused automation и idle pump;
- default build больше не встраивает обязательный Wi‑Fi credential;
- first boot без рабочего SSID переходит в `HydroESP-Setup`;
- PlatformIO serial port больше не привязан к macOS;
- AsyncWebServer/AsyncTCP переведены на ESP32Async packages;
- ArduinoJson и embedded toolchain закреплены версиями;
- OLED и Serial dashboard используют runtime Scheduler;
- Serial показывает фактически применённый UTC offset;
- NTP date conversion не зависит от process timezone;
- README перестроен как продуктовая точка входа;
- install/update/troubleshooting/API/security инструкции разделены по задачам.

### Fixed

- потеря последних циклов исходного 38-slot графика при web-save из-за старого лимита 32;
- выдача сохранённого Wi‑Fi password через read API;
- потенциально блокирующий reconnect в основном loop;
- неверный IP в AP mode;
- rollover relay progress/timeout;
- возможный догоняющий schedule-cycle после manual overlap;
- возможный догоняющий slot сразу после resume automation;
- рассинхронизация OLED/Serial с runtime schedule;
- отображение compile-time timezone вместо runtime настройки;
- риск запуска насоса коротким случайным нажатием BOOT;
- риск интерпретировать удерживаемый во время reset BOOT как pump start.

### Security / Safety

- reboot и OTA сначала выключают насос;
- pause automation останавливает активный schedule-cycle, но не скрыто прерывает ручной/calibration cycle;
- calibration test ограничен сервером 5–120 s;
- любой pump start остаётся ограничен общим maximum runtime;
- API валидирует schedule/config/hydraulics;
- backup не содержит Wi‑Fi password;
- event journal намеренно RAM-only, чтобы не использовать NVS как высокочастотный лог;
- известные ограничения HTTP/auth/signed OTA/open provisioning AP и отсутствующих hardware interlocks документированы.

## До первой стабильной версии

Первый stable tag следует ставить только после:

1. зелёного CI на `main`;
2. стендового smoke-test реальной платы;
3. проверки USB install → provisioning → schedule → pause/resume → manual pump → hydraulic calibration → backup/restore → OTA;
4. повторяемой калибровки фактического Q;
5. фиксации hardware/security ограничений в release notes.
