# Обновление и восстановление HydroESP-C3

Для обычной эксплуатации всё просто:

```text
install = собрать и прошить по USB, существующие настройки не стираются
update  = обновить работающий контроллер по сети, настройки не стираются
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

OTA сохраняет домашний Wi‑Fi, расписание и калибровку.

## Переустановить по USB

```bash
bash install.sh
```

или Windows:

```powershell
.\install.ps1
```

USB-install также не выполняет erase. Новая плата после загрузки создаёт `HydroESP-Setup`; ранее настроенная снова подключается к сохранённой сети. Адрес всегда можно посмотреть на OLED.

## Один BIN для ручного прошивальщика

В GitHub Release есть файл:

```text
hydro-esp-c3-install.bin
```

Это единый полный образ для ручной прошивки с адреса `0x0`. Он нужен только если вы используете сторонний flasher и хотите один файл вместо набора bootloader/partitions/application.

Обычному пользователю проще `install.sh` / `install.ps1`.

> [!IMPORTANT]
> Ручная полная прошивка `install.bin` — recovery/factory-сценарий и может заменить содержимое служебных областей flash. Если контроллер уже работает, используйте обычный `install` или OTA.

## Если обновление оборвалось

Подключите USB и снова выполните:

```bash
bash install.sh
```

После загрузки откройте адрес с OLED. Если устройство было настроено, его настройки должны сохраниться.

## Restore

```bash
python3 tools/hydroctl.py restore hydroesp-backup.json
```

После restore плановая автоматика остаётся на паузе. Проверьте расписание и затем:

```bash
python3 tools/hydroctl.py resume
```

## Для разработчиков

Внутри проекта существуют application image для OTA и merged install image для recovery/manual flasher. CI проверяет оба, но эти детали не входят в обычный пользовательский сценарий.

Если Web UI не находится, см. [WEB_ACCESS.md](WEB_ACCESS.md).
