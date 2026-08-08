# Установка HydroESP-C3

Это руководство рассчитано на чистую **ESP32-C3 Super Mini** и установку по USB. После первой прошивки дальнейшие обновления можно выполнять из веб-интерфейса или через `hydroctl` по Wi‑Fi.

## Что понадобится

- ESP32-C3 Super Mini;
- USB-кабель **с линиями данных**;
- Python 3.9+;
- Git;
- насос/реле и отдельное подходящее питание;
- компьютер с Windows, macOS или Linux.

PlatformIO заранее устанавливать не требуется: `tools/hydroctl.py` при необходимости создаёт локальное `.venv`.

## 1. Получить проект

```bash
git clone https://github.com/f2re/hydro-esp-c3.git
cd hydro-esp-c3
```

## 2. Прошить контроллер

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

Если подключено несколько плат, укажите порт явно:

```bash
python3 tools/hydroctl.py install --port /dev/ttyACM0
```

или в Windows:

```powershell
py tools\hydroctl.py install --port COM5
```

## 3. Первая настройка Wi‑Fi

Обычная сборка намеренно **не содержит пароль домашнего Wi‑Fi**.

Если контроллер не может подключиться к сохранённой сети, он создаёт commissioning AP:

```text
SSID: HydroESP-Setup
URL:  http://192.168.4.1
```

Точка доступа защищена отдельным device key. Ключ генерируется на самом ESP32-C3, сохраняется между reboot/OTA, показывается на OLED и дублируется в Serial как recovery channel. Он не попадает в обычный backup и не является паролем домашнего Wi‑Fi.

Порядок:

1. включите контроллер;
2. на OLED найдите `HydroESP-Setup`, key и `192.168.4.1`;
3. подключитесь к `HydroESP-Setup` с этим ключом;
4. откройте `http://192.168.4.1`;
5. в разделе **Система** задайте домашний Wi‑Fi и часовой пояс;
6. нажмите **Сохранить и перезагрузить**;
7. после подключения к домашней сети откройте `http://hydro.local`.

Если OLED недоступен, используйте Serial:

```bash
python3 tools/hydroctl.py monitor
```

## Проверка после установки

```bash
python3 tools/hydroctl.py doctor
python3 tools/hydroctl.py status
```

## Перед автономной работой

1. проверьте состояние «насос выключен» после boot;
2. выполните короткий ручной цикл;
3. убедитесь, что ESP32 не перезагружается при пуске насоса;
4. поставьте автоматику на паузу;
5. выполните серийную калибровку фактического расхода;
6. проверьте расписание;
7. только после проверки возобновите автоматику.

> Перезагрузка ESP32 при пуске насоса — проблема питания/ЭМС, а не расписания. См. [TROUBLESHOOTING.md](TROUBLESHOOTING.md).

## Документы

- [README](../README.md)
- [UPDATE.md](UPDATE.md)
- [TROUBLESHOOTING.md](TROUBLESHOOTING.md)
- [SECURITY.md](SECURITY.md)
- [COMMISSIONING_SECURITY.md](COMMISSIONING_SECURITY.md)
