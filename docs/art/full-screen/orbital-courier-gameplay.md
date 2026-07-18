# Orbital Courier full-screen gameplay art

Orbital Courier is the first SwanSong Originals cartridge rebuilt around a
complete Imagegen-directed gameplay screen instead of a small art stamp beside
a text interface. The source was generated on 2026-07-16 with Codex's built-in
image-generation tool, then reviewed at native WonderSwan Color resolution.

- Use: full-screen gameplay art master and native renderer direction
- Source: `orbital-courier-gameplay-master.png` (1562 × 1007)
- Source SHA-256: `25c1fccc128c5dfb6262e24b1b72fd6b43200b807700db5779c5bc9ef8840cc3`
- Native proof: `orbital-courier-gameplay-native.png` (224 × 144)
- Runtime palette: cream `#f5f0e6`, near-black `#1a1a1a`, yellow `#ffe566`, pink `#ff6b9d`
- Critique result: pass

## Generation prompt

> Use case: ui-mockup
>
> Asset type: full-screen art master for an actual 224x144 WonderSwan Color gameplay screen
>
> Primary request: Redesign the entire playable screen of the original game Orbital Courier so it reads immediately as a real top-down action-puzzle game, not a poster, illustration panel, terminal, or text interface.
>
> Input images: Image 1 is a style and world reference only; Image 2 shows the current game logic/layout problem and must not be copied.
>
> Scene/backdrop: a curved orbital ring-habitat service deck floating over a sparse starfield. The walkable deck fills nearly the whole screen and forms a crisp, readable 20-column by 9-row movement grid with structural walls, open corridors, a parcel pickup bay near the lower-left, and a circular delivery airlock near the upper-right.
>
> Subject: one tiny original human courier sprite near the upper-left start, one hot-pink parcel crate at the pickup bay, and one unmistakable delivery-ring target at the airlock.
>
> Style/medium: deliberately authored 8-bit pixel art fused with 1970s photocopied zine and risograph printing; hard 8x8-aligned clusters, flat screen-printed shapes, restrained halftone only outside the playfield, slightly imperfect ink registration, torn-paper starfield edges, editorial arrows used only as environmental accents.
>
> Composition/framing: landscape 14:9 gameplay camera, slightly elevated top-down view; the playfield occupies at least 80 percent of the screen. Add a compact icon-only HUD along the top edge: five fuel pips, a parcel icon slot, and a tiny route-progress strip. Everything must remain readable when reduced to exactly 224x144.
>
> Color palette: exactly four dominant colors—warm cream #f5f0e6, near-black #1a1a1a, butter yellow #ffe566, hot pink #ff6b9d.
>
> Text: none.
>
> Constraints: this must look like an in-progress game screenshot at first glance. Clear traversable cells, clear walls, clear player, pickup, target, and fuel state. No words, letters, numbers, pseudo-writing, logo, title card, dialogue box, instructions, portrait, large empty cream region, cinematic close-up, generic spaceship cockpit, military insignia, humanoid mecha, gradient, antialiasing, glossy rendering, tiny clutter, fake UI labels, watermark, or decorative border.

## Production conversion

`tools/build_orbital_gameplay_art.py` center-crops the generated master to the
hardware aspect ratio and reduces it to the exact four-color palette. It writes
the 224×144 opening source and a compact 128×64 production metatile sheet under
`games/orbital-courier/assets/graphics/`. SwanSong SDK 0.3 passes both PNGs
through Wonderful SuperFamiconv, generates typed tile/map/palette symbols, and
tracks their hashes, ownership, and scene budgets. No production graphics are
embedded in a handwritten C header or drawn through `swan/legacy.h`.

The gameplay sheet contains native 16×16 courier, parcel, airlock, floor, and
bulkhead metatiles plus icon-only fuel, cargo, route, success, failure, and
replay states. Renderer lookups use regions of the generated tilemap, so flip
deduplication never leaks converter-specific tile numbers into game code.

The live game retains the tested 20×9 collision map and uses a 14×8 scrolling
camera, so the visual redesign does not substitute generated maze geometry for
the verified route. The critique pass found no pseudo-text, fake branding,
malformed focal anatomy, copied franchise silhouette, or unreadable tiny
detail. The source PNG, native reduction, sprite atlas, generated C assets, and
conversion tool are all retained for reproducibility.
