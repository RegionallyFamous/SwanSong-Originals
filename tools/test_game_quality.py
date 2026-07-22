#!/usr/bin/env python3
"""Audit the anthology against the SwanSong production-game contract.

Report/baseline mode is intentionally descriptive: the current collection is
allowed to have blockers while it is rebuilt. Strict mode turns every blocker
into a failing exit status for explicitly selected upgraded games.

This is a static and artifact audit. It can prove declarations, source hooks,
plan lengths, native PNG dimensions, and the contents of small evidence
summaries. It cannot prove that a game is fun, readable on physical hardware,
musically appealing, or understood by a new player. The strict contract
therefore requires separate local human-playtest evidence.
"""

from __future__ import annotations

import argparse
from dataclasses import asdict, dataclass, field
import hashlib
import json
import math
from pathlib import Path
import re
import struct
import sys
import tomllib
from typing import Any
import wave


ROOT = Path(__file__).resolve().parents[1]
GAMES_ROOT = ROOT / "games"
NATIVE_REFRESH_HZ = 75.472
MIN_SUCCESS_SECONDS = 60
MIN_SUCCESS_FRAMES = math.ceil(NATIVE_REFRESH_HZ * MIN_SUCCESS_SECONDS)
REQUIRED_ORIGINALS = {
    "bug-witch",
    "harpoon-moon",
    "mote-sound-terminal",
    "one-last-lap",
    "orbital-courier",
    "pocket-kaiju-observatory",
    "radio-ghost",
    "rotate-dungeon",
    "scrapframe-garage",
    "turncoat-tactics",
}
REQUIRED_PLAY_SCENARIOS = {
    "neutral",
    "attract",
    "menu-input",
    "tutorial",
    "interaction",
    "pause",
    "success",
    "failure",
    "reset",
    "save-restart",
    "audio",
    "deterministic",
}
REQUIRED_OUTCOME_SCENARIOS = {"success", "failure", "reset", "deterministic"}
PHYSICAL_WIDTH = 224
PHYSICAL_HEIGHT = 144

HARDWARE_BUDGET_MARKERS = {
    "vram_tiles": 512,
    "palettes": 16,
    "sprites": 128,
    "sprites_per_scanline": 32,
}

PRODUCTION_MAXIMUMS = {
    "rom_bytes": 512 * 1024,
    "work_ram_bytes": 12 * 1024,
    "vram_tiles": 448,
    "palettes": 12,
    "sprites": 64,
    "sprites_per_scanline": 24,
    "audio_bytes": 32 * 1024,
}

RUNTIME_PROFILE_LIMITS = {
    "durationFrames": math.ceil(NATIVE_REFRESH_HZ * 300),
    "p95Cycles": 30_528,
    "maxCycles": 37_999,
    "missedVBlanks": 0,
    "linkedInternalRAMBytes": 56 * 1024,
    "linkedMonoAreaBytes": 14 * 1024,
    "linkedColorAreaBytes": 42 * 1024,
    "peakTiles": 448,
    "peakPalettes": 12,
    "peakSprites": 64,
    "peakSpritesPerScanline": 24,
    "romBytes": 512 * 1024,
}

MEDIA_SCHEMA = "swan-song-game-quality-media-observation-v1"
PLAYTEST_SCHEMA = "swan-song-game-quality-human-playtest-v1"
PROFILE_SCHEMA = "swan-song-game-quality-runtime-profile-v1"
SHA256_RE = re.compile(r"^[0-9a-f]{64}$")


@dataclass(frozen=True)
class Finding:
    code: str
    message: str
    evidence: str | None = None


@dataclass
class GameAudit:
    game: str
    title: str = ""
    findings: list[Finding] = field(default_factory=list)
    metrics: dict[str, Any] = field(default_factory=dict)
    fatal: str | None = None

    @property
    def strict_ready(self) -> bool:
        return self.fatal is None and not self.findings

    def add(self, code: str, message: str, evidence: str | None = None) -> None:
        self.findings.append(Finding(code, message, evidence))


class AuditInputError(RuntimeError):
    pass


def _inside_game(game_root: Path, raw: object, label: str) -> Path | None:
    if not isinstance(raw, str) or not raw.strip():
        return None
    candidate = (game_root / raw).resolve()
    try:
        candidate.relative_to(game_root.resolve())
    except ValueError as exc:
        raise AuditInputError(f"{label} escapes the game directory: {raw}") from exc
    return candidate


def _load_json(path: Path, label: str) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text())
    except FileNotFoundError as exc:
        raise AuditInputError(f"{label} does not exist: {path}") from exc
    except (OSError, UnicodeDecodeError, json.JSONDecodeError) as exc:
        raise AuditInputError(f"could not read {label} {path}: {exc}") from exc
    if not isinstance(value, dict):
        raise AuditInputError(f"{label} must contain a JSON object: {path}")
    return value


def _sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for chunk in iter(lambda: source.read(64 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def _png_dimensions(path: Path) -> tuple[int, int] | None:
    try:
        with path.open("rb") as source:
            header = source.read(24)
    except OSError:
        return None
    if len(header) != 24 or header[:8] != b"\x89PNG\r\n\x1a\n" or header[12:16] != b"IHDR":
        return None
    return struct.unpack(">II", header[16:24])


def _is_wave_file(path: Path) -> bool:
    try:
        with wave.open(str(path), "rb") as source:
            return (
                source.getnchannels() in (1, 2)
                and source.getsampwidth() > 0
                and source.getframerate() > 0
                and source.getnframes() > 0
            )
    except (OSError, EOFError, wave.Error):
        return False


def _all_source(game_root: Path) -> str:
    parts: list[str] = []
    for path in sorted((game_root / "src").glob("**/*")):
        if path.suffix in {".c", ".h"} and path.is_file():
            parts.append(path.read_text(errors="replace"))
    return "\n".join(parts)


def _scenario(manifest: dict[str, Any], scenario_id: str) -> dict[str, Any] | None:
    play = manifest.get("play", {})
    if not isinstance(play, dict):
        return None
    scenarios = play.get("scenarios", [])
    if not isinstance(scenarios, list):
        return None
    return next(
        (item for item in scenarios
         if isinstance(item, dict) and item.get("id") == scenario_id),
        None,
    )


def _quality_table(manifest: dict[str, Any]) -> dict[str, Any]:
    value = manifest.get("quality", {})
    return value if isinstance(value, dict) else {}


def _check_native_png(
    audit: GameAudit,
    game_root: Path,
    evidence: dict[str, Any],
    key: str,
    code: str,
    label: str,
    expected_dimensions: tuple[int, int],
    conventional: Path | None = None,
) -> Path | None:
    raw = evidence.get(key)
    path = _inside_game(game_root, raw, f"quality.evidence.{key}")
    if path is None and conventional is not None and conventional.is_file():
        path = conventional.resolve()
    if path is None or not path.is_file():
        audit.add(code, f"Missing native {label} evidence PNG.")
        return None
    dimensions = _png_dimensions(path)
    if dimensions != expected_dimensions:
        audit.add(
            "evidence-native-size-invalid",
            f"{label.capitalize()} evidence must be a {expected_dimensions[0]}x{expected_dimensions[1]} PNG; got {dimensions!r}.",
            str(path.relative_to(ROOT)),
        )
        return None
    return path


def _check_media_observation(
    audit: GameAudit,
    game_root: Path,
    evidence: dict[str, Any],
    expected_pngs: dict[str, Path | None],
    expected_audio: Path | None,
) -> None:
    path = _inside_game(
        game_root, evidence.get("media_observation"),
        "quality.evidence.media_observation",
    )
    if path is None or not path.is_file():
        audit.add(
            "media-unobserved",
            "No local, hash-bound record says a person inspected the title/gameplay/result PNGs and listened across the BGM loop in SwanSong.",
        )
        return
    try:
        document = _load_json(path, "media observation")
    except AuditInputError as exc:
        audit.add("media-observation-invalid", str(exc))
        return
    problems: list[str] = []
    if document.get("schema") != MEDIA_SCHEMA:
        problems.append(f"schema must be {MEDIA_SCHEMA}")
    if document.get("game") != audit.game:
        problems.append("game does not match the manifest")
    if document.get("localOnly") is not True:
        problems.append("localOnly must be true")
    observed = document.get("screens")
    if not isinstance(observed, dict):
        problems.append("screens must be an object")
    else:
        for key, expected_path in expected_pngs.items():
            record = observed.get(key)
            if not isinstance(record, dict) or record.get("inspected") is not True:
                problems.append(f"screens.{key} was not explicitly inspected")
                continue
            digest = record.get("sha256")
            if not isinstance(digest, str) or not SHA256_RE.fullmatch(digest):
                problems.append(f"screens.{key}.sha256 is not lowercase SHA-256")
            elif expected_path is not None and digest != _sha256(expected_path):
                problems.append(f"screens.{key}.sha256 does not bind the current PNG")
    audio = document.get("audio")
    if not isinstance(audio, dict):
        problems.append("audio must be an object")
    else:
        if audio.get("listened") is not True:
            problems.append("audio.listened must be true")
        if audio.get("loopBoundaryInspected") is not True:
            problems.append("audio.loopBoundaryInspected must be true")
        digest = audio.get("wavSHA256")
        if not isinstance(digest, str) or not SHA256_RE.fullmatch(digest):
            problems.append("audio.wavSHA256 is not lowercase SHA-256")
        elif expected_audio is not None and digest != _sha256(expected_audio):
            problems.append("audio.wavSHA256 does not bind the current WAV")
    if problems:
        audit.add(
            "media-observation-invalid",
            "; ".join(problems),
            str(path.relative_to(ROOT)),
        )


def _check_human_playtest(
    audit: GameAudit, game_root: Path, evidence: dict[str, Any]
) -> None:
    path = _inside_game(
        game_root, evidence.get("human_playtest"),
        "quality.evidence.human_playtest",
    )
    if path is None or not path.is_file():
        audit.add(
            "human-playtest-missing",
            "No separate local uncoached human-playtest summary is declared; automation cannot establish clarity, enjoyment, or replay desire.",
        )
        return
    try:
        document = _load_json(path, "human playtest summary")
    except AuditInputError as exc:
        audit.add("human-playtest-invalid", str(exc))
        return
    problems: list[str] = []
    if document.get("schema") != PLAYTEST_SCHEMA:
        problems.append(f"schema must be {PLAYTEST_SCHEMA}")
    if document.get("game") != audit.game:
        problems.append("game does not match the manifest")
    if document.get("localOnly") is not True:
        problems.append("localOnly must be true")
    if document.get("containsIdentifiers") is not False:
        problems.append("containsIdentifiers must be false")
    if document.get("noCoaching") is not True:
        problems.append("noCoaching must be true")
    allowed_keys = {
        "schema",
        "game",
        "localOnly",
        "containsIdentifiers",
        "participants",
        "noCoaching",
        "goalWithin30Seconds",
        "firstMeaningfulActionWithin45Seconds",
        "resultUnderstood",
        "permanentlyStuck",
        "voluntaryRetries",
    }
    unknown_keys = sorted(set(document) - allowed_keys)
    if unknown_keys:
        problems.append(
            "summary has non-aggregate fields: " + ", ".join(unknown_keys)
        )
    participants = document.get("participants")
    if not isinstance(participants, int) or isinstance(participants, bool) or participants < 5:
        problems.append("participants must be at least 5")
        participants = 5
    minimum_comprehension = math.ceil(participants * 0.8)
    thresholds = (
        "goalWithin30Seconds",
        "firstMeaningfulActionWithin45Seconds",
        "resultUnderstood",
    )
    for key in thresholds:
        value = document.get(key)
        if (
            not isinstance(value, int)
            or isinstance(value, bool)
            or value < minimum_comprehension
            or value > participants
        ):
            problems.append(
                f"{key} must be at least 80% ({minimum_comprehension})"
            )
    stuck = document.get("permanentlyStuck")
    if stuck != 0:
        problems.append("permanentlyStuck must be 0")
    retries = document.get("voluntaryRetries")
    minimum_retries = math.ceil(participants * 0.6)
    if not isinstance(retries, int) or isinstance(retries, bool) or retries < minimum_retries or retries > participants:
        problems.append(f"voluntaryRetries must be at least 60% ({minimum_retries})")
    forbidden_keys = {"name", "names", "email", "emails", "userId", "deviceId"}
    if forbidden_keys & set(document):
        problems.append("summary contains an identifying field")
    if problems:
        audit.add(
            "human-playtest-invalid",
            "; ".join(problems),
            str(path.relative_to(ROOT)),
        )


def _check_runtime_profile(
    audit: GameAudit, game_root: Path, evidence: dict[str, Any]
) -> None:
    path = _inside_game(
        game_root, evidence.get("runtime_profile"),
        "quality.evidence.runtime_profile",
    )
    if path is None or not path.is_file():
        audit.add(
            "runtime-profile-missing",
            "No five-minute SwanSong profile proves CPU headroom, zero missed VBlanks, linked RAM, and peak graphics budgets.",
        )
        return
    try:
        document = _load_json(path, "runtime profile")
    except AuditInputError as exc:
        audit.add("runtime-profile-invalid", str(exc))
        return
    problems: list[str] = []
    if document.get("schema") != PROFILE_SCHEMA:
        problems.append(f"schema must be {PROFILE_SCHEMA}")
    if document.get("game") != audit.game:
        problems.append("game does not match the manifest")
    if document.get("source") != "SwanSong":
        problems.append("source must be SwanSong")
    for key, limit in RUNTIME_PROFILE_LIMITS.items():
        value = document.get(key)
        if not isinstance(value, int) or isinstance(value, bool):
            problems.append(f"{key} must be an integer")
            continue
        if value < 0:
            problems.append(f"{key} must not be negative")
            continue
        if key == "durationFrames":
            if value < limit:
                problems.append(f"durationFrames must be at least {limit}")
        elif value > limit:
            problems.append(f"{key} {value} exceeds {limit}")
    if problems:
        audit.add(
            "runtime-profile-invalid",
            "; ".join(problems),
            str(path.relative_to(ROOT)),
        )


def audit_game(game_root: Path) -> GameAudit:
    slug = game_root.name
    audit = GameAudit(game=slug)
    manifest_path = game_root / "swan.toml"
    try:
        with manifest_path.open("rb") as source:
            manifest = tomllib.load(source)
    except (OSError, tomllib.TOMLDecodeError) as exc:
        audit.fatal = f"could not load {manifest_path}: {exc}"
        return audit
    game = manifest.get("game", {})
    cartridge = manifest.get("cartridge", {})
    budgets = manifest.get("budgets", {})
    resources = manifest.get("resources", {})
    assets = manifest.get("assets", [])
    scenes = manifest.get("scenes", [])
    quality = _quality_table(manifest)
    quality_scenes = quality.get("scenes", {}) if isinstance(quality.get("scenes", {}), dict) else {}
    quality_evidence = quality.get("evidence", {}) if isinstance(quality.get("evidence", {}), dict) else {}
    source_text = _all_source(game_root)

    audit.title = str(game.get("title", slug)) if isinstance(game, dict) else slug
    if not quality or quality.get("standard_version") != 1:
        audit.add(
            "production-declaration-missing",
            "Manifest lacks [quality] standard_version = 1 and its scene, progression, and evidence declarations.",
        )

    scene_ids = {
        item.get("id") for item in scenes
        if isinstance(item, dict) and isinstance(item.get("id"), str)
    } if isinstance(scenes, list) else set()
    initial_scene = game.get("initial_scene") if isinstance(game, dict) else None
    title_scene = quality_scenes.get("title", "title")
    if title_scene not in scene_ids or initial_scene != title_scene:
        audit.add(
            "title-scene-missing",
            f"Fresh boot starts in {initial_scene!r}; strict games require a declared, persistent {title_scene!r} title scene.",
        )
    if "swan_game_intro_complete(" in source_text:
        audit.add(
            "title-auto-advances",
            "Source uses the legacy timed intro helper instead of waiting at the title for a semantic confirm action.",
        )
    for role in ("gameplay", "tutorial", "pause", "result"):
        declared = quality_scenes.get(role, role)
        if not isinstance(declared, str) or declared not in scene_ids:
            audit.add(
                f"{role}-scene-missing",
                f"quality.scenes.{role} does not name a declared scene.",
            )
    attract_frames = quality.get("attract_after_frames")
    if not isinstance(attract_frames, int) or isinstance(attract_frames, bool) or not 604 <= attract_frames <= 906:
        audit.add(
            "attract-mode-missing",
            "No deterministic title attract timeout is declared in the 8–12 second range (604–906 native frames).",
        )

    missing_scenarios = sorted(
        scenario_id for scenario_id in REQUIRED_PLAY_SCENARIOS
        if _scenario(manifest, scenario_id) is None
    )
    if missing_scenarios:
        audit.add(
            "production-scenario-matrix-incomplete",
            "Fresh-boot production scenarios are missing.",
            ", ".join(missing_scenarios),
        )
    semantic_scenarios = set(REQUIRED_OUTCOME_SCENARIOS)
    if _scenario(manifest, "solo") is not None:
        semantic_scenarios.add("solo")
    for scenario_id in sorted(semantic_scenarios):
        scenario = _scenario(manifest, scenario_id)
        if scenario is None:
            continue
        try:
            outcome_path = _inside_game(
                game_root,
                scenario.get("outcome"),
                f"{scenario_id} outcome contract",
            )
            if outcome_path is None or not outcome_path.is_file():
                audit.add(
                    "outcome-contract-missing",
                    f"The {scenario_id} scenario has prose checks but no machine-enforced semantic outcome contract.",
                )
                continue
            outcome = _load_json(outcome_path, f"{scenario_id} outcome contract")
            if outcome.get("schema") != "swan-scenario-outcome-contract-v1":
                audit.add(
                    "outcome-contract-invalid",
                    f"The {scenario_id} outcome contract has the wrong schema.",
                    str(outcome_path.relative_to(ROOT)),
                )
                continue
            final = outcome.get("final")
            audio_outcome = outcome.get("audio")
            semantic_problems: list[str] = []
            if not isinstance(final, dict):
                semantic_problems.append("final must be an object")
            else:
                if not isinstance(final.get("scene"), int) or isinstance(
                    final.get("scene"), bool
                ):
                    semantic_problems.append("final.scene is required")
                if not isinstance(final.get("ending"), int) or isinstance(
                    final.get("ending"), bool
                ):
                    semantic_problems.append("final.ending is required")
                if scenario_id in {"success", "solo", "deterministic"} and not any(
                    key in final
                    for key in ("progress", "progressAtLeast", "stateHash")
                ):
                    semantic_problems.append(
                        "completion must require progress or canonical state"
                    )
            if not isinstance(audio_outcome, dict) or not isinstance(
                audio_outcome.get("markerMask"), int
            ) or isinstance(audio_outcome.get("markerMask"), bool) or audio_outcome.get(
                "markerMask", 0
            ) <= 0:
                semantic_problems.append("audio.markerMask must require the outcome cue")
            if scenario_id == "reset":
                reset = outcome.get("reset")
                if not isinstance(reset, dict) or reset.get("expectation", "any") == "any":
                    semantic_problems.append("reset.expectation must be machine-enforced")
            if semantic_problems:
                audit.add(
                    "outcome-contract-vacuous",
                    f"The {scenario_id} outcome contract does not enforce enough game semantics: "
                    + "; ".join(semantic_problems),
                    str(outcome_path.relative_to(ROOT)),
                )
        except AuditInputError as exc:
            audit.add("outcome-contract-invalid", str(exc))
    audio_scenario = _scenario(manifest, "audio")
    if audio_scenario is not None and not (
        audio_scenario.get("audio") is True
        or audio_scenario.get("audio_expectation") == "audible"
    ):
        audit.add(
            "audio-scenario-invalid",
            "The production audio scenario must require audible SwanSong evidence.",
        )
    if (
        isinstance(cartridge, dict)
        and cartridge.get("rtc") is True
        and _scenario(manifest, "rtc") is None
    ):
        audit.add(
            "rtc-scenario-missing",
            "RTC cartridges require a fixed-time, unavailable, and power-loss scenario.",
        )

    if quality.get("menu_uses_both_clusters") is not True:
        audit.add(
            "menu-both-clusters-unproven",
            "Manifest does not affirm that title, menu, pause, and help navigation accept both X and Y directional clusters.",
        )
    actions = manifest.get("controls", {}).get("actions", {}) if isinstance(manifest.get("controls", {}), dict) else {}
    raw_inputs = {
        raw
        for values in actions.values() if isinstance(actions, dict) and isinstance(values, list)
        for raw in values if isinstance(raw, str)
    } if isinstance(actions, dict) else set()
    if not (any(item.startswith("X") for item in raw_inputs) and any(item.startswith("Y") for item in raw_inputs)):
        audit.add(
            "directional-cluster-mapping-incomplete",
            "Semantic controls do not expose at least one input from both directional clusters.",
        )

    asset_list = [item for item in assets if isinstance(item, dict)] if isinstance(assets, list) else []
    music_assets = [item for item in asset_list if item.get("type") == "music"]
    sfx_assets = [item for item in asset_list if item.get("type") == "sfx"]
    audit.metrics["musicAssets"] = len(music_assets)
    audit.metrics["sfxAssets"] = len(sfx_assets)
    if not music_assets:
        audit.add(
            "background-music-missing",
            "No manifest music asset supplies persistent title/gameplay background music.",
        )
    elif len(music_assets) < 2:
        audit.add(
            "background-music-coverage-low",
            "Fewer than two music assets are declared; strict games need distinct title and gameplay musical treatment.",
        )
    music_hook = quality.get("music_runtime_hook")
    music_hook_present = (
        "swan_audio_play_music(" in source_text
        or (isinstance(music_hook, str) and f"{music_hook}(" in source_text)
    )
    if not music_hook_present:
        audit.add(
            "background-music-hook-missing",
            "Game source never starts an SDK music asset.",
        )
    if not sfx_assets:
        audit.add(
            "sfx-bank-missing",
            "No prioritized SDK SFX assets are declared; shared beeps do not constitute a production feedback bank.",
        )
    elif len(sfx_assets) < 6:
        audit.add(
            "sfx-coverage-low",
            "Fewer than six distinct SFX assets cover navigation, confirmation, action/progress, blocked/error, success, and failure.",
        )
    sfx_hook = quality.get("sfx_runtime_hook")
    sfx_hook_present = (
        "swan_audio_play_sfx(" in source_text
        or (isinstance(sfx_hook, str) and f"{sfx_hook}(" in source_text)
    )
    if not sfx_hook_present:
        audit.add(
            "sfx-hook-missing",
            "Game source never invokes deterministic priority-based SDK SFX playback.",
        )

    save_type = cartridge.get("save_type") if isinstance(cartridge, dict) else None
    save_bytes = cartridge.get("save_bytes") if isinstance(cartridge, dict) else None
    records = quality.get("records")
    if save_type == "none" or not isinstance(save_bytes, int) or save_bytes <= 0:
        audit.add(
            "persistent-progression-missing",
            "Cartridge declares no save medium/payload for options, tutorial completion, records, or unlocks.",
        )
    if not isinstance(records, list) or not records or not all(isinstance(item, str) and item for item in records):
        audit.add(
            "records-declaration-missing",
            "quality.records must name at least one persistent best score, time, ending, or unlock.",
        )
    save_load_hook = quality.get("save_load_hook")
    save_store_hook = quality.get("save_store_hook")
    has_save_load = (
        "swan_save_load(" in source_text
        or (isinstance(save_load_hook, str) and f"{save_load_hook}(" in source_text)
    )
    has_save_store = (
        "swan_save_store(" in source_text
        or (isinstance(save_store_hook, str) and f"{save_store_hook}(" in source_text)
    )
    if not has_save_load or not has_save_store:
        audit.add(
            "save-runtime-hook-missing",
            "Game source does not load and transactionally store its declared progression.",
        )

    success = _scenario(manifest, "success")
    success_frames: int | None = None
    if success is not None:
        try:
            success_path = _inside_game(game_root, success.get("plan"), "success plan")
            if success_path is not None:
                success_document = _load_json(success_path, "success plan")
                raw_frames = success_document.get("totalFrames")
                if isinstance(raw_frames, int) and not isinstance(raw_frames, bool):
                    success_frames = raw_frames
        except AuditInputError as exc:
            audit.add("success-plan-invalid", str(exc))
    if success_frames is None:
        audit.add("success-plan-missing", "No readable success scenario reports totalFrames.")
    else:
        success_seconds = success_frames / NATIVE_REFRESH_HZ
        audit.metrics["scriptedSuccessFrames"] = success_frames
        audit.metrics["scriptedSuccessSeconds"] = round(success_seconds, 2)
        if success_frames < MIN_SUCCESS_FRAMES:
            audit.add(
                "scripted-success-under-one-minute",
                f"The deterministic success route is {success_frames} frames ({success_seconds:.1f}s), below the {MIN_SUCCESS_FRAMES}-frame pacing floor. This is a static pacing signal, not measured human playtime.",
            )

    if isinstance(budgets, dict):
        placeholders = [
            key for key, hard in HARDWARE_BUDGET_MARKERS.items()
            if budgets.get(key) == hard
        ]
        if len(placeholders) == len(HARDWARE_BUDGET_MARKERS):
            audit.add(
                "placeholder-hardware-budgets",
                "Tile, palette, sprite, and scanline budgets repeat hardware maxima instead of measured release ceilings.",
                ", ".join(f"{key}={budgets[key]}" for key in placeholders),
            )
        exceeded = [
            f"{key}={budgets.get(key)}>{maximum}"
            for key, maximum in PRODUCTION_MAXIMUMS.items()
            if isinstance(budgets.get(key), int) and budgets[key] > maximum
        ]
        missing = [
            key for key in PRODUCTION_MAXIMUMS
            if not isinstance(budgets.get(key), int)
            or isinstance(budgets.get(key), bool)
        ]
        non_positive = [
            key for key in PRODUCTION_MAXIMUMS
            if isinstance(budgets.get(key), int)
            and not isinstance(budgets.get(key), bool)
            and budgets[key] <= 0
        ]
        if exceeded or missing or non_positive:
            details = (
                exceeded
                + [f"{key}=missing" for key in missing]
                + [f"{key}=non-positive" for key in non_positive]
            )
            audit.add(
                "production-budget-invalid",
                "Manifest budgets exceed the anthology production envelope or are absent.",
                ", ".join(details),
            )
    else:
        audit.add("production-budget-invalid", "Manifest has no [budgets] table.")

    if not isinstance(resources, dict):
        resources = {}
    sprites = resources.get("sprites")
    scanline = resources.get("sprites_per_scanline")
    if not isinstance(sprites, int) or sprites <= 0 or not isinstance(scanline, int) or scanline <= 0:
        audit.add(
            "sprites-not-declared",
            "Scene resources declare no visible sprites/scanline pressure, matching the current static-background presentation.",
        )
    if "swan_gfx_set_sprite(" not in source_text:
        audit.add(
            "sprite-runtime-hook-missing",
            "Game source never submits a hardware sprite.",
        )
    resource_keys = (
        "work_ram_bytes",
        "vram_tiles",
        "palettes",
        "sprites",
        "sprites_per_scanline",
    )
    resource_missing = [
        key for key in resource_keys
        if not isinstance(resources.get(key), int)
        or isinstance(resources.get(key), bool)
    ]
    resource_non_positive = [
        key for key in resource_keys
        if isinstance(resources.get(key), int)
        and not isinstance(resources.get(key), bool)
        and resources[key] <= 0
    ]
    if resource_missing or resource_non_positive:
        details = resource_missing + [
            f"{key}=non-positive" for key in resource_non_positive
        ]
        audit.add(
            "resource-declaration-incomplete",
            "[resources] omits positive measured game-owned capacities.",
            ", ".join(details),
        )
    layer_two_patterns = (
        r"swan_gfx_set_layer_enabled\s*\(\s*1\s*,\s*true",
        r"swan_gfx_(?:put_tile|fill|set_camera|set_scroll)\s*\(\s*1\s*,",
    )
    if not any(re.search(pattern, source_text) for pattern in layer_two_patterns):
        audit.add(
            "second-background-unused",
            "Game source does not enable or draw the second 32x32 hardware tile layer for HUD, depth, overlay, or transition work.",
        )

    conventional_gameplay = ROOT / "docs" / "qa" / "native-frames" / f"{slug.replace('-', '_')}.png"
    orientation = game.get("orientation") if isinstance(game, dict) else "horizontal"
    native_dimensions = (
        (PHYSICAL_HEIGHT, PHYSICAL_WIDTH)
        if orientation == "vertical"
        else (PHYSICAL_WIDTH, PHYSICAL_HEIGHT)
    )
    title_png = _check_native_png(
        audit, game_root, quality_evidence, "title_png",
        "title-evidence-missing", "title", native_dimensions,
    )
    gameplay_png = _check_native_png(
        audit, game_root, quality_evidence, "gameplay_png",
        "gameplay-evidence-missing", "gameplay", native_dimensions,
        conventional=conventional_gameplay,
    )
    result_png = _check_native_png(
        audit, game_root, quality_evidence, "result_png",
        "result-evidence-missing", "result", native_dimensions,
    )
    audio_wav = _inside_game(
        game_root,
        quality_evidence.get("audio_wav"),
        "quality.evidence.audio_wav",
    )
    if audio_wav is None or not audio_wav.is_file():
        audit.add(
            "audio-evidence-missing",
            "Missing decoded SwanSong WAV evidence for the production music and effect mix.",
        )
        audio_wav = None
    elif not _is_wave_file(audio_wav):
        audit.add(
            "audio-evidence-invalid",
            "Audio evidence is not a RIFF/WAVE file.",
            str(audio_wav.relative_to(ROOT)),
        )
        audio_wav = None
    _check_media_observation(
        audit, game_root, quality_evidence,
        {"title": title_png, "gameplay": gameplay_png, "result": result_png},
        audio_wav,
    )
    _check_runtime_profile(audit, game_root, quality_evidence)
    _check_human_playtest(audit, game_root, quality_evidence)
    return audit


def _discover_games(selected: list[str]) -> list[Path]:
    available = {path.name: path for path in GAMES_ROOT.iterdir() if (path / "swan.toml").is_file()}
    if not selected:
        games = sorted(available.values())
    else:
        unknown = sorted(set(selected) - set(available))
        if unknown:
            raise AuditInputError(f"unknown game(s): {', '.join(unknown)}")
        games = [available[name] for name in selected]
    missing_originals = sorted(REQUIRED_ORIGINALS - set(available))
    if not selected and missing_originals:
        raise AuditInputError(
            "missing required SwanSong Original(s): " + ", ".join(missing_originals)
        )
    return games


def _human_report(audits: list[GameAudit], mode: str) -> None:
    print(f"SwanSong Originals game-quality audit ({mode})")
    print(
        "Hardware envelope: 224x144 (28x18 tiles), 75.472 Hz, "
        "40,704 cycles/frame, four native audio channels"
    )
    for audit in audits:
        if audit.fatal is not None:
            print(f"FATAL {audit.game}: {audit.fatal}")
            continue
        verdict = "READY" if audit.strict_ready else "BLOCKED"
        success = audit.metrics.get("scriptedSuccessSeconds")
        suffix = f", scripted success {success:.1f}s" if isinstance(success, float) else ""
        print(
            f"{verdict:7} {audit.game}: {len(audit.findings)} blocker(s), "
            f"{audit.metrics.get('musicAssets', 0)} music, "
            f"{audit.metrics.get('sfxAssets', 0)} SFX{suffix}"
        )
        for finding in audit.findings:
            detail = f" [{finding.evidence}]" if finding.evidence else ""
            print(f"  - {finding.code}: {finding.message}{detail}")
    fatals = sum(audit.fatal is not None for audit in audits)
    blockers = sum(len(audit.findings) for audit in audits)
    ready = sum(audit.strict_ready for audit in audits)
    print(
        f"SUMMARY {ready}/{len(audits)} strict-ready; "
        f"{blockers} production blocker(s); {fatals} fatal audit error(s)"
    )
    if mode != "strict":
        print(
            "REPORT ONLY: blockers document the rebuild baseline and do not fail this mode."
        )
    print(
        "EVIDENCE BOUNDARY: static checks and emulator artifacts cannot prove fun, "
        "musical appeal, physical-display readability, or first-time comprehension; "
        "strict mode requires a separate local, identifier-free human-playtest summary."
    )


def _json_report(audits: list[GameAudit], mode: str) -> None:
    payload = {
        "schema": "swan-song-game-quality-audit-v1",
        "mode": mode,
        "hardware": {
            "displayPixels": [PHYSICAL_WIDTH, PHYSICAL_HEIGHT],
            "visibleTiles": [28, 18],
            "refreshHz": NATIVE_REFRESH_HZ,
            "cpuHz": 3_072_000,
            "cyclesPerFrame": 40_704,
            "audioChannels": 4,
        },
        "games": [
            {
                **asdict(audit),
                "strictReady": audit.strict_ready,
            }
            for audit in audits
        ],
        "summary": {
            "games": len(audits),
            "strictReady": sum(audit.strict_ready for audit in audits),
            "blockers": sum(len(audit.findings) for audit in audits),
            "fatalErrors": sum(audit.fatal is not None for audit in audits),
        },
        "evidenceBoundary": (
            "Static inspection and SwanSong evidence do not establish fun, musical appeal, "
            "physical-hardware readability, or uncoached comprehension."
        ),
    }
    print(json.dumps(payload, indent=2, sort_keys=True))


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--mode", choices=("report", "baseline", "strict"), default="report",
        help="report/baseline inventory blockers; strict fails when any blocker remains",
    )
    parser.add_argument(
        "--game", action="append", default=[],
        help="audit only this game slug; repeat for multiple games",
    )
    parser.add_argument("--json", action="store_true", help="emit canonical structured output")
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(sys.argv[1:] if argv is None else argv)
    mode = "report" if args.mode == "baseline" else args.mode
    try:
        games = _discover_games(args.game)
    except (AuditInputError, OSError) as exc:
        print(f"ERROR {exc}", file=sys.stderr)
        return 2
    audits: list[GameAudit] = []
    for game in games:
        try:
            audits.append(audit_game(game))
        except (AuditInputError, OSError) as exc:
            audits.append(GameAudit(game=game.name, fatal=str(exc)))
    if args.json:
        _json_report(audits, mode)
    else:
        _human_report(audits, mode)
    if any(audit.fatal is not None for audit in audits):
        return 2
    if mode == "strict" and any(not audit.strict_ready for audit in audits):
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
