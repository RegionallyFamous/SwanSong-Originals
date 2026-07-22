#!/usr/bin/env python3
"""Validate SDK migration manifests without requiring a vendored SDK checkout."""

from __future__ import annotations

import json
from pathlib import Path
import tomllib


ROOT = Path(__file__).resolve().parents[1]
GAMES = ROOT / "games"
PLUGIN_CONTRACTS = ROOT / "plugins" / "swansong-playtester" / "scripts" / "games.json"
GAME_BUILD_RECIPE = ROOT / "mk" / "wonderful-game.mk"
INPUTS = {"x1", "x2", "x3", "x4", "y1", "y2", "y3", "y4", "a", "b", "start"}
LEGACY_SCENARIO_ORDER = (
    "neutral", "interaction", "success", "failure", "reset", "deterministic"
)
PRODUCTION_SCENARIO_ORDER = (
    "neutral", "attract", "menu-input", "tutorial", "interaction", "pause",
    "success", "failure", "reset", "save-restart", "audio", "deterministic",
)
EXTRA_SCENARIOS = {"one-last-lap": ("solo",)}
# Production play contracts must be able to exercise a complete short handheld
# session, not only a prototype-sized success path.  Keep the ceiling bounded
# so a malformed plan cannot make CI run forever, while allowing roughly five
# minutes at the WonderSwan's native 75.47 Hz cadence.
MAX_PLAN_FRAMES = 24_000
MAX_PLAN_EVENTS = 2_048
SDK_VERSION = "0.5.0"
SDK_REVISION = "sha256:47c8f48d2e2c7f3d4ed8b6e2adea963007d8ce325b90f410d7509d77c25f3001"
HOLD_INPUTS = {
    "harpoon-moon": {frozenset({"a"}), frozenset({"b"}), frozenset({"a", "b"})},
    "one-last-lap": {
        frozenset({"a"}),
        frozenset({"b"}),
        frozenset({"x2"}),
        frozenset({"a", "b"}),
        frozenset({"a", "start"}),
        frozenset({"b", "x1"}),
        frozenset({"b", "x4"}),
    },
    "pocket-kaiju-observatory": {
        frozenset({"a"}),
        frozenset({"b"}),
        frozenset({"start"}),
        frozenset({"x2"}),
    },
}
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
EXPECTED = {
    "mote-sound-terminal": 1,
    "orbital-courier": 2,
    "scrapframe-garage": 3,
    "radio-ghost": 4,
    "harpoon-moon": 5,
    "turncoat-tactics": 6,
    "pocket-kaiju-observatory": 7,
    "rotate-dungeon": 8,
    "one-last-lap": 9,
    "bug-witch": 10,
}
PHYSICAL_DIRECTIONS = {
    "left": ("X4", "Y1"),
    "right": ("X2", "Y3"),
    "up": ("X3", "Y2"),
    "down": ("X1", "Y4"),
}
SPECIAL_DIRECTION_ACTIONS = {
    "harpoon-moon": {
        "left": PHYSICAL_DIRECTIONS["left"],
        "right": PHYSICAL_DIRECTIONS["right"],
    },
    "bug-witch": {
        "previous_socket": PHYSICAL_DIRECTIONS["left"],
        "next_socket": PHYSICAL_DIRECTIONS["right"],
        "previous_familiar": PHYSICAL_DIRECTIONS["up"],
        "next_familiar": PHYSICAL_DIRECTIONS["down"],
    },
    "mote-sound-terminal": {
        "previous_track": PHYSICAL_DIRECTIONS["left"],
        "next_track": PHYSICAL_DIRECTIONS["right"],
        # The model increases tempo in the helper's positive/down direction.
        "tempo_up": PHYSICAL_DIRECTIONS["down"],
        "tempo_down": PHYSICAL_DIRECTIONS["up"],
    },
    "one-last-lap": {
        "left": PHYSICAL_DIRECTIONS["left"],
        "right": PHYSICAL_DIRECTIONS["right"],
    },
    "pocket-kaiju-observatory": {
        "left": PHYSICAL_DIRECTIONS["left"],
        "right": PHYSICAL_DIRECTIONS["right"],
    },
    "radio-ghost": {
        "tune_down": PHYSICAL_DIRECTIONS["left"],
        "tune_up": PHYSICAL_DIRECTIONS["right"],
        # Gain subtracts the helper direction, so physical up increases it.
        "gain_up": PHYSICAL_DIRECTIONS["up"],
        "gain_down": PHYSICAL_DIRECTIONS["down"],
    },
    "scrapframe-garage": {
        "previous_part": PHYSICAL_DIRECTIONS["left"],
        "next_part": PHYSICAL_DIRECTIONS["right"],
    },
}


def validate_plan(path: Path, slug: str) -> dict[str, object]:
    plan = json.loads(path.read_text())
    assert plan["schema"] == "swan-song-frame-input-plan-v1", path
    total = plan["totalFrames"]
    assert isinstance(total, int) and 120 <= total <= MAX_PLAN_FRAMES, (
        path, total, MAX_PLAN_FRAMES
    )
    previous = -1
    assert plan["events"], path
    assert len(plan["events"]) <= MAX_PLAN_EVENTS, (
        path, len(plan["events"]), MAX_PLAN_EVENTS
    )
    assert plan["events"][0] == {"frameIndex": 0, "inputs": []}, path
    active_frames = [
        event["frameIndex"] for event in plan["events"] if event["inputs"]
    ]
    if active_frames:
        assert active_frames[0] >= 120, (
            path, active_frames[0], "gameplay input occurs before the scene-ready window"
        )
    for event in plan["events"]:
        frame = event["frameIndex"]
        inputs = event["inputs"]
        assert previous < frame < total, (path, frame)
        assert len(inputs) == len(set(inputs)), path
        assert set(inputs) <= INPUTS, (path, inputs)
        previous = frame
    assert plan["events"][-1]["inputs"] == [], (
        path, "the plan must release all held controls before its endpoint"
    )
    for index, event in enumerate(plan["events"][:-1]):
        following = plan["events"][index + 1]
        assert following["inputs"] != event["inputs"], (
            path, event, "adjacent events repeat the same input state"
        )
        duration = following["frameIndex"] - event["frameIndex"]
        # A real button press commonly spans a few native frames. Only longer
        # holds need a game-specific allowlist so scenario recording can keep
        # debounced menu taps without weakening charge/lure contracts.
        if event["inputs"] and duration > 15:
            allowed = HOLD_INPUTS.get(slug, set())
            assert frozenset(event["inputs"]) in allowed, (path, event)
    return plan


def main() -> None:
    build_recipe = GAME_BUILD_RECIPE.read_text()
    for contract in (
        "SWAN_TRACE ?= 0",
        "SWAN_TRACE_CAPACITY ?= 64",
        "-DSWAN_DETERMINISTIC_TRACE=$(SWAN_TRACE)",
        "-DSWAN_DEBUG_FRAME_TRACE_CAPACITY=$(SWAN_TRACE_CAPACITY)",
        "trace$(SWAN_TRACE)-$(SWAN_TRACE_CAPACITY)",
    ):
        assert contract in build_recipe, (
            "shared build recipe dropped the SwanSong trace contract", contract
        )
    assert build_recipe.count("SWAN_DETERMINISTIC_TRACE=$(SWAN_TRACE)") >= 2
    assert build_recipe.count(
        "SWAN_DEBUG_FRAME_TRACE_CAPACITY=$(SWAN_TRACE_CAPACITY)"
    ) >= 2

    plugin = json.loads(PLUGIN_CONTRACTS.read_text())
    assert set(plugin) == set(EXPECTED)
    seen_ids: set[int] = set()
    for slug, expected_game_id in EXPECTED.items():
        root = GAMES / slug
        with (root / "swan.toml").open("rb") as handle:
            manifest = tomllib.load(handle)
        with (root / "wfconfig.toml").open("rb") as handle:
            wonderful = tomllib.load(handle)["cartridge"]
        game = manifest["game"]
        cartridge = manifest["cartridge"]
        actions = manifest["controls"]["actions"]
        ready_frames = manifest["play"]["ready_frames"]
        scenarios = manifest["play"]["scenarios"]
        budgets = manifest["budgets"]

        assert manifest["schema_version"] == 1, slug
        assert manifest["sdk"] == {
            "version": SDK_VERSION,
            "revision": SDK_REVISION,
        }, slug
        assert game["id"] == slug, slug
        assert game["title"] == plugin[slug]["title"], slug
        assert cartridge["game_id"] == expected_game_id == wonderful["game_id"], slug
        assert cartridge["publisher_id"] == wonderful["publisher_id"], slug
        assert cartridge["version"] == wonderful["game_version"], slug
        assert cartridge["save_type"] in SAVE_TYPES, slug
        assert wonderful["save_type"] == SAVE_TYPES[cartridge["save_type"]], slug
        assert isinstance(cartridge.get("save_bytes"), int), slug
        if cartridge["save_type"] == "none":
            assert cartridge["save_bytes"] == 0, slug
        else:
            assert cartridge["save_bytes"] > 0, slug
        assert cartridge["rtc"] is wonderful["rtc"] is False, slug
        assert game["hardware"] == "color-required" and wonderful["color"] is True, slug
        assert actions and len(actions) <= 16, slug
        assert ready_frames == 120, slug
        expected_directions = SPECIAL_DIRECTION_ACTIONS.get(slug, PHYSICAL_DIRECTIONS)
        for action, inputs in expected_directions.items():
            assert tuple(actions[action]) == inputs, (slug, action, actions[action])
        scenario_ids = [item["id"] for item in scenarios]
        allowed_sets: list[set[str]] = []
        for base in (LEGACY_SCENARIO_ORDER, PRODUCTION_SCENARIO_ORDER):
            expected_order = list(base)
            for extra in EXTRA_SCENARIOS.get(slug, ()):
                expected_order.insert(expected_order.index("failure"), extra)
            allowed_sets.append(set(expected_order))
        assert len(scenario_ids) == len(set(scenario_ids)), (slug, scenario_ids)
        assert set(scenario_ids) in allowed_sets, (slug, scenario_ids)
        assert [item for item in scenario_ids
                if item in {"success", "failure", "reset", "deterministic"}] == [
            "success", "failure", "reset", "deterministic"
        ], (slug, "semantic scenario order")
        assert budgets["rom_bytes"] <= 8 * 1024 * 1024, slug
        assert cartridge["game_id"] not in seen_ids, slug
        seen_ids.add(cartridge["game_id"])

        if slug == "orbital-courier":
            assets = {item["id"]: item for item in manifest["assets"]}
            assert {"intro", "gameplay", "title_music", "delivery_music"} <= set(assets)
            assert assets["intro"]["type"] == "fullscreen"
            assert assets["gameplay"]["type"] == "metatiles"
            assert assets["title_music"]["type"] == "music"
            assert assets["delivery_music"]["type"] == "music"
            assert sum(item["type"] == "sfx" for item in assets.values()) >= 6
            scene_assets = {item["id"]: item.get("assets", [])
                            for item in manifest["scenes"]}
            assert {"intro", "title_music"} <= set(scene_assets["intro"])
            assert {"gameplay", "delivery_music"} <= set(scene_assets["delivery"])
            assert "gameplay" in scene_assets["result"]

        plans: dict[str, dict[str, object]] = {}
        for scenario in scenarios:
            assert scenario["required_checks"], (slug, scenario["id"])
            declared_audio = scenario.get("audio_expectation")
            if declared_audio is None:
                declared_audio = "audible" if scenario.get("audio") is True else "any"
            assert declared_audio in {"any", "audible", "silent"}, (
                slug, scenario["id"], declared_audio
            )
            if scenario["id"] == "audio":
                assert declared_audio == "audible", (slug, "audio scenario")
            plan = (root / scenario["plan"]).resolve()
            plan.relative_to(root.resolve())
            plans[scenario["id"]] = validate_plan(plan, slug)
        assert plans["deterministic"] == plans["success"], slug
        assert plans["success"] != plans["failure"], slug
        assert plans["success"] != plans["reset"], slug
        assert plans["failure"] != plans["reset"], slug
        if slug == "orbital-courier":
            exercised = {
                event["inputs"][0]
                for plan in plans.values()
                for event in plan["events"]
                if len(event["inputs"]) == 1 and event["inputs"][0].startswith("x")
            }
            assert {"x1", "x2", "x3", "x4"} <= exercised
            reset = plans["reset"]
            reset_inputs = [event for event in reset["events"] if event["inputs"]]
            assert reset_inputs[0]["inputs"] == ["start"]
            assert sum(event["inputs"] in (["x2"], ["x4"])
                       for event in reset_inputs) >= 20
            assert reset_inputs[-1]["inputs"] == ["a"]
            assert reset["totalFrames"] - reset_inputs[-1]["frameIndex"] >= 60
        rom = next(root.glob("*.wsc"), None)
        if rom is not None:
            assert rom.stat().st_size <= budgets["rom_bytes"], slug

    print("PASS ten SDK manifests and their fresh-boot SwanSong frame plans are consistent")


if __name__ == "__main__":
    main()
