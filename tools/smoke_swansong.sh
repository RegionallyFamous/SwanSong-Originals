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

"$SWANSONG_ROOT/Scripts/build-engine.sh" >/dev/null
SWAN_ARES_ENGINE_DIR="$ENGINE_DIR" \
    "$SWANSONG_ROOT/Scripts/swift-package.sh" build \
    --package-path "$SWANSONG_ROOT" \
    --scratch-path "$SWIFT_DIR" \
    --product SwanSongRouteRunner >/dev/null
RUNNER="$SWIFT_DIR/debug/SwanSongRouteRunner"

for rom in "$@"; do
    stem=$(basename "${rom%.*}")
    slug=$(printf '%s' "$stem" | tr '_' '-')
    manifest="$ROOT/games/$slug/swan.toml"
    if [ ! -f "$manifest" ]; then
        echo "FAIL no SwanSong manifest for $rom" >&2
        exit 1
    fi
    scenarios="$TEMP_ROOT/$stem.scenarios"
    outcomes="$TEMP_ROOT/$stem.outcomes"
    : >"$outcomes"
    python3 - "$manifest" >"$scenarios" <<'PY'
import pathlib
import sys
import tomllib

manifest = pathlib.Path(sys.argv[1])
with manifest.open("rb") as handle:
    data = tomllib.load(handle)
for scenario in data["play"]["scenarios"]:
    expectation = scenario.get("audio_expectation")
    if expectation is None:
        expectation = "audible" if scenario.get("audio") is True else "any"
    print(f"{scenario['id']}|{scenario['plan']}|{expectation}")
PY
    while IFS='|' read -r scenario relative_plan audio_expectation; do
        plan="$ROOT/games/$slug/$relative_plan"
        report="$TEMP_ROOT/$stem-$scenario.json"
        capture="$TEMP_ROOT/$stem-$scenario.png"
        SWAN_ARES_ENGINE_DIR="$ENGINE_DIR" "$RUNNER" playtest-plan \
            --enable-debug-tools \
            --rom "$rom" \
            --plan "$plan" \
            --output "$report" \
            --capture "$capture"
        evidence=$(python3 - "$report" "$capture" "$plan" "$audio_expectation" <<'PY'
import json
import pathlib
import sys

report = json.loads(pathlib.Path(sys.argv[1]).read_text())
capture = pathlib.Path(sys.argv[2])
plan = json.loads(pathlib.Path(sys.argv[3]).read_text())
audio_expectation = sys.argv[4]
assert report["schema"] == "swan-song-playtest-report-v1"
assert report["engineBackend"] != "stub"
assert report["totalFrames"] == plan["totalFrames"]
assert report["finalFrameNumber"] >= plan["totalFrames"]
assert len(report["finalGameRasterSHA256"]) == 64
assert len(report["capturePNG_SHA256"]) == 64
assert report["persistencePolicy"] == "isolated-empty-v1"
assert report["rtcSeedUnixSeconds"] == 946684800
assert report["audio"]["sampleFrames"] > 0
assert capture.read_bytes().startswith(b"\x89PNG\r\n\x1a\n")
peak = report["audio"].get("finalWindowPeakAbsoluteSample", 0.0)
if audio_expectation == "audible":
    assert peak > 0.0001, (sys.argv[1], peak)
elif audio_expectation == "silent":
    assert peak <= 0.0001, (sys.argv[1], peak)
print(f"{report['finalGameRasterSHA256']}|{report['audio']['pcmFloatSHA256']}")
PY
        )
        case "$scenario" in
            success|failure|reset|deterministic)
                printf '%s|%s\n' "$scenario" "$evidence" >>"$outcomes"
                ;;
        esac
        printf 'OK   %s / %s completed in SwanSong\n' "$slug" "$scenario"
    done <"$scenarios"
    python3 - "$outcomes" <<'PY'
import pathlib
import sys

rows = [line.split("|") for line in pathlib.Path(sys.argv[1]).read_text().splitlines()]
assert [row[0] for row in rows] == ["success", "failure", "reset", "deterministic"], rows
assert len({row[1] for row in rows[:3]}) == 3, (
    "success, failure, and reset must end in visibly distinct states", rows
)
assert rows[0][1:] == rows[3][1:], (
    "success and deterministic replays must produce the same frame and audio", rows
)
PY
done
