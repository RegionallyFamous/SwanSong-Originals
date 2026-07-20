#!/usr/bin/env python3
"""Validate SDK migration manifests without requiring a vendored SDK checkout."""

from __future__ import annotations

import json
from pathlib import Path
import tomllib


ROOT = Path(__file__).resolve().parents[1]
GAMES = ROOT / "games"
PLUGIN_CONTRACTS = ROOT / "plugins" / "swansong-playtester" / "scripts" / "games.json"
INPUTS = {"x1", "x2", "x3", "x4", "y1", "y2", "y3", "y4", "a", "b", "start"}
SCENARIO_ORDER = (
    "neutral", "interaction", "success", "failure", "reset", "deterministic"
)
SCENARIOS = set(SCENARIO_ORDER)
EXTRA_SCENARIOS = {"one-last-lap": ("solo",)}
MAX_PLAN_FRAMES = 2700  # About 36 seconds at the WonderSwan's 75 Hz cadence.
MAX_PLAN_EVENTS = 192
SDK_VERSION = "0.5.0"
SDK_REVISION = "sha256:905d1b7683ea55aebb90703bc4dc708ae7a436c98dae1474e67c9df89601a35c"
HOLD_INPUTS = {
    "harpoon-moon": {frozenset({"a"}), frozenset({"b"}), frozenset({"a", "b"})},
    "one-last-lap": {
        frozenset({"a"}),
        frozenset({"b"}),
        frozenset({"x2"}),
        frozenset({"a", "b"}),
        frozenset({"a", "start"}),
    },
    "pocket-kaiju-observatory": {
        frozenset({"a"}),
        frozenset({"b"}),
        frozenset({"start"}),
        frozenset({"x2"}),
    },
}
AUDIO_SCENARIOS = {
    "mote-sound-terminal": SCENARIOS,
    "orbital-courier": {"interaction", "success", "failure", "deterministic"},
    "scrapframe-garage": {"interaction", "success", "failure", "deterministic"},
    "radio-ghost": {"interaction", "success", "failure", "deterministic"},
    "harpoon-moon": {"interaction", "success", "deterministic"},
    "turncoat-tactics": {"interaction", "success", "failure", "deterministic"},
    "pocket-kaiju-observatory": {"interaction", "success", "failure", "deterministic"},
    "rotate-dungeon": {"interaction", "success", "failure", "deterministic"},
    "one-last-lap": {"success", "solo", "deterministic"},
    "bug-witch": {"interaction", "success", "failure", "deterministic"},
}
SILENT_AUDIO_SCENARIOS = {
    "mote-sound-terminal": set(),
    "orbital-courier": {"reset"},
    "scrapframe-garage": {"reset"},
    "radio-ghost": {"reset"},
    "pocket-kaiju-observatory": {"reset"},
    "rotate-dungeon": {"reset"},
    "bug-witch": {"reset"},
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
    for index, event in enumerate(plan["events"]):
        if not event["inputs"]:
            continue
        assert index + 1 < len(plan["events"]), (path, event)
        release = plan["events"][index + 1]
        assert release["inputs"] == [], (path, event, release)
        if release["frameIndex"] != event["frameIndex"] + 1:
            allowed = HOLD_INPUTS.get(slug, set())
            assert frozenset(event["inputs"]) in allowed, (path, event)
        if index + 2 < len(plan["events"]):
            following = plan["events"][index + 2]
            assert following["frameIndex"] >= release["frameIndex"] + 2, (
                path, release, following
            )
    return plan


def main() -> None:
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
        assert cartridge["save_type"] == "none" and wonderful["save_type"] == "NONE", slug
        assert cartridge["rtc"] is wonderful["rtc"] is False, slug
        assert game["hardware"] == "color-required" and wonderful["color"] is True, slug
        assert actions and len(actions) <= 16, slug
        assert ready_frames == 120, slug
        expected_directions = SPECIAL_DIRECTION_ACTIONS.get(slug, PHYSICAL_DIRECTIONS)
        for action, inputs in expected_directions.items():
            assert tuple(actions[action]) == inputs, (slug, action, actions[action])
        expected_order = list(SCENARIO_ORDER)
        for extra in EXTRA_SCENARIOS.get(slug, ()):
            expected_order.insert(expected_order.index("failure"), extra)
        assert [item["id"] for item in scenarios] == expected_order, slug
        assert budgets["rom_bytes"] <= 8 * 1024 * 1024, slug
        assert cartridge["game_id"] not in seen_ids, slug
        seen_ids.add(cartridge["game_id"])

        if slug == "orbital-courier":
            assets = {item["id"]: item for item in manifest["assets"]}
            assert set(assets) == {"intro", "gameplay"}
            assert assets["intro"]["type"] == "fullscreen"
            assert assets["gameplay"]["type"] == "metatiles"
            scene_assets = {item["id"]: item.get("assets", [])
                            for item in manifest["scenes"]}
            assert scene_assets == {
                "intro": ["intro"],
                "delivery": ["gameplay"],
                "result": ["gameplay"],
            }

        plans: dict[str, dict[str, object]] = {}
        for scenario in scenarios:
            assert scenario["required_checks"], (slug, scenario["id"])
            expected_audio = (
                "silent" if scenario["id"] in SILENT_AUDIO_SCENARIOS.get(slug, set())
                else "audible" if scenario["id"] in AUDIO_SCENARIOS[slug]
                else "any"
            )
            declared_audio = scenario.get("audio_expectation")
            if declared_audio is None:
                declared_audio = "audible" if scenario.get("audio") is True else "any"
            assert declared_audio == expected_audio, (
                slug, scenario["id"], "audio evidence declaration"
            )
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
            assert reset_inputs[0]["frameIndex"] >= 90, "Orbital starts before scene-ready"
            assert sum(event["inputs"] in (["x2"], ["x4"])
                       for event in reset_inputs) == 40
            assert reset_inputs[-1]["inputs"] == ["a"]
            assert reset["totalFrames"] - reset_inputs[-1]["frameIndex"] >= 60
        rom = next(root.glob("*.wsc"), None)
        if rom is not None:
            assert rom.stat().st_size <= budgets["rom_bytes"], slug

    print("PASS ten SDK manifests and sixty-one fresh-boot SwanSong frame plans are consistent")


if __name__ == "__main__":
    main()
