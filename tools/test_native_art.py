#!/usr/bin/env python3
"""Check that every ROM ships an approved, cartridge-native art stamp."""

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
    provenance = (ROOT / "docs" / "art" / "source-plates.md").read_text()
    for slug in GAMES:
        source = ROOT / "docs" / "art" / "source-plates" / f"{slug}.png"
        preview = ROOT / "docs" / "art" / "native" / f"{slug}.png"
        header_path = ROOT / "games" / slug / "src" / "native_art.h"
        main_path = ROOT / "games" / slug / "src" / "main.c"
        assert source.is_file() and min(png_size(source)) >= 1000
        assert preview.is_file()
        header = header_path.read_text()
        main_source = main_path.read_text()
        source_sha = hashlib.sha256(source.read_bytes()).hexdigest()
        assert f"source SHA-256: {source_sha}" in header
        assert slug.replace("-", " ") in provenance.lower()
        assert '#include "native_art.h"' in main_source
        assert "RF_LOAD_NATIVE_ART();" in main_source

        cols = int(re.search(r"#define NATIVE_ART_COLS (\d+)", header).group(1))
        rows = int(re.search(r"#define NATIVE_ART_ROWS (\d+)", header).group(1))
        screen_x = int(re.search(r"#define NATIVE_ART_SCREEN_X (\d+)", header).group(1))
        screen_y = int(re.search(r"#define NATIVE_ART_SCREEN_Y (\d+)", header).group(1))
        assert png_size(preview) == (cols * 32, rows * 32)
        assert screen_x + cols <= 28 and screen_y + rows <= 18

        tiles = array_values(header, "native_art_tiles")
        tilemap = array_values(header, "native_art_map")
        palette = array_values(header, "native_art_palette")
        assert len(tiles) % 16 == 0 and len(tiles) > 0
        assert len(tilemap) == cols * rows
        assert len(palette) == 4 and len(set(palette)) == 4
        assert max(value & 0x01FF for value in tilemap) < 392

    combined = "\n".join(
        (ROOT / "games" / slug / "src" / "main.c").read_text() for slug in GAMES
    )
    for obsolete in ("[o_o]", "[?_?]", "[x_x]", ".--/\\--."):
        assert obsolete not in combined, f"obsolete ASCII stand-in remains: {obsolete}"
    print("OK   ten approved source plates, native tilemaps, palettes, and ROM hooks")


if __name__ == "__main__":
    main()
