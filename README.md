# HydroESP-C3

[![Build and verify](https://github.com/f2re/hydro-esp-c3/actions/workflows/build.yml/badge.svg)](https://github.com/f2re/hydro-esp-c3/actions/workflows/build.yml)

**Локальный контроллер гидропонной установки на ESP32-C3 Super Mini:** расписание, безопасное ручное управление насосом, режим обслуживания, повторяемая гидравлическая калибровка, OLED, NTP, desktop/mobile web UI, диагностика, backup и OTA — без облака и внешних CDN.

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

## Возможности

- суточное расписание до **48 циклов**;
- отдельное сохранённое состояние **автоматика активна / пауза** без удаления графика;
- безопасный maintenance mode для обслуживания и калибровки;
- сохранение графика, Wi‑Fi, UTC offset и гидравлической калибровки в NVS;
- ручной запуск с maximum runtime;
- web и физическая BOOT требуют удержания для start;
- stop — одним действием;
- явная причина текущего запуска: schedule / web / BOOT / calibration;
- журнал 32 последних управляющих событий текущей сессии в RAM;
- NTP + явная индикация синхронизации;
- mDNS `hydro.local` и fallback AP `HydroESP-Setup`;
- OLED 72×40 и Serial dashboard;
- self-contained web UI без CDN;
- desktop/tablet/mobile layouts, dark/light/system theme;
- toast/modal UX вместо `alert()` и `confirm()`;
- 24-часовая шкала расписания и dirty-state;
- мастер **серийной** калибровки фактического расхода насоса;
- средний Q, число повторов, CV повторяемости и время последней калибровки;
- инженерный гидравлический калькулятор + диагностический VPD;
- import расписания только как черновик;
- диагностика RAM/flash/reset reason/version в браузере;
- безопасный JSON backup без Wi‑Fi-пароля;
- OTA drag-and-drop с progress;
- CLI для установки, диагностики, pause/resume, журнала, backup/restore и OTA;
- release pipeline с version/build SHA и SHA-256 assets;
- CI-проверка C++, maintenance scripts и embedded JavaScript.

## Интерфейс

### Обзор

На первом экране видны:

- active/paused automation mode;
- текущее время и NTP;
- следующий плановый цикл;
- Wi‑Fi/RSSI/IP и uptime;
- firmware version/build;
- состояние насоса, остаток времени и **источник текущего запуска**;
- готовность таймерного режима;
- последние управляющие события.

Плановую автоматику можно поставить на паузу одним действием. Пауза не удаляет и не меняет расписание. После resume Scheduler не пытается «догнать» слот, пропущенный во время обслуживания.

Запуск насоса вручную — **удержанием**, остановка — одним действием.

### Расписание

Редактор показывает количество циклов, суммарное время работы, duty cycle и 24-hour timeline. Дубли времени и некорректные длительности блокируются до отправки в устройство.

Factory-график в `src/config.h` — только fallback для текущей экспериментальной конфигурации с минеральной ватой. Это **не универсальная агрономическая рекомендация**.

### Гидравлика и калибровка

Калибровочный мастер реализует проверяемый поток:

1. поставить automation на паузу;
2. направить фактический output установки в мерную ёмкость;
3. выбрать test 15/30/60 s;
4. запустить pump как отдельный `calibration`-cycle;
5. измерить собранный объём в мл;
6. добавить измерение в серию;
7. повторить минимум 2 раза, рекомендуется 3+;
8. сравнить individual Q, средний Q и coefficient of variation;
9. при приемлемой повторяемости сохранить средний Q и `η`;
10. использовать сохранённую калибровку в инженерном калькуляторе.

Отдельный замер:

```text
Qi = Vml × 60 / tsec / 1000
```

Для серии:

```text
Qmean = ΣQi / n
CV = sample_stddev(Qi) / Qmean × 100%
```

CV здесь описывает **только повторяемость гидравлических измерений**. Малый CV не означает, что выбрана правильная норма воды для культуры.

UI требует минимум 2 замера. При `n ≥ 3` он оценивает разброс как ориентир: до 5% — хорошая повторяемость, 5–10% — умеренная, выше 10% — повод проверить воздух в магистрали, мерную ёмкость и стабильность потока. Это UX-индикатор, а не метрологический сертификат.

Сохраняются:

- mean Q, л/мин;
- delivery efficiency, %;
- sample count;
- CV, %;
- local NTP timestamp последней серии, если время синхронизировано.

Server calibration-run ограничен 5–120 s и не запускается при активной timer automation.

Инженерный калькулятор использует:

```text
Vcycle = N × d / 1000

ton = 60 × Vcycle / (Q × η)
```

Температура/RH дают VPD только как diagnostic indicator; VPD не масштабирует полив автоматически.

Следующий научно корректный этап: `RAD/PPFD + VPD + level/flow + root-zone/drainage + safety interlocks`.

### Система и журнал

Раздел **Система** объединяет сеть/время, diagnostics, backup и журнал текущей сессии.

Журнал фиксирует boot, pump start/stop, источник start, stop reason, pause/resume, schedule/hydraulics/config changes, OTA и reboot.

Журнал намеренно **RAM-only**: частые pump events не создают постоянные записи во flash. Persistent history должна появиться вместе с кольцевой sensor telemetry.

## Maintenance CLI

Главная утилита: `tools/hydroctl.py`.

| Команда | Назначение |
|---|---|
| `python3 tools/hydroctl.py bootstrap` | подготовить локальный PlatformIO toolchain |
| `python3 tools/hydroctl.py build` | собрать firmware |
| `python3 tools/hydroctl.py install` | собрать и прошить USB |
| `python3 tools/hydroctl.py doctor` | toolchain + serial + API + NTP + automation + calibration |
| `python3 tools/hydroctl.py monitor` | Serial monitor 115200 |
| `python3 tools/hydroctl.py status` | полный operational status |
| `python3 tools/hydroctl.py events` | current-session event log |
| `python3 tools/hydroctl.py pause` | pause timer automation |
| `python3 tools/hydroctl.py resume` | resume timer automation |
| `python3 tools/hydroctl.py backup` | backup v2: schedule + automation + hydraulics/series metadata |
| `python3 tools/hydroctl.py restore FILE` | restore и оставить automation paused |
| `python3 tools/hydroctl.py restore FILE --resume-automation` | restore + explicit resume |
| `python3 tools/hydroctl.py update` | OTA latest Release с SHA-256 |
| `python3 tools/hydroctl.py update --file firmware.bin` | OTA локального файла |

Restore сначала выключает plan-start, затем восстанавливает schedule/hydraulics. Без `--resume-automation` установка остаётся на паузе для проверки результата.

## Обновление

Browser: **Прошивка → `.bin` → подтверждение**.

macOS/Linux:

```bash
bash update.sh
```

Windows:

```powershell
.\update.ps1
```

Перед reboot/OTA pump останавливается. Подробно: [docs/UPDATE.md](docs/UPDATE.md).

## Подключение ESP32-C3 Super Mini

| Компонент | GPIO | Назначение |
|---|---:|---|
| Relay IN | 4 | управление насосом, `OUTPUT_OPEN_DRAIN` |
| OLED SDA | 5 | I²C data |
| OLED SCL | 6 | I²C clock |
| LED | 8 | встроенная индикация |
| BOOT | 9 | hold = manual start, нажатие при работе = stop |

Насос нельзя питать от слабой линии ESP32. Силовая часть, общий провод, relay/MOSFET, suppression выбросов и запас источника питания рассчитываются отдельно.

## Архитектура

```text
.
├── src/
│   ├── main.cpp                 boot + основной цикл
│   ├── config.h                 hardware profile, limits, fallback schedule
│   ├── version.h                firmware/API version contract
│   ├── config_storage.*         NVS + validation + operations/calibration
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

Подробнее: [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md).

## CI и качество

Каждый PR должен пройти:

- Python syntax checks;
- shell syntax checks;
- extraction embedded HTML;
- `node --check` JavaScript;
- duplicate `id` check;
- ban native `alert()` / `confirm()`;
- PlatformIO build;
- temporary firmware artifact upload.

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

1. web/API/OTA — local HTTP без authentication;
2. OTA пока не проверяет cryptographic signature на устройстве;
3. provisioning AP открыт;
4. нет minimum-level/dry-run sensor;
5. нет realtime подтверждения фактического flow;
6. нет post-boot health-check/automatic rollback;
7. сохранён brown-out workaround для старой установки — силовую часть нужно исправить и защиту вернуть;
8. session EventLog не является persistent audit log;
9. сохранённый Q — calibration constant, а не live flow feedback.

Hardening: [docs/SECURITY.md](docs/SECURITY.md). Adaptive sensor loop: issue #2.

## Лицензия

MIT — [LICENSE](LICENSE).
