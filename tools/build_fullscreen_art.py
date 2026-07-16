#!/usr/bin/env python3
"""Build every Yohaku full-screen master, native proof, atlas, and ROM tile bank."""

from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
import hashlib
import math

from PIL import Image, ImageDraw, ImageFont


ROOT = Path(__file__).resolve().parents[1]
ART_DIR = ROOT / "docs" / "art" / "full-screen"
LEGACY_PREVIEW_DIR = ROOT / "docs" / "art" / "native"

CREAM = (0xF5, 0xF0, 0xE6)
INK = (0x1A, 0x1A, 0x1A)
PINK = (0xFF, 0x6B, 0x9D)
YELLOW = (0xFF, 0xE5, 0x66)
BLUE = (0xA8, 0xD5, 0xE5)
PALE_PINK = (0xF4, 0xC7, 0xC3)


@dataclass(frozen=True)
class GameSpec:
	slug: str
	palette: tuple[tuple[int, int, int], ...]
	builder: str


SPECS = (
	GameSpec("mote-sound-terminal", (CREAM, INK, PINK, BLUE), "mote"),
	GameSpec("scrapframe-garage", (CREAM, INK, YELLOW, BLUE), "scrap"),
	GameSpec("radio-ghost", (CREAM, INK, PALE_PINK, PINK), "radio"),
	GameSpec("harpoon-moon", (CREAM, INK, YELLOW, PALE_PINK), "harpoon"),
	GameSpec("turncoat-tactics", (CREAM, INK, BLUE, PINK), "tactics"),
	GameSpec("pocket-kaiju-observatory", (CREAM, INK, PALE_PINK, BLUE), "kaiju"),
	GameSpec("rotate-dungeon", (CREAM, INK, YELLOW, BLUE), "rotate"),
	GameSpec("one-last-lap", (CREAM, INK, PINK, YELLOW), "race"),
	GameSpec("bug-witch", (CREAM, INK, PINK, BLUE), "bug"),
)


def color_distance(a: tuple[int, int, int], b: tuple[int, int, int]) -> float:
	return math.sqrt(sum((int(x) - int(y)) ** 2 for x, y in zip(a, b)))


def quantize_exact(image: Image.Image, palette: tuple[tuple[int, int, int], ...]) -> Image.Image:
	rgb = image.convert("RGB")
	out = Image.new("P", rgb.size)
	out.putpalette([value for color in palette for value in color] + [0] * (768 - 12))
	out.putdata([
		min(range(4), key=lambda index: color_distance(pixel, palette[index]))
		for pixel in rgb.get_flattened_data()
	])
	return out


def native_master(spec: GameSpec) -> Image.Image:
	source = Image.open(ART_DIR / f"{spec.slug}-gameplay-master.png").convert("RGB")
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
	return quantize_exact(source.resize((224, 144), Image.Resampling.LANCZOS), spec.palette)


def flip_h(tile: tuple[int, ...]) -> tuple[int, ...]:
	return tuple(tile[y * 8 + (7 - x)] for y in range(8) for x in range(8))


def flip_v(tile: tuple[int, ...]) -> tuple[int, ...]:
	return tuple(tile[(7 - y) * 8 + x] for y in range(8) for x in range(8))


def pack_tile(tile: tuple[int, ...]) -> bytes:
	data = bytearray()
	for y in range(8):
		plane_0 = 0
		plane_1 = 0
		for x in range(8):
			value = tile[y * 8 + x]
			bit = 7 - x
			plane_0 |= (value & 1) << bit
			plane_1 |= ((value >> 1) & 1) << bit
		data.extend((plane_0, plane_1))
	return bytes(data)


def intro_tiles(image: Image.Image) -> tuple[bytes, list[int]]:
	pixels = tuple(image.get_flattened_data())
	tiles: list[tuple[int, ...]] = []
	lookup: dict[tuple[int, ...], tuple[int, int]] = {}
	tilemap: list[int] = []
	blank = (0,) * 64
	for tile_y in range(18):
		for tile_x in range(28):
			tile = tuple(
				pixels[(tile_y * 8 + y) * 224 + tile_x * 8 + x]
				for y in range(8) for x in range(8)
			)
			if tile == blank:
				tilemap.append(0)
				continue
			match = lookup.get(tile)
			if match is None:
				index = len(tiles) + 1
				tiles.append(tile)
				for variant, flags in ((tile, 0), (flip_h(tile), 0x4000),
					(flip_v(tile), 0x8000), (flip_v(flip_h(tile)), 0xC000)):
					lookup.setdefault(variant, (index, flags))
				match = (index, 0)
			tilemap.append(match[0] | match[1])
	assert len(tiles) <= 511
	return b"".join(pack_tile(tile) for tile in tiles), tilemap


def pal_image(size: tuple[int, int], fill: int = 0) -> Image.Image:
	image = Image.new("P", size, fill)
	image.putpalette([value for color in (CREAM, INK, PINK, BLUE) for value in color] + [0] * (768 - 12))
	return image


class TileBank:
	def __init__(self) -> None:
		self.tiles: list[tuple[int, ...]] = []
		self.lookup: dict[tuple[int, ...], int] = {}

	def add_tile(self, image: Image.Image) -> int:
		tile = tuple(image.get_flattened_data())
		if tile not in self.lookup:
			self.lookup[tile] = len(self.tiles)
			self.tiles.append(tile)
		return self.lookup[tile]

	def add_image(self, image: Image.Image) -> list[int]:
		ids = []
		for tile_y in range(image.height // 8):
			for tile_x in range(image.width // 8):
				ids.append(self.add_tile(image.crop((tile_x * 8, tile_y * 8,
					(tile_x + 1) * 8, (tile_y + 1) * 8))))
		return ids

	def packed(self) -> bytes:
		return b"".join(pack_tile(tile) for tile in self.tiles)


def tile(fill: int, marks: tuple[tuple[int, int, int], ...] = ()) -> Image.Image:
	image = pal_image((8, 8), fill)
	draw = ImageDraw.Draw(image)
	for x, y, color in marks:
		draw.point((x, y), fill=color)
	return image


def pip(full: bool, color: int = 2, shape: str = "diamond") -> Image.Image:
	image = pal_image((8, 8), 1)
	draw = ImageDraw.Draw(image)
	if shape == "circle":
		draw.ellipse((1, 1, 6, 6), fill=color if full else 1, outline=0)
	elif shape == "bar":
		draw.rectangle((1, 2, 6, 5), fill=color if full else 1, outline=0)
	else:
		draw.polygon(((3, 0), (7, 3), (4, 7), (0, 4)),
			fill=color if full else 1, outline=0)
	return image


def result_icon(won: bool) -> Image.Image:
	image = pal_image((32, 32), 1)
	draw = ImageDraw.Draw(image)
	draw.rectangle((0, 0, 31, 31), outline=3, width=2)
	if won:
		draw.ellipse((6, 5, 25, 24), outline=2, width=3)
		draw.line((9, 16, 14, 21, 24, 9), fill=0, width=4)
	else:
		draw.line((8, 8, 23, 23), fill=3, width=4)
		draw.line((23, 8, 8, 23), fill=3, width=4)
	return image


def loop_icon() -> Image.Image:
	image = pal_image((16, 16), 1)
	draw = ImageDraw.Draw(image)
	draw.arc((2, 2, 13, 13), 35, 320, fill=0, width=2)
	draw.polygon(((11, 1), (15, 3), (11, 5)), fill=3)
	draw.ellipse((6, 6, 9, 9), fill=2)
	return image


def icon(kind: str, color: int = 2, size: int = 16, selected: bool = False) -> Image.Image:
	image = pal_image((size, size), 1)
	draw = ImageDraw.Draw(image)
	if selected:
		draw.rectangle((0, 0, size - 1, size - 1), outline=3, width=2)
	if kind == "crystal":
		draw.polygon(((size // 2, 2), (size - 3, size // 2), (size // 2, size - 2), (2, size // 2)), fill=color, outline=0)
		draw.line((size // 2, 2, size // 2, size - 2), fill=1)
	elif kind == "play":
		draw.polygon(((4, 3), (size - 3, size // 2), (4, size - 3)), fill=color)
	elif kind == "pause":
		draw.rectangle((4, 3, 6, size - 3), fill=color)
		draw.rectangle((size - 7, 3, size - 5, size - 3), fill=color)
	elif kind == "aperture":
		draw.ellipse((2, 2, size - 3, size - 3), outline=color, width=2)
		draw.polygon(((size // 2, 3), (size - 4, size // 2), (size // 2, size - 4), (3, size // 2)), fill=0)
	elif kind == "moon":
		draw.ellipse((3, 2, size - 3, size - 2), fill=color)
		draw.ellipse((6, 1, size - 1, size - 4), fill=1)
	elif kind == "leaf":
		draw.ellipse((3, 3, size - 3, size - 4), fill=color)
		draw.line((3, size - 3, size - 4, 3), fill=0, width=2)
	elif kind == "foot":
		draw.ellipse((5, 6, 10, 13), fill=color)
		for x, y in ((4, 3), (7, 2), (10, 3), (12, 5)):
			draw.ellipse((x, y, x + 2, y + 2), fill=color)
	elif kind == "key":
		draw.ellipse((2, 2, 8, 8), outline=color, width=2)
		draw.rectangle((7, 5, 13, 7), fill=color)
		draw.rectangle((11, 7, 13, 10), fill=color)
	elif kind == "exit":
		draw.ellipse((1, 1, size - 2, size - 2), fill=color, outline=0)
		inset = max(3, size // 4)
		draw.ellipse((inset, inset, size - inset - 1, size - inset - 1), fill=3)
	elif kind == "run":
		draw.polygon(((3, 2), (size - 2, size // 2), (3, size - 2)), fill=color)
	return image


def common_assets() -> dict[str, Image.Image]:
	return {
		"hud_bg": tile(1, ((6, 2, 3),)),
		"pip_full": pip(True), "pip_empty": pip(False),
		"result_win": result_icon(True), "result_loss": result_icon(False),
		"loop": loop_icon(),
	}


def mote_assets() -> dict[str, Image.Image]:
	a = common_assets()
	for i, color in enumerate((2, 3, 0)):
		a[f"track_{i}"] = icon("crystal", color, 16, i == 0)
	a["play"] = icon("play", 2)
	a["pause"] = icon("pause", 3)
	a["scope_a"] = icon("aperture", 2)
	a["scope_b"] = icon("aperture", 3)
	for i in range(8):
		bar = pal_image((8, 24), 1)
		d = ImageDraw.Draw(bar)
		height = i * 3
		d.rectangle((2, 23 - height, 5, 23), fill=2 if i & 1 else 3)
		a[f"bar_{i}"] = bar
	for name, fill in (("beat_off", 1), ("beat_on", 2), ("beat_now", 0)):
		im = pal_image((8, 8), 1)
		d = ImageDraw.Draw(im)
		d.rectangle((1, 1, 6, 6), fill=fill, outline=3 if name == "beat_now" else 0)
		a[name] = im
	return a


def machine(job: int) -> Image.Image:
	im = pal_image((112, 56), 0)
	d = ImageDraw.Draw(im)
	d.rounded_rectangle((4, 8, 106, 44), radius=5, fill=1, outline=3, width=2)
	d.rectangle((12, 13, 68, 34), fill=0)
	d.rectangle((71, 12, 98, 36), fill=3 if job == 1 else 2)
	for x in (16, 43, 78):
		d.ellipse((x, 34, x + 20, 54), fill=1, outline=0)
		d.ellipse((x + 5, 39, x + 15, 49), fill=3)
	if job == 0:
		d.line((18, 46, 31, 38), fill=2, width=3)
	elif job == 1:
		d.rectangle((77, 17, 91, 31), fill=2, outline=1, width=2)
	else:
		d.ellipse((78, 15, 96, 33), outline=2, width=3)
	return im


def part(kind: int, selected: bool) -> Image.Image:
	im = pal_image((32, 32), 3)
	d = ImageDraw.Draw(im)
	d.rectangle((0, 0, 31, 31), outline=2 if selected else 1, width=3 if selected else 1)
	if kind == 0:
		d.line((7, 22, 24, 8), fill=0, width=5)
		d.ellipse((4, 19, 12, 27), outline=1, width=2)
		d.ellipse((20, 5, 28, 13), outline=1, width=2)
	elif kind == 1:
		d.rectangle((7, 7, 24, 24), fill=0, outline=1, width=2)
		d.rectangle((12, 12, 19, 19), fill=3)
	else:
		d.ellipse((5, 5, 26, 26), fill=0, outline=1, width=2)
		for angle in range(0, 360, 90):
			# four blocky blades
			cx, cy = 16, 16
			if angle == 0: d.polygon(((cx, cy), (22, 8), (24, 15)), fill=3)
			elif angle == 90: d.polygon(((cx, cy), (24, 22), (17, 24)), fill=3)
			elif angle == 180: d.polygon(((cx, cy), (10, 24), (8, 17)), fill=3)
			else: d.polygon(((cx, cy), (8, 10), (15, 8)), fill=3)
	return im


def scrap_assets() -> dict[str, Image.Image]:
	a = common_assets()
	for i in range(3):
		a[f"machine_{i}"] = machine(i)
		for selected in (False, True):
			a[f"part_{i}_{'on' if selected else 'off'}"] = part(i, selected)
	return a


def radio_assets() -> dict[str, Image.Image]:
	a = common_assets()
	a["panel"] = tile(1, ((1, 1, 2), (6, 5, 3)))
	for i in range(8):
		im = pal_image((8, 64), 1)
		d = ImageDraw.Draw(im)
		height = 3 + i * 4
		d.rectangle((3, 32 - height, 4, 32 + height), fill=0)
		a[f"wave_{i}"] = im
	needle = pal_image((8, 64), 1)
	d = ImageDraw.Draw(needle)
	d.rectangle((3, 0, 4, 60), fill=3)
	d.polygon(((0, 63), (7, 63), (3, 58)), fill=3)
	a["needle"] = needle
	ghost = pal_image((32, 48), 1)
	d = ImageDraw.Draw(ghost)
	d.line((7, 45, 19, 34, 11, 25, 23, 15, 17, 4), fill=3, width=5)
	d.ellipse((14, 1, 24, 11), fill=2)
	a["ghost"] = ghost
	receiver = pal_image((32, 24), 1)
	d = ImageDraw.Draw(receiver)
	d.rectangle((2, 4, 29, 22), fill=0, outline=3, width=2)
	d.rectangle((5, 8, 15, 18), fill=1)
	d.ellipse((19, 8, 27, 16), fill=2, outline=1)
	d.line((6, 4, 2, 0), fill=3, width=2)
	a["receiver"] = receiver
	a["gate_narrow"] = icon("aperture", 2)
	a["gate_wide"] = icon("aperture", 3)
	for i, kind in enumerate(("moon", "leaf", "crystal")):
		a[f"clue_{i}"] = icon(kind, 3)
	return a


def skiff() -> Image.Image:
	im = pal_image((32, 24), 1)
	d = ImageDraw.Draw(im)
	d.polygon(((2, 14), (27, 10), (22, 21), (7, 21)), fill=0, outline=3)
	d.rectangle((8, 7, 17, 15), fill=2, outline=0)
	d.line((17, 7, 23, 3), fill=3, width=2)
	return im


def constellation(boss: bool) -> Image.Image:
	size = (48, 32) if boss else (32, 24)
	im = pal_image(size, 1)
	d = ImageDraw.Draw(im)
	points = ((3, 15), (10, 6), (18, 13), (26, 4), (size[0] - 4, 15), (20, size[1] - 4))
	d.line(points, fill=2, width=2)
	for x, y in points:
		d.ellipse((x - 2, y - 2, x + 2, y + 2), fill=3)
	return im


def harpoon_assets() -> dict[str, Image.Image]:
	a = common_assets()
	a["sky"] = tile(1, ((1, 2, 2), (6, 6, 3)))
	a["ground"] = tile(0, ((2, 2, 1), (5, 6, 1)))
	a["skiff"] = skiff()
	a["creature"] = constellation(False)
	a["boss"] = constellation(True)
	beam = pal_image((8, 8), 1)
	ImageDraw.Draw(beam).line((0, 4, 7, 4), fill=2, width=2)
	a["beam"] = beam
	return a


def unit_tile(team: int, kind: int) -> Image.Image:
	im = pal_image((8, 8), 0)
	d = ImageDraw.Draw(im)
	color = 2 if team == 0 else 3
	if kind == 0:
		d.polygon(((1, 6), (3, 1), (7, 5), (6, 7)), fill=color, outline=1)
	elif kind == 1:
		d.ellipse((1, 1, 6, 6), fill=color, outline=1)
	else:
		d.polygon(((3, 1), (6, 6), (1, 6)), fill=color, outline=1)
	return im


def tactics_assets() -> dict[str, Image.Image]:
	a = common_assets()
	cell = pal_image((24, 16), 0)
	d = ImageDraw.Draw(cell)
	d.line((23, 0, 23, 15), fill=1)
	d.line((0, 15, 23, 15), fill=1)
	a["cell"] = cell
	a["cursor"] = tile(0, ((0, 0, 3), (1, 0, 3), (0, 1, 3), (6, 0, 3), (7, 0, 3), (7, 1, 3), (0, 6, 3), (0, 7, 3), (1, 7, 3), (7, 6, 3), (6, 7, 3), (7, 7, 3)))
	for team in range(2):
		for kind in range(3):
			a[f"unit_{team}_{kind}"] = unit_tile(team, kind)
	a["beacon"] = icon("exit", 3, 8)
	return a


def creature(behavior: int) -> Image.Image:
	im = pal_image((48, 64), 0)
	d = ImageDraw.Draw(im)
	if behavior == 0:
		d.ellipse((8, 23, 43, 55), fill=1)
		d.ellipse((3, 34, 17, 49), fill=1)
	elif behavior == 1:
		d.ellipse((12, 6, 39, 52), fill=1)
		d.ellipse((5, 2, 28, 25), fill=1)
	else:
		d.ellipse((9, 8, 40, 52), fill=1)
		d.ellipse((2, 4, 25, 24), fill=1)
		d.line((16, 48, 10, 62), fill=1, width=6)
		d.line((32, 48, 39, 62), fill=1, width=6)
	d.ellipse((13, 10, 15, 12), fill=3)
	for x, y in ((31, 8), (37, 17), (26, 27), (40, 34), (30, 44)):
		d.rectangle((x, y, x + 3, y + 4), fill=3 if (x + y) & 1 else 2)
	return im


def camera_icon() -> Image.Image:
	im = pal_image((24, 24), 1)
	d = ImageDraw.Draw(im)
	d.rectangle((3, 7, 19, 19), fill=0, outline=3, width=2)
	d.rectangle((15, 10, 23, 15), fill=1, outline=0)
	d.ellipse((7, 10, 15, 18), fill=1, outline=3)
	return im


def kaiju_assets() -> dict[str, Image.Image]:
	a = common_assets()
	a["sky"] = tile(0, ((1, 1, 2), (6, 3, 3)))
	a["ground"] = tile(1, ((2, 2, 3), (5, 6, 2)))
	a["camera"] = camera_icon()
	for i in range(3):
		a[f"creature_{i}"] = creature(i)
	for i, kind in enumerate(("moon", "leaf", "foot")):
		a[f"behavior_{i}"] = icon(kind, 3)
	a["zoom_1"] = icon("aperture", 2)
	a["zoom_2"] = icon("aperture", 3)
	return a


def meta_base(fill: int = 0) -> Image.Image:
	im = pal_image((16, 16), fill)
	d = ImageDraw.Draw(im)
	d.line((0, 0, 15, 0), fill=1)
	d.line((0, 0, 0, 15), fill=1)
	d.point((12, 12), fill=2)
	return im


def rotate_assets() -> dict[str, Image.Image]:
	a = common_assets()
	a["floor"] = meta_base(0)
	wall = pal_image((16, 16), 1)
	d = ImageDraw.Draw(wall)
	d.rectangle((1, 1, 14, 3), fill=3)
	d.rectangle((3, 7, 12, 8), fill=2)
	a["wall"] = wall
	a["key"] = icon("key", 2)
	a["exit"] = icon("exit", 3)
	player = meta_base(0)
	d = ImageDraw.Draw(player)
	d.rectangle((6, 4, 10, 11), fill=2, outline=1)
	d.rectangle((7, 2, 10, 5), fill=1)
	d.line((6, 12, 5, 15), fill=1, width=2)
	d.line((10, 12, 12, 15), fill=1, width=2)
	a["player"] = player
	a["orient_h"] = icon("run", 2)
	a["orient_v"] = icon("run", 3).transpose(Image.Transpose.ROTATE_90)
	return a


def vehicle(size: tuple[int, int], color: int, stranded: bool = False) -> Image.Image:
	im = pal_image(size, 1)
	d = ImageDraw.Draw(im)
	w, h = size
	d.rectangle((3, 5, w - 4, h - 7), fill=0, outline=color, width=2)
	d.rectangle((w // 3, 1, w - 6, h // 2), fill=color)
	for x in (5, w - 10):
		d.ellipse((x, h - 10, x + 6, h - 4), fill=1, outline=0)
	if stranded:
		d.line((w - 8, 2, w - 3, -2), fill=3, width=2)
	return im


def race_assets() -> dict[str, Image.Image]:
	a = common_assets()
	a["sky"] = tile(0, ((2, 2, 2), (6, 5, 3)))
	a["road"] = tile(1, ((1, 1, 0), (6, 5, 0)))
	a["edge"] = tile(1, tuple((3, y, 3) for y in range(8)))
	a["player"] = vehicle((32, 24), 3)
	a["rival"] = vehicle((16, 16), 2)
	a["stranded"] = vehicle((24, 16), 3, True)
	hazard = pal_image((16, 16), 1)
	d = ImageDraw.Draw(hazard)
	d.polygon(((2, 13), (6, 3), (10, 13)), fill=2)
	d.polygon(((7, 13), (11, 3), (15, 13)), fill=2)
	a["hazard"] = hazard
	a["speed"] = icon("run", 2, 8)
	a["tow"] = icon("key", 3)
	return a


def beetle(kind: int, selected: bool, small: bool = False) -> Image.Image:
	size = 16 if small else 24
	im = pal_image((size, size), 1)
	d = ImageDraw.Draw(im)
	if selected:
		d.rectangle((0, 0, size - 1, size - 1), outline=3, width=2)
	color = 2 if kind != 2 else 3
	d.ellipse((5, 5, size - 6, size - 4), fill=color if kind != 2 else 1, outline=0 if kind != 2 else color, width=2)
	d.line((size // 2, 5, size // 2, size - 5), fill=1)
	if kind == 0:
		d.line((5, 9, 1, 5), fill=0, width=2)
		d.line((size - 6, 9, size - 2, 5), fill=0, width=2)
	elif kind == 1:
		d.arc((7, 2, size - 8, 10), 180, 360, fill=3, width=2)
	for y in (9, 15):
		d.line((5, y, 1, y - 2), fill=color)
		d.line((size - 6, y, size - 2, y - 2), fill=color)
	return im


def socket(kind: int) -> Image.Image:
	im = pal_image((32, 32), 0)
	d = ImageDraw.Draw(im)
	d.rectangle((1, 3, 30, 28), outline=1, width=2)
	d.rectangle((5, 7, 26, 24), outline=3)
	if kind:
		bug = beetle(kind - 1, False, True)
		im.paste(bug, (8, 8))
	return im


def bug_assets() -> dict[str, Image.Image]:
	a = common_assets()
	a["field"] = tile(0, ((1, 1, 2), (6, 5, 3)))
	wire = pal_image((8, 8), 0)
	ImageDraw.Draw(wire).line((0, 4, 7, 4), fill=1, width=2)
	a["wire"] = wire
	for i in range(4):
		a[f"socket_{i}"] = socket(i)
	for i in range(3):
		a[f"beetle_small_{i}"] = beetle(i, False, True)
		for selected in (False, True):
			a[f"beetle_{i}_{'on' if selected else 'off'}"] = beetle(i, selected)
	a["signal_on"] = icon("exit", 3)
	a["signal_off"] = icon("exit", 0)
	a["run"] = icon("run", 2, 24)
	return a


BUILDERS = {
	"mote": mote_assets, "scrap": scrap_assets, "radio": radio_assets,
	"harpoon": harpoon_assets, "tactics": tactics_assets, "kaiju": kaiju_assets,
	"rotate": rotate_assets, "race": race_assets, "bug": bug_assets,
}


def c_bytes(data: bytes, width: int = 12) -> str:
	return "\n".join("\t" + ", ".join(f"0x{value:02X}" for value in data[i:i + width]) + ","
		for i in range(0, len(data), width))


def c_words(data: list[int], width: int = 8) -> str:
	return "\n".join("\t" + ", ".join(f"0x{value:04X}" for value in data[i:i + width]) + ","
		for i in range(0, len(data), width))


def rgb12(color: tuple[int, int, int]) -> int:
	r, g, b = (round(value * 15 / 255) for value in color)
	return (r << 8) | (g << 4) | b


def build_game(spec: GameSpec) -> None:
	native = native_master(spec)
	native_path = ART_DIR / f"{spec.slug}-gameplay-native.png"
	atlas_path = ART_DIR / f"{spec.slug}-gameplay-atlas.png"
	native.save(native_path)
	native.resize((448, 288), Image.Resampling.NEAREST).save(LEGACY_PREVIEW_DIR / f"{spec.slug}.png")
	intro_data, intro_map = intro_tiles(native)

	bank = TileBank()
	images = BUILDERS[spec.builder]()
	assets = {name: bank.add_image(image) for name, image in images.items()}
	assert len(bank.tiles) <= 511

	# Contact-style atlas without labels: every live asset on a cream grid.
	atlas = Image.new("P", (160, math.ceil(len(images) / 5) * 40), 0)
	atlas.putpalette(native.getpalette())
	for index, image in enumerate(images.values()):
		preview = image.copy()
		preview.putpalette(native.getpalette())
		scale = min(1.0, 30 / preview.width, 30 / preview.height)
		if scale < 1:
			preview = preview.resize((max(1, round(preview.width * scale)),
				max(1, round(preview.height * scale))), Image.Resampling.NEAREST)
		x = (index % 5) * 32 + (32 - preview.width) // 2
		y = (index // 5) * 40 + (32 - preview.height) // 2
		atlas.paste(preview, (x, y))
	atlas.resize((atlas.width * 3, atlas.height * 3), Image.Resampling.NEAREST).save(atlas_path)

	asset_defs = "\n\n".join(
		f"static const uint16_t __far art_{name}[] = {{\n{c_words(values)}\n}};"
		for name, values in assets.items()
	)
	palette = [rgb12(color) for color in spec.palette]
	source = ART_DIR / f"{spec.slug}-gameplay-master.png"
	source_hash = hashlib.sha256(source.read_bytes()).hexdigest()
	guard = f"SWANSONG_{spec.slug.upper().replace('-', '_')}_GAMEPLAY_ART_H"
	header = ROOT / "games" / spec.slug / "src" / "gameplay_art.h"
	header.write_text(f"""/* Generated by tools/build_fullscreen_art.py.
 * Imagegen source SHA-256: {source_hash}
 */
#ifndef {guard}
#define {guard}

#include <stdint.h>
#include <wonderful.h>

static const uint16_t __far game_palette[] = {{
{c_words(palette, 4)}
}};

static const uint8_t __far game_intro_tiles[] = {{
{c_bytes(intro_data)}
}};

static const uint16_t __far game_intro_map[] = {{
{c_words(intro_map)}
}};

static const uint8_t __far game_tiles[] = {{
{c_bytes(bank.packed())}
}};

{asset_defs}

#define GAME_INTRO_TILE_COUNT {len(intro_data) // 16}
#define GAME_TILE_COUNT {len(bank.tiles)}

#endif
""", encoding="utf-8")
	print(f"{spec.slug}: intro {len(intro_data) // 16}, live {len(bank.tiles)} tiles")


def build_contact(paths: list[Path], output: Path) -> None:
	cell_width = 475
	cell_height = 326
	canvas = Image.new("RGB", (cell_width * 2, cell_height * 5), CREAM)
	draw = ImageDraw.Draw(canvas)
	try:
		font = ImageFont.truetype("DejaVuSans.ttf", 22)
	except OSError:
		font = ImageFont.load_default()
	for index, path in enumerate(sorted(paths)):
		image = Image.open(path).convert("RGB")
		scale = min(448 / image.width, 270 / image.height)
		image = image.resize((round(image.width * scale), round(image.height * scale)),
			Image.Resampling.NEAREST)
		cell_x = (index % 2) * cell_width
		cell_y = (index // 2) * cell_height
		canvas.paste(image, (cell_x + (cell_width - image.width) // 2,
			cell_y + 14 + (270 - image.height) // 2))
		draw.text((cell_x + 18, cell_y + 294),
			path.stem.replace("-gameplay-native", ""), fill=INK, font=font)
	canvas.save(output)


def main() -> None:
	from build_orbital_gameplay_art import main as build_orbital

	ART_DIR.mkdir(parents=True, exist_ok=True)
	LEGACY_PREVIEW_DIR.mkdir(parents=True, exist_ok=True)
	build_orbital()
	for spec in SPECS:
		build_game(spec)
	build_contact(list(LEGACY_PREVIEW_DIR.glob("*.png")),
		ROOT / "docs" / "art" / "native-contact-sheet.png")
	build_contact(list(ART_DIR.glob("*-gameplay-native.png")),
		ART_DIR / "native-contact-sheet.png")
	build_contact(list(ART_DIR.glob("*-gameplay-atlas.png")),
		ART_DIR / "atlas-contact-sheet.png")


if __name__ == "__main__":
	main()
