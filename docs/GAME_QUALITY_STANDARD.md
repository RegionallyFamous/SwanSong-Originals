# SwanSong Originals game quality standard

This is the release contract for rebuilding SwanSong Originals from complete
microgames into games that invite a first play, teach themselves, respond with
clarity, and reward another run. Passing compilation, deterministic replay, or
a changing screenshot is necessary but not sufficient.

`python3 tools/test_game_quality.py --mode report` inventories the current
collection without failing. Strict mode is reserved for a game that has been
deliberately rebuilt and has all required artifacts:

```sh
python3 tools/test_game_quality.py --mode strict --game one-last-lap
```

Do not add a game to a strict aggregate target merely to make the report look
better. It joins strict validation only after the game, manifest, captured
media, runtime profile, and local human-playtest summary actually satisfy this
document.

## Evidence boundary

The quality audit can establish that required scenes and assets are declared,
source hooks exist, plans are long enough to expose obvious pacing problems,
native images have the expected dimensions, budgets are not hardware-max
placeholders, and evidence summaries bind inspected artifacts.

It cannot establish that a game is fun, that its music is appealing, that art
is readable on every physical display, or that a new player understands it.
Those claims require people to play the cartridge without coaching. A local
human-playtest summary is consequently a separate strict requirement; it is
not inferred from SwanSong automation.

## WonderSwan hardware contract

The hard facts below come from Wonderful and the WSdev hardware documentation.
The production limits are conservative SwanSong Originals release policy, not
additional hardware claims.

| Area | Hardware fact | Originals production limit |
|---|---:|---:|
| Physical display | 224×144 pixels | exact 224×144 evidence; vertical work is authored/normalized as 144×224 |
| Visible tile grid | 28×18 8×8 tiles | all HUD, prompts, and hit feedback stay in this viewport |
| Background maps | two 32×32 tile maps | use the second layer for HUD, overlay, depth, or transition work |
| CPU | NEC V30MZ at 3.072 MHz | no floating point, heap, recursion, or unbounded work |
| Video cadence | about 75.472 Hz; 40,704 CPU clocks per frame | p95 ≤30,528 cycles; maximum ≤37,999; zero missed VBlanks |
| Shared WSC RAM | 64 KiB total | linked total ≤56 KiB |
| Mono-compatible RAM area | 16 KiB | linked mono area ≤14 KiB |
| Color extension | 48 KiB | linked Color area ≤42 KiB |
| WSC 2BPP tiles | up to 1,024; sprites use the first 512 | 448 peak, leaving 64 tiles of the 512-tile production reservation free |
| Common tiles | project policy | ≤96 for font, HUD, shared effects, and actors |
| Palettes | 16 | ≤12 peak and no critical color-only distinction |
| Sprites | 128 total | ≤64 visible |
| Sprites per scanline | 32; later entries disappear | ≤24, warning at 20 |
| Native audio | four 32×4-bit wavetable channels; shared 64-byte wave block | ≤12 instruments normally; ≤32 KiB total authored audio |
| Audio output | 24 kHz; internal speaker is mono | every required cue survives the mono mix |
| Inputs | X1–X4, Y1–Y4, A, B, Start | both directional clusters navigate title, menus, help, and pause |
| ROM | SDK release ceiling 8 MiB | target ≤256 KiB; strict ceiling ≤512 KiB per game |

At 75.472 Hz one frame is about 13.25 ms. The strict presentation response
gate is visible or audible acknowledgement by the second observed frame after
an action. Palette flashes normally last 3–6 frames, hit-stop 2–4 frames,
one- or two-pixel scroll shake 4–8 frames, and scene fades/wipes 12–30 frames.
These are defaults, not permission to add constant visual noise.

The 448-tile production ceiling deliberately keeps the anthology within the
512-tile reservation even though WonderSwan Color backgrounds can address
1,024 tiles. Sprite art must remain in the first 512 indices. In 2BPP modes,
each tile uses four palette entries; the asset review must also respect the
hardware's opaque versus transparent palette classes.

## Required player-facing structure

Every strict game has this observable flow:

```text
Boot → persistent Title → Menu → playable Tutorial → Gameplay ↔ Pause
       ↓ idle 8–12 s                              ↓
       deterministic Attract demo              Results → Retry / Next / Title
```

- The title waits for semantic confirm input. It never exits because a boot
  counter reached a fixed number.
- The title shows deliberately authored lettering. Image-generated concept art
  may supply an illustration, but generated-image text is not a shippable logo.
- After 604–906 idle frames, a deterministic attract route demonstrates the
  core action. Any real input exits the demo, drains held input, and returns to
  title/menu control.
- New players learn one action through play, see and hear confirmation, and
  then receive the next action. There is no opening instruction wall.
- The immediate objective is visible through world composition, animation, or
  a compact icon HUD.
- Pause offers Resume, Controls, Restart, and Title with consistent focus and
  cancel behavior.
- Results visibly distinguish success and failure, compare a result with a
  saved best where applicable, offer one useful improvement cue, and put Retry
  no more than two actions away.
- Assist, Standard, and Challenge change named barriers such as grace windows,
  hints, rival pressure, or time. Hidden difficulty manipulation is not used.
- A complete run produces a record, alternate ending, unlock, or mastery target
  worth revisiting. A bare reset to the same state is not progression.

An audio utility may map `quality.scenes.result` to a durable utility outcome
screen rather than a win/loss screen, but it must still provide title,
gameplay/utility, and outcome evidence.

## Art, motion, and information density

Each title has a one-sentence visual thesis of its own: a dominant silhouette,
palette logic, material language, and motion motif tied to the mechanic. The
framework supplies neutral primitives, never an anthology-wide skin. Title,
gameplay, and result screens remain recognizably related without being the same
full-screen image under different text.

- Keep asset provenance, source hashes, licenses, conversion settings, and the
  reviewed native output with the game. Generated concept images are source
  material, not anonymous final proof.
- Lettering, icons, grids, and repeated mechanical forms are deliberately drawn
  on the native pixel grid. Reject malformed glyphs, merged silhouettes,
  inconsistent perspective, accidental texture, and decorative shapes that
  resemble controls but have no function.
- Use sprite animation and the second tile layer to establish depth, state, and
  response. A static illustrated backdrop does not substitute for moving game
  objects or interaction feedback.
- Prefer a visible objective, a compact icon or meter, and contextual prompts
  over paragraphs. Help may explain nuance, but a first action never depends on
  reading a wall of text.
- Inspect the title, densest gameplay state, every distinct result, transitions,
  and animation on the native raster and a physical display. Human art review
  remains required even when every conversion and capacity check passes.

## Feedback and audio

An important action uses at least two information channels: animation and
sound, visual displacement and HUD change, or another equally clear pair.
Color alone is never the only signal for danger, success, selection, or state.

Strict games declare at least:

- two music assets, normally title and gameplay;
- six prioritized SFX assets covering navigation, confirmation,
  action/progress, blocked/error, success, and failure; and
- direct SDK playback calls or manifest-named wrapper hooks visible in the
  game source.

The recommended production bank also includes cancel, pickup, damage/danger,
and a separate major-result cue. Action games normally leave one voice
available for important effects. A music-focused utility may use all four
voices but still needs deterministic effect arbitration for its UI.

Gameplay music provides at least 32 seconds of distinct A/B/turnaround form;
45–75 seconds is preferred for puzzle and exploratory play. A title loop may
be 8–20 seconds. The hook should survive with melody and bass alone, each
channel should have a primary role, and the loop boundary must sound composed.

Current shared one-note beeps are not an SFX bank. Before background music is
accepted broadly, the SDK/runtime must choose a silent effect voice first,
steal only an eligible lowest-priority voice, and restore the resolved held
music state rather than replaying an unresolved `NO_CHANGE` row.

Every audio release scenario returns a decoded WAV. A person listens through
at least one complete loop and its seam, checks speaker-safe mono, and rejects
silence, clipping, stuck notes, missing parts, or effects that erase the music.

## Manifest production declaration

Strict games add a project-owned table to `swan.toml`. The current SDK reserves
unknown top-level data for forward-compatible tools, so this declaration
documents production evidence without moving game rules into Desktop.

```toml
[quality]
standard_version = 1
attract_after_frames = 755
menu_uses_both_clusters = true
records = ["best_time", "cooperative_ending", "challenge_clear"]

# Optional only when a shared wrapper is used instead of the direct SDK call.
music_runtime_hook = "game_music_play"
sfx_runtime_hook = "game_sfx_play"
save_load_hook = "game_progress_load"
save_store_hook = "game_progress_store"

[quality.scenes]
title = "title"
gameplay = "race"
tutorial = "tutorial"
pause = "pause"
result = "result"

[quality.evidence]
title_png = "evidence/quality/title.png"
gameplay_png = "evidence/quality/gameplay.png"
result_png = "evidence/quality/result.png"
audio_wav = "evidence/quality/gameplay-mix.wav"
media_observation = "evidence/quality/media-observation.json"
runtime_profile = "evidence/quality/runtime-profile.json"
human_playtest = "evidence/quality/human-playtest.json"
```

All paths remain inside the game directory. Horizontal evidence is an exact
224×144 gameplay viewport; vertical evidence is a normalized exact 144×224
viewport. A wider SwanSong full-console capture containing LCD icon segments
may be retained in addition, but does not replace the viewport PNG. The WAV is
the decoded SwanSong output actually reviewed; its current hash must match the
media-observation record.

The cartridge also declares a save medium and positive payload. A 1 KiB EEPROM
with a payload target of 256 bytes is sufficient for options, tutorial state,
records, and small unlock sets while leaving room for the SDK's two-slot
journal. The game loads and transactionally stores this state through the SDK.

## Required evidence documents

These compact summaries deliberately contain no names, email addresses,
device identifiers, wall-clock timestamps, or free-form player transcripts.

### Inspected media

```json
{
  "schema": "swan-song-game-quality-media-observation-v1",
  "game": "one-last-lap",
  "localOnly": true,
  "screens": {
    "title": {"sha256": "<64 lowercase hex>", "inspected": true},
    "gameplay": {"sha256": "<64 lowercase hex>", "inspected": true},
    "result": {"sha256": "<64 lowercase hex>", "inspected": true}
  },
  "audio": {
    "wavSHA256": "<64 lowercase hex>",
    "listened": true,
    "loopBoundaryInspected": true
  }
}
```

### SwanSong runtime profile

```json
{
  "schema": "swan-song-game-quality-runtime-profile-v1",
  "game": "one-last-lap",
  "source": "SwanSong",
  "durationFrames": 22642,
  "p95Cycles": 30000,
  "maxCycles": 37000,
  "missedVBlanks": 0,
  "linkedInternalRAMBytes": 50000,
  "linkedMonoAreaBytes": 14000,
  "linkedColorAreaBytes": 36000,
  "peakTiles": 420,
  "peakPalettes": 10,
  "peakSprites": 42,
  "peakSpritesPerScanline": 19,
  "romBytes": 262144
}
```

The profile covers at least five minutes, including title, attract, gameplay,
heavy feedback, results, and replay. Until SwanSong exports emulated cycle and
missed-VBlank measurements, no game may fabricate this document or claim the
strict CPU gate has passed.

### Local human playtest

```json
{
  "schema": "swan-song-game-quality-human-playtest-v1",
  "game": "one-last-lap",
  "localOnly": true,
  "containsIdentifiers": false,
  "participants": 5,
  "noCoaching": true,
  "goalWithin30Seconds": 4,
  "firstMeaningfulActionWithin45Seconds": 4,
  "resultUnderstood": 4,
  "permanentlyStuck": 0,
  "voluntaryRetries": 3
}
```

This five-person gate is the minimum rebuild checkpoint. At least 80% must meet
each comprehension threshold and at least 60% must voluntarily retry. Before a
public production release, test at least 12 people across the intended skill
groups; retain the same aggregate, identifier-free form. Repeated confusion
from two people remains a design blocker even if the numeric minimums pass.

## Automated scenario gate

Each rebuilt game adds these exact fresh-boot scenario IDs:

1. `neutral` — persistent title and menu;
2. `attract` — title idle → attract → input return;
3. `menu-input` — both directional clusters in menu/help;
4. `tutorial` — first-run lesson and tutorial replay/skip;
5. `interaction` — meaningful gameplay interaction;
6. `pause` — pause/resume and restart;
7. `success` — complete success or utility outcome;
8. `failure` — distinct failure or safe boundary behavior;
9. `reset` — results and retry;
10. `save-restart` — new save, continue, and corrupt-save recovery;
11. `audio` — complete BGM loop plus effect arbitration; and
12. `deterministic` — exact replay and reset.

`success`, `failure`, `reset`, and `deterministic` each bind a checked-in
`swan-scenario-outcome-contract-v1`; any additional ending scenario does too.
The contracts gate final scene, ending, progress/reset semantics, and required
audio markers through the trace ROM. Prose `required_checks` remain the human
inspection guide, not a machine verdict. A successful SwanSong process is not
allowed to pass a scenario whose declared ending was never reached.

Canonical game-state hashes come from one portable model field inventory used
to declare the state and hash it field by field in a fixed byte order. Never
hash C struct bytes or padding. Host coverage mutates every inventory field and
requires the hash to change, so adding score, AI, hazard, or result state cannot
silently escape deterministic replay evidence.

RTC cartridges additionally provide `rtc` with fixed-time, unavailable,
power-loss, and new-boot time-travel coverage. The five-minute runtime profile
supplies soak evidence. The seeded SDK fuzzer runs separately because generated
input search is not a hand-authored scenario; findings must be resolved or
minimized before release.

The canonical success plan must span at least 4,529 frames, approximately one
minute at the native refresh rate. This is an early warning against another
single-screen microloop, not a claim about human session length. Target human
sessions and difficulty are established by playtesting.

## Audit commands

```sh
# Descriptive baseline; exits successfully when the audit itself runs.
make quality

# A selected rebuilt game must have zero blockers.
make quality-strict GAME=one-last-lap

# Machine-readable report for Studio or CI.
python3 tools/test_game_quality.py --mode report --json
```

Report/baseline mode fails only for malformed audit inputs or an incomplete
collection. Strict mode fails for every production finding. One Last Lap is
the intended first strict canary, but it is not placed in a passing strict list
until its rebuild and evidence are real.

## Primary hardware sources

- [Wonderful Toolchain WonderSwan target](https://wonderful.asie.pl/doc/general/target-wonderswan/)
- [Wonderful WonderSwan platform overview](https://wonderful.asie.pl/wiki/doku.php?id=wswan%3Aplatform_overview)
- [WSdev display](https://ws.nesdev.org/wiki/Display)
- [WSdev tile data](https://ws.nesdev.org/wiki/Display/Tile_Data)
- [WSdev sprites](https://ws.nesdev.org/wiki/Display/Sprites)
- [WSdev palette](https://ws.nesdev.org/wiki/Display/Palette)
- [WSdev timing](https://ws.nesdev.org/wiki/Timing)
- [WSdev memory map](https://ws.nesdev.org/wiki/Memory_map)
- [WSdev keypad](https://ws.nesdev.org/wiki/Keypad)
- [WSdev sound](https://ws.nesdev.org/wiki/Sound)

The enjoyment criteria are informed by the
[GameFlow model](https://www.valuesatplay.org/wp-content/uploads/2007/09/sweetser.pdf)—clear
goals, feedback, control, and challenge matched to skill—and by research
connecting competence and autonomy with game enjoyment
([Przybylski, Rigby, and Ryan](https://selfdeterminationtheory.org/wp-content/uploads/2014/04/2010_PrzybylskiRigbyRyan_ROGP.pdf)).
The multimodal feedback, control, difficulty, and menu rules also draw on
Microsoft's [additional cue channels](https://learn.microsoft.com/en-us/xbox/accessibility/xbox-accessibility-guidelines/103),
[input](https://learn.microsoft.com/en-us/xbox/accessibility/xbox-accessibility-guidelines/107),
[difficulty](https://learn.microsoft.com/en-us/xbox/accessibility/xbox-accessibility-guidelines/108),
and [UI navigation](https://learn.microsoft.com/en-us/xbox/accessibility/xbox-accessibility-guidelines/112)
guidance. These are design inputs and hypotheses to test with the actual
audience, not substitutes for play.
