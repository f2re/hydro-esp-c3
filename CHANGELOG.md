# Changelog

Значимые изменения HydroESP-C3 фиксируются здесь. Версии выпускаются тегами `vMAJOR.MINOR.PATCH`; release firmware и checksum формирует GitHub Actions.

## Unreleased

### Added

- полностью переработанный self-contained responsive web UI;
- desktop sidebar и mobile bottom navigation;
- light/dark/system theme и reduced-motion support;
- hold-to-start для web и физической кнопки;
- визуальная 24-часовая шкала расписания и dirty-state;
- инженерный гидравлический калькулятор и диагностический VPD;
- безопасный JSON backup/import draft;
- device diagnostics endpoint/UI;
- firmware version, build SHA и API version;
- PWA manifest;
- `tools/hydroctl.py` для bootstrap/build/install/doctor/status/monitor/backup/restore/update;
- macOS/Linux и Windows installer/updater wrappers;
- SHA-256 verified latest-release updater;
- release workflow для versioned firmware assets;
- embedded web UI quality gate;
- эксплуатационная документация `docs/*`.

### Changed

- default build больше не встраивает фиктивный/обязательный Wi‑Fi credential;
- first boot без рабочего SSID сразу переходит в `HydroESP-Setup`;
- PlatformIO serial port больше не привязан к macOS;
- AsyncWebServer/AsyncTCP переведены на ESP32Async packages;
- ArduinoJson и embedded toolchain закреплены версиями;
- OLED и Serial dashboard используют фактическое runtime-расписание Scheduler;
- Serial dashboard показывает фактически применённый UTC offset;
- NTP date conversion не зависит от process timezone;
- README перестроен как продуктовая точка входа;
- installer/update/troubleshooting/API/security инструкции разделены по задачам.

### Fixed

- потеря последних циклов исходного 38-slot графика при web-save из-за старого лимита 32;
- выдача сохранённого Wi‑Fi пароля через read API;
- потенциально блокирующий reconnect в основном цикле;
- неверный IP в AP mode;
- rollover-ошибки relay progress/timeout;
- возможный догоняющий плановый цикл после ручного полива в той же минуте;
- рассинхронизация OLED/Serial с изменённым через web расписанием;
- отображение compile-time timezone вместо runtime настройки;
- риск запуска насоса коротким случайным нажатием BOOT;
- риск интерпретировать удерживаемый во время reset BOOT как команду запуска насоса.

### Security

- reboot и OTA теперь сначала выключают насос;
- API валидирует расписание и длительность на стороне ESP;
- backup не содержит Wi‑Fi password;
- известные ограничения HTTP/auth/signed OTA/open provisioning AP явно документированы.

## До первой стабильной версии

Первый стабильный tag следует ставить только после:

1. зелёного CI на `main`;
2. стендового smoke-test реальной платы;
3. проверки USB install → provisioning → schedule → manual pump → backup → OTA;
4. фиксации известных hardware/security ограничений в release notes.
