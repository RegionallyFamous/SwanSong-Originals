#!/usr/bin/env python3
"""Perform quick structural checks on Wonderful-built WonderSwan ROMs."""

from __future__ import annotations

import argparse
from pathlib import Path
import tomllib


SUPPORTED_BANKS = {
    2: 0x00,
    4: 0x01,
    8: 0x02,
    16: 0x03,
    32: 0x04,
    48: 0x05,
    64: 0x06,
    96: 0x07,
    128: 0x08,
    256: 0x09,
}

ROOT = Path(__file__).resolve().parents[1]

EXPECTED_GAMES = {
    "mote_sound_terminal.wsc": "mote-sound-terminal",
    "orbital_courier.wsc": "orbital-courier",
    "scrapframe_garage.wsc": "scrapframe-garage",
    "radio_ghost.wsc": "radio-ghost",
    "harpoon_moon.wsc": "harpoon-moon",
    "turncoat_tactics.wsc": "turncoat-tactics",
    "pocket_kaiju_observatory.wsc": "pocket-kaiju-observatory",
    "rotate_dungeon.wsc": "rotate-dungeon",
    "one_last_lap.wsc": "one-last-lap",
    "bug_witch.wsc": "bug-witch",
}

SAVE_TYPES = {
    "none": 0x00,
    "sram-8kb": 0x01,
    "sram-32kb": 0x02,
    "sram-128kb": 0x03,
    "sram-256kb": 0x04,
    "sram-512kb": 0x05,
    "eeprom-128b": 0x10,
    "eeprom-2kb": 0x20,
    "eeprom-1kb": 0x50,
}


def bcd(value: int) -> int:
    return ((value // 10) << 4) | value % 10


def expected_cartridge(filename: str) -> dict[str, object] | None:
    slug = EXPECTED_GAMES.get(filename)
    if slug is None:
        return None
    with (ROOT / "games" / slug / "swan.toml").open("rb") as handle:
        return tomllib.load(handle)["cartridge"]


def verify(path: Path) -> list[str]:
    errors: list[str] = []
    data = path.read_bytes()
    if len(data) < 0x10000 or len(data) % 0x10000:
        errors.append(f"size {len(data)} is not whole 64 KiB banks")
    banks = len(data) // 0x10000
    if banks not in SUPPORTED_BANKS:
        errors.append(f"unsupported cartridge size of {banks} banks")
    if len(data) >= 16:
        footer = data[-16:]
        expected_cartridge_data = expected_cartridge(path.name)
        if footer[0] != 0xEA or footer[1:5] == b"\x00\x00\x00\x00":
            errors.append("missing valid far-jump cartridge entry point")
        if footer[5] != 0x00:
            errors.append(f"maintenance byte is 0x{footer[5]:02X}, expected 0x00")
        expected_publisher = (
            int(expected_cartridge_data["publisher_id"])
            if expected_cartridge_data else 0
        )
        if footer[6] != expected_publisher:
            errors.append(
                f"publisher is 0x{footer[6]:02X}, expected "
                f"0x{expected_publisher:02X}"
            )
        if footer[7] != 0x01:
            errors.append(f"color flag is 0x{footer[7]:02X}, expected 0x01")
        if expected_cartridge_data is None:
            errors.append("unexpected ROM filename")
        elif footer[8] != bcd(int(expected_cartridge_data["game_id"])):
            errors.append(
                f"game ID is 0x{footer[8]:02X}, expected BCD "
                f"{bcd(int(expected_cartridge_data['game_id'])):02X}"
            )
        expected_version = (
            int(expected_cartridge_data["version"])
            if expected_cartridge_data else 1
        )
        if footer[9] != expected_version:
            errors.append(
                f"game version is 0x{footer[9]:02X}, expected "
                f"0x{expected_version:02X}"
            )
        if banks in SUPPORTED_BANKS and footer[10] != SUPPORTED_BANKS[banks]:
            errors.append(
                f"ROM-size code is 0x{footer[10]:02X}, expected "
                f"0x{SUPPORTED_BANKS[banks]:02X}"
            )
        expected_save = (
            SAVE_TYPES[str(expected_cartridge_data["save_type"])]
            if expected_cartridge_data else 0
        )
        if footer[11] != expected_save:
            errors.append(
                f"save type is 0x{footer[11]:02X}, expected "
                f"0x{expected_save:02X}"
            )
        if footer[12] & 0x05 != 0x04:
            errors.append(
                f"flags are 0x{footer[12]:02X}, expected horizontal 16-bit ROM"
            )
        if footer[13] != 0x00:
            errors.append(f"mapper is 0x{footer[13]:02X}, expected 2001")
        expected_checksum = int.from_bytes(data[-2:], "little")
        actual = sum(data[:-2]) & 0xFFFF
        if expected_checksum != actual:
            errors.append(
                f"footer checksum is 0x{expected_checksum:04X}; "
                f"calculated 0x{actual:04X}"
            )
    return errors


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("roms", nargs="+", type=Path)
    args = parser.parse_args()

    failed = False
    names = [rom.name for rom in args.roms]
    if len(args.roms) != len(EXPECTED_GAMES) or set(names) != set(EXPECTED_GAMES):
        failed = True
        print(
            f"FAIL expected exactly {len(EXPECTED_GAMES)} named ROMs; "
            f"received {len(args.roms)}"
        )
    for rom in args.roms:
        errors = verify(rom)
        if errors:
            failed = True
            print(f"FAIL {rom}")
            for error in errors:
                print(f"  - {error}")
        else:
            print(f"OK   {rom} ({rom.stat().st_size // 1024} KiB)")
    return 1 if failed else 0


if __name__ == "__main__":
    raise SystemExit(main())
