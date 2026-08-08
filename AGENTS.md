# Repository Guidelines

## Project Structure & Module Organization
This repository contains PlatformIO firmware for an ESP32-C3 hydroponics controller. Source files live in `src/`; each feature is split into paired modules such as `wifi_manager.{h,cpp}`, `scheduler.{h,cpp}`, and `web_server.{h,cpp}`. Hardware and schedule defaults are centralized in `src/config.h`. Build configuration is in `platformio.ini`, and flash layout is defined in `partitions.csv`. GitHub templates for issues and pull requests live under `.github/`.

Web identity/static assets live in `src/web_assets.h`; the documentation copy of the canonical SVG is `docs/assets/hydroesp-favicon.svg`. Keep those two SVG representations byte-equivalent through `tools/check_web_assets.py`.

## Build, Test, and Development Commands
Prefer the repository wrappers so developers and CI share the same behavior:

- `bash install.sh` / `.\install.ps1` performs first USB installation and provisions Wi-Fi when requested.
- `./deploy.sh` / `.\deploy.ps1` performs a normal repeat USB deployment using persistent PlatformIO caches.
- `./deploy.sh --pull` / `.\deploy.ps1 -Pull` fast-forwards the current branch, then deploys; it refuses a dirty working tree.
- `./wifi-flash.sh <IP>` / `.\wifi-flash.ps1 <IP>` performs verified Wi-Fi update/recovery with progress; do not replace it with a Web UI-only flow.
- `./wifi-flash.sh <IP> --transport arduino` explicitly uses the independent ArduinoOTA/espota channel on port 3232.
- `python3 tools/hydroctl.py build` builds the firmware for `esp32c3_supermini`.
- `python3 tools/check_web_assets.py` verifies favicon format, synchronization, routes and flash budget.
- `python3 tools/check_recovery_contract.py`, `check_display_contract.py`, and `check_build_cache_contract.py` protect OTA/OLED/build behavior from regression.
- `pio device monitor` opens the serial monitor at `115200`.
- `pio run -t clean` removes project build artifacts when genuinely required; do not routinely delete `~/.platformio` or the shared build cache.

PlatformIO platforms/toolchains are intentionally persistent under `~/.platformio`, and `platformio.ini` uses `~/.platformio/build-cache` for compiled objects. CI must restore the same PlatformIO cache directories instead of downloading the ESP32 toolchain on every run.

## Build Identity & Cache Rules
Commit/version identity changes on nearly every source update and must **not** be injected as global `BUILD_FLAGS`. Global flag changes invalidate Arduino/framework objects and turn incremental builds back into full rebuilds.

- `scripts/build_flags.py` writes volatile identity to ignored `src/generated_build_info.h`;
- only `src/version.cpp` consumes that generated header;
- actual configuration flags such as explicit provisioning credentials/timezone may remain compile flags because they intentionally alter firmware configuration;
- CI changes build identity and verifies that no `FrameworkArduino/*` object is recompiled.

Do not move `HYDRO_VERSION`/`HYDRO_BUILD_SHA` back into global `-D` flags.

## Coding Style & Naming Conventions
Follow the existing C++ style in `src/`: 4-space indentation, opening braces on the same line, and include headers in a stable, grouped order. Use `PascalCase` for classes (`RelayController`), `camelCase` for functions and variables (`printBootStep`, `lastSerial`), and `UPPER_SNAKE_CASE` for macros and pin constants (`RELAY_PIN`, `WIFI_TIMEOUT_MS`). Prefer small feature-focused modules and non-blocking logic based on `millis()`; avoid introducing new long `delay()` calls.

## Web UI & Static Asset Rules
The ESP32-C3 has limited contiguous heap. Treat Web resources as firmware data, not as desktop-style files:

- large `PROGMEM` HTML/assets must be sent through explicit-length `const uint8_t* + size` responses (`AsyncProgmemResponse` path);
- never reintroduce `request->send(..., WEB_UI_HTML)` using the `const char*` overload, because it copies the full page into Arduino `String` and can return an empty `200 OK` on allocation failure;
- favicon/static identity assets stay in flash and must not require NVS, LittleFS/SPIFFS, Base64 blobs or permanent heap buffers;
- keep the combined favicon SVG+ICO payload under the CI budget (currently 4096 bytes) unless there is a measured product requirement to raise it;
- do not add large 180/192/512 PNGs for speculative PWA support; measure firmware size and document the need first;
- keep `/favicon.ico` as a real ICO route because browsers probe it automatically, and keep `/favicon.svg` as the scalable identity asset.

## OTA Reliability Contract
OTA success is a two-stage contract: **transfer acceptance** and **post-reboot verification**.

- normal `wifi-flash` uses HTTP when `/api/status` is alive and streams firmware in chunks with visible progress;
- ArduinoOTA/espota on port `3232` remains an independent explicit/recovery transport and must not be removed;
- the HTTP upload handler must send final `200 OK` before reset and schedule reboot outside the AsyncWebServer callback; never restore `send(); delay(); ESP.restart();` in that callback;
- 100% on OLED means `Update.end(true)` succeeded, not merely that enough multipart bytes arrived;
- after OTA starts, the relay is OFF and the ordinary scheduler/button/OLED/status loop must not run over the flashing context;
- `wifi-flash` returns success only after the controller comes back and `/api/status` confirms the reboot/build when the expected build is known.

## OLED UX Contract
The visible logical OLED area is 72×40. Keep it as a concise local safety/diagnostic UI, not a miniature Web dashboard.

- human-facing states are Russian using U8g2 UTF-8 Cyrillic rendering;
- technical identifiers `IP`, `Wi-Fi`, `NTP`, SSID and `hydro.local` remain literal;
- explicit priority states include `ПАУЗА`, `НЕТ NTP`, `НЕТ ПЛАНА`, active `ПОЛИВ`, `ПРОШИВКА`, `ГОТОВО` and `ОШИБКА`;
- a paused scheduler must not display the next slot as if it were an active promise;
- OTA display redraws must be throttled; do not push a full I2C framebuffer for every network chunk.

See `docs/OLED.md`.

## Testing Guidelines
There is no standalone `test/` directory yet, so every change must at minimum compile with `pio run`. For hardware-facing changes, verify upload plus runtime behavior with `pio device monitor` and include relevant logs in the PR. Web changes must pass `tools/check_web_ui.py` and `tools/check_web_assets.py`. OTA/OLED/build changes must pass their dedicated contract checks and the CI incremental-build gate. When adding tests later, place them in a new `test/` directory and name files after the module under test, for example `test_scheduler.cpp`.

## Commit & Pull Request Guidelines
Current history uses short, imperative commit subjects (`init`, `fix`, `continue`). Keep subjects concise and action-oriented, and add detail in the body when behavior changes. Follow `.github/PULL_REQUEST_TEMPLATE.md`: describe what changed, list exact verification steps, and link related issues. Include serial logs or UI screenshots when modifying the web UI, OLED output, or boot flow.

## Configuration & Safety Notes
Do not hardcode real credentials in source files; prefer shell environment variables consumed by `platformio.ini`. Changes touching `RELAY_PIN`, timing, OTA, or scheduling should call out hardware risk explicitly in the PR.

Repeat deploy and Wi-Fi recovery are safety contracts: normal `deploy` and `wifi-flash` must not inject `WIFI_SSID`, `WIFI_PASSWORD` or a new `WIFI_SEED_ID`. Only explicit install/provisioning may change stored network credentials.
