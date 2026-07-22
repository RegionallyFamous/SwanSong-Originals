#!/bin/sh
set -eu

if [ "$#" -lt 2 ]; then
    printf 'usage: %s OUTPUT_DIR ROM...\n' "$0" >&2
    exit 2
fi

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)
OUTPUT_DIR=$1
shift

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
    echo "SwanSong Desktop was not found. Set SWANSONG_DESKTOP_DIR to its checkout." >&2
    exit 2
fi

ENGINE_DIR=${SWAN_ARES_ENGINE_DIR:-$SWANSONG_ROOT/.engine/build}
SWIFT_DIR=${SWAN_PLAYTEST_SWIFT_DIR:-$SWANSONG_ROOT/.build/playtest-swift}
TEMP_ROOT=$(mktemp -d "${TMPDIR:-/tmp}/swansong-originals-capture.XXXXXX")
trap 'rm -rf "$TEMP_ROOT"' EXIT INT TERM
mkdir -p "$OUTPUT_DIR"

"$SWANSONG_ROOT/Scripts/build-engine.sh" >/dev/null
SWAN_ARES_ENGINE_DIR="$ENGINE_DIR" \
    "$SWANSONG_ROOT/Scripts/swift-package.sh" build \
    --package-path "$SWANSONG_ROOT" \
    --scratch-path "$SWIFT_DIR" \
    --product SwanSongRouteRunner >/dev/null
RUNNER="$SWIFT_DIR/debug/SwanSongRouteRunner"

for rom in "$@"; do
    name=$(basename "${rom%.*}")
    case "$name" in
        bug_witch|harpoon_moon|one_last_lap|orbital_courier|radio_ghost)
            capture_frames=90
            ;;
        *)
            capture_frames=40
            ;;
    esac
    plan="$TEMP_ROOT/$name-boot-plan.json"
    capture="$TEMP_ROOT/$name.png"
    printf '%s\n' \
        '{' \
        '  "schema": "swan-song-frame-input-plan-v1",' \
        "  \"totalFrames\": $capture_frames," \
        '  "events": [{"frameIndex": 0, "inputs": []}]' \
        '}' >"$plan"
    SWAN_ARES_ENGINE_DIR="$ENGINE_DIR" "$RUNNER" playtest-plan \
        --enable-debug-tools \
        --rom "$rom" \
        --plan "$plan" \
        --output "$TEMP_ROOT/$name.json" \
        --capture "$capture"
    test -s "$capture"
    mv "$capture" "$OUTPUT_DIR/$name.png"
    printf 'CAP  %s\n' "$OUTPUT_DIR/$name.png"
done

if command -v magick >/dev/null 2>&1; then
    find "$OUTPUT_DIR/contact-sheet.png" -delete 2>/dev/null || true
    row_dir=$(mktemp -d "${TMPDIR:-/tmp}/swansong-capture-rows.XXXXXX")
    row=0
    first=""
    for file in "$OUTPUT_DIR"/*.png; do
        if [ -z "$first" ]; then
            first=$file
        else
            magick "$first" "$file" +append -strip "$row_dir/$row.png"
            row=$((row + 1))
            first=""
        fi
    done
    if [ -n "$first" ]; then
        cp "$first" "$row_dir/$row.png"
    fi
    magick "$row_dir"/*.png -append -strip "$OUTPUT_DIR/contact-sheet.png"
    printf 'SHEET %s\n' "$OUTPUT_DIR/contact-sheet.png"
fi
