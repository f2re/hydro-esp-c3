# 🔐 Защищённый commissioning HydroESP-C3

Fallback-точка `HydroESP-Setup` больше не открытая. Устройство создаёт отдельный 10-символьный commissioning key и использует его как пароль точки настройки.

## Что видит пользователь

Когда домашний Wi‑Fi недоступен, OLED остаётся на простом recovery-экране:

```text
SETUP WIFI
HydroESP-Setup
ABCD234EFG
192.168.4.1
```

То есть для обычной первой настройки **Serial не нужен**: SSID, key и IP находятся прямо на устройстве.

Serial дублирует эти данные как физический recovery channel:

```text
[SEC] Setup Wi-Fi: HydroESP-Setup
[SEC] Setup key: ABCD234EFG
[SEC] Setup URL: http://192.168.4.1
```

## Как устроен key

- создаётся из аппаратного `esp_random()`;
- 10 символов из алфавита без `0/O`, `1/I` и других неоднозначных символов;
- около 50 бит пространства вариантов;
- хранится в отдельном NVS namespace `hydrosec`;
- переживает reboot и OTA;
- не является паролем домашнего Wi‑Fi;
- не входит в `/api/config`;
- не входит в обычный `hydroctl backup`.

## Первый запуск

1. прошить ESP32-C3;
2. прочитать key на OLED;
3. подключиться к `HydroESP-Setup`;
4. открыть `http://192.168.4.1`;
5. задать домашний Wi‑Fi;
6. сохранить настройки и дождаться reboot;
7. открыть `http://hydro.local`.

## Если домашний Wi‑Fi позже пропал

Контроллер снова использует тот же сохранённый key. Новый secret при каждом reboot не генерируется.

Если OLED повреждён или недоступен:

```bash
python3 tools/hydroctl.py monitor
```

Автоматического HTTP endpoint «покажи секрет» намеренно нет.

## Что это НЕ решает

Защищённый AP закрывает только commissioning network. Это **не** TLS, web-auth, signed OTA или аппаратная защита насоса.

Следующие реальные приоритеты проекта:

1. level + live flow interlock;
2. исправление питания и возврат brown-out protection;
3. простая авторизация опасных HTTP-действий;
4. signed OTA/rollback, если это потребуется сценарию эксплуатации.
