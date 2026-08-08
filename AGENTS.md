# Repository Guidelines

## Project Structure & Module Organization
This repository contains PlatformIO firmware for an ESP32-C3 hydroponics controller. Source files live in `src/`; each feature is split into paired modules such as `wifi_manager.{h,cpp}`, `scheduler.{h,cpp}`, and `web_server.{h,cpp}`. Hardware and schedule defaults are centralized in `src/config.h`. Build configuration is in `platformio.ini`, and flash layout is defined in `partitions.csv`. GitHub templates for issues and pull requests live under `.github/`.

## Build, Test, and Development Commands
Prefer the repository wrappers so developers and CI share the same behavior:

- `bash install.sh` / `.\install.ps1` performs first USB installation and provisions Wi-Fi when requested.
- `./deploy.sh` / `.\deploy.ps1` performs a normal repeat USB deployment using persistent PlatformIO caches.
- `./deploy.sh --pull` / `.\deploy.ps1 -Pull` fast-forwards the current branch, then deploys; it refuses a dirty working tree.
- `python3 tools/hydroctl.py build` builds the firmware for `esp32c3_supermini`.
- `pio device monitor` opens the serial monitor at `115200`.
- `pio run -t clean` removes project build artifacts when genuinely required; do not routinely delete `~/.platformio` or the shared build cache.

PlatformIO platforms/toolchains are intentionally persistent under `~/.platformio`, and `platformio.ini` uses `~/.platformio/build-cache` for compiled objects. CI must restore the same PlatformIO cache directories instead of downloading the ESP32 toolchain on every run.

## Coding Style & Naming Conventions
Follow the existing C++ style in `src/`: 4-space indentation, opening braces on the same line, and include headers in a stable, grouped order. Use `PascalCase` for classes (`RelayController`), `camelCase` for functions and variables (`printBootStep`, `lastSerial`), and `UPPER_SNAKE_CASE` for macros and pin constants (`RELAY_PIN`, `WIFI_TIMEOUT_MS`). Prefer small feature-focused modules and non-blocking logic based on `millis()`; avoid introducing new long `delay()` calls.

## Testing Guidelines
There is no standalone `test/` directory yet, so every change must at minimum compile with `pio run`. For hardware-facing changes, verify upload plus runtime behavior with `pio device monitor` and include relevant logs in the PR. When adding tests later, place them in a new `test/` directory and name files after the module under test, for example `test_scheduler.cpp`.

## Commit & Pull Request Guidelines
Current history uses short, imperative commit subjects (`init`, `fix`, `continue`). Keep subjects concise and action-oriented, and add detail in the body when behavior changes. Follow `.github/PULL_REQUEST_TEMPLATE.md`: describe what changed, list exact verification steps, and link related issues. Include serial logs or UI screenshots when modifying the web UI, OLED output, or boot flow.

## Configuration & Safety Notes
Do not hardcode real credentials in source files; prefer shell environment variables consumed by `platformio.ini`. Changes touching `RELAY_PIN`, timing, OTA, or scheduling should call out hardware risk explicitly in the PR.
