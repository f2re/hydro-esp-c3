<div align="center">

# 🌱💧 HydroESP-C3

**Локальный контроллер гидропоники на ESP32-C3 Super Mini**  
Расписание · ручной полив · обслуживание · калибровка расхода · OLED · Web UI · OTA

[![Build and verify](https://github.com/f2re/hydro-esp-c3/actions/workflows/build.yml/badge.svg)](https://github.com/f2re/hydro-esp-c3/actions/workflows/build.yml)
![ESP32-C3](https://img.shields.io/badge/ESP32--C3-Super%20Mini-E7352C?logo=espressif&logoColor=white)
![API](https://img.shields.io/badge/API-v3-36d399)
![Local first](https://img.shields.io/badge/cloud-not%20required-70aaff)
![License](https://img.shields.io/badge/license-MIT-8b95a1)

**Без облака. Без CDN. Без обязательного сервера.**  
Открыл `hydro.local` — увидел состояние установки и управляешь ей напрямую.

</div>

![HydroESP-C3 — главный экран](docs/assets/ui-desktop-overview.png)

> [!NOTE]
> Скриншоты README генерируются из **того же embedded HTML, который компилируется в прошивку**, с mock API. Цифры на них демонстрационные; это не телеметрия конкретной установки.

## ✨ Что это

HydroESP-C3 — небольшой автономный контроллер полива. ESP32-C3 сам хранит расписание, включает насос, показывает состояние на OLED и отдаёт локальный адаптивный Web UI.

| ⏱ Автоматика | 👆 Вручную | 🛠 Обслуживание |
|---|---|---|
| До 48 циклов в сутки | Hold-to-start, stop одним нажатием | Плановые запуски ставятся на паузу без удаления графика |
| Работает локально | Каждый запуск ограничен таймером | Калибровка и обслуживание без неожиданного автополива |
| Не «догоняет» пропущенный цикл после паузы | Видно, откуда пришла команда | После restore автоматика остаётся paused до проверки |

### Что уже умеет

- 📅 суточное расписание и наглядная 24-часовая шкала;
- 💧 безопасное ручное управление насосом;
- ⏸ отдельный режим обслуживания;
- 🧪 серийная калибровка фактического расхода насоса;
- 📊 mean `Q`, число повторов и CV повторяемости;
- 🧠 инженерный расчёт длительности полива + диагностический VPD;
- 📱 отдельные desktop/mobile layouts;
- 🌗 тёмная, светлая и системная темы;
- 🧾 журнал последних действий текущей сессии;
- 🩺 диагностика RAM / flash / reset reason / version;
- 💾 backup/restore без Wi‑Fi password;
- ⬆️ OTA из браузера или через `hydroctl`;
- 🔐 защищённый commissioning Wi‑Fi с device key на OLED/Serial;
- 🧰 один CLI для install / doctor / pause / backup / update.

## 🖥 Интерфейс

<table>
<tr>
<td width="68%" valign="top">

### Desktop

Большой обзор состояния, управление насосом, расписание, журнал и диагностика — без перегруженных форм и браузерных `alert()`.

<img src="docs/assets/ui-desktop-overview.png" alt="HydroESP desktop overview">

</td>
<td width="32%" valign="top">

### Mobile

На телефоне sidebar превращается в нижнюю навигацию, элементы управления остаются крупными и пригодными для работы рядом с установкой.

<img src="docs/assets/ui-mobile-overview.png" alt="HydroESP mobile overview">

</td>
</tr>
</table>

### 🧪 Гидравлика без магии

<img src="docs/assets/ui-desktop-hydraulics.png" alt="HydroESP hydraulic calibration">

Калибровочный мастер предлагает сделать несколько коротких тестов через мерную ёмкость:

```text
Qi = Vml × 60 / tsec / 1000
Qmean = ΣQi / n
CV = sample_stddev(Qi) / Qmean × 100%
```

Сохраняются `Qmean`, число повторов, CV, время серии и версия протокола калибровки. CV показывает **повторяемость измерений**, а не «уверенность» в агрономической норме.

> [!IMPORTANT]
> Текущий автоматический режим — честный **таймерный контур**. VPD не превращается в псевдонаучный коэффициент полива. Настоящий adaptive mode требует реальных `T/RH + light + level + live flow + root-zone/drainage` сенсоров.

## 🚀 Быстрый старт

Нужны ESP32-C3 Super Mini, Python 3.9+, Git и USB data-кабель. PlatformIO вручную устанавливать не требуется.

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

После прошивки, если домашний Wi‑Fi ещё не настроен:

```text
SSID: HydroESP-Setup
KEY:  показан на OLED и в Serial
URL:  http://192.168.4.1
```

Задайте домашнюю сеть в разделе **Система**. После reboot устройство обычно доступно по:

```text
http://hydro.local
```

📖 Подробно: [docs/INSTALL.md](docs/INSTALL.md)

## 🔐 Безопасность без лишней сложности

Commissioning AP больше не открытый: устройство генерирует отдельный 10-символьный key и хранит его в отдельном NVS namespace. Ключ не является домашним Wi‑Fi password и не экспортируется обычным backup.

При этом проект не изображает «enterprise security», которой ещё нет:

- локальный HTTP пока без пользовательской web-auth;
- application OTA пока без device-side cryptographic signature check;
- сохранённый расход `Q` не заменяет live flow sensor;
- аппаратного dry-run interlock пока нет.

Для обычной домашней/лабораторной установки контроллер нужно держать в доверенной LAN без WAN port-forward.

📖 [Security model](docs/SECURITY.md) · [Commissioning key](docs/COMMISSIONING_SECURITY.md)

## 🧰 `hydroctl` — обслуживание одной командой

| Команда | Что делает |
|---|---|
| `python3 tools/hydroctl.py doctor` | проверяет toolchain, serial и контроллер |
| `python3 tools/hydroctl.py status` | показывает operational state |
| `python3 tools/hydroctl.py pause` | ставит плановую автоматику на паузу |
| `python3 tools/hydroctl.py resume` | возобновляет её |
| `python3 tools/hydroctl.py events` | показывает журнал текущей сессии |
| `python3 tools/hydroctl.py backup` | сохраняет schedule + automation + calibration |
| `python3 tools/hydroctl.py restore FILE` | fail-safe restore, оставляет automation paused |
| `python3 tools/hydroctl.py update` | обновляет из последнего GitHub Release |
| `python3 tools/hydroctl.py monitor` | открывает Serial monitor 115200 |

## 🔌 Подключение

| Компонент | GPIO | Назначение |
|---|---:|---|
| Relay IN | `4` | управление насосом |
| OLED SDA | `5` | I²C data |
| OLED SCL | `6` | I²C clock |
| LED | `8` | локальная индикация |
| BOOT | `9` | hold → manual start, tap при работе → stop |

> [!WARNING]
> Насос нельзя питать от слабой линии ESP32. Если плата перезагружается при пуске — исправляйте питание/ЭМС, а не расписание. Brown-out workaround в проекте пока оставлен только для совместимости со старой установкой.

## 🧭 Куда проект движется

- [x] responsive Web UI;
- [x] safe manual control;
- [x] timer automation + maintenance pause;
- [x] повторяемая гидравлическая калибровка;
- [x] backup / OTA / diagnostics;
- [x] protected commissioning AP;
- [ ] minimum-level interlock;
- [ ] live flow confirmation / `pump ON → no flow` alarm;
- [ ] вернуть штатный brown-out после исправления силовой части;
- [ ] простая авторизация опасных HTTP-действий;
- [ ] `T/RH + light + solution T` telemetry;
- [ ] adaptive irrigation только после появления реальной обратной связи.

Актуальный инженерный roadmap ведётся в [issue #2](https://github.com/f2re/hydro-esp-c3/issues/2).

## 🧱 Архитектура

```text
ESP32-C3
├── RelayController      pump timeout + source/reason
├── Scheduler            schedule + explicit pause
├── ConfigStorage        validated NVS
├── SecurityManager      commissioning credential
├── NTP / Wi‑Fi          local time + STA/AP
├── EventLog             short RAM session history
├── OLED / Serial        local recovery UI
└── AsyncWebServer
    ├── responsive embedded UI
    ├── API v3
    └── OTA
```

UI полностью self-contained: внешние JS/CSS/CDN для работы устройства не нужны.

## ✅ CI

Каждый PR проверяет:

- Python/shell maintenance tools;
- embedded HTML extraction;
- JavaScript через `node --check`;
- duplicate HTML `id`;
- отсутствие native `alert()` / `confirm()`;
- воспроизводимые desktop/mobile screenshots из embedded UI;
- PlatformIO build ESP32-C3;
- firmware + mock UI preview как CI artifact.

## 📚 Документация

- 🚀 [Установка](docs/INSTALL.md)
- ⬆️ [Обновление и recovery](docs/UPDATE.md)
- 🩺 [Диагностика](docs/TROUBLESHOOTING.md)
- 🔐 [Безопасность](docs/SECURITY.md)
- 📡 [HTTP API v3](docs/API.md)
- 🧱 [Архитектура](docs/ARCHITECTURE.md)
- 🧪 [Инженерный аудит и научная граница](AUDIT.md)
- 🤝 [CONTRIBUTING](CONTRIBUTING.md)

---

<div align="center">

**MIT License** · ESP32-C3 · local-first 🌱

</div>
