# WonderSwan hardware-shaped playtest contract

Use this reference for every ROM. Keep hardware facts distinct from SwanSong
Originals production policy, and keep both distinct from observations that
still require people or physical hardware.

## Hardware facts that shape the game

| Area | Hardware fact | Consequence to inspect |
|---|---:|---|
| LCD | 224×144 pixels, or 28×18 visible 8×8 tiles | Judge text, HUD, threats, and hit feedback at native size; normalize vertical work to 144×224 |
| CPU/cadence | NEC V30MZ at 3.072 MHz; about 75.472 Hz and 40,704 clocks per frame | Use bounded integer/fixed-point work; test the densest valid frame, not an empty scene |
| Internal memory | 16 KiB mono / 64 KiB Color, unified across CPU data, graphics, sprite tables, palettes, and audio waves | Verify linked ownership and fixed pools; do not infer headroom from a successful link |
| Backgrounds | two 32×32 tile maps with scrolling, layering, and clipping | Look for an intentional world/playfield layer plus HUD, foreground, preview, or other mechanic-facing use of the second layer |
| Tiles | 8×8; WSC 2BPP supports 1,024 patterns, but sprites address only the first 512 | Inspect tile ownership, flip reuse, deduplication, and scene swaps; reject full-screen bitmap waste when reusable tiles fit |
| Palettes | 16; screen palettes 0–15 and sprite palettes 8–15, with opaque and transparent zero classes | Require silhouette, motion, icon, or pattern redundancy for critical state; color alone is not enough |
| Sprites | 128 total 8×8 entries; only 32 intersecting a scanline are selected | Capture the worst overlap; confirm player, cursor, danger, and essential projectiles do not disappear under effects |
| Audio | four 32-sample 4-bit wavetable voices at 24 kHz; ch2 PCM, ch3 sweep, ch4 noise; internal speaker mono | Listen to a complete loop and seam, critical SFX steals/restoration, clipping, silence, and speaker-safe mono |
| Input | X1–X4, Y1–Y4, A, B, Start; two directional diamonds support vertical use | Test generated orientation mapping, raw X/Y behavior where used, both clusters in menus, and held-input drain |
| Cartridge | banked ROM; EEPROM/SRAM and RTC depend on cartridge hardware | Test declared media, corrupt/interrupted saves, unavailable RTC, power loss, and deterministic injection rather than live polling |

Primary documentation: Wonderful's WonderSwan target and platform overview, and
WSdev's Timing, Memory map, Display, Sprites, Sound, and Keypad references.

## Conservative Originals margins

These are project release targets, not undocumented hardware limits:

- p95 frame cost at most 30,528 clocks; maximum at most 37,999; no missed
  VBlank across a representative five-minute profile;
- linked WSC internal occupancy at most 56 KiB, with the mono-visible area at
  most 14 KiB and the Color extension at most 42 KiB;
- at most 448 resident tiles, 12 palettes, 64 visible sprites, and 24 sprites
  per scanline, with a scanline warning at 20;
- normally at most three music voices during action so priority effects have a
  deterministic path; and
- exact 224×144 viewport evidence, an 8 MiB absolute ROM ceiling, and a 512 KiB
  anthology target ceiling unless a reviewed design need justifies more.

Do not report these margins as passing unless SwanSong exported the measurement
or the build report proves the corresponding static capacity. A screenshot can
show visible sprite loss but cannot prove cycle or RAM headroom.

## Player-facing checks caused by those limits

- Decide horizontal or vertical composition before assessing controls and
  framing; a rotated layout should exploit the physical control geometry.
- Require visible or audible acknowledgement by the second observed frame
  after an important action, about 26.5 ms at native cadence.
- Prefer a compact visible objective, icons, motion, and contextual prompts over
  instruction walls that cover the small decision surface.
- Count metasprite pieces on the busiest scanline. Off-screen X and clipping do
  not necessarily remove an entry from hardware scanline selection.
- Exercise maximum actors, effects, overlay work, scrolling, and music plus a
  priority SFX in the same scenario.
- Require a complete handheld flow around the core: persistent title, playable
  teaching, pause/recovery, readable result, quick retry, and durable record or
  replay target. A short rules demonstration is not complete merely because it
  reaches an ending.

## Evidence boundary

SwanSong may establish fresh-boot execution, exact input delivery, visible
outcomes, decoded audio, deterministic replay, and the measurements it exports.
It cannot establish physical CSTN/TFT readability, speaker behavior, intuitive
controls, fair challenge, musical appeal, or voluntary replay. Keep those as
explicit physical-device and uncoached-human gates; never synthesize a pass.
