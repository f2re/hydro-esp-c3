# Обновление и восстановление HydroESP-C3

Для обычной эксплуатации достаточно помнить два действия:

```text
install  = поставить с нуля, настройки очищаются
update   = обновить работающий контроллер, настройки сохраняются
```

## Обновить работающий контроллер

Самый простой вариант:

```bash
python3 tools/hydroctl.py update
```

macOS/Linux также:

```bash
bash update.sh
```

Windows:

```powershell
.\update.ps1
```

Можно обновиться и из Web UI через раздел **Прошивка**.

Перед обновлением полезно сделать backup:

```bash
python3 tools/hydroctl.py backup
```

OTA не очищает домашний Wi‑Fi, расписание и калибровку.

## Установить заново

```bash
bash install.sh
```

или Windows:

```powershell
.\install.ps1
```

Обычная установка очищает старые настройки, поэтому следующий boot гарантированно ведёт в:

```text
HydroESP-Setup
http://192.168.4.1
```

Если нужна именно USB-переустановка **без** очистки данных:

```bash
python3 tools/hydroctl.py install --keep-settings
```

## Один BIN для ручного прошивальщика

В GitHub Release есть файл:

```text
hydro-esp-c3-install.bin
```

Это готовый полный образ для чистой установки. Его можно писать одним файлом с адреса `0x0` в прошивальщиках ESP32-C3.

Пользователю не требуется отдельно выбирать bootloader, partition table или их адреса.

> [!IMPORTANT]
> `hydro-esp-c3-install.bin` предназначен для установки с нуля и очищает прежнюю конфигурацию. Для обычного обновления используйте `hydroctl update` или Web UI.

## Если обновление оборвалось

Подключите USB и снова выполните:

```bash
bash install.sh
```

После этого настройте `HydroESP-Setup` заново и при необходимости восстановите backup.

## Restore

```bash
python3 tools/hydroctl.py restore hydroesp-backup.json
```

После restore плановая автоматика остаётся на паузе. Проверьте расписание и затем:

```bash
python3 tools/hydroctl.py resume
```

## Для разработчиков

Внутри проекта по-прежнему существуют отдельный application image и merged install image. CI проверяет оба. Это техническая деталь сборки и не должна усложнять обычную установку.

Если Web UI не находится, см. [WEB_ACCESS.md](WEB_ACCESS.md).
