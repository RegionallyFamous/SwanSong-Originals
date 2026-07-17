#!/bin/sh
set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
PLUGIN_DIR=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)
REPO_DIR=$(CDPATH= cd -- "$PLUGIN_DIR/../.." && pwd)

if [ -n "${SWANSONG_DESKTOP_DIR:-}" ]; then
    SWANSONG_ROOT=$SWANSONG_DESKTOP_DIR
elif [ -x "$REPO_DIR/../GitHub/SwanSong-Desktop/Scripts/run-swansong-playtest-mcp.sh" ]; then
    SWANSONG_ROOT=$(CDPATH= cd -- "$REPO_DIR/../GitHub/SwanSong-Desktop" && pwd)
elif [ -n "${HOME:-}" ] \
    && [ -x "$HOME/Documents/GitHub/SwanSong-Desktop/Scripts/run-swansong-playtest-mcp.sh" ]; then
    SWANSONG_ROOT=$(CDPATH= cd -- "$HOME/Documents/GitHub/SwanSong-Desktop" && pwd)
else
    SWANSONG_ROOT=""
fi

RUNNER=${SWANSONG_ROOT:+$SWANSONG_ROOT/Scripts/run-swansong-playtest-mcp.sh}
if [ -z "$RUNNER" ] || [ ! -x "$RUNNER" ]; then
    echo "SwanSong Desktop was not found. Set SWANSONG_DESKTOP_DIR to its checkout." >&2
    exit 2
fi

exec "$RUNNER"
