#!/usr/bin/env python3
"""Fast host checks for authored puzzle and route invariants."""

from __future__ import annotations

from collections import deque
from itertools import product
from pathlib import Path
import json
import re
import tomllib


ROOT = Path(__file__).resolve().parents[1]

SAVE_TYPES = {
    "none": "NONE",
    "eeprom-128b": "EEPROM_128B",
    "eeprom-1kb": "EEPROM_1KB",
    "eeprom-2kb": "EEPROM_2KB",
    "sram-8kb": "SRAM_8KB",
    "sram-32kb": "SRAM_32KB",
    "sram-128kb": "SRAM_128KB",
    "sram-256kb": "SRAM_256KB",
    "sram-512kb": "SRAM_512KB",
}


def reachable(start, goal, blocked, width, height):
    queue = deque([start])
    seen = {start}
    while queue:
        point = queue.popleft()
        if point == goal:
            return True
        for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            nxt = point[0] + dx, point[1] + dy
            if (
                0 <= nxt[0] < width
                and 0 <= nxt[1] < height
                and nxt not in seen
                and not blocked(*nxt)
            ):
                seen.add(nxt)
                queue.append(nxt)
    return False


def test_cartridge_headers():
    ids = set()
    configs = sorted((ROOT / "games").glob("*/wfconfig.toml"))
    assert len(configs) == 10
    for config_path in configs:
        text = config_path.read_text()
        with (config_path.parent / "swan.toml").open("rb") as handle:
            manifest = tomllib.load(handle)["cartridge"]
        game_id_match = re.search(r"^game_id = (\d+)$", text, re.MULTILINE)
        assert game_id_match, f"missing game_id in {config_path}"
        game_id = int(game_id_match.group(1))
        assert re.search(r"^color = true$", text, re.MULTILINE)
        assert re.search(
            rf'^save_type = "{SAVE_TYPES[str(manifest["save_type"])]}"$',
            text,
            re.MULTILINE,
        )
        assert re.search(
            rf'^game_version = {int(manifest["version"])}$', text, re.MULTILINE
        )
        assert game_id == int(manifest["game_id"])
        assert game_id not in ids
        ids.add(game_id)
    assert ids == set(range(1, 11))


def test_graphical_front_ends():
    for source in sorted((ROOT / "games").glob("*/src/main.c")):
        text = source.read_text()
        assert '#include "gfx.h"' in text
        assert "gfx_render(" in text
        for terminal_call in ("printf(", "putchar(", "rf_header(", "rf_footer("):
            assert terminal_call not in text, f"terminal call remains in {source}"


def test_deterministic_sessions_and_documented_directions():
    sources = {path.parent.parent.name: path.read_text()
               for path in sorted((ROOT / "games").glob("*/src/main.c"))}
    for slug, text in sources.items():
        assert "void main(" not in text, f"{slug} still owns the platform loop"
        for callback in (
            "swan_game_boot(", "swan_scene_enter(", "swan_scene_update(",
            "swan_scene_render(", "swan_scene_exit(",
        ):
            assert callback in text, f"{slug} is missing {callback}"
        assert "swan_core_reset_session(" in text, (
            f"{slug} must drain input and reset deterministic session time"
        )
        assert "frame->input" in text, f"{slug} bypasses immutable SDK input"
    assert "SWAN_ACTION_LEFT" in sources["harpoon-moon"]
    assert "SWAN_ACTION_RIGHT" in sources["harpoon-moon"]
    assert "swan_game_primary_axis(frame->input->pressed)" in sources[
        "pocket-kaiju-observatory"
    ]
    assert "SWAN_ACTION_LEFT" in sources["one-last-lap"]
    assert "SWAN_ACTION_RIGHT" in sources["one-last-lap"]
    for action in ("UP", "RIGHT", "DOWN", "LEFT"):
        assert f"SWAN_ACTION_{action}" in sources["orbital-courier"]
    for slug in ("turncoat-tactics", "rotate-dungeon"):
        assert "swan_input_dx(frame->input->pressed)" in sources[slug]
        assert "swan_input_dy(frame->input->pressed)" in sources[slug]
    lap = (ROOT / "games/one-last-lap/src/model.c").read_text()
    assert "update_rivals(state, previous_distance, event)" in lap
    assert "state->rival_distance[rival]" in lap
    assert "state->helped ? LAP_RESULT_COOPERATIVE : LAP_RESULT_SOLO" in lap


def test_sdk_runtime_ownership():
    shared_make = (ROOT / "mk/wonderful-game.mk").read_text()
    root_make = (ROOT / "Makefile").read_text()
    assert "SWANSONG_RUNTIME" in shared_make
    assert "swan_config.c" in shared_make
    assert "HOST_TEST_NAME ?= $(NAME)" in shared_make
    assert "test: all" in shared_make
    assert "$(HOST_TEST)" in shared_make
    assert "HOST_TEST_NAME := pocket_kaiju" in (
        ROOT / "games/pocket-kaiju-observatory/Makefile"
    ).read_text()
    assert "librf_swan.a" not in shared_make
    assert "../../engine/include" not in shared_make
    assert "all: engine" not in root_make
    for renderer in sorted((ROOT / "games").glob("*/src/gfx.c")):
        text = renderer.read_text()
        if renderer.parent.parent.name in {
            "harpoon-moon", "mote-sound-terminal", "orbital-courier"
        }:
            assert "#include <swan/legacy.h>" not in text, renderer
            assert '#include "swan_assets.h"' in text, renderer
            assert "swan_gfx_load_tiles(" in text, renderer
        else:
            assert "#include <swan/legacy.h>" in text, renderer
        assert '#include "rf_swan.h"' not in text, renderer


def test_radio_ghost_timing():
    # Held-input repeat every three frames reaches all clues well before dawn.
    targets = (934, 995, 1042)
    tuning_steps = sum((abs(target - 880) + 1) // 2 for target in targets)
    assert tuning_steps * 3 + 3 < 4500


def test_courier_route():
    def blocked(x, y):
        return (
            x in (0, 19)
            or y in (0, 8)
            or (y == 3 and 3 < x < 15 and x != 9)
            or (x == 12 and 3 < y < 8 and y != 6)
        )

    assert reachable((2, 1), (3, 7), blocked, 20, 9)
    assert reachable((3, 7), (17, 1), blocked, 20, 9)


def test_rotate_rooms():
    def blocked(room, vertical, x, y):
        if x in (0, 11) or y in (0, 7):
            return True
        if not vertical:
            return x == 3 + room % 5 and y != 1 + room % 6
        return y == 2 + room % 4 and x != 1 + (room * 2) % 10

    for room in range(5):
        key = 2 + room, 6 - (room & 1)
        goal = 10, 1
        queue = deque([(1, 1, False, False)])
        seen = set(queue)
        solved = False
        while queue:
            x, y, vertical, has_key = queue.popleft()
            has_key = has_key or (x, y) == key
            if has_key and (x, y) == goal:
                solved = True
                break
            for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                nx, ny = x + dx, y + dy
                state = nx, ny, vertical, has_key
                if state not in seen and not blocked(room, vertical, nx, ny):
                    seen.add(state)
                    queue.append(state)
            new_vertical = not vertical
            nx, ny = ((1, 1) if blocked(room, new_vertical, x, y) else (x, y))
            state = nx, ny, new_vertical, has_key
            if state not in seen:
                seen.add(state)
                queue.append(state)
        assert solved, f"Rotate Dungeon room {room + 1} is unsolvable"

    success_path = ROOT / "games/rotate-dungeon/tests/play/success.json"
    deterministic_path = ROOT / "games/rotate-dungeon/tests/play/deterministic.json"
    assert success_path.read_bytes() == deterministic_path.read_bytes()
    plan = json.loads(success_path.read_text())
    room = x = y = 0
    x = y = 1
    vertical = has_key = False
    input_actions = {
        "start": "rotate", "x3": "up", "x2": "right",
        "x1": "down", "x4": "left",
    }
    actions = [input_actions[event["inputs"][0]] for event in plan["events"]
               if event["inputs"]]
    for action in actions:
        if action == "rotate":
            vertical = not vertical
            if blocked(room, vertical, x, y):
                x = y = 1
        else:
            dx, dy = {
                "up": (0, -1), "right": (1, 0),
                "down": (0, 1), "left": (-1, 0),
            }[action]
            if not blocked(room, vertical, x + dx, y + dy):
                x += dx
                y += dy
        has_key = has_key or (x, y) == (2 + room, 6 - (room & 1))
        if has_key and (x, y) == (10, 1):
            room += 1
            if room < 5:
                x = y = 1
                has_key = False
    assert room == 5, "Rotate Dungeon canonical SwanSong route must finish all rooms"


def test_turncoat_canonical_plan():
    success_path = ROOT / "games/turncoat-tactics/tests/play/success.json"
    deterministic_path = ROOT / "games/turncoat-tactics/tests/play/deterministic.json"
    assert success_path.read_bytes() == deterministic_path.read_bytes()
    plan = json.loads(success_path.read_text())
    token_to_action = {
        "x3": "U", "x2": "R", "x1": "D", "x4": "L",
        "a": "A", "b": "B",
    }
    route = "".join(token_to_action[event["inputs"][0]]
                    for event in plan["events"] if event["inputs"])
    assert route == "UARARAUAALLDDDADAUUARABADA"


def test_bug_witch_puzzles():
    inputs = [0, 0, 1, 1, 0]
    targets = [1, 1, 0, 1, 0]
    limits = [1, 2, 1, 2, 3]
    required_masks = [1, 3, 4, 5, 7]
    for puzzle in range(5):
        solutions = 0
        for cells in product(range(4), repeat=5):
            signal = inputs[puzzle]
            used = mask = 0
            for cell in cells:
                if cell == 1:
                    signal ^= 1
                    mask |= 1
                    used += 1
                elif cell == 2:
                    signal = 1
                    mask |= 2
                    used += 1
                elif cell == 3:
                    signal = 0
                    mask |= 4
                    used += 1
            if (
                signal == targets[puzzle]
                and used <= limits[puzzle]
                and mask & required_masks[puzzle] == required_masks[puzzle]
            ):
                solutions += 1
        assert solutions, f"Bug Witch puzzle {puzzle + 1} is unsolvable"


def main():
    test_cartridge_headers()
    test_graphical_front_ends()
    test_deterministic_sessions_and_documented_directions()
    test_sdk_runtime_ownership()
    test_radio_ghost_timing()
    test_courier_route()
    test_rotate_rooms()
    test_turncoat_canonical_plan()
    test_bug_witch_puzzles()
    print("OK   cartridge IDs, graphical front ends, timing, routes, rooms, and puzzles")


if __name__ == "__main__":
    main()
