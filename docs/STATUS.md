# Version 2 completion status

All ten short-session game scopes are implemented and pass the public software
release gate.

| Lane | Current evidence |
| --- | --- |
| Native compile | Ten independent `wswan/medium` `.wsc` targets build with `-Wall -Wextra -Werror` |
| ROM structure | Exactly ten named ROMs; supported bank sizes, valid entry/header fields, unique IDs, and valid checksums |
| Game invariants | UI bounds, courier routes, radio timing, dungeon rooms, and all circuit puzzles are verified |
| Gameplay paths | All ten games have deterministic utility, success, failure/retry, completion, or reset-path coverage as applicable |
| Native art | All ten cartridges have complete full-screen graphical renderers derived from hash-linked Imagegen masters, exact four-color palettes, and checked-in 2BPP tile banks |
| Emulator boot | Every ROM reaches a captured nonblank Mednafen gameplay frame with its icon HUD and dominant playfield visible |
| Audio | Original wavetable sequences and feedback calls compile; physical listening remains device-dependent QA |
| Saves | Intentionally absent: every v1 game is a short replayable session |
| Orientation | Logical state and LCD icon path are implemented for Rotate Dungeon |
| Public assets | Ten full-screen masters, ten sprite atlases, prompt/style contracts, hashes, critique results, native proofs, learning notes, and cartridge captures are retained publicly |
| Hardware | Emulator-verified; physical cartridge, display, and speaker checks remain pending |

The retained emulator contact sheet and its evidence limits are documented in
[`docs/qa/`](qa/README.md).

“Complete” here means the documented microgame and graphical-overhaul scope is implemented,
buildable, testable, and replayable. It does not claim validation on every
flash cartridge, display revision, emulator, or physical console.
