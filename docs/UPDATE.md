# Обновление и восстановление HydroESP-C3

У проекта теперь три разные операции, и смешивать их нельзя:

```text
install     = первая USB-установка / явное задание Wi-Fi
             может применить WIFI_SSID/WIFI_PASSWORD из env/.env

deploy      = обычная повторная USB-прошивка
             никогда не меняет сохранённый Wi-Fi/NVS

wifi-flash  = повторная прошивка по Wi-Fi
             использует отдельный recovery OTA порт 3232 и не зависит от Web UI
```

## Обычный повторный deploy по USB

После первой установки используйте:

```bash
./deploy.sh
```

Windows:

```powershell
.\deploy.ps1
```

`deploy` собирает и прошивает application image, но намеренно не передаёт в сборку `WIFI_SSID`, `WIFI_PASSWORD` или `WIFI_SEED_ID`. Поэтому локальный `.env` не может незаметно заменить сеть контроллера.

Для обновления текущей ветки перед прошивкой:

```bash
./deploy.sh --pull
```

## Аварийное обновление по Wi-Fi без Web UI

Начиная с прошивки с recovery OTA контроллер постоянно обслуживает отдельный OTA-канал на TCP/UDP-порту `3232`. Он не использует страницу `/`, JavaScript или `ESPAsyncWebServer`.

Узнайте IP на OLED и выполните:

```bash
python3 tools/wifi_flash.py 192.168.1.57
```

Команда:

1. собирает текущую прошивку без Wi-Fi provisioning flags;
2. отправляет `firmware.bin` через независимый ArduinoOTA/espota канал `3232`;
3. контроллер перезагружается с сохранёнными настройками.

Можно отправить готовый application image без сборки:

```bash
python3 tools/wifi_flash.py 192.168.1.57 --file firmware.bin
```

`hydro.local` тоже поддерживается, но при диагностике надёжнее использовать числовой IP с OLED.

### Переход со старой прошивки

Старая прошивка ещё не знает recovery-порт `3232`. Поэтому `wifi_flash.py` сначала пробует новый независимый канал, а при отсутствии ответа автоматически пробует старый HTTP endpoint `/ota/upload` напрямую. Для этого главная Web-страница не обязана открываться или корректно отрисовываться — достаточно, чтобы HTTP-сервер устройства работал.

Если и `3232`, и `/ota/upload` недоступны, один раз подключите USB и выполните `./deploy.sh` либо первичную установку с корректным Wi-Fi. После этого recovery OTA будет встроен в прошивку.

## Обычное OTA через HTTP

Старый способ сохранён для совместимости:

```bash
python3 tools/hydroctl.py update --host 192.168.1.57 --file firmware.bin
```

Или для опубликованного GitHub Release:

```bash
python3 tools/hydroctl.py update --host 192.168.1.57
```

Он использует `/ota/upload`. Web UI через браузер для этой команды не требуется, но сам HTTP-сервер контроллера должен работать.

## Первая установка / смена Wi-Fi

Только для новой платы либо когда нужно намеренно записать другую сеть:

```bash
bash install.sh
```

Windows:

```powershell
.\install.ps1
```

`install` может брать Wi-Fi из окружения или `.env`. Это provisioning-сценарий, а не команда для повседневного повторного deploy.

Если Wi-Fi не подходит, контроллер после таймаута поднимает открытую сеть `HydroESP-Setup`; её адрес — `http://192.168.4.1`. IP текущего режима также выводится на OLED и в Serial.

## Один BIN для ручного прошивальщика

В GitHub Release есть файл:

```text
hydro-esp-c3-install.bin
```

Это единый полный образ для ручной прошивки с адреса `0x0`. Он нужен только для factory/recovery через сторонний flasher.

> [!IMPORTANT]
> Полный `install.bin` может заменить служебные области flash. Для работающего контроллера используйте `deploy`, `wifi_flash.py` или application OTA.

## Backup / restore

Перед заметными изменениями можно сделать backup:

```bash
python3 tools/hydroctl.py backup --host 192.168.1.57
```

Restore:

```bash
python3 tools/hydroctl.py restore hydroesp-backup.json --host 192.168.1.57
```

После restore плановая автоматика остаётся на паузе до явного возобновления.

Если Web UI не находится, см. [WEB_ACCESS.md](WEB_ACCESS.md).
