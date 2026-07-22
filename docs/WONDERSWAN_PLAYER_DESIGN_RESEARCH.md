# Designing WonderSwan games people want to replay

## Purpose and evidence boundary

This document turns hardware research, historical WonderWitch examples, and
player-experience research into implementation guidance for SwanSong Originals.
It is not a claim that any current game is fun, nor is it a substitute for the
release contract in [GAME_QUALITY_STANDARD.md](GAME_QUALITY_STANDARD.md). If the
two documents ever differ, the quality standard is normative and this document
should be updated.

Three labels are used deliberately:

- **Hardware evidence** is a documented WonderSwan or WonderSwan Color fact.
- **Originals policy** is a conservative project limit or product decision. It
  is not an undocumented hardware limit.
- **Design consequence** is an implementation hypothesis derived from the
  evidence. It must still survive profiling and uncoached human play.

Automated tests can establish that a ROM boots, responds to an input plan,
reaches a declared state, stays within measured resource limits, emits a
decoded WAV, and replays deterministically. They cannot establish that a goal
is understood, a control feels good, music is appealing, challenge is fair, or
a player wants another run. Those are human findings.

## Executive conclusion

The WonderSwan is unusually well suited to small, expressive games: a fast
75.472 Hz update, two tile layers, a large sprite list, four native sound
voices, two directional diamonds, and a form factor designed for horizontal or
vertical use. Its limits reward a specific kind of completeness:

1. one immediately understandable verb;
2. sharp audiovisual response;
3. a short sequence that teaches, tests, twists, and climaxes;
4. a persistent title, pause, result, record, and quick retry around that core;
5. enough variation or mastery that the second run is meaningfully different;
6. measured headroom on the densest real frame; and
7. uncoached people choosing to play again.

A thirty-second rules demo is not automatically a finished handheld game.
Neither is a long game automatically better. The target is a compact system in
which the player can form an intention, act, read the consequence, improve, and
have a reason to return.

## What the enjoyment research supports—and what it does not

Ryan, Rigby, and Przybylski found associations between enjoyment and perceived
competence and autonomy, and also connected intuitive control with those
experiences ([original paper and abstract](https://doi.org/10.1007/s11031-006-9051-8)).
The GameFlow model organizes game enjoyment around concentration, challenge,
skill, control, clear goals, feedback, immersion, and social interaction
([Sweetser and Wyeth](https://doi.org/10.1145/1077246.1077253)). Nintendo's
Rhythm Heaven developers likewise described deliberately making button presses
produce immediate, sharply distinguishable feedback
([Iwata Asks](https://iwataasks.nintendo.com/interviews/ds/rhythm-heaven/0/2/)).

These sources support design priorities, not exact SwanSong timing values or a
universal formula for fun. SwanSong Originals translates them into testable
hypotheses:

- **Competence:** teach one action in play, acknowledge it by the second
  observed frame, show why an attempt succeeded or failed, and preserve a
  record the player can beat.
- **Autonomy:** offer at least one real choice—route, timing, resource use,
  loadout, risk, target, or optional objective—whose consequences are legible.
- **Control:** use the console's physical layout directly; avoid input chords
  or mode changes when a spare physical direction or button can express the
  action.
- **Clear goals and feedback:** compose the objective into the playfield and
  use animation, sound, and state change together. A paragraph is not a
  replacement for a readable objective.
- **Challenge matched to skill:** introduce a rule, test it safely, combine it
  with another rule, then apply pressure. Named Assist, Standard, and Challenge
  settings change visible barriers instead of silently manipulating outcomes.
- **Replay desire:** results identify what improved, records preserve it, and
  another route, rank, stage, seed, or mastery target makes Retry meaningful.

The exact thresholds later in this document are SwanSong Originals production
policy. Their validity is determined by local playtests, not by citing these
papers.

## Hardware facts, production margins, and game consequences

### 1. Physical display, orientation, and composition

**Hardware evidence.** The physical LCD is 224×144 pixels. The console's two
directional diamonds permit the same hardware to be held vertically; the LCD
does not become a different 144×224 device, but the player's view and controls
rotate. At 8×8 pixels per tile, the visible raster is 28×18 tiles. Wonderful
documents the display, eleven inputs, and vertical use in its
[platform overview](https://wonderful.asie.pl/wiki/doku.php?id=wswan%3Aplatform_overview)
and [WonderSwan target reference](https://wonderful.asie.pl/doc/general/target-wonderswan/).

**Originals policy.** Horizontal evidence is an exact 224×144 viewport.
Vertical art and evidence are authored or normalized as 144×224. Every HUD,
prompt, target, and hit response remains legible in the corresponding 28×18 or
18×28 tile view. Critical state is never communicated by orientation-dependent
text alone.

**Design consequence.** Decide orientation before designing the mechanic.
Horizontal framing favors lanes, side-by-side choices, broad arenas, and
landscape routes. Vertical framing favors ascent, falling objects, shooters,
portrait puzzle columns, and a long tactical axis. A vertical game should use
the rotated control geometry as an advantage, as Judgement Silversword did,
not merely rotate a horizontal layout at the end. Keep the immediate objective,
player, nearest threat, and essential meter readable at native size without
zooming the capture.

### 2. Frame timing and CPU work

**Hardware evidence.** The NEC V30MZ CPU runs at 3.072 MHz. A default frame is
159 lines × 256 clocks = 40,704 CPU clocks; 144 lines are visible. The resulting
refresh is approximately 75.472 Hz, so one frame is 13.25 ms. DMA uses the
cartridge bus and stalls the CPU. These values come from the
[WSdev timing reference](https://ws.nesdev.org/wiki/Timing); Wonderful provides
the CPU context and notes that V30MZ instruction timing is substantially better
than an original 3 MHz 80186 in its
[target reference](https://wonderful.asie.pl/doc/general/target-wonderswan/).

**Originals policy.** Across a five-minute representative profile, including
the title, attract mode, densest play, feedback, result, and replay:

- p95 frame cost is at most 30,528 clocks, 75% of a frame;
- the maximum frame is at most 37,999 clocks;
- no VBlank is missed; and
- the game does not claim this gate until SwanSong exports the measurements.

The p95 margin leaves 10,176 clocks, about 3.31 ms. The maximum margin leaves
2,705 clocks, about 0.88 ms, for variance in presentation, audio, and system
work. These are project reserves, not extra hardware capacity.

**Design consequence.** Gameplay updates use bounded integer or fixed-point
work. AI, collision, particles, path searches, and procedural generation have
fixed pools and per-frame limits. Large initialization or generation work is
split across loading frames or performed before play begins. Render only dirty
regions and changed actors. Precompute tables when that costs less shared RAM
than the repeated calculation. A button press should receive visible or audible
acknowledgement no later than the second observed frame—about 26.5 ms—even if
the longer action continues.

Do not budget only an empty room. Profile the largest metasprites overlapping
the busiest scanline, maximum projectiles, active HUD animation, music plus
high-priority SFX, and the most expensive valid model state at once.

### 3. Unified internal RAM

**Hardware evidence.** The mono WonderSwan has 16 KiB of internal memory; the
WonderSwan Color has 64 KiB. CPU data, display maps and tiles, sprite tables,
palettes, and sound waves all occupy the unified internal memory rather than
separate CPU RAM and VRAM. The layout and alignment restrictions are detailed
in the [WSdev memory map](https://ws.nesdev.org/wiki/Memory_map) and summarized
by [Wonderful](https://wonderful.asie.pl/wiki/doku.php?id=wswan%3Aplatform_overview).

**Originals policy.** A Color game keeps linked internal occupancy at or below
56 KiB, with the mono-visible 16 KiB area at or below 14 KiB and the 48 KiB
Color extension at or below 42 KiB. The build report must also account for
tile, map, sprite-shadow, palette, audio-wave, stack, and fixed gameplay-pool
ownership; a C link succeeding is not by itself a memory budget.

**Design consequence.** Static art, level data, music patterns, and lookup
tables stay in cartridge ROM and are declared with the toolchain's far-data
mechanism or generated asset declarations. Wonderful warns that ordinary
read-only C data uses near pointers by default and can be copied into scarce
RAM unless declared far
([C compiler caveats](https://wonderful.asie.pl/doc/general/target-wonderswan/#c-compiler)).
Do not duplicate gameplay tables in the renderer. Load scene-owned asset groups,
reuse tile patterns, use fixed pools, and size every pool from a real worst case.
Avoid a heap: fragmentation and failure paths add risk without creating a
better player experience here.

### 4. Tile maps and the two background layers

**Hardware evidence.** The display provides two overlapping 32×32 tile maps,
each representing a 256×256-pixel surface that can scroll horizontally and
vertically. Screen 1 is behind sprites; sprites may be placed below or above
Screen 2. Screen 2 and sprites have window/clipping support. The ordering and
scroll behavior are documented by [WSdev Display](https://ws.nesdev.org/wiki/Display)
and the [Wonderful platform overview](https://wonderful.asie.pl/wiki/doku.php?id=wswan%3Aplatform_overview).

**Originals policy.** A production game uses the second layer deliberately—for
a HUD, route preview, signal flow, foreground occlusion, focus mask, transition,
or another mechanic-facing purpose. It is not required to animate every tile,
but a static full-screen illustration on Screen 1 is not treated as a complete
game presentation.

**Design consequence.** A useful default is:

- Screen 1: terrain, track, room, puzzle board, or world;
- low-priority sprites: actors that can pass behind foreground detail;
- Screen 2: HUD, foreground, route/signal overlay, or clipped effect;
- high-priority sprites: player, cursor, projectiles, and critical feedback.

The visible view is smaller than either map, leaving scroll room, but a 32×32
map is not a complete large world. Use metatiles and stream or rewrite map edges
for longer routes. Keep the HUD compact and icon-led. Text belongs on title,
menu, help, and results screens when it helps; it should not cover the active
decision surface.

### 5. 2BPP tiles and tile ownership

**Hardware evidence.** In WonderSwan Color 2BPP mode, 8×8 tiles occupy 16 bytes
each and up to 1,024 tile patterns are stored at fixed internal-memory
locations. The mono system has 512. Sprites can address only the first 512 Color
tiles. Hardware horizontal and vertical flips are available. See
[WSdev tile data](https://ws.nesdev.org/wiki/Display/Tile_Data) and
[WSdev sprites](https://ws.nesdev.org/wiki/Display/Sprites).

**Originals policy.** Peak production use is at most 448 tiles inside the
anthology's 512-tile reservation, leaving 64 tiles free. At most 96 common tiles
cover the fixed font, HUD, shared feedback, and common actor needs. This is much
stricter than the WSC background maximum so that sprite art always remains
addressable, scene ownership stays simple, and unified RAM retains headroom.

**Design consequence.** Build scenes from reusable 8×8 pieces and metatiles,
not full-screen bitmaps. Deduplicate exact and flipped tiles before deciding to
reduce visual variety. Keep animated actor frames and shared effects in the
first tile bank. A palette swap, flip, or two-frame silhouette change is often
cheaper and clearer than another unique full-screen plate. The asset report must
show which scene owns each tile group and whether loading one scene can evict
another safely.

### 6. Palettes, transparency, and real displays

**Hardware evidence.** There are sixteen palettes. Palettes 0–7 are screen-only;
8–15 can color screens and sprites. In 2BPP mode each tile selects four indices.
Palettes 0–3 and 8–11 treat index zero as opaque, while 4–7 and 12–15 treat it as
transparent, leaving three visible colors for those tiles. Color entries use
12-bit RGB. WonderSwan Color uses a CSTN display and SwanCrystal uses TFT with
different reproduction characteristics; WSdev explicitly says there is no
single canonical system palette. See [WSdev palette](https://ws.nesdev.org/wiki/Display/Palette).

**Originals policy.** Use at most twelve palettes at peak, preserve spare slots
for feedback or transitions, and never make color the only indication of danger,
selection, success, or failure. Inspect the densest scene on WonderSwan Color
and SwanCrystal-class output in addition to exact native PNG evidence.

**Design consequence.** Give every functional object a silhouette, motion,
pattern, or icon distinction before color. Reserve transparent palette classes
for sprites and overlays that truly need them. Use short palette flashes for
selection, damage, or success, but pair them with motion or sound. Author color
groups around material and gameplay roles—terrain, actor, danger, reward—not a
large illustration's unconstrained local colors.

### 7. Sprites and scanline pressure

**Hardware evidence.** The hardware sprite table can hold 128 8×8 sprites. Only
the first 32 sprites whose vertical spans intersect a scanline are selected for
that line; entries later in the list disappear. The count includes sprites
outside visible X and sprites hidden by a window because selection depends on Y.
Earlier table entries draw above later ones. Sprite tile indices are limited to
0–511. See [WSdev sprites](https://ws.nesdev.org/wiki/Display/Sprites).

**Originals policy.** Use at most 64 visible sprites and at most 24 on any
scanline, with a warning beginning at 20. Profile the worst valid overlap, not
the average frame. Player, cursor, essential projectile, and danger indicators
receive deterministic list priority.

**Design consequence.** Count metasprite pieces by the scanlines they occupy.
A crowd of small actors can fail before the total reaches 128. Move unused
entries outside the visible vertical range or shrink the active table; moving
them only beyond the left or right edge does not remove scanline pressure. Use
Screen 1 or Screen 2 tiles for non-colliding crowds, trails, weather, and broad
particles. Use sprites for objects that move independently, collide, animate,
or must layer around foregrounds. Keep hitboxes simpler than silhouettes and
verify that feedback bursts cannot erase an essential enemy tell.

For dense action, design around roughly 16–20 normal sprites on the busiest
line so a short impact burst stays under 24. This is an Originals working target,
not a hardware limit.

### 8. Native audio

**Hardware evidence.** The native sound unit has four channels. Each can play a
32-sample, 4-bit wavetable; all four waves occupy a shared 64-byte RAM block.
Channel 2 can use unsigned 8-bit sample mode, Channel 3 adds sweep, and Channel 4
can use LFSR noise. Output is 24 kHz; the internal speaker is mono. Hyper Voice
on Color hardware is headphone-only and therefore cannot be a required speaker
path. The wavetable sample-position counter has no known reset mechanism. See
[WSdev Sound](https://ws.nesdev.org/wiki/Sound) and the
[Wonderful target reference](https://wonderful.asie.pl/doc/general/target-wonderswan/).

**Originals policy.** Normal action gameplay uses up to three music voices and
leaves a preferred or reserved path for prioritized SFX. A quiet title or music
utility may use all four if deterministic arbitration still protects critical
cues. Typical projects use at most twelve instruments and 32 KiB of authored
audio. A title loop is normally 8–20 seconds; gameplay music has at least 32
seconds of distinct A/B/turnaround form, with 45–75 seconds preferred for
puzzle or exploratory play. Every required cue survives speaker-safe mono.

**Design consequence.** Assign each channel a role before writing notes:
melody, bass, harmony/response, and percussion/effect are more readable than
four voices competing in one register. Channel 4 noise is valuable for impact,
engine texture, water, or percussion; Channel 3 sweep is an accent, not a
substitute for a melody. Reuse patterns and instruments to buy musical form
instead of spending ROM on many unrelated fragments.

An effect system should choose a silent or reserved voice first, interrupt only
eligible lowest-priority music, restore the fully resolved music state after
HOLD/no-change rows, and optionally apply deterministic ducking. Navigation,
confirm, action/progress, blocked/error, success, and failure need distinct
effects; a shared one-note test beep is not a sound design.

Because raw wavetable phase cannot be reset through a known hardware control,
do not promise byte-identical restarted PCM across arbitrary power histories.
Promise deterministic sequencer state, audible cue timing, clean silence where
required, and inspected SwanSong and physical-speaker behavior.

### 9. Inputs and the two orientations

**Hardware evidence.** Software receives X1–X4, Y1–Y4, A, B, and Start. Sound
and Power are not gameplay inputs. The physical arrangement is two directional
diamonds plus the face buttons, documented by [WSdev Keypad](https://ws.nesdev.org/wiki/Keypad)
and [Wonderful](https://wonderful.asie.pl/wiki/doku.php?id=wswan%3Aplatform_overview).

**Originals policy.** Horizontal games use the X diamond as the primary
direction set; vertical games use the orientation-appropriate generated primary
mapping. Raw X and Y remain available. Both clusters must navigate title, help,
options, and pause. Start normally pauses. Intro and attract exits drain held
input before gameplay.

**Design consequence.** Treat the second diamond as four direct affordances,
not redundant arrows. It can select four items, stances, tools, targets, camera
functions, or tactical commands without a radial menu. Dicing Knight's four
direct item buttons are a strong historical example. Avoid asking players to
remember many chords or to use an arcade control density that the handheld was
not built around. Every semantic action must be available in the documented
orientation and tested from a fresh boot.

### 10. Cartridge ROM, banking, and the C memory model

**Hardware evidence.** Official mapper layouts expose two movable 64 KiB ROM
banks and a 768 KiB linear window, with larger physical ROM reached through
bank selection. Official mapper families support theoretical ROM sizes above
8 MiB, but Wonderful notes that broadly available flash cartridges do not
support games above 8 MiB. The toolchain's medium model permits more than 64 KiB
of code; ordinary data pointers remain near by default. See the
[Wonderful platform cartridge overview](https://wonderful.asie.pl/wiki/doku.php?id=wswan%3Aplatform_overview)
and [memory-model documentation](https://wonderful.asie.pl/doc/general/target-wonderswan/#memory-models).

**Originals policy.** The release gate never exceeds 8 MiB. A game targets
256 KiB and has a strict anthology ceiling of 512 KiB unless a reviewed design
need justifies more. Small ROM is a discipline for asset ownership and iteration,
not a claim that the mapper stops there.

**Design consequence.** Keep game rules in portable C modules and generated
assets in far ROM data. Prefer enum/switch dispatch to far function-pointer
tables with documented compiler hazards. Package scenes into static asset groups
that can be loaded and released predictably. More ROM should buy authored
variation, music form, tutorial clarity, or replay structure—not duplicate
full-screen images and unused content.

### 11. Saves, suspend, and RTC

**Hardware evidence.** Cartridge hardware may provide SRAM or external EEPROM;
the Bandai 2003 mapper may also provide an RTC. Availability and size are
cartridge properties, not guaranteed console services. Wonderful summarizes
these options in its [platform overview](https://wonderful.asie.pl/wiki/doku.php?id=wswan%3Aplatform_overview).
The SwanSong SDK exposes the concrete supported media in its
[manifest reference](../vendor/swansong-sdk/docs/manifest.md).

**Originals policy.** A production game does not ship with `save_type = none`
when it has records, options, tutorial completion, unlocks, or a suspend need.
For small games, use 1 KiB EEPROM with a logical payload around 256 bytes and
the SDK's two-slot journal. Store version, schema, generation, length, and CRC;
recover the newest valid slot after a corrupt or interrupted write. Capture RTC
once and inject normalized time into state; deterministic updates never poll it.

**Design consequence.** Save what makes a handheld game hospitable: volume and
difficulty, completed tutorial, records/ranks, discoveries, endings, and small
unlock sets. A tactics or longer exploratory session should offer explicit
suspend/resume. Commit at stable boundaries such as results, settings exit, or
suspend—not every frame. Test empty media, corrupt newest slot, interrupted
commit, schema migration, capacity failure, unavailable RTC, invalid time, and
power loss. A save is part of the game loop because it makes improvement and
return play durable.

## Historical WonderWitch and WWGP lessons

These examples are primary documentation from their creators and contest host.
They are evidence of shipped structure and developer intent, not permission to
copy code, art, characters, music, text, or branding.

### WWGP 2002: breadth was a strength

**Historical evidence.** Qute's
[WWGP 2002 all-works page](http://wwgp.qute.co.jp/2002/allworks.htm) records 74
valid entries across game, tool, and hardware categories, made by entrants aged
17–38. The catalog includes action, puzzle, RPG, card and table games, sound
drivers, sound players, sequencers, development environments, clocks, a
calculator, and hardware experiments. Qute also warns that entries remain in
their submitted state and that each author's documentation controls use and
rights.

**Production inference.** WonderSwan homebrew does not need to imitate a modern
console catalog. A focused utility, sound instrument, one-screen score game,
compact tactics title, or procedural action game can fit the device. What makes
it valuable is a coherent interaction loop and complete handheld package, not
genre size or technical spectacle. Historical archives are research inputs;
SwanSong Originals keeps its implementation and assets independently original.

### Judgement Silversword: a small core inside a complete package

**Historical evidence.** M-KAI's original
[Judgement Silversword WWGP manual](http://wwgp.qute.co.jp/2001/entry/00072/JUDGE_SS/README.HTM)
describes a vertical WonderSwan Color shooter with:

- a persistent title menu with Game Start and Options;
- an idle transition to a ranking display;
- movement on the Y diamond and separate shot/field actions on X;
- pause and a return-to-title chord;
- a force field that blocks bullets, drains on contact, and regenerates when
  not used;
- a shooting-pace score multiplier, with especially strong gain from field
  interaction;
- saved top-ten scores and three-character name entry;
- a sound test, game-speed and ship settings, and options unlocked through
  continued play; and
- ranking disabled for settings that would make scores incomparable.

The author's Q&A also explains a deliberate choice not to award lives directly
from score because extreme score play could make that behavior mandatory, and
describes focusing limited development time on the essence of the game rather
than a movement-tech demonstration.

**Production inference.** The lesson is not “make a shooter.” It is:

1. use the vertical hardware and its controls as part of the design;
2. make one resource serve survival and scoring, creating mastery depth;
3. surround immediate play with pause, options, records, sound test, unlocks,
   and an idle attract/records behavior;
4. separate assists from ranked comparability honestly; and
5. reject a progression rule when it coerces one narrow style of play.

This is a useful model for a three-minute game that still feels like a product
rather than a demo.

### Dicing Knight: direct controls, tradeoffs, variation, and portability

**Historical evidence.** The creator's original
[Dicing Knight WWGP 2002 manual](http://wwgp.qute.co.jp/2002/entry/00067/dicingknight/dknight/readme.htm)
documents a procedurally generated action dungeon with double-tap dash and a
hover continuation, a hold-to-guard/charge then release-to-attack verb, four
items mapped directly to Y1–Y4, a four-item carrying limit, disposable tools
with situational costs, an automap, bosses and escalating enemies, stat upgrades
during a run, a post-death legacy pickup, scoring, and a suspend screen designed
for turning off the handheld and resuming later.

In the postmortem, the developer says the sprite-count limit constrained large
multi-jointed characters and explicitly favors things that are enjoyable to
move and play over technical novelty. The hover/drift feel is called out as a
favorite result.

**Production inference.** The durable lessons are:

- one button can create readable depth through press, hold, release, facing,
  and risk, provided feedback makes every phase clear;
- the second directional diamond can remove inventory-menu friction;
- a small item capacity creates choices only when items have real tradeoffs;
- procedural variation needs authored safeguards so generation cannot produce
  broken or dull runs;
- death can create a reason to return without erasing consequence; and
- suspend/resume is a gameplay feature on a portable device, not merely a save
  implementation detail.

Technical tricks should strengthen motion, decisions, feedback, or replay. If
they do not, they are not the player's feature.

## The minimum complete SwanSong Originals game

The following is a production hypothesis to test, not a hardware requirement:

```text
Boot → persistent Title → Play / How / Records / Options
       ↓ idle 8–12 s
       deterministic Attract demonstration

Tutorial (20–45 s) → Teach → Test → Twist → Climax
                         ↕ Pause
                    Results → Retry / Next / Title
```

A strict game should provide all of these observable properties:

1. **Persistent title.** It waits for confirm, has authored readable lettering,
   an animated focal object, title music, and an 8–12 second deterministic
   attract or records behavior.
2. **Compact menu.** Play, How, Records, and Options are enough for most games.
   Both directional clusters work; focus and cancel behavior stay consistent.
3. **Playable onboarding.** Teach one action, confirm it, then add the next.
   Keep explanatory prose optional and short.
4. **Immediate feel.** Every meaningful press receives a visible or audible
   response within two observed frames. An important action uses at least two
   information channels.
5. **Escalating run.** A normal action or puzzle run targets roughly 3–8 minutes.
   Tactics may target 8–12 minutes with suspend. Utilities should become useful
   immediately and preserve work or settings.
6. **Pause and recovery.** Resume, Controls, Restart, and Title are predictable;
   Retry is at most two actions from results.
7. **Readable results.** Success and failure look and sound different, compare
   against a saved best where appropriate, and identify one improvement target.
8. **Durable return value.** Records, ranks, medals, discoveries, endings,
   challenge stages, seeds, or unlocks make another session meaningful.
9. **Complete sound.** Title and gameplay music, prioritized action/UI SFX,
   success and failure stingers, and an intentional result state replace silence
   or test beeps.
10. **Headroom and evidence.** The densest five-minute profile, decoded audio,
    native screens, deterministic reset, save recovery, and physical display/
    speaker checks all pass independently.

An intentionally sub-minute arcade loop can still work when score, rank, speed,
variation, and immediate retry make it a repeatable mastery game. A sub-minute
entire experience with one solved challenge and no persistent target does not
meet this standard.

## Mechanic, presentation, and audio patterns that fit the hardware

| Game need | Hardware-shaped implementation |
| --- | --- |
| Fast action | Screen 1 terrain, Screen 2 HUD/foreground, pooled actor sprites, 3-voice BGM plus a protected SFX path, fixed collision buckets |
| Grid puzzle | Board/metatiles on Screen 1, signal/preview overlay on Screen 2, sprite cursor/familiars, dirty-cell redraw, undo and par records |
| Small tactics | Fixed unit pool, map on Screen 1, range/intent preview on Screen 2, scanline-aware unit sprites, bounded AI phases, suspend save |
| Racing | Scrolling road layer, overlay HUD/curve warnings, rival/hazard/player sprites, engine/noise texture, split times and saved ghost/input route |
| Observation/tuning | World layer plus focus/scope overlay, subject or indicator sprites, stereo cue with visual equivalent, saved album/discoveries |
| Music utility | All four voices when needed, direct per-channel mute/solo, pattern browser, visible scope, UI SFX arbitration, saved last track/favorites |

Across genres, use a compact `teach → test → twist → climax` content plan:

- **Teach:** one safe example with immediate confirmation.
- **Test:** the same rule under mild pressure without a new exception.
- **Twist:** combine it with a resource, second rule, route, or adversary.
- **Climax:** require intentional mastery, then clearly resolve the run.

This structure can describe four shooter waves, four puzzle chapters, four
race sections, four repair jobs, or four tactical objectives. It is a pacing
tool, not a demand for identical games.

## Implementation workflow for a new game

Before code, complete this one-page brief:

| Field | Required answer |
| --- | --- |
| Player promise | “I get to…” in one sentence, expressed as an action rather than lore |
| Orientation | Horizontal 224×144 or normalized vertical 144×224, with a reason |
| Primary controls | Exact semantic actions on X/Y/A/B/Start; use of second diamond |
| Core decision | What can a player choose, risk, or optimize every 2–10 seconds? |
| Feedback | What appears and sounds within two frames for each important action? |
| Mastery | What can an expert do better than a first-time successful player? |
| Run arc | Concrete Teach, Test, Twist, and Climax beats |
| Replay target | Record, rank, route, challenge, seed, unlock, discovery, or ending |
| Save payload | Options, tutorial, records, unlocks, and suspend requirements |
| Presentation | Screen 1 role, Screen 2 role, sprite roles, densest scanline estimate |
| Audio | Voice roles, SFX priorities, title/gameplay/result transitions, mono check |
| Budgets | CPU p95/max, RAM split, tiles, palettes, sprites/line, audio bytes, ROM |

Then implement in this order:

1. Build a portable deterministic model and a native-feeling control prototype.
2. Put it on the actual 75.472 Hz timing and semantic orientation mapping.
3. Test the core with people before producing a large content or art set.
4. Author the Teach/Test/Twist/Climax sequence and result metrics.
5. Add persistent title/menu/tutorial/pause/results and journaled records.
6. Add actor animation, second-layer information, transitions, and the complete
   music/SFX plan.
7. Profile the densest valid state and reduce work or content ownership until
   the production margins pass.
8. Run exact model tests and SwanSong scenarios; inspect PNG, WAV, trace, and
   resource evidence.
9. Run the five-person uncoached gate and change the game when people are
   confused, bored, or unwilling to retry.
10. Only after a pattern works for players, extract recurring work into a public
    SDK API, recipe, asset rule, Studio control, or regression test.

Art direction, pacing values, level content, musical themes, and game rules stay
game-owned. The framework should remove mechanical repetition without making
the ten titles feel like one reskinned game.

## 100-point production rubric

This rubric is an Originals review instrument, not a scientifically validated
psychometric scale. It forces a team to discuss player value separately from
technical correctness.

| Category | Points | What earns the points |
| --- | ---: | --- |
| Core feel and mastery | 25 | Controls respond sharply; actions have readable consequences; the core creates real decisions; skill produces observable improvement |
| Replay structure | 20 | Content or rules vary; records/ranks/unlocks/saves matter; retry is fast; another run has a specific purpose |
| Challenge curve | 15 | Teach/Test/Twist/Climax is present; threats are telegraphed; failure is attributable; named difficulties alter clear barriers |
| Clarity and onboarding | 15 | Goal is legible; tutorial is interactive; HUD is compact; pause/results explain state without a text wall |
| Audio and presentation | 15 | Persistent title and related result art; meaningful animation/layering; composed BGM; distinct prioritized SFX; mono-safe redundant cues |
| Hardware and handheld fit | 10 | Measured CPU/RAM/tile/palette/sprite headroom; orientation-aware controls; native readability; pause/save/suspend suit portable sessions |
| **Total** | **100** | |

Use the following fixed subscore allocation so that different reviewers do not
silently redefine a category:

| Category | Subscore | Points |
| --- | --- | ---: |
| Core feel and mastery | Input response and consequence readability | 8 |
| | Meaningful recurring decisions | 8 |
| | Learnable mastery beyond first completion | 9 |
| Replay structure | Authored, systemic, modal, or seeded variation | 8 |
| | Saved record/rank/unlock/discovery/ending value | 7 |
| | Fast retry and a specific next-run target | 5 |
| Challenge curve | Teach/Test/Twist/Climax construction | 5 |
| | Telegraphing, fairness, and attributable failure | 5 |
| | Named Assist/Standard/Challenge differences | 5 |
| Clarity and onboarding | Goal and immediate objective | 5 |
| | Interactive tutorial and controls access | 5 |
| | HUD, pause, and result comprehension | 5 |
| Audio and presentation | Related title/gameplay/result art and motion | 5 |
| | Composed title/gameplay/result music behavior | 5 |
| | Distinct SFX, cue priority, mono mix, and redundant signals | 5 |
| Hardware and handheld fit | Measured resource and frame headroom | 4 |
| | Native raster, orientation, and control fit | 3 |
| | Pause, save/suspend, interruption, and session fit | 3 |

Award full points only when the property is present in the ROM and supported by
the appropriate inspected or human evidence. Award partial points when it is
present but inconsistent. Award zero when it is absent or only planned. Every
review records a short evidence path or playtest observation for each subscore;
the numeric total is not a substitute for the reasons behind it.

Pass requires at least 80/100 and at least half of every category, rounded up:
13/25, 10/20, 8/15, 8/15, 8/15, and 5/10. A high presentation score cannot
hide a weak core, and mechanical depth cannot excuse unreadable feedback.

The following are hard blockers regardless of score:

- the opening is an art-only splash that advances on a timer;
- gameplay is silent or limited to undifferentiated test beeps;
- the whole fixed experience ends in under a minute without a score, rank,
  variation, progression, or mastery wrapper;
- there is one fixed challenge with no meaningful choice or improvement target;
- failure is unreadable, input feedback is late, sprites flicker away, the game
  can softlock, or reset diverges;
- saved records/settings/suspend are promised but not recoverable after corrupt
  or interrupted writes;
- only automated or emulator evidence exists for display, speaker, comprehension,
  or replay desire; or
- the game fails the five-person uncoached gate.

## Five-player human gate

Automation must not infer these results. Recruit five people, vary familiarity
where possible, provide no coaching, collect no identifiers, and retain only an
aggregate local report. The minimum rebuild checkpoint is:

| Measure | Pass threshold |
| --- | ---: |
| Identifies the immediate goal within 30 seconds | at least 4 of 5 |
| Performs the first meaningful action within 45 seconds | at least 4 of 5 |
| Understands the result state | at least 4 of 5 |
| Becomes permanently stuck | 0 of 5 |
| Voluntarily chooses Retry | at least 3 of 5 |

“Voluntary” means the observer does not ask whether the participant wants to
try again. Record the choice after the result screen provides its normal options.
If two participants independently show the same confusion, treat it as a design
blocker even when the aggregate threshold narrowly passes. Fix the game and run
a new group; do not coach the existing group into passing.

This five-person gate is an iteration checkpoint, not enough evidence for a
broad public claim. Before production release, test at least twelve people
across intended skill groups and report the same identifier-free aggregates.
Genre-experienced players are especially useful for identifying whether the
game has a credible mastery target; newcomers reveal onboarding and readability
failures.

## What each validation lane may claim

| Lane | It may establish | It may not establish |
| --- | --- | --- |
| Portable C tests | Rules, reset, RNG, clocks, transitions, solvability, serialization | Control feel, visual readability, music quality, fun |
| Build/report gates | ROM validity and declared RAM/tile/palette/sprite/audio budgets | Actual worst-frame cost unless measured during play |
| SwanSong scenarios | Fresh-boot execution, exact inputs, outcomes, deterministic replay, captured PNG/WAV/trace | Human comprehension, physical LCD/speaker behavior, replay desire |
| SwanSong media review | A person inspected the actual captured frame and listened to decoded audio | Physical hardware equivalence or universal aesthetic quality |
| Physical WSC/SwanCrystal checks | LCD readability, speaker-safe cues, controls, battery/power interruption | Broad audience appeal |
| Uncoached human play | Goal comprehension, confusion, perceived control, result understanding, voluntary retry | Absence of software bugs outside observed play |

SwanSong remains the sole automated emulator and gameplay-validation backend.
Physical WonderSwan Color and SwanCrystal checks are separate release evidence,
not a second automated emulator lane.

## Source hierarchy and references

### Hardware and toolchain

- [Wonderful Toolchain: WonderSwan target](https://wonderful.asie.pl/doc/general/target-wonderswan/)
- [Wonderful Toolchain Wiki: WonderSwan platform overview](https://wonderful.asie.pl/wiki/doku.php?id=wswan%3Aplatform_overview)
- [WSdev: Timing](https://ws.nesdev.org/wiki/Timing)
- [WSdev: Memory map](https://ws.nesdev.org/wiki/Memory_map)
- [WSdev: Display](https://ws.nesdev.org/wiki/Display)
- [WSdev: Tile data](https://ws.nesdev.org/wiki/Display/Tile_Data)
- [WSdev: Palette](https://ws.nesdev.org/wiki/Display/Palette)
- [WSdev: Sprites](https://ws.nesdev.org/wiki/Display/Sprites)
- [WSdev: Keypad](https://ws.nesdev.org/wiki/Keypad)
- [WSdev: Sound](https://ws.nesdev.org/wiki/Sound)
- [SwanSong SDK manifest reference](../vendor/swansong-sdk/docs/manifest.md)

Wonderful and WSdev are the authoritative implementation references used here.
When a project observation conflicts with them, investigate and document the
conflict rather than silently turning the observation into a hardware rule.

### Primary historical material

- [Qute: WWGP 2002 all submitted works](http://wwgp.qute.co.jp/2002/allworks.htm)
- [M-KAI: Judgement Silversword WWGP manual](http://wwgp.qute.co.jp/2001/entry/00072/JUDGE_SS/README.HTM)
- [紫雨陽樹: Dicing Knight WWGP 2002 manual and postmortem](http://wwgp.qute.co.jp/2002/entry/00067/dicingknight/dknight/readme.htm)

The Qute pages are retained historical submissions, may be served only over
HTTP, and remain copyrighted by their respective authors. This document
paraphrases interaction and production lessons; it does not redistribute the
works.

### Player-experience research and practitioner evidence

- Ryan, Rigby, and Przybylski,
  [“The Motivational Pull of Video Games”](https://doi.org/10.1007/s11031-006-9051-8)
- Sweetser and Wyeth,
  [“GameFlow: a model for evaluating player enjoyment in games”](https://doi.org/10.1145/1077246.1077253)
- Nintendo,
  [Iwata Asks: Rhythm Heaven—immediate input feedback](https://iwataasks.nintendo.com/interviews/ds/rhythm-heaven/0/2/)

These sources inform hypotheses about competence, autonomy, control, clear
goals, feedback, and challenge. Only play with the intended audience determines
whether a particular SwanSong Originals implementation realizes them.
