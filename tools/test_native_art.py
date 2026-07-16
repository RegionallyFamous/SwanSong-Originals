#!/usr/bin/env python3
"""Check every ROM's Imagegen master, native proof, tile banks, and renderer hook."""

from __future__ import annotations

from pathlib import Path
import hashlib
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

        header = header_path.read_text()
        main_source = main_path.read_text()
        gfx_source = gfx_path.read_text()
        source_sha = hashlib.sha256(master.read_bytes()).hexdigest()
        assert f"Imagegen source SHA-256: {source_sha}" in header

        if slug == "orbital-courier":
            intro_tiles_name = "orbital_intro_tiles"
            intro_map_name = "orbital_intro_map"
            game_tiles_name = "orbital_game_tiles"
            palette_name = "orbital_palette"
            intro_hook = "orbital_gfx_show_intro();"
            render_hook = "orbital_gfx_render("
        else:
            intro_tiles_name = "game_intro_tiles"
            intro_map_name = "game_intro_map"
            game_tiles_name = "game_tiles"
            palette_name = "game_palette"
            intro_hook = "gfx_show_intro();"
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

        assert '#include "gfx.h"' in main_source
        assert intro_hook in main_source
        assert render_hook in main_source
        assert '#include "gameplay_art.h"' in gfx_source
        for terminal_call in (
            "#include <stdio.h>", "printf(", "putchar(", "rf_header(",
            "rf_footer(", "rf_clear(", "RF_LOAD_NATIVE_ART",
        ):
            assert terminal_call not in main_source, f"terminal UI in {main_path}: {terminal_call}"

    print("OK   ten Imagegen masters, native proofs, tile banks, and graphical renderers")


if __name__ == "__main__":
    main()
