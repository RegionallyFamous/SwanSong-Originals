# Yohaku Style Contract

Status: `stable`
Version: `v1.0.0`

## Intent

Yohaku is SwanSong Originals' repeatable art and interface grammar for complete
224×144 homebrew game screens. It combines hard native pixel structure with a
worn photocopied-risograph voice while keeping play state readable without
words.

## Core read

A running 8-bit game screen printed like a 1970s instruction-zine spread:
80 percent playable clarity and 20 percent handmade print oddness.

## Invariants

- The playfield owns 82–88 percent of the landscape 14:9 frame.
- Every state formerly written as prose becomes a named icon family.
- Warm cream and near-black are joined by exactly two accent inks.
- Player, target, pickup, selection, and hazard differ by silhouette first.
- Critical shapes respect an implied 8×8 tile rhythm; live actors use 16×16 or
  larger metatiles when possible.
- Halftone, torn paper, and registration slip stay outside critical gameplay
  boundaries.
- Generated layouts direct appearance only. Tested collision, puzzle, timing,
  and state geometry remain authoritative.

## Rendering rules

- Flat fills; no gradients, antialiasing, glossy reflections, or airbrush.
- Near-black one-to-two-pixel-equivalent contours.
- One controlled print texture per quiet region.
- One imperfect edge or registration accent is sufficient.
- Result screens retain the play scene and overlay a large success/failure
  glyph plus replay loop.

## Composition adapters

- Grid games: regular grid uses at least 82 percent; HUD gets at most two tile
  rows.
- Side-view action: the action lane gets at least two thirds of the height.
- Instruments and repair games: controls become large physical game objects,
  never labeled panels.
- Racing games: three lane silhouettes and hazards must read before scenery.
- Logic games: sockets, connections, cursor, and input/output lamps dominate.

## Master prompt

```text
Create a full-screen art master for an actual 224x144 original homebrew game.
Make it read as active gameplay at first glance. Use a landscape 14:9 frame,
devote 82 to 88 percent to the playfield, and express every changing state
through a compact icon-only HUD. Use deliberately authored 8-bit pixel art
fused with 1970s photocopied zine and risograph printing: hard 8x8-aligned
clusters, flat four-color screen-printed shapes, sparse halftone outside
critical gameplay silhouettes, slightly imperfect ink registration, and one
restrained printed edge. Preserve verified game geometry and make player,
target, pickup, selection, hazards, resources, progress, and results
distinguishable by silhouette. Text: none.
```

## Hard avoid list

No words, letters, numbers, pseudo-writing, fake labels, logos, title cards,
dialogue, instructions, prose panels, commercial silhouettes, franchise
insignia, ornamental borders, gradients, antialiasing, glossy rendering, tiny
clutter, meaningless emblems, malformed focal anatomy, or watermarks.

## Failure modes learned

- Generated grids often look good but disagree with tested collision logic;
  rebuild them from verified data in the native renderer.
- Source masters tolerate more texture than 224×144 output; remove halftone
  from sprites, cursor brackets, walls, and meters during conversion.
- Reusing the same round HUD container makes a batch look synthetic; keep the
  icon grammar but adapt containers to the game object.
- A tiny art stamp plus prose still reads as software, not a game. The renderer
  test forbids terminal calls and requires a dominant native playfield.
