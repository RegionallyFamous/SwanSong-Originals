# SwanSong agent playtest proof

This retained proof exercises the first agent-controlled game entirely through
SwanSong's native `playtest-plan` path. Both branches boot the same built Mote
Sound Terminal ROM from clean isolated persistence for 120 frames with the fixed
proof RTC.

- `mote-neutral-a` and `mote-neutral-b` use the same neutral plan and reproduce
  bit-exact native-raster, PNG, PCM, and WAV hashes.
- `mote-controls` presses Right, Up, B, and A through SwanSong's native input
  names. The final frame visibly shows the changed track/tempo/scope state and
  paused play icon. Its native-raster, PNG, PCM, and WAV hashes all differ from
  the neutral branch.
- All three reports bind the same ROM SHA-256
  `d5a741934a425f3e3a4c1a58ffb8e4f2a948a09063874ce8789d0c438ba813e0`
  to SwanSong's `ares-449b93716fb162632de2fd43bf2eba2064fa43f2-swan-abi5`
  engine build.

The matching screenshots and full JSON reports are retained beside this file.
This proves deterministic SwanSong execution and observable control response;
it does not silently promote the remaining game-specific checks to passes.
