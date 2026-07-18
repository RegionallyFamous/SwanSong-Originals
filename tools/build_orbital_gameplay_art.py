#!/usr/bin/env python3
"""Build Orbital Courier's full-screen native art from its Imagegen master."""

from __future__ import annotations

from pathlib import Path
import hashlib
import math

from PIL import Image, ImageDraw


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "docs" / "art" / "full-screen" / "orbital-courier-gameplay-master.png"
NATIVE = ROOT / "docs" / "art" / "full-screen" / "orbital-courier-gameplay-native.png"
ATLAS = ROOT / "docs" / "art" / "full-screen" / "orbital-courier-gameplay-atlas.png"
LEGACY_PREVIEW = ROOT / "docs" / "art" / "native" / "orbital-courier.png"
INTRO_ASSET = ROOT / "games" / "orbital-courier" / "assets" / "graphics" / "intro.png"
GAMEPLAY_ASSET = ROOT / "games" / "orbital-courier" / "assets" / "graphics" / "gameplay.png"

CREAM = (0xF5, 0xF0, 0xE6)
INK = (0x1A, 0x1A, 0x1A)
YELLOW = (0xFF, 0xE5, 0x66)
PINK = (0xFF, 0x6B, 0x9D)
PALETTE = (CREAM, INK, YELLOW, PINK)


def color_distance(a: tuple[int, int, int], b: tuple[int, int, int]) -> float:
	return math.sqrt(sum((int(x) - int(y)) ** 2 for x, y in zip(a, b)))


def quantize_exact(image: Image.Image) -> Image.Image:
	rgb = image.convert("RGB")
	out = Image.new("P", rgb.size)
	out.putpalette([value for color in PALETTE for value in color] + [0] * (768 - 12))
	out.putdata([
		min(range(4), key=lambda index: color_distance(pixel, PALETTE[index]))
		for pixel in rgb.get_flattened_data()
	])
	return out


def native_master() -> Image.Image:
	source = Image.open(SOURCE).convert("RGB")
	target_ratio = 224 / 144
	current_ratio = source.width / source.height
	if current_ratio > target_ratio:
		width = round(source.height * target_ratio)
		left = (source.width - width) // 2
		source = source.crop((left, 0, left + width, source.height))
	else:
		height = round(source.width / target_ratio)
		top = (source.height - height) // 2
		source = source.crop((0, top, source.width, top + height))
	source = source.resize((224, 144), Image.Resampling.LANCZOS)
	return quantize_exact(source)


def image16(fill: int = 0) -> Image.Image:
	image = Image.new("P", (16, 16), fill)
	image.putpalette([value for color in PALETTE for value in color] + [0] * (768 - 12))
	return image


def floor_meta(variant: int = 0) -> Image.Image:
	image = image16(0)
	draw = ImageDraw.Draw(image)
	for x in range(0, 16, 2):
		draw.point((x, 0), fill=1)
	for y in range(0, 16, 2):
		draw.point((0, y), fill=1)
	draw.point((12 if variant else 4, 12), fill=2)
	if variant:
		draw.point((5, 5), fill=3)
	return image


def wall_meta(outer: bool = False) -> Image.Image:
	image = image16(1)
	draw = ImageDraw.Draw(image)
	draw.rectangle((0, 0, 15, 2), fill=3)
	draw.line((0, 3, 15, 3), fill=0)
	for x in (3, 12):
		draw.rectangle((x, 7, x + 1, 8), fill=2)
	if outer:
		draw.line((0, 14, 15, 14), fill=2)
		for x, y, color in ((2, 11, 3), (7, 5, 0), (13, 12, 2)):
			draw.point((x, y), fill=color)
	return image


def parcel_meta() -> Image.Image:
	image = floor_meta(1)
	draw = ImageDraw.Draw(image)
	draw.rectangle((3, 4, 12, 12), fill=1)
	draw.polygon(((4, 5), (8, 2), (12, 5), (8, 8)), fill=3)
	draw.polygon(((4, 5), (8, 8), (8, 13), (4, 10)), fill=3)
	draw.polygon(((8, 8), (12, 5), (12, 10), (8, 13)), fill=2)
	draw.line((4, 5, 8, 8, 12, 5), fill=1)
	return image


def depot_meta() -> Image.Image:
	image = floor_meta(0)
	draw = ImageDraw.Draw(image)
	draw.ellipse((1, 1, 14, 14), fill=1)
	draw.ellipse((3, 3, 12, 12), fill=2)
	draw.ellipse((5, 5, 10, 10), fill=3)
	draw.rectangle((7, 4, 8, 11), fill=1)
	draw.rectangle((4, 7, 11, 8), fill=1)
	return image


def courier_meta(carrying: bool, step: bool) -> Image.Image:
	image = floor_meta(1 if step else 0)
	draw = ImageDraw.Draw(image)
	if carrying:
		draw.rectangle((10, 6, 13, 11), fill=3)
		draw.rectangle((11, 7, 12, 8), fill=2)
	draw.rectangle((6, 4, 10, 10), fill=2)
	draw.rectangle((5, 5, 6, 10), fill=1)
	draw.rectangle((7, 1, 10, 4), fill=0)
	draw.rectangle((6, 2, 11, 3), fill=1)
	draw.rectangle((9, 4, 11, 5), fill=1)
	draw.line((7, 11, 6 if step else 7, 14), fill=1, width=2)
	draw.line((9, 11, 11 if step else 10, 14), fill=1, width=2)
	draw.point((10, 3), fill=3)
	return image


def hud_tile(kind: str) -> Image.Image:
	image = Image.new("P", (8, 8), 1)
	image.putpalette([value for color in PALETTE for value in color] + [0] * (768 - 12))
	draw = ImageDraw.Draw(image)
	if kind == "bg":
		draw.point((6, 2), fill=3)
	elif kind == "fuel_full":
		draw.polygon(((3, 0), (7, 3), (4, 7), (0, 4)), fill=2)
		draw.point((4, 2), fill=0)
	elif kind == "fuel_empty":
		draw.line(((3, 0), (7, 3), (4, 7), (0, 4), (3, 0)), fill=0)
	elif kind == "route_on":
		draw.rectangle((2, 2, 5, 5), fill=3)
	elif kind == "route_off":
		draw.rectangle((2, 2, 5, 5), outline=0)
	return image


def cargo_hud(loaded: bool) -> Image.Image:
	image = image16(1)
	draw = ImageDraw.Draw(image)
	draw.rectangle((1, 1, 14, 14), outline=0)
	if loaded:
		draw.polygon(((5, 5), (8, 3), (11, 5), (8, 7)), fill=3)
		draw.polygon(((5, 5), (8, 7), (8, 12), (5, 9)), fill=3)
		draw.polygon(((8, 7), (11, 5), (11, 9), (8, 12)), fill=2)
	else:
		draw.rectangle((5, 5, 10, 10), outline=3)
	return image


def target_hud() -> Image.Image:
	image = image16(1)
	draw = ImageDraw.Draw(image)
	draw.ellipse((2, 2, 13, 13), fill=2)
	draw.ellipse((5, 5, 10, 10), fill=3)
	draw.rectangle((7, 3, 8, 12), fill=1)
	draw.rectangle((3, 7, 12, 8), fill=1)
	return image


def result_icon(won: bool) -> Image.Image:
	image = Image.new("P", (32, 32), 1)
	image.putpalette([value for color in PALETTE for value in color] + [0] * (768 - 12))
	draw = ImageDraw.Draw(image)
	draw.rectangle((0, 0, 31, 31), outline=3, width=2)
	if won:
		draw.ellipse((6, 5, 25, 24), outline=2, width=3)
		draw.line((9, 16, 14, 21, 24, 9), fill=0, width=4)
	else:
		draw.line((8, 8, 23, 23), fill=3, width=4)
		draw.line((23, 8, 8, 23), fill=3, width=4)
		draw.line((9, 27, 22, 27), fill=2, width=2)
	return image


def loop_icon() -> Image.Image:
	image = image16(1)
	draw = ImageDraw.Draw(image)
	draw.arc((2, 2, 13, 13), 35, 320, fill=0, width=2)
	draw.polygon(((11, 1), (15, 3), (11, 5)), fill=3)
	draw.ellipse((6, 6, 9, 9), fill=2)
	return image


def main() -> None:
	native = native_master()
	native.save(NATIVE)
	native.resize((448, 288), Image.Resampling.NEAREST).save(LEGACY_PREVIEW)
	INTRO_ASSET.parent.mkdir(parents=True, exist_ok=True)
	native.save(INTRO_ASSET)

	# Versioned, native-size production sheet consumed by the SDK asset compiler.
	# The fixed layout is deliberately simple: the renderer addresses regions of
	# the generated tilemap, never Wonderful's deduplicated tile numbers.
	gameplay = Image.new("P", (128, 64), 0)
	gameplay.putpalette(native.getpalette())
	for x, image in enumerate((
		floor_meta(0), floor_meta(1), wall_meta(False), wall_meta(True),
		parcel_meta(), depot_meta(), courier_meta(False, False),
		courier_meta(False, True),
	)):
		gameplay.paste(image, (x * 16, 0))
	for x, image in enumerate((
		courier_meta(True, False), courier_meta(True, True), cargo_hud(False),
		cargo_hud(True), target_hud(), loop_icon(),
	)):
		gameplay.paste(image, (x * 16, 16))
	for x, kind in enumerate((
		"bg", "fuel_full", "fuel_empty", "route_on",
	)):
		gameplay.paste(hud_tile(kind), (96 + x * 8, 16))
	gameplay.paste(hud_tile("route_off"), (96, 24))
	gameplay.paste(result_icon(True), (0, 32))
	gameplay.paste(result_icon(False), (32, 32))
	gameplay.save(GAMEPLAY_ASSET)

	# Human-review atlas: native assets enlarged 4x, grouped without labels.
	atlas_native = Image.new("P", (128, 80), 0)
	atlas_native.putpalette(native.getpalette())
	previews = (
		floor_meta(0), floor_meta(1), wall_meta(False), wall_meta(True),
		parcel_meta(), depot_meta(), courier_meta(False, False), courier_meta(True, True),
		cargo_hud(False), cargo_hud(True), target_hud(), loop_icon(),
		result_icon(True), result_icon(False),
	)
	for index, preview in enumerate(previews):
		x = (index % 8) * 16
		y = (index // 8) * 32
		atlas_native.paste(preview, (x, y))
	atlas_native.resize((512, 320), Image.Resampling.NEAREST).save(ATLAS)

	source_hash = hashlib.sha256(SOURCE.read_bytes()).hexdigest()
	print(f"Orbital Courier SDK assets generated from master {source_hash}")


if __name__ == "__main__":
	main()
