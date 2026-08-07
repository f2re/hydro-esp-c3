# Обновление и восстановление HydroESP-C3

После первичной USB-прошивки штатный способ обновления — OTA в доверенной локальной сети.

## Перед обновлением

1. убедитесь, что питание ESP и насоса стабильно;
2. не отключайте питание во время записи flash;
3. сделайте backup;
4. используйте только application firmware HydroESP-C3 для ESP32-C3 Super Mini;
5. не выполняйте OTA через публичную или недоверенную сеть.

Backup v2:

```bash
python3 tools/hydroctl.py backup --output hydroesp-backup.json
```

Он содержит:

- расписание;
- состояние timer automation;
- фактический гидравлический расход и коэффициент доставки;
- безопасные сведения об устройстве.

Wi‑Fi password **никогда не экспортируется**.

## Вариант 1 — браузер

1. откройте `http://hydro.local`;
2. перейдите в **Прошивка**;
3. выберите или перетащите `.bin`;
4. проверьте имя и размер;
5. подтвердите операцию;
6. дождитесь завершения записи и reboot.

Перед фактической OTA-записью прошивка сама останавливает насос и фиксирует событие `ota_started`.

## Вариант 2 — последний GitHub Release

macOS/Linux:

```bash
bash update.sh
```

Windows:

```powershell
.\update.ps1
```

Универсально:

```bash
python3 tools/hydroctl.py update
```

`hydroctl` получает latest Release, скачивает `.bin` и `.sha256`, проверяет SHA-256 при наличии checksum и отправляет образ на `/ota/upload`.

Другой адрес устройства:

```bash
python3 tools/hydroctl.py update --host http://192.168.1.50
```

## Вариант 3 — локальный `.bin`

```bash
python3 tools/hydroctl.py update \
  --file .pio/build/esp32c3_supermini/firmware.bin
```

## Сборка собственной версии

```bash
python3 tools/hydroctl.py build
```

Результат:

```text
.pio/build/esp32c3_supermini/firmware.bin
```

Чистая сборка:

```bash
python3 tools/hydroctl.py build --clean
```

## Release-процесс

Tag `v*` запускает `.github/workflows/release.yml`:

1. build из конкретного commit;
2. version/build SHA в firmware;
3. versioned `.bin`;
4. `hydro-esp-c3-latest.bin`;
5. SHA-256 список;
6. GitHub Release.

Тег должен указывать только на проверенный `main` с зелёным CI.

## Проверка после OTA

```bash
python3 tools/hydroctl.py doctor
python3 tools/hydroctl.py status
python3 tools/hydroctl.py events
```

Проверьте:

- version/build SHA и API version;
- NTP;
- `automation_enabled`;
- сохранённое расписание;
- гидравлическую калибровку;
- Wi‑Fi;
- причину последнего reboot;
- короткий ручной тест, если это безопасно для установки.

## Восстановление backup — безопасный порядок

```bash
python3 tools/hydroctl.py restore hydroesp-backup.json
```

Restore намеренно выполняется fail-safe:

1. **сначала ставит timer automation на паузу**;
2. восстанавливает расписание;
3. для backup v2 восстанавливает гидравлическую калибровку;
4. оставляет автоматику на паузе;
5. оператор проверяет график и состояние;
6. только затем явно возобновляет работу.

Возобновить отдельно:

```bash
python3 tools/hydroctl.py resume
```

Или сразу после restore, если результат уже проверен и это действительно требуется:

```bash
python3 tools/hydroctl.py restore hydroesp-backup.json --resume-automation
```

Опция названа явно, чтобы восстановление конфигурации не могло незаметно запустить старый график.

Backup v1 по-прежнему принимается для совместимости, но не содержит гидравлическую калибровку.

В web UI импорт JSON по-прежнему создаёт только **несохранённый черновик расписания**. Никакого скрытого POST в NVS нет.

## Режим обслуживания перед работами

Для монтажа, калибровки, обслуживания гидравлики или проверки трубок:

```bash
python3 tools/hydroctl.py pause
```

Пауза сохраняется в NVS и не удаляет расписание. После завершения:

```bash
python3 tools/hydroctl.py resume
```

Пропущенный во время паузы слот не запускается задним числом.

## Если OTA оборвалась

1. не запускайте насос до проверки устройства;
2. дождитесь возможного reboot;
3. проверьте `doctor`, `status`, `events`;
4. если web/API не поднимаются — выполните USB recovery:

```bash
python3 tools/hydroctl.py install --clean
```

Таблица разделов содержит две OTA app-партиции, но автоматический post-boot health confirmation/rollback пока не реализован. Две партиции сами по себе не являются гарантированным rollback-контуром.

## Ограничение безопасности

Локальный HTTP API/OTA пока не имеют web-auth и device-side проверки криптографической подписи пользовательского образа. Используйте их только в доверенной изолированной LAN. Подробности: [SECURITY.md](SECURITY.md).
