# Full-screen gameplay art system

SwanSong Originals uses the original **Yohaku** house style for all ten native
224×144 game screens. Each cartridge has four related public artifacts:

- `*-gameplay-master.png` — high-resolution Imagegen gameplay composition;
- `*-gameplay-native.png` — exact four-color 224×144 reduction;
- `*-gameplay-atlas.png` — live sprite, metatile, meter, and result proof; and
- `games/orbital-courier/assets/graphics/` and
  `games/mote-sound-terminal/assets/graphics/` — native PNG sources compiled
  and owned by the SwanSong SDK asset pipeline.

The other eight titles temporarily retain checked-in `gameplay_art.h` 2BPP ROM
data linked to their masters by SHA-256 until they receive the same migration.

The stable style contract and benchmark scores live in
[`artist-personas/yohaku/`](../../../artist-personas/yohaku/). Imagegen used the
built-in tool in `ui-mockup` mode. No third-party franchise screenshots,
characters, logos, or commercial art were used as references.

## Prompt set

Every generation used three project-owned references with explicit roles:

1. Orbital Courier's approved full-screen master — style, density, icon HUD,
   and native-readability reference;
2. that game's original source plate — subject and world reference only; and
3. that game's prior emulator capture — state hierarchy and problem reference,
   never a composition to copy.

The shared prompt block was:

> Use case: ui-mockup. Asset type: full-screen art master for an actual 224x144
> WonderSwan Color gameplay screen. Redesign the entire playable screen so it
> reads immediately as a running game, not a terminal, poster, illustration
> panel, title card, or text interface. Use deliberately authored 8-bit pixel
> art fused with 1970s photocopied zine and risograph printing: hard
> 8x8-aligned clusters, flat four-color screen-printed shapes, restrained
> halftone only outside critical gameplay silhouettes, slightly imperfect ink
> registration, one torn-paper edge, and functional editorial marks. Landscape
> 14:9; the playfield occupies at least 82 percent; compact icon-only HUD;
> everything readable at exactly 224x144. Text: none. No words, letters,
> numbers, pseudo-writing, logos, dialogue, instructions, commercial
> silhouettes, gradients, antialiasing, glossy rendering, tiny clutter, fake
> labels, watermarks, or decorative border.

The game-specific briefs and state inventories were:

| Game | Full-screen brief | Required state icons |
| --- | --- | --- |
| Mote Sound Terminal | Physical pocket synthesizer with three crystal tracks, three waveform lanes, and sixteen beat cells | play/pause, track, beat, tempo, scope |
| Orbital Courier | Scrolling orbital service-deck grid with courier, parcel, bulkheads, and delivery ring | fuel, cargo, route, success/failure/replay |
| Scrapframe Garage | Headless six-wheel utility machine with open service hatch and three large part trays | job bolts, credits, fault, selection, test result |
| Radio Ghost | Analog receiver, large tuning band, noise waveform, one spectral ribbon, and dawn arc | needle, gain, gate, clues, dawn |
| Harpoon Moon | Side-view lunar skiff, constellation creature, charge/reach beam, and leviathan phase | oxygen, tags, boss health, charge |
| Turncoat Tactics | Verified 8×6 top-down board with distinct nonhumanoid allies, rivals, cursor, and eastern beacon | turns, command HP, recruits, unit HP |
| Pocket Kaiju Observatory | Side-view camera blind and one gentle original mossy giant with a safe framing band | behavior, evidence, zoom, disturbance, sunset |
| Rotate Dungeon | Verified 12×8 top-down chamber with mutable wall strip, key, exit, and player | room progress, orientation, key state |
| One Last Lap | Three-lane pseudo-3D sunset road with delivery machines, hazards, and tow opportunity | laps, battery, speed, progress, tow |
| Bug Witch | Exactly five circuit sockets and exactly three beetle-familiar families | input, target, required mix, placement budget, run, puzzle progress |

The full Orbital Courier generation prompt is preserved separately in
[`orbital-courier-gameplay.md`](orbital-courier-gameplay.md). The reusable
portable prompt and hard negative block are versioned in Yohaku's
`contract.json` and `STYLE_CONTRACT.md`.

## Source identity

| Game | Master dimensions | SHA-256 | Critique |
| --- | --- | --- | --- |
| Bug Witch | 1563×1006 | `3c6c38840e9a44252dec267e7cf7fa9ab839e21e05f0f79e8954b942dc576f40` | pass |
| Harpoon Moon | 1562×1007 | `8a9852044697b6b7b615d433eaed15a016c157958fec75736bc1903f766fc23a` | pass-with-notes: native pass simplifies crater texture |
| Mote Sound Terminal | 1564×1006 | `6e6ccaf3d8fdc6a84a04156fbb2bcb1e70c9d91df2165929b4def9f238f1b960` | pass |
| One Last Lap | 1564×1006 | `9df4a93de3605a105c11a2c4d2b47b683f28034860ab6e345779c66df8c524ff` | pass-with-notes: native pass simplifies road scenery |
| Orbital Courier | 1562×1007 | `25c1fccc128c5dfb6262e24b1b72fd6b43200b807700db5779c5bc9ef8840cc3` | pass |
| Pocket Kaiju Observatory | 1563×1006 | `d094abecb2f864c7dd6685f262b41c4d2f97d95dd82861ed7cddeefcb5e53ede` | pass |
| Radio Ghost | 1564×1006 | `fd1b7e5483474c796077cec0cd5cd41100a47eb6f8da1d3f3fa9a2274d1a8bed` | pass |
| Rotate Dungeon | 1563×1006 | `56f78a0019554d14e4ec88dd8e433c7160020d0f0a5d449d9cb4a1dd5051e44d` | pass |
| Scrapframe Garage | 1566×1005 | `25ce892531f03d6eecb71eb125100ba3642a668758526c06bb0a05e2ca2a3dff` | pass-with-notes: native pass removes machine micro-panels |
| Turncoat Tactics | 1564×1006 | `7b9cb1a7b9ba2ca6bb331923dc23676a236afd7c1ec047c6c1091255d827bf46` | pass |

The generated-art critique used composition, focal subject, prop/world logic,
AI-tell, and native-readability passes. No selected master contains pseudo-text,
fake branding, malformed focal anatomy, copied commercial silhouettes, or an
unreadable primary interaction. The three `pass-with-notes` masters require
only the deliberate detail reduction already performed by their live atlases.

## Repeatable production loop

1. Inventory the verified game state before prompting.
2. Translate every displayed value into a named icon family.
3. Generate one complete game-first master with the Yohaku prompt contract.
4. Reject pseudo-text, ambiguous silhouettes, decorative meters, and
   composition that leaves less than 82 percent for play.
5. Reduce the master to its exact four-color 224×144 proof.
6. Author a low-noise live tile bank from the master grammar; do not treat
   generated board or maze geometry as game logic. Store native PNG sources
   under the game and let `swan assets` own conversion and resource identity.
7. Render directly from the existing C state machine.
8. Run tile-count, palette, provenance, no-terminal-call, route/puzzle, ROM,
   and emulator-frame tests.
9. Record drift in Yohaku's evaluation notes; revise the contract when the same
   failure appears twice.

Run the whole reproducible art build with:

```sh
make art
```

That command rebuilds all ten native proofs, atlases, contact sheets, and
checked-in gameplay headers. `tools/build_native_art.py` is a compatibility
alias for existing workflows.

## What each game taught the next

- **Orbital Courier:** a dominant playfield and icon HUD change the first read
  from “software” to “game.”
- **Mote Sound Terminal:** utilities become playful when controls are depicted
  as physical objects and live motion, not labels.
- **Scrapframe Garage:** high-resolution machine detail must be reduced to fault
  zones, wheels, hatch, and three unmistakable choices.
- **Radio Ghost:** dark screens need cream waveform anchors and one spectral
  subject; extra noise destroys hierarchy.
- **Harpoon Moon:** charge, reach, resources, and phase transition need separate
  silhouettes even when they share one action lane.
- **Turncoat Tactics:** grid regularity, team silhouette, cursor, selection, and
  health need independent channels.
- **Pocket Kaiju Observatory:** a gentle original creature remains readable
  when behavior and safe distance are shown spatially.
- **Rotate Dungeon:** attractive generated geometry must never override tested
  room geometry.
- **One Last Lap:** perspective can be constructed from simple widening tile
  bands; scenery is secondary to lane occupancy.
- **Bug Witch:** exact-count puzzles need an explicit input/output path,
  selection, required mix, and remaining budget before decorative character
  art.
