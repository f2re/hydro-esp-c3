# Защищённый commissioning HydroESP-C3

## Что изменилось

Fallback-точка `HydroESP-Setup` больше не должна быть открытой. При первом запуске SecurityManager создаёт случайный 10-символьный operator key из алфавита без визуально неоднозначных символов.

Ключ:

- создаётся из аппаратного `esp_random()`;
- хранится в отдельном NVS namespace `hydrosec`;
- используется как WPA2 password provisioning AP;
- переживает обычные OTA/update и изменения расписания;
- не входит в `/api/config`;
- не входит в `hydroctl backup`;
- выводится в Serial при запуске provisioning AP как физический recovery channel.

Пример Serial:

```text
[WiFi] Starting protected AP: HydroESP-Setup
[WiFi] AP IP: 192.168.4.1
[SEC] Provisioning/operator key: ABCD234EFG
```

## Почему ключ отдельный

Wi‑Fi password домашней сети и operator/commissioning credential решают разные задачи. Нельзя использовать домашний Wi‑Fi password как ключ управления устройством и нельзя возвращать его браузеру для повторного использования.

Отдельный namespace также позволяет в следующем security-этапе добавить rotation/reset policy без миграции основного `Config`.

## Энтропия

Текущий формат: 10 символов из 32-символьного алфавита, то есть около 50 бит пространства ключей. Это существенно лучше открытой точки доступа и достаточно для локального commissioning credential при защите WPA2.

Это **не заменяет**:

- TLS;
- web/API authentication;
- signed OTA;
- Secure Boot;
- физическую защиту устройства.

## Первый запуск

1. прошить устройство по USB;
2. открыть Serial monitor:

```bash
python3 tools/hydroctl.py monitor
```

3. найти строку `Provisioning/operator key`;
4. подключиться к `HydroESP-Setup` с этим ключом;
5. открыть `http://192.168.4.1`;
6. задать домашний Wi‑Fi;
7. сохранить operator key в защищённом password manager для будущего recovery.

## Если домашний Wi‑Fi позже недоступен

Контроллер снова может перейти в provisioning AP. Используется тот же сохранённый operator key; новый ключ при каждом reboot не генерируется.

Если ключ утрачен, физический Serial остаётся recovery channel. Автоматический удалённый «показать секрет» endpoint намеренно не предусмотрен.

## Trust boundary

Защищённый AP закрывает только риск случайного/неавторизованного подключения к commissioning network. После подключения текущий HTTP API всё ещё работает без TLS и полноценной авторизации опасных операций.

Поэтому устройство по-прежнему должно эксплуатироваться только в доверенной LAN без WAN port-forward. Следующий security-этап — operator authentication для state-changing API, затем signed OTA/rollback policy.
