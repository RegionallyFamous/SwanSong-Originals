# SwanSong SDK integration record

## Current integration boundary

All ten cartridges build against the content-addressed SwanSong SDK v0.2.0
submodule. Every schema-v1 `swan.toml` records the resolved 0.2.0 payload hash,
so `swan doctor` and `swan release` reject an accidental SDK substitution. The
manifest remains the source for cartridge configuration, semantic controls,
scene IDs, budgets, and fresh-boot SwanSong plans. The SDK owns `main`, VBlank,
the single input sample per frame, scene transitions, graphics presentation,
audio ticking, and session time. Every game supplies `swan_game_boot` plus the
five scene lifecycle callbacks and consumes the immutable `swan_frame_t` input
snapshot.

The portable `model.c` in each game remains the single rules implementation
compiled by both the ROM and its host gameplay-path test. The old
`librf_swan.a` is no longer built or linked. Orbital Courier is the first full
asset canary: approved PNG sources are declared in its manifest, compiled by
Wonderful behind `swan assets`, and rendered exclusively through `swan_gfx_*`
and typed generated symbols. The remaining nine approved renderers temporarily
use `swan/legacy.h`; startup, input, timing, orientation, reset, and audio are
already native SDK APIs. A small `shared/swan_game_runtime` adapter preserves
their one-based intro maps and the anthology feedback instrument without
changing SDK source.

Game builds expose this override:

```sh
make -C games/orbital-courier \
  SWANSONG_SDK_DIR=/absolute/path/to/swansong-sdk
```

It defaults to the initialized `vendor/swansong-sdk` submodule, pinned to the
v0.2.0 release. The override is intended for forward-compatibility testing;
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
75.472 Hz) and 96 input transitions; the current maxima are 1350 frames and
83 transitions.

Scenarios whose final capture window intentionally contains a tone declare
`audio = true`. Mote Sound Terminal requires audio evidence for all six
scenarios because playback is the game's core output. Orbital Courier's reset
scenario deliberately leaves the flag unset: its stronger reset contract is a
silent final-window WAV byte-identical to neutral boot, while the accumulated
failure prefix and the dedicated boundary scenario retain non-silent feedback
evidence. A scenario is not considered visually or aurally correct until an
agent inspects the returned PNG and the applicable WAV evidence; successful
execution and changing hashes are only transport checks.

All current cartridges declare `save_type = "none"` and `rtc = false`, so
the contract set does not invent save/restart or fixed-RTC scenarios.

## Initial budget baseline

The original ten-game lifecycle baseline was measured before asset migration.
Orbital Courier's row now comes from SDK v0.2.0 `swan report --json`; the other
nine rows remain their pre-asset-migration baseline until each canary is
converted. Values are decimal bytes and show actual usage followed by the
applicable project gate or hardware ceiling. Every measured report has an empty
`budgetFailures` list.

| Game | ROM / gate | Linked IRAM / 64 KiB | Mono / 16 KiB | Color / 48 KiB | Tiles / gate | Palettes / gate | Sprites / gate (scanline) | Audio / gate |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| Mote Sound Terminal | 131072 / 262144 | 35852 / 65536 | 13304 / 16384 | 22548 / 49152 | 0 / 512 | 0 / 16 | 0 / 128 (0 / 32) | 0 / 8192 |
| Orbital Courier | 131072 / 262144 | 35805 / 65536 | 13257 / 16384 | 22548 / 49152 | 310 / 512 | 1 / 16 | 0 / 128 (0 / 32) | 0 / 8192 |
| Scrapframe Garage | 131072 / 262144 | 35808 / 65536 | 13260 / 16384 | 22548 / 49152 | 0 / 512 | 0 / 16 | 0 / 128 (0 / 32) | 0 / 8192 |
| Radio Ghost | 131072 / 262144 | 35832 / 65536 | 13284 / 16384 | 22548 / 49152 | 0 / 512 | 0 / 16 | 0 / 128 (0 / 32) | 0 / 8192 |
| Harpoon Moon | 131072 / 262144 | 35805 / 65536 | 13257 / 16384 | 22548 / 49152 | 0 / 512 | 0 / 16 | 0 / 128 (0 / 32) | 0 / 8192 |
| Turncoat Tactics | 131072 / 262144 | 35864 / 65536 | 13316 / 16384 | 22548 / 49152 | 0 / 512 | 0 / 16 | 0 / 128 (0 / 32) | 0 / 8192 |
| Pocket Kaiju Observatory | 131072 / 262144 | 35819 / 65536 | 13271 / 16384 | 22548 / 49152 | 0 / 512 | 0 / 16 | 0 / 128 (0 / 32) | 0 / 8192 |
| Rotate Dungeon | 131072 / 262144 | 35803 / 65536 | 13255 / 16384 | 22548 / 49152 | 0 / 512 | 0 / 16 | 0 / 128 (0 / 32) | 0 / 8192 |
| One Last Lap | 131072 / 262144 | 35804 / 65536 | 13256 / 16384 | 22548 / 49152 | 0 / 512 | 0 / 16 | 0 / 128 (0 / 32) | 0 / 8192 |
| Bug Witch | 131072 / 262144 | 35822 / 65536 | 13274 / 16384 | 22548 / 49152 | 0 / 512 | 0 / 16 | 0 / 128 (0 / 32) | 0 / 8192 |

Each manifest also reserves 8192 bytes of game work RAM against a 16384-byte
project gate. Orbital Courier owns 369 unique generated tiles across two
mutually exclusive scene groups: the intro peaks at 310 tiles and gameplay at
59, each with one palette. The remaining zero tile and palette counts still
describe SDK-generated assets only: those nine approved graphics remain
compile-time C data behind their renderer shims. Audio-source counts remain
zero because the shared feedback instrument is synthesized at runtime. Sprite
and scanline use really are zero; these versions draw characters into
background tilemaps.

## Lessons to feed back into the framework

| Migration canary | Friction found | Framework/process response |
|---|---|---|
| Orbital Courier | Model, renderer, and route data used to be coupled; the approved art lived in C arrays invisible to SDK ownership reports. | The production model remains shared with host tests. Native PNG intro/metatile sources now pass through `swan assets`, scene groups report 310/59 peak tiles and one palette, and the renderer uses generated map regions instead of converter tile IDs or `swan/legacy.h`. Add generated graphic source discovery to a future SDK build fragment so anthology Makefiles do not list those derived C filenames. |
| Mote Sound Terminal | Existing sequences emit direct note events rather than SDK pattern assets. | The shared SDK feedback adapter preserves the current utility; a future pattern importer must be proven against SwanSong WAV evidence before replacement. |
| Rotate Dungeon | Orientation changes during play; cartridge orientation is only the boot default. | Runtime orientation is now changed through `swan_core_set_vertical`; keep testing both icon states. |
| All ten games | Direction helpers intentionally accept both WonderSwan key clusters. | Manifests and the SDK helper path preserve both clusters with horizontal X3 up, X2 right, X1 down, and X4 left. |
| All ten games | Approved art is generated by a repository-wide pipeline, outside each game's root. | Add a workspace-safe asset-root mechanism or migrate source declarations without weakening path traversal checks. |
| All ten games | The legacy engine and SDK both used to own startup and VBlank. | The SDK is now the sole runtime owner; regression checks reject game `main` functions and legacy engine linkage. `swan/legacy.h` remains renderer-only in the nine games not yet converted. |
| Per-project CLI tests | The anthology root owned host-test execution, so SDK v0.2 `swan test` found no `test` target inside an individual cartridge. | The shared game build fragment now maps each cartridge to its exact production-C host binary and exposes a real `test` target. A repository invariant prevents this CLI contract from disappearing during later migrations. |
| Shared feedback audio | Reinitializing the sequencer for every cue made rapid input/evidence paths unreliable even though every game already initialized it at boot. | Initialize the shared instrument once in `swan_game_boot`, then update/play cues without resetting global sequencer state. Keep interaction plans far enough apart to inspect each visible and audible response. |
| Orbital scene readiness | Re-rendering the complete 14-by-8 metatile view for 76 consecutive frames made one-frame SwanSong edges land while the title was still doing scene work. The first fuel-drain edge could be missed, so the apparent replay mismatch was actually a failure path that never reached zero fuel. | Render once per invalidation, keep scene-entry work bounded, and start gameplay evidence only after the documented scene-ready point. The replay plan now begins at frame 90, observes the zero-fuel result before A, then settles for 59 neutral frames. Its final PNG and final-window WAV are byte-exact with neutral boot. Add an SDK scene-ready marker or incremental-render budget diagnostic before later migrations repeat this friction. |
| SwanSong contracts | Repeated scenario wiring and input-drain mistakes are easy to introduce by hand. | Require the six-plan fresh-boot matrix, validate physical press/hold timing, pair deterministic plans with success paths, and declare audio evidence per scenario. |
| Radio Ghost audio evidence | A short isolated lock could leave a near-silent final capture even though repeated bad locks were audibly nonzero. | Align the capture window with transient feedback and require a repeated-event path before accepting short audio cues. |

After each new game, update this table with actual ROM/RAM/VRAM/audio reports
and turn repeated friction into an SDK API, generator rule, recipe,
documentation item, or regression test before starting the next game.
