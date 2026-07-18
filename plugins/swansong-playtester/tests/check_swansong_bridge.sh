#!/bin/sh
set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
PLUGIN_DIR=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)
REPO_DIR=$(CDPATH= cd -- "$PLUGIN_DIR/../.." && pwd)

if [ -n "${SWANSONG_DESKTOP_DIR:-}" ]; then
    SWANSONG_ROOT=$SWANSONG_DESKTOP_DIR
elif [ -x "$REPO_DIR/../GitHub/SwanSong-Desktop/Scripts/check-mcp-server.sh" ]; then
    SWANSONG_ROOT=$(CDPATH= cd -- "$REPO_DIR/../GitHub/SwanSong-Desktop" && pwd)
elif [ -n "${HOME:-}" ] \
    && [ -x "$HOME/Documents/GitHub/SwanSong-Desktop/Scripts/check-mcp-server.sh" ]; then
    SWANSONG_ROOT=$(CDPATH= cd -- "$HOME/Documents/GitHub/SwanSong-Desktop" && pwd)
else
    SWANSONG_ROOT=""
fi

CHECK=${SWANSONG_ROOT:+$SWANSONG_ROOT/Scripts/check-mcp-server.sh}
PLAYTEST_CHECK=${SWANSONG_ROOT:+$SWANSONG_ROOT/Scripts/check-playtest-mcp-server.sh}
if [ -z "$CHECK" ] || [ ! -x "$CHECK" ] \
    || [ -z "$PLAYTEST_CHECK" ] || [ ! -x "$PLAYTEST_CHECK" ]; then
    echo "SwanSong Desktop was not found. Set SWANSONG_DESKTOP_DIR to its checkout." >&2
    exit 2
fi

"$CHECK"
"$PLAYTEST_CHECK"
python3 - "$PLUGIN_DIR/.mcp.json" <<'PY'
import json
import pathlib
import sys

servers = json.loads(pathlib.Path(sys.argv[1]).read_text())["mcpServers"]
assert servers == {
    "swansong-playtester": {
        "command": "./scripts/run_swansong_playtest_mcp.sh",
        "cwd": ".",
    },
    "swansong-translation-lab": {
        "command": "./scripts/run_swansong_mcp.sh",
        "cwd": ".",
    },
}
print("PASS SwanSong plugin retains one-shot comparison and exposes full automation")
PY
python3 - "$PLUGIN_DIR/scripts/games.json" <<'PY'
import json
import pathlib
import sys

games = json.loads(pathlib.Path(sys.argv[1]).read_text())
assert len(games) == 10
required = {"title", "rom", "goal", "controls", "required_checks"}
for slug, game in games.items():
    assert required <= game.keys(), slug
    assert game["controls"], slug
print("PASS SwanSong playtester manifest describes ten game contracts")
PY
python3 - "$PLUGIN_DIR" <<'PY'
import json
import pathlib
import sys

plugin = pathlib.Path(sys.argv[1])
manifest = json.loads((plugin / ".codex-plugin" / "plugin.json").read_text())
skill = (plugin / "skills" / "play-swansong-games" / "SKILL.md").read_text()
patterns = (
    plugin
    / "skills"
    / "play-swansong-games"
    / "references"
    / "wwgp-design-patterns.md"
).read_text()
assert manifest["version"] == "0.3.1"
assert "references/wwgp-design-patterns.md" in skill
for required in (
    "Dual-cluster gestures",
    "Audio-led play",
    "Suspend and persistence",
    "Utility editor",
    "Clean-room boundary",
):
    assert required in patterns, required
print("PASS SwanSong playtester packages clean-room homebrew patterns")
PY
python3 "$PLUGIN_DIR/tests/test_live_mcp.py"
