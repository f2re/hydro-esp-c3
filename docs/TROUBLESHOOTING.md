# Диагностика HydroESP-C3

Начинайте с:

```bash
python3 tools/hydroctl.py doctor
```

Для текущего API v3 команда показывает toolchain/serial, firmware/build, NTP, режим автоматики, гидравлическую калибровку и состояние насоса.

Журнал текущей сессии:

```bash
python3 tools/hydroctl.py events
```

## Контроллер не появляется после прошивки

1. откройте Serial monitor:

```bash
python3 tools/hydroctl.py monitor
```

2. найдите строку `HydroESP` и этап boot;
3. без рабочего домашнего Wi‑Fi устройство должно создать `HydroESP-Setup`;
4. подключитесь и откройте `http://192.168.4.1`.

Если AP нет — повторите USB flash и проверьте питание.

## `hydro.local` не открывается

Проверьте одну LAN/VLAN, отсутствие client isolation, IP устройства и прямой доступ по IP:

```bash
python3 tools/hydroctl.py status --host http://192.168.1.50
```

## В интерфейсе «Нет связи», но Wi‑Fi есть

```bash
curl http://hydro.local/ping
curl http://hydro.local/api/status
```

или:

```bash
python3 tools/hydroctl.py status
```

При периодических обрывах проверяйте RSSI, питание и reboot reason.

## Расписание не запускается

Для plan-start должны одновременно выполняться два условия:

1. `time_synced = true`;
2. `automation_enabled = true`.

Проверьте:

```bash
python3 tools/hydroctl.py status
```

Если автоматика на паузе:

```bash
python3 tools/hydroctl.py resume
```

Если NTP отсутствует, текущая версия не имеет RTC holdover и не может гарантировать абсолютное время после reboot.

Пауза не удаляет график. Пропущенный слот не запускается задним числом после resume.

## Как безопасно обслуживать установку

Перед работой с трубками, насосом или мерной ёмкостью:

```bash
python3 tools/hydroctl.py pause
```

Убедитесь в status, что `automation_enabled=false`.

После обслуживания и проверки:

```bash
python3 tools/hydroctl.py resume
```

Не используйте удаление расписания как способ «выключить автоматику».

## Насос не включается вручную

Web и физическая BOOT требуют удержания для start. Active pump останавливается одним действием.

Проверьте GPIO4, GND/логику реле, питание насоса, силовой ключ и проводку.

В `/api/status` поле `pump_source` показывает причину текущего start:

- `schedule`;
- `web_manual`;
- `button_manual`;
- `calibration`.

## Калибровочный тест не запускается

Endpoint намеренно возвращает конфликт, если:

- automation active;
- pump уже работает.

Правильный порядок:

1. **Гидравлика → Поставить автоматику на паузу**;
2. убедиться, что pump OFF;
3. подготовить мерную ёмкость;
4. выбрать 15/30/60 s;
5. запустить test.

Через API duration разрешён только 5–120 s.

Если UI показывает ошибку `automation_must_be_paused`, pausing не был применён либо страница показывает устаревшее состояние — обновите status.

## Получился странный расход после калибровки

Расход считается по фактически собранному объёму:

```text
Q [л/мин] = V [мл] × 60 / t [с] / 1000
```

Проверьте:

- весь ли поток попал в мерную ёмкость;
- не было ли воздуха/неполного заполнения магистрали в начале теста;
- не перепутаны ли мл и л;
- соответствует ли введённый объём выбранной длительности;
- повторяется ли результат 2–3 раза.

Для коротких тестов относительная ошибка измерения выше. Если система стабильна, 30–60 s обычно дают более воспроизводимый результат, чем 5–15 s.

Если распределение по ярусам неодинаковое, один общий Q описывает только суммарную подачу — нужен отдельный тест равномерности.

## Гидравлика откалибрована, но расчёт всё равно неверен

Калибровка определяет Q, а не потребность растения. Проверьте также `delivery_efficiency_pct`, число растений и целевую подачу на цикл.

Не пытайтесь «исправлять» неизвестное водопотребление коэффициентом VPD: без RAD/PPFD, root-zone/drainage и sensor feedback это ложная точность.

## ESP перезагружается при старте насоса

Это питание/ЭМС, а не schedule bug. Проверьте отдельное питание насоса, землю, драйвер/реле, suppression индуктивных выбросов, ёмкости, просадку напряжения, кабели и помехи.

Для совместимости пока сохранён `HYDRO_DISABLE_BROWNOUT_WORKAROUND=1`. После аппаратного исправления установите 0 и повторите stress-test.

## Лишний полив после manual или resume

Scheduler потребляет текущую минуту даже если pump busy или automation mode переключается. Поэтому «догоняющего» slot в этой же минуте быть не должно.

Если он возник:

```bash
python3 tools/hydroctl.py events
python3 tools/hydroctl.py backup
```

Сохраните version/build, точное время и Serial log.

## Как читать журнал событий

```bash
python3 tools/hydroctl.py events
```

Журнал показывает start/stop, источник, stop reason, pause/resume, schedule/hydraulics changes, OTA/reboot.

Он хранится только в RAM и очищается при reboot. Это сделано намеренно, чтобы частые pump events не записывались в NVS.

## Расписание или калибровка повреждены

Сделайте backup до изменений:

```bash
python3 tools/hydroctl.py backup
```

Restore:

```bash
python3 tools/hydroctl.py restore hydroesp-backup.json
```

После restore автоматика **остаётся на паузе**. Проверьте status/график/калибровку, затем:

```bash
python3 tools/hydroctl.py resume
```

Автоматический resume возможен только явной опцией `--resume-automation`.

## OTA выдаёт ошибку

Проверьте `.bin`, размер >10 KB, стабильное питание, сеть и `partitions.csv`.

USB recovery:

```bash
python3 tools/hydroctl.py install --clean
```

## Диагностический пакет для issue

Приложите:

- модель платы;
- схему питания pump/relay;
- version/build/API;
- `doctor`;
- `events`, если reboot ещё не произошёл;
- последние строки Serial;
- backup JSON без секретов;
- browser/device;
- точные шаги воспроизведения.

Не публикуйте Wi‑Fi password; приватный SSID при необходимости маскируйте.
