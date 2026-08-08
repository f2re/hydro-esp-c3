#!/usr/bin/env python3
"""Keep volatile build identity from invalidating the whole Arduino toolchain cache."""

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def read(path: str) -> str:
    return (ROOT / path).read_text(encoding="utf-8")


def require(path: str, needle: str) -> None:
    if needle not in read(path):
        raise SystemExit(f"build-cache contract failed: {path} missing {needle!r}")


def forbid(path: str, needle: str) -> None:
    if needle in read(path):
        raise SystemExit(f"build-cache contract failed: {path} contains forbidden {needle!r}")


require("platformio.ini", "build_cache_dir = ~/.platformio/build-cache")
require("scripts/build_flags.py", "generated_build_info.h")
require("scripts/build_flags.py", "write_build_info(version, sha)")
require("scripts/build_flags.py", "env.Append(BUILD_FLAGS=flags)")
forbid("scripts/build_flags.py", 'flags.append(f"-DHYDRO_VERSION=')
forbid("scripts/build_flags.py", 'flags.append(f"-DHYDRO_BUILD_SHA=')

require("src/version.h", "extern const char HYDRO_VERSION[]")
require("src/version.h", "extern const char HYDRO_BUILD_SHA[]")
require("src/version.cpp", '#include "generated_build_info.h"')
require("src/version.cpp", "HYDRO_GENERATED_VERSION")
require("src/version.cpp", "HYDRO_GENERATED_BUILD_SHA")
require(".gitignore", "src/generated_build_info.h")

print("build-cache contract: OK (commit identity isolated to version.cpp)")
