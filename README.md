# HydroESP-C3

[![Build and verify](https://github.com/f2re/hydro-esp-c3/actions/workflows/build.yml/badge.svg)](https://github.com/f2re/hydro-esp-c3/actions/workflows/build.yml)

**Локальный контроллер гидропонной установки на ESP32-C3 Super Mini:** расписание и ручное управление насосом, OLED, NTP, адаптивный desktop/mobile web UI, диагностика, резервные копии и OTA — без облака и внешних CDN.

> Базовая автоматика проекта — проверяемый **таймерный контур**, а не «магическая» универсальная модель водопотребления. Инженерный расчёт отделён от будущего adaptive irrigation; научная методика и ограничения разобраны в [AUDIT.md](AUDIT.md).

## Быстрый старт

Нужны Python 3.9+, Git, ESP32-C3 Super Mini и USB-кабель с передачей данных. PlatformIO вручную устанавливать не требуется.

```bash
git clone https://github.com/f2re/hydro-esp-c3.git
cd hydro-esp-c3
```

**macOS / Linux:**

```bash
bash install.sh
```

**Windows PowerShell:**

```powershell
.\install.ps1
```

После первой прошивки обычная сборка **не содержит пароль Wi‑Fi**. Контроллер создаст точку доступа:

```text
Wi‑Fi: HydroESP-Setup
URL:   http://192.168.4.1
```

Укажите домашний Wi‑Fi и часовой пояс в разделе **Система**. После перезагрузки устройство обычно доступно по:

```text
http://hydro.local
```

Полная инструкция: [docs/INSTALL.md](docs/INSTALL.md).

## Что умеет

- суточное расписание до **48 циклов**;
- сохранение графика, Wi‑Fi и UTC offset в NVS;
- безопасный ручной запуск с максимальным временем работы;
- защита от случайного старта: web и физическая кнопка требуют удержания;
- мгновенная остановка насоса;
- NTP + явная индикация, синхронизировано ли время;
- mDNS `hydro.local`;
- fallback provisioning AP `HydroESP-Setup`;
- OLED 72×40 и Serial dashboard;
- self-contained web UI без CDN;
- desktop, tablet и mobile layouts;
- тёмная / светлая / системная тема;
- toast/modal UX вместо `alert()` и `confirm()`;
- визуальная 24-часовая шкала расписания;
- явное состояние «есть несохранённые изменения»;
- инженерный гидравлический калькулятор + диагностический VPD;
- импорт расписания только как **черновик**, без скрытой записи;
- диагностика RAM/flash/reset reason/version прямо в браузере;
- безопасный JSON backup без Wi‑Fi-пароля;
- OTA drag-and-drop с прогрессом;
- CLI для установки, диагностики, backup/restore и OTA;
- release pipeline с version/build SHA и SHA-256 release assets;
- CI-проверка C++, maintenance scripts и встроенного JavaScript.

## Интерфейс

### Обзор

Оператор сразу видит:

- текущее время и статус NTP;
- следующий плановый цикл;
- Wi‑Fi/RSSI/IP;
- uptime;
- версию прошивки и build SHA;
- состояние насоса и остаток текущего цикла.

Запуск насоса — **удержанием**, чтобы случайный tap на телефоне не включил подачу. Остановка — одним действием.

### Расписание

Редактор показывает:

- количество циклов;
- суммарное время работы насоса;
- duty cycle за сутки;
- 24-часовую визуальную шкалу;
- дубли времени и некорректные длительности до отправки в устройство;
- отдельный dirty-state до сохранения.

Заводской график в `src/config.h` — только fallback для существующей экспериментальной конфигурации с минеральной ватой. Это **не универсальная агрономическая рекомендация**.

### Расчёт

Калькулятор использует измеряемую гидравлику:

```text
Vcycle = N × d / 1000

ton = 60 × Vcycle / (Q × η)
```

где:

- `N` — число растений;
- `d` — целевая подача на растение за цикл, мл;
- `Q` — **фактически измеренный** расход системы, л/мин;
- `η` — эффективность доставки;
- `ton` — длительность включения насоса.

Температура/RH дают VPD как диагностический показатель атмосферного спроса. VPD намеренно **не масштабирует полив автоматически**: без света/радиации, фактического расхода, дренажа и состояния корневой зоны это было бы ложной точностью.

Следующий научно корректный этап: `RAD/PPFD + VPD + water/root-zone feedback + safety interlocks`. План зафиксирован в [AUDIT.md](AUDIT.md) и issue про adaptive irrigation.

## Установка, диагностика и обновление одной утилитой

Главный maintenance CLI:

```text
tools/hydroctl.py
```

Основные команды:

| Команда | Назначение |
|---|---|
| `python3 tools/hydroctl.py bootstrap` | подготовить локальный PlatformIO toolchain |
| `python3 tools/hydroctl.py build` | собрать прошивку |
| `python3 tools/hydroctl.py install` | собрать и прошить по USB |
| `python3 tools/hydroctl.py doctor` | проверить Python, PlatformIO, serial и устройство |
| `python3 tools/hydroctl.py monitor` | открыть Serial monitor 115200 |
| `python3 tools/hydroctl.py status` | получить оперативный статус |
| `python3 tools/hydroctl.py backup` | экспортировать безопасный backup |
| `python3 tools/hydroctl.py restore FILE` | восстановить расписание |
| `python3 tools/hydroctl.py update` | OTA из последнего GitHub Release с SHA-256 проверкой |
| `python3 tools/hydroctl.py update --file firmware.bin` | OTA локального файла |

Справка:

```bash
python3 tools/hydroctl.py --help
```

## Обновление

Из браузера: **Прошивка → перетащить `.bin` → подтвердить**.

Из CLI:

```bash
bash update.sh
```

или Windows:

```powershell
.\update.ps1
```

Перед reboot/OTA прошивка останавливает насос. Release workflow публикует versioned `.bin`, `hydro-esp-c3-latest.bin` и SHA-256 список.

Подробно: [docs/UPDATE.md](docs/UPDATE.md).

## Подключение ESP32-C3 Super Mini

| Компонент | GPIO | Назначение |
|---|---:|---|
| Relay IN | 4 | управление насосом, `OUTPUT_OPEN_DRAIN` |
| OLED SDA | 5 | I²C data |
| OLED SCL | 6 | I²C clock |
| LED | 8 | встроенная индикация |
| BOOT | 9 | ручное управление: удержание = старт, нажатие при работе = стоп |

Насос нельзя питать от слабой линии питания ESP32. Силовая часть, земля, реле/ключ, подавление выбросов и запас источника питания должны быть рассчитаны отдельно.

## Архитектура проекта

```text
.
├── src/
│   ├── main.cpp                 boot + основной цикл
│   ├── config.h                 hardware profile, limits, factory schedule
│   ├── version.h                firmware/API version contract
│   ├── config_storage.*         NVS + validation
│   ├── relay_controller.*       ограниченный по времени насос
│   ├── scheduler.*              суточный Scheduler
│   ├── wifi_manager.*           STA / provisioning AP
│   ├── ntp_manager.*            время
│   ├── web_server.*             API / UI / OTA
│   ├── web_ui_v2.h              self-contained responsive web app
│   ├── oled_display.*           OLED
│   ├── status_display.*         Serial dashboard
│   └── ota_manager.*            OTA progress state
├── tools/
│   ├── hydroctl.py              install/update/doctor/backup CLI
│   └── check_web_ui.py          UI quality gate
├── scripts/
│   └── build_flags.py           version/build metadata
├── docs/
│   ├── INSTALL.md
│   ├── UPDATE.md
│   ├── TROUBLESHOOTING.md
│   ├── ARCHITECTURE.md
│   ├── API.md
│   └── SECURITY.md
├── .github/workflows/
│   ├── build.yml                CI build + UI/tool checks
│   └── release.yml              tag → release firmware + checksums
├── install.sh / install.ps1
├── update.sh / update.ps1
├── partitions.csv
└── platformio.ini
```

Подробная архитектура и инварианты безопасности: [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md).

## Сборка для разработки

```bash
python3 tools/hydroctl.py build
```

Или напрямую:

```bash
pio run
```

Порт не зашит в `platformio.ini`: PlatformIO определяет его автоматически, либо его можно передать в `hydroctl --port`.

Зависимости и toolchain закреплены в `platformio.ini` для воспроизводимой сборки. Каждый build получает `HYDRO_VERSION` и короткий commit SHA через `scripts/build_flags.py`.

## CI и качество

Pull Request должен пройти:

- Python syntax checks для maintenance tooling;
- shell syntax checks;
- извлечение embedded HTML;
- `node --check` встроенного JavaScript;
- проверку отсутствия duplicate `id`;
- запрет возврата нативных `alert()` / `confirm()`;
- PlatformIO build;
- публикацию временного firmware artifact.

## Документация

- [Установка](docs/INSTALL.md)
- [Обновление и восстановление](docs/UPDATE.md)
- [Диагностика](docs/TROUBLESHOOTING.md)
- [Архитектура](docs/ARCHITECTURE.md)
- [HTTP API v2](docs/API.md)
- [Безопасность](docs/SECURITY.md)
- [Инженерный/научный аудит](AUDIT.md)
- [Как участвовать в разработке](CONTRIBUTING.md)

## Эксплуатационные ограничения

До закрытия security/hardware этапа не следует считать текущую версию промышленным safety-controller:

1. web/API и OTA работают по локальному HTTP без аутентификации;
2. application OTA ещё не проверяет криптографическую подпись образа на устройстве;
3. provisioning AP пока открыт;
4. нет датчика минимального уровня/сухого хода;
5. нет подтверждения фактического потока;
6. нет автоматического post-boot health-check/rollback;
7. для совместимости со старой установкой пока сохранён brown-out workaround — силовую часть нужно исправить и штатную защиту вернуть.

Граница безопасной эксплуатации и hardening-план: [docs/SECURITY.md](docs/SECURITY.md).

## Лицензия

MIT — см. [LICENSE](LICENSE).
