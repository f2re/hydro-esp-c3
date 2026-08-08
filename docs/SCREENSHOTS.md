# UI screenshots

README использует воспроизводимые снимки встроенного `src/web_ui_v2.h`. CI генерирует их из реального embedded HTML с демонстрационным mock API, поэтому вёрстка соответствует прошивке, а значения на экране не являются телеметрией конкретной установки.

Снимки:

- desktop overview;
- desktop hydraulics;
- mobile overview.

Исходный preview создаётся `tools/export_ui_preview.py`; workflow `.github/workflows/build.yml` выполняет headless-render при каждом PR.
