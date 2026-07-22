#!/usr/bin/env python3
"""Check every ROM's Imagegen master, native proof, tile banks, and renderer hook."""

from __future__ import annotations

from pathlib import Path
import hashlib
import json
import re
import struct


ROOT = Path(__file__).resolve().parents[1]
GAMES = (
    "mote-sound-terminal",
    "orbital-courier",
    "scrapframe-garage",
    "radio-ghost",
    "harpoon-moon",
    "turncoat-tactics",
    "pocket-kaiju-observatory",
    "rotate-dungeon",
    "one-last-lap",
    "bug-witch",
)
SDK_NATIVE_GAMES = {"mote-sound-terminal", "orbital-courier"}


def png_size(path: Path) -> tuple[int, int]:
    data = path.read_bytes()[:24]
    assert data[:8] == b"\x89PNG\r\n\x1a\n", f"not a PNG: {path}"
    return struct.unpack(">II", data[16:24])


def array_values(header: str, name: str) -> list[int]:
    match = re.search(
        rf"{name}\[\]\s*=\s*\{{(?P<body>.*?)\}};", header, re.DOTALL
    )
    assert match, f"missing {name} array"
    return [int(value, 16) for value in re.findall(r"0x([0-9A-Fa-f]+)", match["body"])]


def main() -> None:
    contract = (ROOT / "artist-personas" / "yohaku" / "contract.json").read_text()
    assert '"status": "stable"' in contract
    for slug in GAMES:
        art_dir = ROOT / "docs" / "art" / "full-screen"
        master = art_dir / f"{slug}-gameplay-master.png"
        native = art_dir / f"{slug}-gameplay-native.png"
        atlas = art_dir / f"{slug}-gameplay-atlas.png"
        header_path = ROOT / "games" / slug / "src" / "gameplay_art.h"
        main_path = ROOT / "games" / slug / "src" / "main.c"
        gfx_path = ROOT / "games" / slug / "src" / "gfx.c"

        assert master.is_file() and min(png_size(master)) >= 1000
        assert png_size(native) == (224, 144)
        assert atlas.is_file()
        assert not (ROOT / "games" / slug / "src" / "native_art.h").exists()

        main_source = main_path.read_text()
        gfx_source = gfx_path.read_text()
        source_sha = hashlib.sha256(master.read_bytes()).hexdigest()

        if slug in SDK_NATIVE_GAMES:
            game_root = ROOT / "games" / slug
            intro_asset = game_root / "assets" / "graphics" / "intro.png"
            gameplay_asset = game_root / "assets" / "graphics" / "gameplay.png"
            report = json.loads(
                (game_root / "build" / "generated" / "asset-report.json").read_text()
            )

            assert not header_path.exists()
            assert png_size(intro_asset) == (224, 144)
            assert intro_asset.read_bytes() == native.read_bytes()
            expected_size = (112, 40) if slug == "mote-sound-terminal" else (128, 64)
            expected_assets = (
                ["intro", "gameplay", "track_0", "track_1", "track_2"]
                if slug == "mote-sound-terminal" else ["intro", "gameplay"]
            )
            assert png_size(gameplay_asset) == expected_size
            assert [asset["id"] for asset in report["assets"]] == expected_assets
            graphic_assets = [asset for asset in report["assets"]
                              if asset["type"] not in {"music", "sfx"}]
            assert all(asset["converter"] == "wonderful-superfamiconv"
                       for asset in graphic_assets)
            assert report["uniqueTiles"] > 0 and report["generatedTileBytes"] > 0
            scene_usage = {item["scene"]: item for item in report["sceneUsage"]}
            assert scene_usage["intro"]["vramTiles"] > 0
            gameplay_scene = "terminal" if slug == "mote-sound-terminal" else "delivery"
            assert scene_usage[gameplay_scene]["vramTiles"] > 0
            if slug == "orbital-courier":
                assert scene_usage["result"]["vramTiles"] == scene_usage["delivery"]["vramTiles"]
                assert scene_usage["result"]["palettes"] == scene_usage["delivery"]["palettes"]
            else:
                assert report["audioBytes"] > 0
                assert all(asset["converter"] == "swansong-toml-audio"
                           for asset in report["assets"] if asset["type"] == "music")
                assert "swan_game_audio_" not in main_source
                assert "swan_audio_play_music(&active_song)" in main_source
                assert "swan_asset_track_0_rows" in main_source
            assert '"Imagegen source SHA-256: ' not in gfx_source
            assert "#include <swan/legacy.h>" not in gfx_source
            assert '#include "gameplay_art.h"' not in gfx_source
            assert '#include "swan_assets.h"' in gfx_source
            assert "swan_asset_intro_tiles" in gfx_source
            assert "swan_asset_gameplay_tiles" in gfx_source
            intro_hook = (
                "gfx_show_intro();" if slug == "mote-sound-terminal"
                else "orbital_gfx_show_intro();"
            )
            render_hook = (
                "gfx_render(" if slug == "mote-sound-terminal"
                else "orbital_gfx_render("
            )
        else:
            header = header_path.read_text()
            assert f"Imagegen source SHA-256: {source_sha}" in header
            intro_tiles_name = "game_intro_tiles"
            intro_map_name = "game_intro_map"
            game_tiles_name = "game_tiles"
            palette_name = "game_palette"
            intro_hook = (
                "gfx_show_intro(title_prompt);" if slug == "one-last-lap"
                else "gfx_show_intro();"
            )
            render_hook = "gfx_render("

            intro_tiles = array_values(header, intro_tiles_name)
            intro_map = array_values(header, intro_map_name)
            game_tiles = array_values(header, game_tiles_name)
            palette = array_values(header, palette_name)
            assert len(intro_tiles) % 16 == 0 and 0 < len(intro_tiles) // 16 <= 511
            assert len(intro_map) == 28 * 18
            assert max(value & 0x01FF for value in intro_map) <= len(intro_tiles) // 16
            assert len(game_tiles) % 16 == 0 and 0 < len(game_tiles) // 16 <= 511
            assert len(palette) == 4 and len(set(palette)) == 4
            assert '#include "gameplay_art.h"' in gfx_source

        assert '#include "gfx.h"' in main_source
        assert intro_hook in main_source
        assert render_hook in main_source
        for terminal_call in (
            "#include <stdio.h>", "printf(", "putchar(", "rf_header(",
            "rf_footer(", "rf_clear(", "RF_LOAD_NATIVE_ART",
        ):
            assert terminal_call not in main_source, f"terminal UI in {main_path}: {terminal_call}"

    print("OK   ten Imagegen masters, native proofs, tile banks, and graphical renderers")


if __name__ == "__main__":
    main()
