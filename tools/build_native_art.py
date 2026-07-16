#!/usr/bin/env python3
"""Convert approved source plates into cartridge-native 2BPP art stamps.

The generated headers are deliberately checked in so builds stay independent of
Pillow. Re-run this tool only when an approved source plate changes.
"""

from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
import hashlib
import math

from PIL import Image, ImageDraw, ImageFont


ROOT = Path(__file__).resolve().parents[1]
SOURCE_DIR = ROOT / "docs" / "art" / "source-plates"
PREVIEW_DIR = ROOT / "docs" / "art" / "native"
CONTACT_SHEET = ROOT / "docs" / "art" / "native-contact-sheet.png"

CREAM = (0xF5, 0xF0, 0xE6)
INK = (0x1A, 0x1A, 0x1A)
PINK = (0xFF, 0x6B, 0x9D)
YELLOW = (0xFF, 0xE5, 0x66)
BLUE = (0xA8, 0xD5, 0xE5)
PALE_PINK = (0xF4, 0xC7, 0xC3)


@dataclass(frozen=True)
class ArtSpec:
    slug: str
    palette: tuple[tuple[int, int, int], ...]
    cols: int
    rows: int
    screen_x: int
    screen_y: int


SPECS = (
    ArtSpec("mote-sound-terminal", (CREAM, INK, PINK, BLUE), 10, 9, 18, 4),
    ArtSpec("scrapframe-garage", (CREAM, INK, YELLOW, BLUE), 10, 9, 18, 4),
    ArtSpec("radio-ghost", (CREAM, INK, PALE_PINK, PINK), 10, 9, 18, 4),
    ArtSpec("harpoon-moon", (CREAM, INK, YELLOW, PALE_PINK), 7, 9, 21, 4),
    ArtSpec("turncoat-tactics", (CREAM, INK, BLUE, PINK), 12, 9, 16, 4),
    ArtSpec("pocket-kaiju-observatory", (CREAM, INK, PALE_PINK, BLUE), 10, 9, 18, 4),
    ArtSpec("rotate-dungeon", (CREAM, INK, YELLOW, BLUE), 12, 9, 16, 4),
    ArtSpec("one-last-lap", (CREAM, INK, PINK, YELLOW), 10, 9, 18, 4),
    ArtSpec("bug-witch", (CREAM, INK, PINK, BLUE), 9, 9, 19, 4),
)


def color_distance(a: tuple[int, int, int], b: tuple[int, int, int]) -> float:
    return math.sqrt(sum((int(x) - int(y)) ** 2 for x, y in zip(a, b)))


def foreground_crop(image: Image.Image) -> Image.Image:
    rgb = image.convert("RGB")
    pixels = rgb.load()
    xs: list[int] = []
    ys: list[int] = []
    for y in range(rgb.height):
        for x in range(rgb.width):
            if color_distance(pixels[x, y], CREAM) >= 52:
                xs.append(x)
                ys.append(y)
    if not xs:
        return rgb
    pad_x = max(8, int((max(xs) - min(xs)) * 0.025))
    pad_y = max(8, int((max(ys) - min(ys)) * 0.025))
    box = (
        max(0, min(xs) - pad_x),
        max(0, min(ys) - pad_y),
        min(rgb.width, max(xs) + pad_x + 1),
        min(rgb.height, max(ys) + pad_y + 1),
    )
    return rgb.crop(box)


def nearest_palette(image: Image.Image, palette: tuple[tuple[int, int, int], ...]) -> Image.Image:
    out = Image.new("P", image.size)
    flat_palette = [component for color in palette for component in color]
    out.putpalette(flat_palette + [0] * (768 - len(flat_palette)))
    src = image.convert("RGB")
    data = []
    for pixel in src.get_flattened_data():
        data.append(min(range(4), key=lambda i: color_distance(pixel, palette[i])))
    out.putdata(data)
    return out


def make_native_image(spec: ArtSpec) -> Image.Image:
    source = Image.open(SOURCE_DIR / f"{spec.slug}.png")
    crop = foreground_crop(source)
    target = (spec.cols * 8, spec.rows * 8)
    scale = min(target[0] / crop.width, target[1] / crop.height)
    resized = crop.resize(
        (max(1, round(crop.width * scale)), max(1, round(crop.height * scale))),
        Image.Resampling.LANCZOS,
    )
    canvas = Image.new("RGB", target, CREAM)
    canvas.paste(resized, ((target[0] - resized.width) // 2, (target[1] - resized.height) // 2))
    return nearest_palette(canvas, spec.palette)


def flip_h(tile: tuple[int, ...]) -> tuple[int, ...]:
    return tuple(tile[y * 8 + (7 - x)] for y in range(8) for x in range(8))


def flip_v(tile: tuple[int, ...]) -> tuple[int, ...]:
    return tuple(tile[(7 - y) * 8 + x] for y in range(8) for x in range(8))


def pack_tile(tile: tuple[int, ...]) -> bytes:
    packed = bytearray()
    for y in range(8):
        plane_0 = 0
        plane_1 = 0
        for x in range(8):
            value = tile[y * 8 + x]
            bit = 7 - x
            plane_0 |= (value & 1) << bit
            plane_1 |= ((value >> 1) & 1) << bit
        packed.extend((plane_0, plane_1))
    return bytes(packed)


def tile_data(image: Image.Image) -> tuple[bytes, list[int]]:
    pixels = tuple(image.get_flattened_data())
    tiles: list[tuple[int, ...]] = []
    tile_lookup: dict[tuple[int, ...], tuple[int, int]] = {}
    tilemap: list[int] = []
    blank = (0,) * 64

    for tile_y in range(image.height // 8):
        for tile_x in range(image.width // 8):
            tile = tuple(
                pixels[(tile_y * 8 + y) * image.width + tile_x * 8 + x]
                for y in range(8)
                for x in range(8)
            )
            if tile == blank:
                tilemap.append(0)
                continue
            match = tile_lookup.get(tile)
            if match is None:
                index = len(tiles) + 1
                tiles.append(tile)
                variants = (
                    (tile, 0),
                    (flip_h(tile), 0x4000),
                    (flip_v(tile), 0x8000),
                    (flip_v(flip_h(tile)), 0xC000),
                )
                for variant, flags in variants:
                    tile_lookup.setdefault(variant, (index, flags))
                match = (index, 0)
            tilemap.append(match[0] | match[1] | 0x0200)

    return b"".join(pack_tile(tile) for tile in tiles), tilemap


def c_bytes(data: bytes, width: int = 12) -> str:
    chunks = [data[i : i + width] for i in range(0, len(data), width)]
    return "\n".join("\t" + ", ".join(f"0x{value:02X}" for value in chunk) + "," for chunk in chunks)


def c_words(data: list[int], width: int = 8) -> str:
    chunks = [data[i : i + width] for i in range(0, len(data), width)]
    return "\n".join("\t" + ", ".join(f"0x{value:04X}" for value in chunk) + "," for chunk in chunks)


def rgb12(color: tuple[int, int, int]) -> int:
    r, g, b = (round(component * 15 / 255) for component in color)
    return (r << 8) | (g << 4) | b


def write_header(spec: ArtSpec, image: Image.Image) -> None:
    tiles, tilemap = tile_data(image)
    palette = [rgb12(color) for color in spec.palette]
    sha256 = hashlib.sha256((SOURCE_DIR / f"{spec.slug}.png").read_bytes()).hexdigest()
    guard = f"SWANSONG_{spec.slug.upper().replace('-', '_')}_NATIVE_ART_H"
    output = ROOT / "games" / spec.slug / "src" / "native_art.h"
    output.write_text(
        f"""/* Generated by tools/build_native_art.py; source SHA-256: {sha256} */
#ifndef {guard}
#define {guard}

#include <stdint.h>
#include <wonderful.h>

static const uint16_t __far native_art_palette[] = {{
{c_words(palette, 4)}
}};

static const uint8_t __far native_art_tiles[] = {{
{c_bytes(tiles)}
}};

static const uint16_t __far native_art_map[] = {{
{c_words(tilemap)}
}};

#define NATIVE_ART_COLS {spec.cols}
#define NATIVE_ART_ROWS {spec.rows}
#define NATIVE_ART_SCREEN_X {spec.screen_x}
#define NATIVE_ART_SCREEN_Y {spec.screen_y}

#define RF_LOAD_NATIVE_ART() \
	rf_art_load(native_art_tiles, sizeof(native_art_tiles), native_art_map, \
		NATIVE_ART_COLS, NATIVE_ART_ROWS, NATIVE_ART_SCREEN_X, \
		NATIVE_ART_SCREEN_Y, native_art_palette)

#endif
""",
        encoding="utf-8",
    )


def build_contact_sheet() -> None:
    paths = sorted(PREVIEW_DIR.glob("*.png"))
    cell_width = 475
    cell_height = 326
    margin = 18
    art_width = cell_width - margin * 2
    art_height = 270
    canvas = Image.new("RGB", (cell_width * 2, cell_height * 5), CREAM)
    draw = ImageDraw.Draw(canvas)
    try:
        font = ImageFont.truetype("DejaVuSans.ttf", 22)
    except OSError:
        font = ImageFont.load_default()
    for index, path in enumerate(paths):
        image = Image.open(path).convert("RGB")
        scale = min(1.0, art_width / image.width, art_height / image.height)
        if scale < 1.0:
            image = image.resize(
                (round(image.width * scale), round(image.height * scale)),
                Image.Resampling.NEAREST,
            )
        cell_x = (index % 2) * cell_width
        cell_y = (index // 2) * cell_height
        image_x = cell_x + (cell_width - image.width) // 2
        image_y = cell_y + margin + (art_height - image.height) // 2
        canvas.paste(image, (image_x, image_y))
        draw.text((cell_x + margin, cell_y + 294), path.stem, fill=INK, font=font)
    canvas.save(CONTACT_SHEET)


def main() -> None:
    PREVIEW_DIR.mkdir(parents=True, exist_ok=True)
    for spec in SPECS:
        image = make_native_image(spec)
        preview = image.convert("RGB").resize(
            (spec.cols * 32, spec.rows * 32), Image.Resampling.NEAREST
        )
        preview.save(PREVIEW_DIR / f"{spec.slug}.png")
        write_header(spec, image)
        print(f"{spec.slug}: {spec.cols}x{spec.rows} tiles")
    build_contact_sheet()


if __name__ == "__main__":
    main()
