#!/usr/bin/env sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
PYTHON=${PYTHON:-python3}

if [ "${1:-}" = "--pull" ]; then
    if [ -n "$(git -C "$ROOT" status --porcelain)" ]; then
        echo "[deploy] ERROR: local changes detected; commit/stash them before --pull" >&2
        exit 2
    fi
    echo "[deploy] Updating current branch (fast-forward only)"
    git -C "$ROOT" pull --ff-only
    shift
fi

echo "[deploy] Repeat deployment: cached toolchain, stored Wi-Fi/NVS preserved"
exec "$PYTHON" "$ROOT/tools/usb_deploy.py" "$@"
