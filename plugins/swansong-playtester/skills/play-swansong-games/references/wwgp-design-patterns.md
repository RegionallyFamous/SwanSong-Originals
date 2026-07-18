# Clean-room WonderSwan design patterns

Use this reference to choose playtest coverage and identify reusable SDK work.
It summarizes behavior patterns visible in public descriptions of historical
WonderSwan homebrew; it is not a source-code or asset library.

## High-value patterns

| Pattern | Required evidence | Reusable SDK opportunity |
|---|---|---|
| Dual-cluster gestures | Tap, double-tap, hold threshold, release, simultaneous chord, orientation mapping, and input drain from fresh boot | Declarative gesture state over semantic actions |
| Dual-actor control | Independent X/Y movement, same-frame input, collision outcomes, and deterministic reset | Two action maps sharing one sampled input frame |
| Compact action loop | Score change, hazard, boundary, game over, result, replay, and fixed-pool exhaustion | Wave, round, combo, rank, and results modules |
| Grid or board loop | Legal move, combo, no-move/deadlock, deterministic shuffle, timer, save, and reset | Grid matching and turn-controller modules |
| Procedural rooms | Seed identity, reachability, automap, boss access, resource bounds, and unchanged replay | Room generator plus reachability validator |
| Audio-led play | Cue onset, expected left/center/right energy, hit/miss, timeout, silence, clipping, steals, and reset | Spatial metrics and audio scenario thresholds |
| Suspend and persistence | Suspend commit, power-cycle resume, one-shot consumption, profile/high-score retention, corruption fallback, and clean reset | Separate checkpoint, suspend, profile, and settings records |
| Utility editor | Wraparound cursor, text entry, long-press alternatives, delete, dirty state, save/open, and utility outcome | `utility-app` recipe and fixed-capacity editor widgets |
| Capacity pressure | Per-scene ROM/RAM/VRAM, sprite scanline pressure, audio work area, and feature-level deltas | Build-time budgets and Studio previews |
| Frame cadence | Exact input transition delivery, game update/session ticks, missed VBlanks, dirty-region cost, and unchanged replay | Input bridge fixture plus profiler traces that distinguish host frames from sampled game frames |

## Scenario packs

- `micro_action`: neutral, score, hit/boundary, game over, replay; fuzz holds,
  double-taps, and chords.
- `dual_actor`: independent and simultaneous X/Y movement, collision
  divergence, and reset.
- `grid_puzzle`: legal match, combo/rank, deadlock, deterministic reset, and
  save/resume.
- `audio_locator`: fixed spatial seed, left/center/right cue, hit/miss, timeout,
  dropout, and audio reset.
- `utility_editor`: enter, alternate, edit, delete, save, fresh-boot reopen,
  corrupt-save fallback, and discard/reset.
- `procedural_rooms`: seed, reachability, automap, boss access, boundary,
  resource exhaustion, and replay identity.

## Learning loop

After each title, record friction and choose exactly one durable home for every
recurring problem: runtime API, asset rule, recipe, documentation, or regression
test. Keep game-specific rules, art direction, names, story, and authored content
in the game.

Treat milestones and endings separately. A route planner or model test can prove
that completion is reachable, but the play verdict still requires the shipping
ROM to display the final result through SwanSong. If a transition is missed,
measure both the scheduled host frame and the sampled game tick before changing
button durations or assigning blame to the emulator bridge.

## Clean-room boundary

- Treat public descriptions as requirements evidence only.
- Create original names, art, music, text, maps, and interface composition.
- Do not redistribute historical collection archives or extract third-party
  source, binaries, screenshots, or assets without clear permission.
- Record SPDX/license and provenance for every shipped dependency and asset.
- Send intentionally close commercial homages for legal review; generic mechanics
  are not a substitute for clearance.
