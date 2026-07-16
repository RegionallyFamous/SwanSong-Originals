#!/usr/bin/env python3
"""Perform quick structural checks on Wonderful-built WonderSwan ROMs."""

from __future__ import annotations

import argparse
from pathlib import Path


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

EXPECTED_GAMES = {
    "mote_sound_terminal.wsc": 1,
    "orbital_courier.wsc": 2,
    "scrapframe_garage.wsc": 3,
    "radio_ghost.wsc": 4,
    "harpoon_moon.wsc": 5,
    "turncoat_tactics.wsc": 6,
    "pocket_kaiju_observatory.wsc": 7,
    "rotate_dungeon.wsc": 8,
    "one_last_lap.wsc": 9,
    "bug_witch.wsc": 10,
}


def bcd(value: int) -> int:
    return ((value // 10) << 4) | value % 10


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
        expected_id = EXPECTED_GAMES.get(path.name)
        if footer[0] != 0xEA or footer[1:5] == b"\x00\x00\x00\x00":
            errors.append("missing valid far-jump cartridge entry point")
        if footer[5] != 0x00:
            errors.append(f"maintenance byte is 0x{footer[5]:02X}, expected 0x00")
        if footer[6] != 0x00:
            errors.append(f"publisher is 0x{footer[6]:02X}, expected homebrew ID 0x00")
        if footer[7] != 0x01:
            errors.append(f"color flag is 0x{footer[7]:02X}, expected 0x01")
        if expected_id is None:
            errors.append("unexpected ROM filename")
        elif footer[8] != bcd(expected_id):
            errors.append(
                f"game ID is 0x{footer[8]:02X}, expected BCD {bcd(expected_id):02X}"
            )
        if footer[9] != 0x01:
            errors.append(f"game version is 0x{footer[9]:02X}, expected 0x01")
        if banks in SUPPORTED_BANKS and footer[10] != SUPPORTED_BANKS[banks]:
            errors.append(
                f"ROM-size code is 0x{footer[10]:02X}, expected "
                f"0x{SUPPORTED_BANKS[banks]:02X}"
            )
        if footer[11] != 0x00:
            errors.append(f"save type is 0x{footer[11]:02X}, expected NONE")
        if footer[12] & 0x05 != 0x04:
            errors.append(
                f"flags are 0x{footer[12]:02X}, expected horizontal 16-bit ROM"
            )
        if footer[13] != 0x00:
            errors.append(f"mapper is 0x{footer[13]:02X}, expected 2001")
        expected = int.from_bytes(data[-2:], "little")
        actual = sum(data[:-2]) & 0xFFFF
        if expected != actual:
            errors.append(
                f"footer checksum is 0x{expected:04X}; calculated 0x{actual:04X}"
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
