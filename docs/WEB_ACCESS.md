# Если Web UI не открывается

У HydroESP-C3 есть два сетевых режима. Адрес зависит от того, в каком режиме загрузилась плата.

## 1. На OLED написано `SETUP WIFI`

Контроллер не подключился к сохранённому Wi‑Fi и поднял защищённую точку настройки.

```text
SSID: HydroESP-Setup
KEY:  показан на OLED и в Serial
URL:  http://192.168.4.1
```

Подключите телефон/ноутбук именно к `HydroESP-Setup` и открывайте **`http://192.168.4.1`**, а не `hydro.local`.

Если устройство сообщает «без доступа к Интернету» — это нормально: setup AP предназначен только для локальной настройки ESP.

## 2. На OLED обычный экран, а `HydroESP-Setup` нет

Скорее всего ESP уже подключилась к сохранённой домашней сети. Прошивка не должна стирать NVS при обычном обновлении, поэтому старый SSID может сохраниться.

Новая прошивка выводит отдельную OLED-страницу `WEB ADDRESS` с полным IPv4. Откройте:

```text
http://<IP с OLED>/
```

`http://hydro.local` — удобный mDNS alias, но numeric IP является основным fallback.

## 3. Быстрая проверка HTTP

```bash
curl http://<IP>/ping
```

Ожидаемый ответ:

```text
pong
```

Затем:

```bash
curl http://<IP>/api/status
```

Если `/ping` отвечает, а страница браузера нет — проблема уже на уровне браузера/cache, а не Wi‑Fi/HTTP сервера.

## 4. Посмотреть, куда реально загрузилась ESP

```bash
python3 tools/hydroctl.py monitor
```

Ищите строки:

```text
[WiFi] IP: ...
[WEB] Open: http://.../
[mDNS] http://hydro.local
```

или setup-flow:

```text
[SEC] Setup Wi-Fi: HydroESP-Setup
[SEC] Setup key: ...
[SEC] Setup URL: http://192.168.4.1
```

## 5. Важно: OTA BIN и полная первичная прошивка — не одно и то же

`firmware.bin` — application image, подходящий для OTA и для штатного PlatformIO upload. Если flash был полностью очищен, для восстановления нужны также bootloader и partition table либо единый merged/factory image.

Поэтому для первичной USB-установки используйте:

```bash
bash install.sh
```

или:

```bash
python3 tools/hydroctl.py install
```

Не записывайте standalone application `firmware.bin` в `0x0` как «полный образ».

## 6. Если по-прежнему ничего нет

Проверьте Serial boot log. Если нет строки `[HydroESP]`, проблема ниже web-уровня: загрузка image, bootloader/partition table, USB/питание или reset loop.

Если `[HydroESP]` есть, но нет ни `[WiFi] IP`, ни setup AP — приложите последние 50–100 строк Serial.
