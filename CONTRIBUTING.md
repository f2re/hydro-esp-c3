# Разработка HydroESP-C3

Изменения в этом проекте затрагивают одновременно **воду, насос, питание, сеть и embedded UI**. Поэтому критерий качества — не «код компилируется», а предсказуемое и объяснимое поведение установки.

## Перед началом

```bash
git clone https://github.com/f2re/hydro-esp-c3.git
cd hydro-esp-c3
python3 tools/hydroctl.py bootstrap
```

Сборка:

```bash
python3 tools/hydroctl.py build
```

Полный локальный минимум перед PR:

```bash
python3 -m py_compile tools/hydroctl.py tools/check_web_ui.py scripts/build_flags.py
python3 tools/check_web_ui.py
pio run
```

## Обязательные инженерные правила

### 1. Насос должен fail-safe выключаться

- boot/reboot/update не должны оставлять насос включённым;
- любой программный запуск должен иметь конечный timeout;
- новые пути управления обязаны использовать `RelayController`, а не писать GPIO напрямую;
- dangerous action в UI не должен выполняться случайным tap.

### 2. Основной `loop()` не блокировать

Не добавляйте долгие `delay()`, циклы ожидания сети/датчика или синхронные операции в регулярный `loop()`. Насосные таймеры, Scheduler и локальная индикация должны продолжать обслуживаться при проблемах сети.

Короткие задержки допустимы только в контролируемых boot/reboot переходах, где состояние насоса уже безопасно.

### 3. Один источник лимитов

Используйте константы из `src/config.h`:

- `MAX_SCHEDULE_SLOTS`;
- `MAX_WATERING_SECONDS`;
- `DEFAULT_MANUAL_SECONDS`;
- Wi‑Fi/NTP интервалы.

Не размножайте `48`, `3600` и аналогичные ограничения по исходникам.

### 4. Входные данные проверяются на ESP

Frontend validation улучшает UX, но не является границей доверия. Любой HTTP/API input должен валидироваться сервером до изменения NVS или hardware state.

### 5. Не выдавать эвристику за научную модель

Если формула не опирается на измеряемые величины/калибровку, её нельзя подписывать как «научный расчёт» или использовать как скрытую автоматическую коррекцию полива.

Изменения adaptive irrigation должны описывать:

- измерения;
- единицы;
- калибровку;
- ограничения применимости;
- fallback;
- safety interlocks.

### 6. Web UI остаётся self-contained

Встроенный интерфейс не должен зависеть от CDN, облачных шрифтов, внешних JavaScript/CSS ресурсов.

Не использовать:

- `alert()`;
- `confirm()`;
- скрытые автосохранения опасных настроек;
- цвет как единственный индикатор состояния;
- мелкие touch targets для критичных действий.

Учитывать:

- desktop + mobile;
- safe area;
- keyboard/focus;
- `prefers-reduced-motion`;
- online/offline/AP/NTP states;
- loading/error/empty/dirty states.

`tools/check_web_ui.py` проверяет базовые регрессии embedded UI.

### 7. Секреты не коммитить

Обычная сборка не требует Wi‑Fi credentials. `.env` игнорируется Git.

Нельзя добавлять в репозиторий:

- реальные пароли;
- production SSID, если он чувствителен;
- токены;
- приватные сетевые адреса/ключи.

`.env.example` содержит только пример необязательных factory defaults.

## Стиль C++

- понятные имена;
- фиксированные типы для persisted/wire данных;
- rollover-safe сравнения `millis()`;
- явные safety comments там, где решение неочевидно;
- минимум динамической памяти в горячих embedded путях;
- отсутствие «магических» hardware constants вне `config.h`.

## HTTP API

Контракт описан в [docs/API.md](docs/API.md).

При breaking change:

1. увеличить `HYDRO_API_VERSION`;
2. обновить web UI;
3. обновить `hydroctl`;
4. обновить API docs;
5. описать миграцию/совместимость.

Пароль Wi‑Fi нельзя возвращать ни в одном read endpoint или backup.

## Документация как часть изменения

Изменение считается незавершённым, если код и инструкции расходятся.

Проверьте необходимость обновить:

- `README.md`;
- `AUDIT.md`;
- `docs/INSTALL.md`;
- `docs/UPDATE.md`;
- `docs/TROUBLESHOOTING.md`;
- `docs/ARCHITECTURE.md`;
- `docs/API.md`;
- `docs/SECURITY.md`.

## Pull Request

PR должен отвечать на четыре вопроса:

1. Что изменилось для пользователя/установки?
2. Какие риски затронуты?
3. Как это проверено?
4. Что намеренно осталось за рамками?

Для hardware-sensitive изменений приложите сценарий стендовой проверки.

## Release

Release создаётся tag workflow’ом `v*`. Тег ставится только на проверенный `main` после зелёного CI.

Не публикуйте вручную неизвестный `.bin` как release asset — build metadata и checksum должны формироваться CI.
