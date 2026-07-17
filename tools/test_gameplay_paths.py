#!/usr/bin/env python3
"""Verify every gameplay contract is exercised by its production C model test."""

from __future__ import annotations

from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
GAMES = (
    ("mote-sound-terminal", "test_mote_sound_terminal.c"),
    ("orbital-courier", "test_orbital_courier.c"),
    ("scrapframe-garage", "test_scrapframe_garage.c"),
    ("radio-ghost", "test_radio_ghost.c"),
    ("harpoon-moon", "test_harpoon_moon.c"),
    ("turncoat-tactics", "test_turncoat_tactics.c"),
    ("pocket-kaiju-observatory", "test_pocket_kaiju.c"),
    ("rotate-dungeon", "test_rotate_dungeon.c"),
    ("one-last-lap", "test_one_last_lap.c"),
    ("bug-witch", "test_bug_witch.c"),
)


def main() -> None:
    native = ROOT / "tests/native"
    for slug, test_name in GAMES:
        source = ROOT / "games" / slug / "src"
        model_c = source / "model.c"
        model_h = source / "model.h"
        main_c = source / "main.c"
        test_c = native / test_name
        assert model_c.is_file() and model_h.is_file(), f"missing production model: {slug}"
        assert test_c.is_file(), f"missing native path test: {slug}"
        assert '#include "model.h"' in main_c.read_text(), f"ROM bypasses model: {slug}"
        model_text = model_c.read_text()
        assert "wonderful.h" not in model_text and "ws.h" not in model_text, (
            f"production model is not host-portable: {slug}"
        )
        assert f"../../games/{slug}/src/model.c" in (native / "Makefile").read_text(), (
            f"native test does not compile production model: {slug}"
        )
        print(f"OK   {slug.replace('-', ' ')} production C contract")
    print("OK   all ten ROMs and host tests share the same gameplay code")


if __name__ == "__main__":
    main()
