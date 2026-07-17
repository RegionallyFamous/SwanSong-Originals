# SwanSong SDK integration record

## Current integration boundary

All ten cartridges build against the pinned SwanSong SDK v0.1.0 submodule.
Each schema-v1 `swan.toml` is the source for generated cartridge configuration,
semantic controls, scene IDs, budgets, and fresh-boot SwanSong plans. The SDK
owns `main`, VBlank, the single input sample per frame, scene transitions,
graphics presentation, audio ticking, and session time. Every game supplies
`swan_game_boot` plus the five scene lifecycle callbacks and consumes the
immutable `swan_frame_t` input snapshot.

The portable `model.c` in each game remains the single rules implementation
compiled by both the ROM and its host gameplay-path test. The old
`librf_swan.a` is no longer built or linked. Existing approved gameplay art is
temporarily drawn through `swan/legacy.h` tile helpers in the renderer only;
startup, input, timing, orientation, reset, and audio use native SDK APIs. A
small `shared/swan_game_runtime` adapter preserves the one-based approved intro
maps and the anthology feedback instrument without changing SDK source.

Game builds expose this override:

```sh
make -C games/orbital-courier \
  SWANSONG_SDK_DIR=/absolute/path/to/swansong-sdk
```

It defaults to the initialized `vendor/swansong-sdk` submodule, pinned to the
v0.1.0 release. The override is intended for forward-compatibility testing;
release evidence must record the SDK revision actually used.

Run the contract verifier independently with:

```sh
python3 tools/test_sdk_manifests.py
```

It is also part of `make test`. A clean release check is:

```sh
git submodule update --init
make clean test
make smoke
```

ROM execution and media capture use SwanSong exclusively.

## Fresh-boot play contract

Every game now checks in six independent SwanSong plans: neutral boot,
meaningful interaction, success or useful progress, failure or a boundary,
reset/replay, and a deterministic copy of the success/utility path. Each plan
starts at frame zero with neutral input and allows at least 60 neutral frames
before its first action. The deterministic plan is byte-for-byte identical to
the success/utility plan; SwanSong executes the unchanged plan twice and rejects
divergent structured, PNG, or WAV evidence.

Plans use physical WonderSwan inputs. For horizontal games, primary directions
are `x3` up, `x2` right, `x1` down, and `x4` left. Ordinary presses last
one frame and are followed by at least two neutral frames. Longer holds are
allowed only for mechanics that require them: Harpoon Moon charge/lure, One
Last Lap acceleration/braking, and Pocket Kaiju Observatory hiding.
The manifest verifier caps every plan at 1500 frames (about 19.9 seconds at
75.472 Hz) and 96 input transitions; the current maxima are 1300 frames and
83 transitions.

Scenarios that intentionally trigger tones declare `audio = true`. Mote Sound
Terminal requires audio evidence for all six scenarios because playback is the
game's core output. A scenario is not considered visually or aurally correct
until an agent inspects the returned PNG and, when declared, WAV evidence;
successful execution and changing hashes are only transport checks.

All current cartridges declare `save_type = "none"` and `rtc = false`, so
the contract set does not invent save/restart or fixed-RTC scenarios.

## Initial budget baseline

These figures come from the pinned v0.1.0 SDK's `swan report --json` after the
ten lifecycle builds. Values are decimal bytes and show actual usage followed
by the applicable project gate or hardware ceiling. Every report has an empty
`budgetFailures` list.

| Game | ROM / gate | Linked IRAM / 64 KiB | Mono / 16 KiB | Color / 48 KiB | Tiles / gate | Palettes / gate | Sprites / gate (scanline) | Audio / gate |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| Mote Sound Terminal | 131072 / 262144 | 35852 / 65536 | 13304 / 16384 | 22548 / 49152 | 0 / 512 | 0 / 16 | 0 / 128 (0 / 32) | 0 / 8192 |
| Orbital Courier | 131072 / 262144 | 35804 / 65536 | 13256 / 16384 | 22548 / 49152 | 0 / 512 | 0 / 16 | 0 / 128 (0 / 32) | 0 / 8192 |
| Scrapframe Garage | 131072 / 262144 | 35808 / 65536 | 13260 / 16384 | 22548 / 49152 | 0 / 512 | 0 / 16 | 0 / 128 (0 / 32) | 0 / 8192 |
| Radio Ghost | 131072 / 262144 | 35832 / 65536 | 13284 / 16384 | 22548 / 49152 | 0 / 512 | 0 / 16 | 0 / 128 (0 / 32) | 0 / 8192 |
| Harpoon Moon | 131072 / 262144 | 35805 / 65536 | 13257 / 16384 | 22548 / 49152 | 0 / 512 | 0 / 16 | 0 / 128 (0 / 32) | 0 / 8192 |
| Turncoat Tactics | 131072 / 262144 | 35864 / 65536 | 13316 / 16384 | 22548 / 49152 | 0 / 512 | 0 / 16 | 0 / 128 (0 / 32) | 0 / 8192 |
| Pocket Kaiju Observatory | 131072 / 262144 | 35819 / 65536 | 13271 / 16384 | 22548 / 49152 | 0 / 512 | 0 / 16 | 0 / 128 (0 / 32) | 0 / 8192 |
| Rotate Dungeon | 131072 / 262144 | 35803 / 65536 | 13255 / 16384 | 22548 / 49152 | 0 / 512 | 0 / 16 | 0 / 128 (0 / 32) | 0 / 8192 |
| One Last Lap | 131072 / 262144 | 35804 / 65536 | 13256 / 16384 | 22548 / 49152 | 0 / 512 | 0 / 16 | 0 / 128 (0 / 32) | 0 / 8192 |
| Bug Witch | 131072 / 262144 | 35822 / 65536 | 13274 / 16384 | 22548 / 49152 | 0 / 512 | 0 / 16 | 0 / 128 (0 / 32) | 0 / 8192 |

Each manifest also reserves 8192 bytes of game work RAM against a 16384-byte
project gate. The zero tile, palette, and audio-source counts are exact SDK
report values, but they describe SDK-generated assets only: approved graphics
remain compile-time C data behind the renderer shim, and the shared feedback
instrument is synthesized at runtime. They therefore must not be read as peak
hardware VRAM or proof of silence. Sprite and scanline use really are zero;
these versions draw characters into background tilemaps.

## Lessons to feed back into the framework

| Migration canary | Friction found | Framework/process response |
|---|---|---|
| Orbital Courier | Model, renderer, and route data used to be coupled. | The production model is now shared with host tests; keep view adapters thin and rules out of scene callbacks. |
| Mote Sound Terminal | Existing sequences emit direct note events rather than SDK pattern assets. | The shared SDK feedback adapter preserves the current utility; a future pattern importer must be proven against SwanSong WAV evidence before replacement. |
| Rotate Dungeon | Orientation changes during play; cartridge orientation is only the boot default. | Runtime orientation is now changed through `swan_core_set_vertical`; keep testing both icon states. |
| All ten games | Direction helpers intentionally accept both WonderSwan key clusters. | Manifests and the SDK helper path preserve both clusters with horizontal X3 up, X2 right, X1 down, and X4 left. |
| All ten games | Approved art is generated by a repository-wide pipeline, outside each game's root. | Add a workspace-safe asset-root mechanism or migrate source declarations without weakening path traversal checks. |
| All ten games | The legacy engine and SDK both used to own startup and VBlank. | The SDK is now the sole runtime owner; regression checks reject game `main` functions and legacy engine linkage. `swan/legacy.h` remains renderer-only. |
| SwanSong contracts | Repeated scenario wiring and input-drain mistakes are easy to introduce by hand. | Require the six-plan fresh-boot matrix, validate physical press/hold timing, pair deterministic plans with success paths, and declare audio evidence per scenario. |
| Radio Ghost audio evidence | A short isolated lock could leave a near-silent final capture even though repeated bad locks were audibly nonzero. | Align the capture window with transient feedback and require a repeated-event path before accepting short audio cues. |

After each new game, update this table with actual ROM/RAM/VRAM/audio reports
and turn repeated friction into an SDK API, generator rule, recipe,
documentation item, or regression test before starting the next game.
