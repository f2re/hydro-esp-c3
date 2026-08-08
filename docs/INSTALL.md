# Установка HydroESP-C3

Основной сценарий: **одна команда → адрес на OLED → Web UI**.

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

`install` просто собирает и прошивает контроллер. Он **не стирает flash и не удаляет сохранённые настройки**.

Если плата новая или Wi‑Fi ещё не настроен, после загрузки появится:

```text
SSID: HydroESP-Setup
Пароль: нет
URL: http://192.168.4.1
```

Если плата уже использовалась, она сохранит прежние Wi‑Fi/расписание/калибровку и подключится как раньше. Точный IP показывается на OLED.

Если подключено несколько ESP, можно явно указать порт:

```bash
python3 tools/hydroctl.py install --port /dev/ttyACM0
```

Windows:

```powershell
py tools\hydroctl.py install --port COM5
```

## 3. Первая настройка новой платы

1. подключитесь к открытой сети `HydroESP-Setup`;
2. откройте `http://192.168.4.1`;
3. в разделе **Система** укажите домашний Wi‑Fi;
4. сохраните настройки;
5. контроллер перезагрузится;
6. откройте IP, показанный на OLED, либо `http://hydro.local`.

Никаких setup key, логинов или дополнительных паролей установки нет.

## Обновление

Для обычного обновления работающего устройства USB не требуется:

```bash
python3 tools/hydroctl.py update
```

или используйте раздел **Прошивка** в Web UI. Настройки также сохраняются.

## Если сайт не появился

1. посмотрите OLED — там должен быть полный IP;
2. на новой ненастроенной плате используйте `http://192.168.4.1`;
3. на ранее настроенной плате используйте IP вида `http://192.168.1.57`, показанный на OLED;
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
