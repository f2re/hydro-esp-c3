# Установка HydroESP-C3

Основной сценарий: **одна команда → Wi‑Fi → адрес на OLED → Web UI**.

## Что понадобится

- ESP32-C3 Super Mini;
- USB data-кабель;
- Python 3.9+;
- Git;
- Windows, macOS или Linux.

PlatformIO заранее ставить не нужно.

## 1. Скачать проект

```bash
git clone https://github.com/f2re/hydro-esp-c3.git
cd hydro-esp-c3
```

## 2. Установить

### macOS / Linux

```bash
bash install.sh
```

### Windows PowerShell

```powershell
.\install.ps1
```

### Универсально

```bash
python3 tools/hydroctl.py install
```

Установщик выбирает Wi‑Fi в простом порядке:

1. `WIFI_SSID` / `WIFI_PASSWORD` из окружения;
2. значения из локального `.env`;
3. если ничего не задано — спрашивает SSID и пароль в терминале.

Пароль вводится скрыто. Если на вопрос SSID просто нажать Enter, установка не блокируется: контроллер будет настроен позже через открытую сеть `HydroESP-Setup`.

`install` **не стирает flash, расписание или калибровку**. Если при установке указан новый Wi‑Fi, он одноразово заменяет сохранённые сетевые данные после первого boot новой прошивки.

## Кэш: тяжёлые утилиты скачиваются один раз

Первый запуск обязан скачать выбранную ESP32-платформу, Arduino framework, toolchain и upload tools. Дальше они не должны скачиваться заново при каждой сборке.

HydroESP-C3 использует штатные постоянные каталоги PlatformIO:

- `~/.platformio/platforms` — платформы;
- `~/.platformio/packages` — toolchain, framework и служебные утилиты;
- `~/.platformio/.cache` — кэш скачанных пакетов и registry;
- `~/.platformio/build-cache` — общий кэш уже скомпилированных объектов;
- `.pio/` — рабочие артефакты только текущего checkout.

Поэтому `git pull`, новая сборка и обычная перепрошивка используют уже скачанное. Повторная большая загрузка ожидаема только если изменился `platformio.ini`, выбран другой пользователь/HOME, кэш был удалён или PlatformIO действительно требует новый пакет.

Не удаляйте `~/.platformio` в deploy-скриптах и не запускайте сборку каждый раз под новым временным HOME.

## Обычный deploy после первой установки

Повторно клонировать репозиторий и разворачивать toolchain не нужно.

macOS / Linux:

```bash
./deploy.sh
```

Забрать свежие изменения текущей ветки и сразу прошить:

```bash
./deploy.sh --pull
```

Windows PowerShell:

```powershell
.\deploy.ps1
.\deploy.ps1 -Pull
```

Режим `--pull` / `-Pull` использует только `git pull --ff-only` и откажется обновлять дерево при незакоммиченных изменениях. Это исключает скрытые merge-коммиты и потерю локальной работы.

Если порт нужно задать вручную:

```bash
./deploy.sh --port /dev/ttyACM0
```

```powershell
.\deploy.ps1 --port COM5
```

## Вариант без вопросов: `.env`

Файл `.env` уже исключён из Git. Создайте его один раз:

```bash
cp .env.example .env
```

Пример:

```dotenv
WIFI_SSID=MyHomeWiFi
WIFI_PASSWORD=my-secret-password
TIMEZONE_OFFSET=3
```

После этого обычный `bash install.sh` / `.\install.ps1` ничего про Wi‑Fi не спрашивает.

Переменные окружения имеют приоритет над `.env`.

macOS/Linux:

```bash
WIFI_SSID='MyHomeWiFi' WIFI_PASSWORD='secret' bash install.sh
```

PowerShell:

```powershell
$env:WIFI_SSID = 'MyHomeWiFi'
$env:WIFI_PASSWORD = 'secret'
.\install.ps1
```

## Если Wi‑Fi не задан

После загрузки появляется:

```text
SSID: HydroESP-Setup
Пароль: нет
URL: http://192.168.4.1
```

Подключитесь к сети и задайте домашний Wi‑Fi в разделе **Система**.

## После установки

Если Wi‑Fi был передан установщику, контроллер сразу попробует подключиться к нему. Точный IP показывается на OLED и в Serial. Открывайте:

```text
http://<IP с OLED>
```

`http://hydro.local` остаётся дополнительным удобным адресом и зависит от mDNS вашей сети.

Если подключено несколько ESP, можно явно указать порт:

```bash
python3 tools/hydroctl.py install --port /dev/ttyACM0
```

Windows:

```powershell
py tools\hydroctl.py install --port COM5
```

## Обновление

Для обычного обновления работающего устройства USB не требуется:

```bash
python3 tools/hydroctl.py update
```

или используйте раздел **Прошивка** в Web UI. Настройки сохраняются.

## Если сайт не появился

1. посмотрите OLED — там должен быть полный IP;
2. если Wi‑Fi был задан при install, используйте IP домашней сети;
3. если Wi‑Fi был пропущен, подключитесь к `HydroESP-Setup` и откройте `http://192.168.4.1`;
4. `hydro.local` — удобное дополнение, но не обязательный способ доступа.

Подробнее: [WEB_ACCESS.md](WEB_ACCESS.md).

## Проверка

```bash
python3 tools/hydroctl.py doctor
```

Перед автономной работой проверьте питание насоса, короткий ручной запуск, расписание и калибровку расхода.

## Ручной flasher

В Release публикуется единый `hydro-esp-c3-install.bin`. Это вариант для ручного прошивальщика; обычному пользователю проще использовать `install.sh` / `install.ps1`.

## Документы

- [README](../README.md)
- [WEB_ACCESS.md](WEB_ACCESS.md)
- [UPDATE.md](UPDATE.md)
- [TROUBLESHOOTING.md](TROUBLESHOOTING.md)
