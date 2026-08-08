# Защищённый commissioning HydroESP-C3

Fallback-сеть `HydroESP-Setup` защищена отдельным device key.

Ключ:

- создаётся на устройстве через `esp_random()`;
- состоит из 10 символов без визуально неоднозначных знаков;
- хранится в отдельном NVS namespace `hydrosec`;
- переживает reboot и OTA;
- не совпадает с паролем домашнего Wi‑Fi;
- не возвращается HTTP API и не входит в обычный backup.

## Первый запуск

1. Прошейте ESP32-C3 по USB.
2. Если домашняя сеть не настроена, на OLED появятся `HydroESP-Setup`, `KEY` и `192.168.4.1`.
3. Подключитесь к `HydroESP-Setup` с показанным ключом.
4. Откройте `http://192.168.4.1` и задайте домашний Wi‑Fi.

Serial monitor остаётся физическим recovery-каналом:

```bash
python3 tools/hydroctl.py monitor
```

Он также показывает setup key при запуске commissioning AP.

## Граница защиты

Это защищает именно commissioning Wi‑Fi. Локальный HTTP API пока не имеет отдельной пользовательской авторизации, а application OTA пока не проверяет криптографическую подпись образа на устройстве. Поэтому контроллер следует держать в доверенной LAN без WAN port-forward.
