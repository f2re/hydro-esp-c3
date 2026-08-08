<div align="center">

<img src="docs/assets/hydroesp-favicon.svg" width="88" alt="HydroESP-C3 icon">

# 🌱💧 HydroESP-C3

**Простой локальный контроллер полива на ESP32-C3 Super Mini**  
Расписание · ручной полив · обслуживание · калибровка · OLED · Web UI · OTA

[![Build and verify](https://github.com/f2re/hydro-esp-c3/actions/workflows/build.yml/badge.svg)](https://github.com/f2re/hydro-esp-c3/actions/workflows/build.yml)
![ESP32-C3](https://img.shields.io/badge/ESP32--C3-Super%20Mini-E7352C?logo=espressif&logoColor=white)
![API](https://img.shields.io/badge/API-v3-36d399)
![Local first](https://img.shields.io/badge/cloud-not%20required-70aaff)
![License](https://img.shields.io/badge/license-MIT-8b95a1)

**Подключил USB → установщик подхватил или спросил Wi‑Fi → открыл адрес с OLED. Всё.**

</div>

![HydroESP-C3 — главный экран](docs/assets/ui-desktop-overview.png)

> [!NOTE]
> Скриншоты сделаны из того же embedded Web UI, который компилируется в прошивку. Данные на них демонстрационные.

## 🚀 Установка за несколько минут

Нужны ESP32-C3 Super Mini, USB data-кабель, Python 3.9+ и Git. PlatformIO отдельно ставить не нужно.

```bash
git clone https://github.com/f2re/hydro-esp-c3.git
cd hydro-esp-c3
```

**macOS / Linux**

```bash
bash install.sh
```

**Windows PowerShell**

```powershell
.\install.ps1
```

Первый запуск скачивает PlatformIO/ESP32 toolchain и библиотеки. Дальше тяжёлые пакеты берутся из постоянного `~/.platformio`, а скомпилированные framework/library-объекты — из `~/.platformio/build-cache`. Обычные `git pull`, сборки и перепрошивки не должны скачивать весь toolchain заново. Build/version identity изолирован в одном объекте и не должен заставлять PlatformIO пересобирать весь Arduino framework при каждом новом commit SHA.

Для повторного deploy по USB:

```bash
./deploy.sh
```

А чтобы сначала безопасно забрать свежий код текущей ветки, затем прошить:

```bash
./deploy.sh --pull
```

На Windows: `.\deploy.ps1` или `.\deploy.ps1 -Pull`. Обновление кода выполняется только через fast-forward и блокируется при незакоммиченных изменениях.

Если Web UI повреждён или не отрисовывается, но контроллер остаётся в Wi‑Fi, прошивку можно обновить независимо от страницы:

```bash
./wifi-flash.sh 192.168.1.57
```

Windows:

```powershell
.\wifi-flash.ps1 192.168.1.57
```

`wifi-flash` показывает прогресс передачи и считает обновление успешным только после того, как контроллер перезагрузился и снова подтвердил `/api/status`. Штатно используется HTTP OTA; независимый ArduinoOTA/`espota.py` на порту 3232 можно вызвать явно:

```bash
./wifi-flash.sh 192.168.1.57 --transport arduino
```

Установщик сам ищет домашний Wi‑Fi в таком порядке:

1. `WIFI_SSID` / `WIFI_PASSWORD` из окружения;
2. локальный `.env`;
3. если ничего нет — спрашивает SSID и пароль в терминале.

Пароль вводится скрыто. Если на вопрос SSID нажать Enter, установка продолжается без блокировки и плата поднимает:

```text
Wi‑Fi: HydroESP-Setup
Пароль: нет
Сайт: http://192.168.4.1
```

Если Wi‑Fi был передан установщику, ESP сразу попробует подключиться к нему, а адрес появится на OLED. При переустановке остальные данные — расписание и калибровка — не стираются; новый SSID/password применяются одноразово.

> [!TIP]
> Чтобы вообще не отвечать на вопросы, скопируйте `.env.example` в `.env` и один раз заполните `WIFI_SSID` / `WIFI_PASSWORD`. `.env` уже исключён из Git.

Пример:

```dotenv
WIFI_SSID=MyHomeWiFi
WIFI_PASSWORD=my-secret-password
TIMEZONE_OFFSET=3
```

Или передайте переменные прямо в shell:

```bash
WIFI_SSID='MyHomeWiFi' WIFI_PASSWORD='secret' bash install.sh
```

> [!IMPORTANT]
> Никаких setup-key, паролей установочной сети, логинов, erase-flash и мастеров безопасности нет. Wi‑Fi-пароль не печатается в терминал.

📖 Подробно: [docs/INSTALL.md](docs/INSTALL.md) · Если сайт не виден: [docs/WEB_ACCESS.md](docs/WEB_ACCESS.md)

## ✨ Что умеет

| ⏱ Автоматика | 👆 Вручную | 🛠 Обслуживание |
|---|---|---|
| До 48 циклов в сутки | Hold-to-start | Пауза без удаления расписания |
| Локальная работа без облака | Stop одним нажатием | Калибровка без неожиданного автополива |
| Пропущенный цикл не «догоняется» | Ограничение времени запуска | Backup / restore / диагностика |

- 📅 редактируемое суточное расписание и визуальное автопостроение по рассвету/закату;
- 💧 ручное управление насосом;
- ⏸ режим обслуживания;
- 🧪 калибровка фактического расхода через мерную ёмкость;
- 📱 адаптивный desktop/mobile Web UI;
- 🌗 светлая, тёмная и системная темы;
- 🎨 встроенный favicon SVG + ICO без файловой системы;
- 🧾 журнал действий текущей сессии;
- 🩺 диагностика RAM / flash / reset reason;
- 💾 backup/restore без Wi‑Fi-пароля;
- ⬆️ HTTP OTA с прогрессом и независимый ArduinoOTA recovery по Wi‑Fi;
- 🖥 русифицированный OLED: IP, пауза автоматики, полив, OTA и ошибки видны без Web UI.

## 🖥 Интерфейс

<table>
<tr>
<td width="68%" valign="top">

### Desktop

<img src="docs/assets/ui-desktop-overview.png" alt="HydroESP desktop overview">

</td>
<td width="32%" valign="top">

### Mobile

<img src="docs/assets/ui-mobile-overview.png" alt="HydroESP mobile overview">

</td>
</tr>
</table>

### 📅 Расписание

Во вкладке **Расписание** есть конструктор: рассвет → закат или свои часы, периодичность либо точное число запусков, длительность вручную либо из гидравлики, 24-часовой preview и перенос в черновик перед явным сохранением.

Подробно: [docs/SCHEDULE.md](docs/SCHEDULE.md).

### 🎨 Favicon и Web-ресурсы

Браузерная иконка встроена прямо в application image: реальный `/favicon.ico` содержит 16×16 и 32×32, а `/favicon.svg` — масштабируемый вариант. Payload изображений занимает около **2.7 КБ flash** и не использует NVS/LittleFS или постоянный heap. Оба ресурса отдаются из `PROGMEM` explicit-length ответом и кэшируются браузером на семь суток.

Подробно: [docs/WEB_ASSETS.md](docs/WEB_ASSETS.md).

### 🧪 Калибровка расхода

<img src="docs/assets/ui-desktop-hydraulics.png" alt="HydroESP hydraulic calibration">

Мастер запускает короткие тесты насоса, вы вводите объём из мерной ёмкости, а контроллер считает фактический расход. Несколько повторов позволяют увидеть стабильность результата.

> [!IMPORTANT]
> Автоматика сейчас остаётся понятным таймерным контуром. Адаптивный полив имеет смысл добавлять только после появления реальных датчиков уровня/потока и окружающей среды.

## 🔌 Подключение

| Компонент | GPIO | Назначение |
|---|---:|---|
| Relay IN | `4` | насос |
| OLED SDA | `5` | I²C data |
| OLED SCL | `6` | I²C clock |
| LED | `8` | индикация |
| BOOT | `9` | удержание → ручной старт; при работе → stop |

> [!WARNING]
> Насос должен иметь нормальное отдельное питание/силовой драйвер. Если ESP32 перезагружается при старте насоса, исправляйте питание и ЭМС, а не расписание.

## 🔄 Установка и обновление

| Действие | Команда | Что происходит |
|---|---|---|
| Первая установка / переустановка по USB | `bash install.sh` / `.\install.ps1` | PlatformIO ставится при необходимости; Wi‑Fi берётся из env/.env или спрашивается; настройки устройства сохраняются |
| Быстрый повторный deploy по USB | `./deploy.sh` / `.\deploy.ps1` | использует уже скачанный toolchain и постоянный build-cache; сохранённый Wi‑Fi/NVS не меняется |
| Забрать код и сразу deploy | `./deploy.sh --pull` / `.\deploy.ps1 -Pull` | только `git pull --ff-only`, затем сборка/прошивка; dirty tree блокируется |
| Проверенное OTA по Wi‑Fi | `./wifi-flash.sh <IP>` / `.\wifi-flash.ps1 <IP>` | потоковая загрузка с прогрессом → финальное подтверждение → reboot → проверка build/uptime |
| Явный ArduinoOTA recovery | `./wifi-flash.sh <IP> --transport arduino` | штатный `espota.py`, порт 3232, Web UI/HTTP не требуются |
| Обновить через Web UI | раздел **Прошивка** | XHR progress через `/ota/upload`, настройки сохраняются |

Для ручного flasher в Release используется единый `hydro-esp-c3-install.bin`; внутренние bootloader/partition offsets пользователю выставлять не нужно.

## 🧰 Полезные команды

```bash
python3 tools/hydroctl.py doctor             # диагностика
python3 tools/hydroctl.py build              # локальная сборка с кэшем
python3 tools/hydroctl.py monitor            # Serial 115200
python3 tools/hydroctl.py status             # состояние
python3 tools/hydroctl.py pause              # пауза автоматики
python3 tools/hydroctl.py resume             # возобновить
python3 tools/hydroctl.py backup             # резервная копия
./wifi-flash.sh 192.168.1.57                 # проверенное OTA по Wi-Fi
./wifi-flash.sh 192.168.1.57 --transport arduino  # ArduinoOTA recovery
python3 tools/check_recovery_contract.py     # OTA/deploy contract
python3 tools/check_display_contract.py      # OLED contract
python3 tools/check_build_cache_contract.py  # incremental-build contract
```

## 🌐 Если сайт не открывается

Сначала смотрите OLED. Он показывает экран **САЙТ** и полный адрес, например:

```text
САЙТ
192.168.1.57
hydro.local
```

Открывайте:

```text
http://192.168.1.57
```

`hydro.local` — только удобное имя и зависит от mDNS вашей сети. Если Wi‑Fi был пропущен при install, setup-адрес — `http://192.168.4.1`.

Быстрая проверка HTTP:

```bash
curl http://192.168.1.57/ping
```

Если приходит `pong`, сеть и HTTP живы. Даже если сама `/` сломана, свежую прошивку можно залить через `./wifi-flash.sh 192.168.1.57`. Если отказал сам HTTP-сервер, используйте `--transport arduino`.

## 🔓 Сеть без лишних барьеров

`HydroESP-Setup` намеренно открытая сеть для локальной настройки, когда Wi‑Fi не был передан установщику или подключение не удалось. Никаких device key, логинов или дополнительных паролей установки нет.

Web API также рассчитан на доверенную домашнюю/лабораторную LAN. Не публикуйте порт контроллера напрямую в интернет.

## 🧱 Архитектура

```text
ESP32-C3
├── RelayController   насос + timeout
├── Scheduler         расписание + пауза
├── ConfigStorage     NVS + one-shot Wi-Fi seed
├── Wi‑Fi / NTP       STA или простой setup AP
├── RecoveryOTA       независимый ArduinoOTA, порт 3232
├── EventLog          журнал текущей сессии
├── OLED / Serial     русские состояния + IP/диагностика
└── AsyncWebServer
    ├── Web UI        потоково из flash/PROGMEM
    ├── favicon       ICO + SVG, ~2.7 КБ flash
    ├── API v3
    └── HTTP OTA      final ACK → deferred reboot
```

UI self-contained: внешние CDN/серверы для работы контроллера не нужны. Крупные статические ресурсы не должны копироваться целиком в Arduino `String`; они отдаются explicit-length ответами прямо из flash.

## 🧭 Дальше

- [x] простой USB install;
- [x] Wi‑Fi через env / `.env` / интерактивный ввод;
- [x] простой открытый setup AP как fallback;
- [x] Web UI desktop/mobile;
- [x] favicon без файловой системы;
- [x] независимый Wi‑Fi recovery OTA;
- [x] проверенное OTA с прогрессом и post-reboot контролем;
- [x] русифицированный OLED со статусами безопасности;
- [x] расписание + автопостроение + обслуживание;
- [x] калибровка расхода;
- [x] backup / OTA / диагностика;
- [ ] minimum-level interlock;
- [ ] live flow confirmation;
- [ ] T/RH + температура раствора;
- [ ] adaptive irrigation только после реальной обратной связи.

Roadmap: [issue #2](https://github.com/f2re/hydro-esp-c3/issues/2).

## 📚 Документация

- 🚀 [Установка](docs/INSTALL.md)
- 🌐 [Доступ к Web UI](docs/WEB_ACCESS.md)
- 📅 [Расписание и автопостроение](docs/SCHEDULE.md)
- 🖥 [OLED: экраны и статусы](docs/OLED.md)
- 🎨 [Web-ресурсы и favicon](docs/WEB_ASSETS.md)
- ⬆️ [Обновление и recovery](docs/UPDATE.md)
- 🩺 [Диагностика](docs/TROUBLESHOOTING.md)
- 📡 [HTTP API v3](docs/API.md)
- 🧱 [Архитектура](docs/ARCHITECTURE.md)
- 🧪 [Аудит](AUDIT.md)

---

<div align="center">

**MIT License** · ESP32-C3 · local-first 🌱

</div>