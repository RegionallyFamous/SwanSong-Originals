#!/bin/sh
set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)

if [ -n "${SWANSONG_DESKTOP_DIR:-}" ]; then
    SWANSONG_ROOT=$SWANSONG_DESKTOP_DIR
elif [ -x "$ROOT/../GitHub/SwanSong-Desktop/Scripts/build-engine.sh" ]; then
    SWANSONG_ROOT=$(CDPATH= cd -- "$ROOT/../GitHub/SwanSong-Desktop" && pwd)
elif [ -n "${HOME:-}" ] \
    && [ -x "$HOME/Documents/GitHub/SwanSong-Desktop/Scripts/build-engine.sh" ]; then
    SWANSONG_ROOT=$(CDPATH= cd -- "$HOME/Documents/GitHub/SwanSong-Desktop" && pwd)
else
    SWANSONG_ROOT=""
fi
if [ -z "$SWANSONG_ROOT" ] || [ ! -x "$SWANSONG_ROOT/Scripts/build-engine.sh" ]; then
    echo "FAIL SwanSong Desktop was not found. Set SWANSONG_DESKTOP_DIR to its checkout." >&2
    exit 2
fi

ENGINE_DIR=${SWAN_ARES_ENGINE_DIR:-$SWANSONG_ROOT/.engine/build}
SWIFT_DIR=${SWAN_PLAYTEST_SWIFT_DIR:-$SWANSONG_ROOT/.build/playtest-swift}
TEMP_ROOT=$(mktemp -d "${TMPDIR:-/tmp}/swansong-originals-smoke.XXXXXX")
trap 'rm -rf "$TEMP_ROOT"' EXIT INT TERM

printf '%s\n' \
    '{' \
    '  "schema": "swan-song-frame-input-plan-v1",' \
    '  "totalFrames": 120,' \
    '  "events": [{"frameIndex": 0, "inputs": []}]' \
    '}' >"$TEMP_ROOT/boot-plan.json"

"$SWANSONG_ROOT/Scripts/build-engine.sh" >/dev/null
SWAN_ARES_ENGINE_DIR="$ENGINE_DIR" \
    "$SWANSONG_ROOT/Scripts/swift-package.sh" build \
    --package-path "$SWANSONG_ROOT" \
    --scratch-path "$SWIFT_DIR" \
    --product SwanSongRouteRunner >/dev/null
RUNNER="$SWIFT_DIR/debug/SwanSongRouteRunner"

for rom in "$@"; do
    name=$(basename "$rom")
    report="$TEMP_ROOT/$name.json"
    capture="$TEMP_ROOT/$name.png"
    SWAN_ARES_ENGINE_DIR="$ENGINE_DIR" "$RUNNER" playtest-plan \
        --enable-debug-tools \
        --rom "$rom" \
        --plan "$TEMP_ROOT/boot-plan.json" \
        --output "$report" \
        --capture "$capture"
    python3 - "$report" "$capture" <<'PY'
import json
import pathlib
import sys

report = json.loads(pathlib.Path(sys.argv[1]).read_text())
capture = pathlib.Path(sys.argv[2])
assert report["schema"] == "swan-song-playtest-report-v1"
assert report["engineBackend"] != "stub"
assert report["totalFrames"] == 120
assert report["finalFrameNumber"] >= 120
assert len(report["finalGameRasterSHA256"]) == 64
assert len(report["capturePNG_SHA256"]) == 64
assert report["persistencePolicy"] == "isolated-empty-v1"
assert report["rtcSeedUnixSeconds"] == 946684800
assert report["audio"]["sampleFrames"] > 0
assert capture.read_bytes().startswith(b"\x89PNG\r\n\x1a\n")
PY
    printf 'OK   %s reached a rendered SwanSong frame\n' "$rom"
done
