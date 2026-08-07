# HydroESP-C3

[![Build and verify](https://github.com/f2re/hydro-esp-c3/actions/workflows/build.yml/badge.svg)](https://github.com/f2re/hydro-esp-c3/actions/workflows/build.yml)

**Локальный контроллер гидропонной установки на ESP32-C3 Super Mini:** расписание, безопасное ручное управление насосом, режим обслуживания, калибровка гидравлики, OLED, NTP, desktop/mobile web UI, диагностика, backup и OTA — без облака и внешних CDN.

> Базовая автоматика проекта — проверяемый **таймерный контур**, а не универсальная модель водопотребления. Калибровка измеряет фактический расход установки; будущий adaptive irrigation требует реальных сенсоров и отдельной верификации. Методика и ограничения: [AUDIT.md](AUDIT.md).

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

Укажите домашний Wi‑Fi и часовой пояс в разделе **Система**. После перезагрузки устройство обычно доступно по `http://hydro.local`.

Полная инструкция: [docs/INSTALL.md](docs/INSTALL.md).

## Что умеет

- суточное расписание до **48 циклов**;
- отдельное сохранённое состояние **автоматика активна / пауза** без удаления графика;
- безопасный режим обслуживания и калибровки при приостановленной автоматике;
- сохранение графика, Wi‑Fi, UTC offset и гидравлической калибровки в NVS;
- ручной запуск с максимальным временем работы;
- защита от случайного старта: web и физическая BOOT требуют удержания;
- мгновенная остановка насоса;
- явная причина текущего запуска: расписание / web / BOOT / калибровка;
- журнал 32 последних управляющих событий текущей сессии в RAM;
- NTP + явная индикация синхронизации;
- mDNS `hydro.local` и fallback AP `HydroESP-Setup`;
- OLED 72×40 и Serial dashboard;
- self-contained web UI без CDN;
- desktop/tablet/mobile layouts, dark/light/system theme;
- toast/modal UX вместо `alert()` и `confirm()`;
- визуальная 24-часовая шкала расписания и dirty-state;
- пошаговый мастер измерения **фактического расхода насоса**;
- инженерный гидравлический калькулятор + диагностический VPD;
- импорт расписания только как черновик;
- диагностика RAM/flash/reset reason/version в браузере;
- безопасный JSON backup без Wi‑Fi-пароля;
- OTA drag-and-drop с прогрессом;
- CLI для установки, диагностики, режимов автоматики, журнала, backup/restore и OTA;
- release pipeline с version/build SHA и SHA-256 assets;
- CI-проверка C++, maintenance scripts и embedded JavaScript.

## Интерфейс

### Обзор

На первом экране видны:

- активна ли автоматика или установка находится в режиме обслуживания;
- текущее время и NTP;
- следующий плановый цикл;
- Wi‑Fi/RSSI/IP и uptime;
- версия прошивки;
- состояние насоса, остаток времени и **источник текущего запуска**;
- готовность таймерного режима;
- последние управляющие события.

Плановую автоматику можно поставить на паузу одним действием. Пауза не удаляет и не изменяет расписание. После возобновления Scheduler не пытается «догнать» слот, пропущенный во время обслуживания.

Запуск насоса вручную — **удержанием**, остановка — одним действием.

### Расписание

Редактор показывает количество циклов, суммарное время работы, duty cycle и 24-часовую шкалу. Дубли времени и некорректные длительности блокируются до отправки в устройство.

Заводской график в `src/config.h` — только fallback для текущей экспериментальной конфигурации с минеральной ватой. Это **не универсальная агрономическая рекомендация**.

### Гидравлика и калибровка

Калибровочный мастер реализует воспроизводимый поток:

1. поставить автоматику на паузу;
2. направить выход установки в мерную ёмкость;
3. выбрать короткий тест 15/30/60 секунд;
4. запустить насос как отдельный `calibration`-цикл;
5. измерить собранный объём в миллилитрах;
6. сохранить рассчитанный фактический расход, л/мин;
7. использовать этот расход в инженерном калькуляторе.

Тест аппаратно ограничен сервером диапазоном 5–120 секунд и не запускается при активной таймерной автоматике.

Калькулятор использует:

```text
Vcycle = N × d / 1000

ton = 60 × Vcycle / (Q × η)
```

где `Q` — **измеренный фактический расход**, а `η` — коэффициент эффективной доставки. Температура/RH дают VPD только как диагностический показатель; VPD не масштабирует полив автоматически.

Следующий научно корректный этап: `RAD/PPFD + VPD + level/flow + root-zone/drainage + safety interlocks`.

### Система и журнал

Раздел **Система** объединяет сеть/время, диагностику, резервное копирование и журнал текущей сессии.

Журнал фиксирует:

- boot;
- start/stop насоса;
- источник запуска;
- причину остановки;
- pause/resume автоматики;
- изменения расписания;
- сохранение гидравлики;
- OTA и reboot.

Журнал намеренно **RAM-only**: частые поливы не создают постоянные записи во flash. Долговременная история будет вводиться вместе с кольцевой сенсорной телеметрией.

## Установка, диагностика и обновление одной утилитой

Главный maintenance CLI: `tools/hydroctl.py`.

| Команда | Назначение |
|---|---|
| `python3 tools/hydroctl.py bootstrap` | подготовить локальный PlatformIO toolchain |
| `python3 tools/hydroctl.py build` | собрать прошивку |
| `python3 tools/hydroctl.py install` | собрать и прошить по USB |
| `python3 tools/hydroctl.py doctor` | проверить toolchain, serial, API, NTP, automation и калибровку |
| `python3 tools/hydroctl.py monitor` | Serial monitor 115200 |
| `python3 tools/hydroctl.py status` | получить полный оперативный статус |
| `python3 tools/hydroctl.py events` | показать журнал текущей сессии |
| `python3 tools/hydroctl.py pause` | поставить таймерную автоматику на паузу |
| `python3 tools/hydroctl.py resume` | возобновить таймерную автоматику |
| `python3 tools/hydroctl.py backup` | backup v2: график + automation + hydraulics |
| `python3 tools/hydroctl.py restore FILE` | восстановить график/калибровку, оставить автоматику на паузе |
| `python3 tools/hydroctl.py restore FILE --resume-automation` | восстановить и явно возобновить автоматику |
| `python3 tools/hydroctl.py update` | OTA последнего GitHub Release с SHA-256 |
| `python3 tools/hydroctl.py update --file firmware.bin` | OTA локального файла |

`restore` специально сначала отключает плановую автоматику. Без `--resume-automation` установка остаётся на паузе для проверки результата.

## Обновление

Из браузера: **Прошивка → `.bin` → подтверждение**.

Из CLI:

```bash
bash update.sh
```

или Windows:

```powershell
.\update.ps1
```

Перед reboot/OTA насос останавливается. Подробно: [docs/UPDATE.md](docs/UPDATE.md).

## Подключение ESP32-C3 Super Mini

| Компонент | GPIO | Назначение |
|---|---:|---|
| Relay IN | 4 | управление насосом, `OUTPUT_OPEN_DRAIN` |
| OLED SDA | 5 | I²C data |
| OLED SCL | 6 | I²C clock |
| LED | 8 | встроенная индикация |
| BOOT | 9 | удержание = ручной старт, нажатие при работе = стоп |

Насос нельзя питать от слабой линии ESP32. Силовая часть, общий провод, реле/ключ, подавление выбросов и запас источника питания рассчитываются отдельно.

## Архитектура проекта

```text
.
├── src/
│   ├── main.cpp                 boot + основной цикл
│   ├── config.h                 hardware profile, limits, factory schedule
│   ├── version.h                firmware/API version contract
│   ├── config_storage.*         NVS + validation + operational settings
│   ├── event_log.*              RAM-only operation journal
│   ├── relay_controller.*       timeout + source/reason tracking
│   ├── scheduler.*              timer automation + explicit pause
│   ├── wifi_manager.*           STA / provisioning AP
│   ├── ntp_manager.*            local time
│   ├── web_server.*             API v3 / UI / OTA
│   ├── web_ui_v2.h              responsive operations UI
│   ├── oled_display.*           OLED
│   ├── status_display.*         Serial dashboard
│   └── ota_manager.*            OTA progress state
├── tools/
│   ├── hydroctl.py              install/update/doctor/operations/backup CLI
│   └── check_web_ui.py          UI quality gate
├── scripts/build_flags.py       version/build metadata
├── docs/                        эксплуатационная документация
├── .github/workflows/           CI + release
├── install.sh / install.ps1
├── update.sh / update.ps1
├── partitions.csv
└── platformio.ini
```

Подробности: [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md).

## CI и качество

Pull Request должен пройти:

- Python syntax checks maintenance tooling;
- shell syntax checks;
- извлечение embedded HTML;
- `node --check` JavaScript;
- отсутствие duplicate `id`;
- запрет нативных `alert()` / `confirm()`;
- PlatformIO build;
- публикацию временного firmware artifact.

## Документация

- [Установка](docs/INSTALL.md)
- [Обновление и восстановление](docs/UPDATE.md)
- [Диагностика](docs/TROUBLESHOOTING.md)
- [Архитектура](docs/ARCHITECTURE.md)
- [HTTP API v3](docs/API.md)
- [Безопасность](docs/SECURITY.md)
- [Инженерный/научный аудит](AUDIT.md)
- [Как участвовать](CONTRIBUTING.md)

## Эксплуатационные ограничения

Текущая версия ещё не является промышленным safety-controller:

1. web/API и OTA работают по локальному HTTP без аутентификации;
2. OTA пока не проверяет криптографическую подпись образа на устройстве;
3. provisioning AP открыт;
4. нет датчика минимального уровня/сухого хода;
5. нет аппаратного подтверждения фактического потока;
6. нет post-boot health-check/automatic rollback;
7. сохранён brown-out workaround для старой установки — силовую часть нужно исправить и штатную защиту вернуть;
8. журнал событий текущей версии не является долговременным audit log.

Hardening-план: [docs/SECURITY.md](docs/SECURITY.md). Адаптивный сенсорный контур: issue #2.

## Лицензия

MIT — см. [LICENSE](LICENSE).
