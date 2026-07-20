# SwanSong SDK integration record

## Current integration boundary

All ten cartridges build against the content-addressed SwanSong SDK v0.5.0
submodule. Every schema-v1 `swan.toml` records the resolved 0.5.0 payload hash,
so `swan doctor` and `swan release` reject an accidental SDK substitution. The
manifest remains the source for cartridge configuration, semantic controls,
scene IDs, budgets, and fresh-boot SwanSong plans. The SDK owns `main`, VBlank,
the single input sample per frame, scene transitions, graphics presentation,
audio ticking, and session time. Every game supplies `swan_game_boot` plus the
five scene lifecycle callbacks and consumes the immutable `swan_frame_t` input
snapshot.

The portable `model.c` in each game remains the single rules implementation
compiled by both the ROM and its host gameplay-path test. The old
`librf_swan.a` is no longer built or linked. Orbital Courier and Mote Sound
Terminal are the first full asset canaries: approved PNG sources are declared
in their manifests, compiled by Wonderful behind `swan assets`, and rendered
exclusively through `swan_gfx_*` and typed generated symbols. Mote also compiles
its three checked-in TOML patterns into typed sequencer rows. The remaining
eight approved renderers temporarily use `swan/legacy.h`; startup, input,
timing, orientation, reset, and audio are already native SDK APIs. A small
`shared/swan_game_runtime` adapter preserves their one-based intro maps and the
anthology feedback instrument without changing SDK source.

Game builds expose this override:

```sh
make -C games/orbital-courier \
  SWANSONG_SDK_DIR=/absolute/path/to/swansong-sdk
```

It defaults to the initialized `vendor/swansong-sdk` submodule, pinned to the
v0.5.0 release. The override is intended for forward-compatibility testing;
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

Every game checks in six required SwanSong plans: neutral boot, meaningful
interaction, a complete success or utility outcome, failure or a boundary,
reset/replay, and a deterministic copy of the success/utility path. Games with
alternate endings may add named outcome scenarios. Each plan starts at frame
zero with neutral input and waits for its declared fresh-boot readiness boundary
before its first action. The deterministic plan is byte-for-byte identical to
the success/utility plan; SwanSong executes the unchanged plan twice and rejects
divergent structured, PNG, or WAV evidence.

Plans use physical WonderSwan inputs. For horizontal games, primary directions
are `x3` up, `x2` right, `x1` down, and `x4` left. Ordinary presses last one
frame and are followed by at least two neutral frames. Longer holds are
allowlisted only when the interaction or evidence window needs them: Harpoon
Moon charge/lure, One Last Lap racing/towing, and Pocket Kaiju Observatory
camera, shutter, zoom, and hiding actions. The manifest verifier caps every
plan at 2700 frames (about 35.8 seconds at 75.472 Hz) and 192 input events; the
current maxima are 2380 frames and 181 events.

Scenarios declare an `audio_expectation` of audible, silent, or unconstrained;
the v0.2 `audio = true` spelling remains accepted as audible during migration.
Mote Sound Terminal requires audible evidence for all six utility paths,
including the deterministic row-zero restart after reset. Orbital Courier's reset
scenario also declares silence: its final-window WAV is byte-identical to
neutral boot, while the accumulated failure prefix and the dedicated boundary
scenario retain non-silent feedback evidence. A scenario is not considered
visually or aurally correct until an agent inspects the returned PNG and WAV;
successful execution and changing hashes are only transport checks.

All current cartridges declare `save_type = "none"` and `rtc = false`, so
the contract set does not invent save/restart or fixed-RTC scenarios.

## Initial budget baseline

The original ten-game lifecycle baseline was measured before asset migration.
Orbital Courier and Mote Sound Terminal now come from the SDK
`swan report --json`; the other eight rows remain their pre-asset-migration
baseline until each canary is converted. Values are decimal bytes and show actual usage followed by the
applicable project gate or hardware ceiling. Every measured report has an empty
`budgetFailures` list.

| Game | ROM / gate | Linked IRAM / 64 KiB | Mono / 16 KiB | Color / 48 KiB | Tiles / gate | Palettes / gate | Sprites / gate (scanline) | Audio / gate |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| Mote Sound Terminal | 131072 / 262144 | 35890 / 65536 | 13342 / 16384 | 22548 / 49152 | 421 / 512 | 1 / 16 | 0 / 128 (0 / 32) | 3790 / 8192 |
| Orbital Courier | 131072 / 262144 | 35921 / 65536 | 13373 / 16384 | 22548 / 49152 | 310 / 512 | 1 / 16 | 0 / 128 (0 / 32) | 0 / 8192 |
| Scrapframe Garage | 131072 / 262144 | 35915 / 65536 | 13367 / 16384 | 22548 / 49152 | 0 / 512 | 0 / 16 | 0 / 128 (0 / 32) | 0 / 8192 |
| Radio Ghost | 131072 / 262144 | 35940 / 65536 | 13392 / 16384 | 22548 / 49152 | 0 / 512 | 0 / 16 | 0 / 128 (0 / 32) | 0 / 8192 |
| Harpoon Moon | 131072 / 262144 | 35912 / 65536 | 13364 / 16384 | 22548 / 49152 | 0 / 512 | 0 / 16 | 0 / 128 (0 / 32) | 0 / 8192 |
| Turncoat Tactics | 131072 / 262144 | 36011 / 65536 | 13463 / 16384 | 22548 / 49152 | 0 / 512 | 0 / 16 | 0 / 128 (0 / 32) | 0 / 8192 |
| Pocket Kaiju Observatory | 131072 / 262144 | 35926 / 65536 | 13378 / 16384 | 22548 / 49152 | 0 / 512 | 0 / 16 | 0 / 128 (0 / 32) | 0 / 8192 |
| Rotate Dungeon | 131072 / 262144 | 35915 / 65536 | 13367 / 16384 | 22548 / 49152 | 0 / 512 | 0 / 16 | 0 / 128 (0 / 32) | 0 / 8192 |
| One Last Lap | 131072 / 262144 | 35911 / 65536 | 13363 / 16384 | 22548 / 49152 | 0 / 512 | 0 / 16 | 0 / 128 (0 / 32) | 0 / 8192 |
| Bug Witch | 131072 / 262144 | 35929 / 65536 | 13381 / 16384 | 22548 / 49152 | 0 / 512 | 0 / 16 | 0 / 128 (0 / 32) | 0 / 8192 |

Each manifest also reserves 8192 bytes of game work RAM against a 16384-byte
project gate. Orbital Courier owns 369 unique generated tiles across two
mutually exclusive scene groups: the intro peaks at 310 tiles and gameplay at
59, each with one palette. Mote Sound Terminal owns 463 generated tiles across
its mutually exclusive groups: the intro peaks at 421, the terminal at 42, and
each uses one palette. Its three typed pattern sources total 3790 bytes. The
remaining zero tile and palette counts still describe SDK-generated assets
only: those eight approved graphics remain compile-time C data behind their
renderer shims. Audio-source counts remain zero for those games because the
shared feedback instrument is synthesized at runtime. Sprite and scanline use
really are zero; these versions draw characters into background tilemaps.

## Lessons to feed back into the framework

| Migration canary | Friction found | Framework/process response |
|---|---|---|
| Orbital Courier | Model, renderer, and route data used to be coupled; the approved art lived in C arrays invisible to SDK ownership reports. | The production model remains shared with host tests. Native PNG intro/metatile sources now pass through `swan assets`, scene groups report 310/59 peak tiles and one palette, and the renderer uses generated map regions instead of converter tile IDs or `swan/legacy.h`. Add generated graphic source discovery to a future SDK build fragment so anthology Makefiles do not list those derived C filenames. |
| Mote Sound Terminal | Restarting a one-row loop on every beat made the terminal sound like isolated test tones and left the scope disconnected from the sequencer. | Run each typed 16-row pattern continuously on four native voices, pause/resume the SDK sequencer without losing phase, restart track/tempo changes at a clear row-zero boundary, and derive each visible scope lane from live voice note/volume state. The success and reset contracts now require audible four-channel evidence. |
| Mote presentation diagnostics | Full-map and full-sprite diagnostics inside normal frame presentation could make one-frame input edges expire before the next game sample. | SDK 0.4.0 computes exact usage only when the profiler requests it. Mote still declares its actual fixed capacities and updates only changing scope columns; its six fresh-boot plans now prove control response while four voices are active. |
| Incremental asset builds | The shared Make fragment originally watched only `swan.toml` and play plans, so edited PNG or TOML sources could leave a stale generated asset stamp until a clean build. | Include every checked-in file under each game's `assets/` directory in the generated asset stamp dependencies; clean and incremental builds now converge on the same conversion inputs. |
| Rotate Dungeon | Treating room rotation as a live display-orientation change made a landscape game harder to read and test. | Keep the cartridge in landscape, expose rotation as a clear room-state icon and geometry change, and test both logical orientations without rotating the display. |
| All ten games | Direction helpers intentionally accept both WonderSwan key clusters. | Manifests and the SDK helper path preserve both clusters with horizontal X3 up, X2 right, X1 down, and X4 left. |
| All ten games | Approved art is generated by a repository-wide pipeline, outside each game's root. | Add a workspace-safe asset-root mechanism or migrate source declarations without weakening path traversal checks. |
| All ten games | The legacy engine and SDK both used to own startup and VBlank. | The SDK is now the sole runtime owner; regression checks reject game `main` functions and legacy engine linkage. `swan/legacy.h` remains renderer-only in the eight games not yet converted. |
| Per-project CLI tests | The anthology root owned host-test execution, so `swan test` found no `test` target inside an individual cartridge. | The shared game build fragment now maps each cartridge to its exact production-C host binary and exposes a real `test` target. A repository invariant prevents this CLI contract from disappearing during later migrations. |
| Shared feedback audio | Reinitializing the sequencer for every cue made rapid input/evidence paths unreliable even though every game already initialized it at boot. | Initialize the shared instrument once in `swan_game_boot`, then update/play cues without resetting global sequencer state. Keep interaction plans far enough apart to inspect each visible and audible response. |
| Orbital scene readiness | Re-rendering the complete 14-by-8 metatile view after every move made input cadence depend on camera work. | Cache the viewport and HUD, redraw the whole view only when the camera moves, and otherwise restore only the courier's previous cell plus changed HUD regions. SDK 0.4.0 removes live profiler sweeps, while the replay plan still respects the declared scene-ready boundary. |
| True outcome evidence | Several success plans originally stopped after the first correct action, even when the game documented a multi-stage ending. | A success plan must reach the actual result state. Deterministic plans are byte-for-byte copies; alternate endings get explicit scenarios; reset input follows a visibly stable result. Host planners produce canonical routes for state-heavy games. |
| Static-board rendering | Full-board redraws in Rotate Dungeon, Turncoat Tactics, and Radio Ghost could delay game updates enough to swallow short scheduled inputs. | Cache static layouts, restore only changed cells or widgets, and animate at an explicit cadence. Diagnose input delivery and game-update cadence separately. |
| Outcome variety | One Last Lap documented a moral choice but exercised only one finish. | Give materially different endings their own named fresh-boot scenarios; cooperative and solo results must end in visibly distinct states. |
| SwanSong contracts | Repeated scenario wiring and input-drain mistakes are easy to introduce by hand. | Require the six-plan fresh-boot matrix, validate physical press/hold timing, pair deterministic plans with success paths, and declare audio evidence per scenario. |
| Radio Ghost audio evidence | A short isolated lock could leave a near-silent final capture even though repeated bad locks were audibly nonzero. | Align the capture window with transient feedback and require a repeated-event path before accepting short audio cues. |

After each new game, update this table with actual ROM/RAM/VRAM/audio reports
and turn repeated friction into an SDK API, generator rule, recipe,
documentation item, or regression test before starting the next game.
