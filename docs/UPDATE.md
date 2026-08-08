# Обновление и восстановление HydroESP-C3

После первичной USB-прошивки штатный способ обновления — OTA в доверенной локальной сети.

> [!IMPORTANT]
> В проекте теперь два разных типа BIN: **OTA** и **FACTORY**. Они не взаимозаменяемы.

| Образ | Для чего | Куда писать | NVS |
|---|---|---|---|
| `*-ota.bin` | обновление работающего контроллера | Web UI / `hydroctl update` | сохраняется |
| `*-factory.bin` | чистая плата, полностью стёртый flash, аварийное USB recovery | flash с адреса `0x0` | сбрасывается |

Не загружайте `factory.bin` через Web OTA.

## Перед обновлением

1. убедитесь, что питание ESP и насоса стабильно;
2. не отключайте питание во время записи flash;
3. сделайте backup;
4. для OTA используйте только `*-ota.bin`;
5. не выполняйте OTA через публичную или недоверенную сеть.

Backup v2:

```bash
python3 tools/hydroctl.py backup --output hydroesp-backup.json
```

Он содержит расписание, состояние timer automation, гидравлическую калибровку и безопасные сведения об устройстве. Wi-Fi password не экспортируется.

## Вариант 1 — браузер

1. откройте `http://hydro.local` или numeric IP с OLED;
2. перейдите в **Прошивка**;
3. выберите `*-ota.bin`;
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

`hydroctl` выбирает именно `hydro-esp-c3-latest-ota.bin`, проверяет SHA-256 и отправляет его на `/ota/upload`. Factory image updater намеренно отвергает.

Другой адрес устройства:

```bash
python3 tools/hydroctl.py update --host http://192.168.1.50
```

## Вариант 3 — локальный OTA BIN

```bash
python3 tools/hydroctl.py update \
  --file .pio/build/esp32c3_supermini/firmware.bin
```

Локальный `firmware.bin` PlatformIO является application/OTA image. Его нельзя записывать как полный образ с адреса `0x0` на чистую плату.

## Первичная USB-прошивка и factory recovery

Предпочтительный способ:

```bash
bash install.sh
```

или:

```bash
python3 tools/hydroctl.py install
```

PlatformIO сам пишет компоненты по правильным адресам ESP32-C3:

```text
0x0000   bootloader.bin
0x8000   partitions.bin
0xE000   boot_app0.bin
0x10000  firmware.bin
```

Для прошивальщиков, которые принимают один файл, Release содержит `hydro-esp-c3-<version>-factory.bin`. Это merged image, его пишут с адреса `0x0`.

> [!CAUTION]
> Factory image заполняет служебную область flash заново и предназначен для чистой установки/recovery. Сохранённые Wi-Fi, расписание и калибровка NVS при таком восстановлении считаются сброшенными. Если устройство ещё доступно, сначала сделайте backup.

## Сборка собственной версии

```bash
python3 tools/hydroctl.py build
```

Application/OTA image:

```text
.pio/build/esp32c3_supermini/firmware.bin
```

Полный factory image после build:

```bash
python3 tools/make_factory_image.py dist/hydro-esp-c3-factory.bin
```

Чистая сборка:

```bash
python3 tools/hydroctl.py build --clean
```

## Release-процесс

Tag `v*` запускает `.github/workflows/release.yml` и публикует:

```text
hydro-esp-c3-<version>-ota.bin
hydro-esp-c3-latest-ota.bin
hydro-esp-c3-<version>-factory.bin
hydro-esp-c3-latest-factory.bin
hydro-esp-c3.sha256
```

Тег должен указывать только на проверенный `main` с зелёным CI.

## Проверка после OTA

```bash
python3 tools/hydroctl.py doctor
python3 tools/hydroctl.py status
python3 tools/hydroctl.py events
```

Проверьте version/build SHA, NTP, automation, расписание, калибровку, Wi-Fi и причину последнего reboot.

## Восстановление backup — безопасный порядок

```bash
python3 tools/hydroctl.py restore hydroesp-backup.json
```

Restore намеренно сначала ставит timer automation на паузу, восстанавливает данные и оставляет автоматику paused до проверки.

Возобновить:

```bash
python3 tools/hydroctl.py resume
```

Или явно:

```bash
python3 tools/hydroctl.py restore hydroesp-backup.json --resume-automation
```

## Если Web UI после прошивки не открывается

См. отдельную короткую диагностику: [WEB_ACCESS.md](WEB_ACCESS.md).

Основной принцип:

- `SETUP WIFI` на OLED → подключиться к `HydroESP-Setup` и открыть `http://192.168.4.1`;
- обычный экран → использовать полный numeric IP со страницы OLED `WEB ADDRESS`;
- нет строки `[HydroESP]` в Serial после записи одиночного `firmware.bin` в `0x0` → выполнить правильную USB/factory recovery.

## Ограничение безопасности

Локальный HTTP API/OTA пока не имеют web-auth и device-side проверки криптографической подписи пользовательского образа. Используйте их только в доверенной LAN. Подробности: [SECURITY.md](SECURITY.md).
