# Установка HydroESP-C3

Установка должна быть предсказуемой: **одна команда → `HydroESP-Setup` → `192.168.4.1` → домашний Wi‑Fi**.

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

Обычный `install` выполняет **чистую установку**: сначала собирает прошивку, затем очищает старые настройки контроллера и прошивает его. Поэтому после успешной установки не нужно угадывать, к какой старой Wi‑Fi сети подключилась плата.

Если подключено несколько ESP, можно явно указать порт:

```bash
python3 tools/hydroctl.py install --port /dev/ttyACM0
```

Windows:

```powershell
py tools\hydroctl.py install --port COM5
```

## 3. Настроить Wi‑Fi

После чистой установки появляется сеть:

```text
SSID: HydroESP-Setup
Пароль: нет
URL: http://192.168.4.1
```

Дальше:

1. подключитесь к `HydroESP-Setup`;
2. откройте `http://192.168.4.1`;
3. в разделе **Система** укажите домашний Wi‑Fi;
4. сохраните настройки;
5. контроллер перезагрузится;
6. откройте IP, показанный на OLED, либо `http://hydro.local`.

Никаких setup key, отдельных логинов или паролей commissioning-сети нет.

## Переустановка без удаления настроек

Только если это действительно нужно:

```bash
python3 tools/hydroctl.py install --keep-settings
```

Этот режим сохраняет NVS: домашний Wi‑Fi, расписание и калибровку.

Для обычного обновления работающего устройства USB вообще не требуется:

```bash
python3 tools/hydroctl.py update
```

или используйте раздел **Прошивка** в Web UI.

## Если сайт не появился

1. посмотрите OLED — там должен быть полный IP;
2. при первой настройке откройте именно `http://192.168.4.1`;
3. если ESP уже в домашней сети, используйте показанный на OLED адрес вида `http://192.168.1.57`;
4. `hydro.local` считайте удобным дополнением, а не обязательным адресом.

Подробнее: [WEB_ACCESS.md](WEB_ACCESS.md).

## Проверка

```bash
python3 tools/hydroctl.py doctor
```

Перед автономной работой проверьте питание насоса, короткий ручной запуск, расписание и калибровку расхода.

## Документы

- [README](../README.md)
- [WEB_ACCESS.md](WEB_ACCESS.md)
- [UPDATE.md](UPDATE.md)
- [TROUBLESHOOTING.md](TROUBLESHOOTING.md)
