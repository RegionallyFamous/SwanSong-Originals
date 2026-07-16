# Native source-plate provenance

These ten raster plates were generated on 2026-07-16 with Codex's built-in
image-generation tool. The anthology concept sheet was supplied as a **style
reference only**. No franchise art, screenshots, logos, or third-party assets
were supplied or used.

The high-resolution plates are retained in `docs/art/source-plates/`.
`tools/build_native_art.py` converts nine of them into the checked-in art stamps
used by those games. Orbital Courier's first plate is retained as historical
direction, but its cartridge now uses the complete full-screen master and
renderer documented in
[`full-screen/orbital-courier-gameplay.md`](full-screen/orbital-courier-gameplay.md).
Enlarged nearest-neighbor proofs are retained in `docs/art/native/`.

| Game | Source dimensions | SHA-256 | Critique |
| --- | --- | --- | --- |
| Mote Sound Terminal | 1562 x 1007 | `b2e228a2b54255bd474a72cda1a468c2564d18dce2a448098d3a3bd7e6dbb190` | pass |
| Orbital Courier (superseded plate) | 1563 x 1006 | `97e5147227055eddd3d30fab54b2bcf9eb2a89144169514495ceca034a0ffb0e` | pass; replaced in ROM by full-screen master |
| Scrapframe Garage | 1563 x 1006 | `5829d3bdd6990d7955abd4b615d355de89344c9a9a3b3c5c94eae2f7be7a426b` | pass after targeted reroll |
| Radio Ghost | 1563 x 1006 | `2715ab15b669860f69ad314412d4185ab39f4d7e30b6a00e2a8dc5c21d4f4db1` | pass |
| Harpoon Moon | 1563 x 1006 | `21836a178bb10fa2fde8debd7e8fc840c5661fa3ce694bb4616a8665703b4c4d` | pass |
| Turncoat Tactics | 1563 x 1006 | `078ba042c74973cd6df05982957885476db4f1be183c3522c2dd410e58a793e0` | pass |
| Pocket Kaiju Observatory | 1561 x 1007 | `3f8173d35c7b718cd02a91675c649d99e6cf540303b96f01c96400a6c42029ac` | pass |
| Rotate Dungeon | 1563 x 1006 | `f650e4559308eabbbb455af87a30d93ee9e631a17fdeb900a14f6d59789edf99` | pass |
| One Last Lap | 1562 x 1007 | `658b4008bd8958af8e4b0a51d8dd9a08571c6b21ecebc240a381637ab2487a95` | pass |
| Bug Witch | 1564 x 1006 | `0074cb5f83a921e1eb4e90a14febdaca7a3c04f624b05acda1c65c35222a2b88` | pass |

## Prompt set

Except for Mote and the final Scrapframe reroll, each final prompt was the
following shared block followed by its named scene block.

> Use case: stylized-concept
> Asset type: source art for a 224x144 WonderSwan Color game background plate
> Input image: style reference only; redraw a new composition, do not copy its exact panel.
> Style/medium: deliberately authored 8-bit pixel art fused with 1970s photocopied zine collage; hard 8x8-aligned clusters, flat risograph shapes, sparse halftone, imperfect registration, one torn-paper edge, one piece of tape, and one editorial mark.
> Composition/framing: landscape 14:9 with one strong focal action, clear silhouette, strong asymmetry, and generous cream UI-safe negative space; must remain readable after reduction to 224x144.
> Constraints: no text, letters, numbers, logos, franchise designs, gradients, antialiasing, glossy rendering, tiny clutter, pseudo-writing, random emblems, malformed anatomy, watermarks, or surrounding border. Crisp square pixels and flat fills.

### Mote Sound Terminal

> Use case: stylized-concept
> Asset type: source art for a 224x144 WonderSwan Color game background plate
> Primary request: Create a landscape pixel-zine background plate for the original game Mote Sound Terminal, matching the supplied anthology sheet's visual grammar while redrawing the scene as a clean game-ready composition.
> Input image: style reference only; do not copy its exact panel.
> Scene/backdrop: a compact spacecraft listening console and one angular faceted crystal companion floating beside it; large quiet cream UI-safe areas across the upper-left and lower strip.
> Style/medium: deliberately authored 8-bit pixel art fused with 1970s photocopied zine collage; hard 8x8-aligned clusters, flat risograph shapes, sparse halftone, one torn-paper edge, one piece of tape, one editorial circle.
> Composition/framing: landscape 14:9, one clear focal console at right, crystal at lower right, strong asymmetry and generous blank space; readable after reduction to 224x144.
> Color palette: exactly four dominant colors—warm cream #f5f0e6, near-black #1a1a1a, hot pink #ff6b9d, baby blue #a8d5e5.
> Constraints: no text, letters, numbers, logos, recognizable franchise designs, humanoid mecha, gradients, antialiasing, tiny clutter, pseudo-writing, watermarks, or border frame. No repeated glowing eyes. Crisp square pixels and flat fills.

### Orbital Courier

> Primary request: Create a pixel-zine background plate for the original game Orbital Courier.
> Scene/backdrop: one human courier carrying a single parcel along a curved orbital walkway, ring habitat and sparse stars behind; wide quiet cream band across the top and a clean playfield opening in the center.
> Color palette: exactly four dominant colors—warm cream #f5f0e6, near-black #1a1a1a, butter yellow #ffe566, hot pink #ff6b9d.
> Avoid: branded uniforms, spacecraft insignia, extra parcels, repeated props.

### Scrapframe Garage

> Use case: stylized-concept
> Asset type: replacement source art for a 224x144 WonderSwan Color game background plate
> Primary request: Redraw Scrapframe Garage as an original pixel-zine scene with a mechanic repairing one headless low-profile utility machine.
> Input image: style reference only; do not copy its exact panel.
> Scene/backdrop: mechanic's gloved hands repairing a long turtle-shaped six-wheel chassis with an open side access hatch, asymmetrical antenna arm, and no head or face; one large wrench and exactly three separated parts; wide quiet cream UI-safe space across top and left.
> Style/medium: deliberately authored 8-bit pixel art fused with 1970s photocopied zine collage; hard 8x8-aligned clusters, flat risograph shapes, sparse halftone, imperfect registration, one torn-paper workbench edge, one baby-blue tape scrap, one butter-yellow editorial circle.
> Composition/framing: landscape 14:9; machine in lower right; hands enter from right edge; strong asymmetry and generous blank space; readable after reduction to 224x144.
> Color palette: exactly four dominant colors—warm cream #f5f0e6, near-black #1a1a1a, butter yellow #ffe566, baby blue #a8d5e5.
> Constraints: the machine has absolutely no head, screen face, eyes, mouth, humanoid torso, or glowing features. No text, letters, numbers, logos, franchise designs, gradients, antialiasing, glossy rendering, tiny clutter, pseudo-writing, random emblems, malformed or fused fingers, watermarks, or border. Crisp square pixels and flat fills.

### Radio Ghost

> Primary request: Create a pixel-zine background plate for the original game Radio Ghost.
> Scene/backdrop: a lone radio receiver at lower left emitting a single ribbon-like ghost signal into a dark cut-paper night patch; headphones and one antenna only; quiet cream panel across top-right.
> Color palette: exactly four dominant colors—warm cream #f5f0e6, near-black #1a1a1a, pale pink #f4c7c3, hot pink #ff6b9d.
> Avoid: human face, extra controls, unreadable dial text, horror gore, repeated spectral figures.

### Harpoon Moon

> Primary request: Create a pixel-zine background plate for the original game Harpoon Moon.
> Scene/backdrop: a tiny lunar skiff at lower left casting a curved light lure toward one constellation-like creature above a cratered moon horizon; broad quiet cream space at upper left and bottom.
> Color palette: exactly four dominant colors—warm cream #f5f0e6, near-black #1a1a1a, butter yellow #ffe566, pale pink #f4c7c3.
> Avoid: weapons, combat, humanoid aliens, star-map labels, copied constellations.

### Turncoat Tactics

> Primary request: Create a pixel-zine background plate for the original game Turncoat Tactics.
> Scene/backdrop: an isometric paper grid with three radically different abstract machine silhouettes in a tense standoff—one tall tripod, one low tracked wedge, one round shield drone; clean cream space across top and right.
> Color palette: exactly four dominant colors—warm cream #f5f0e6, near-black #1a1a1a, baby blue #a8d5e5, hot pink #ff6b9d.
> Avoid: humanoid mecha, helmets, glowing eyes, military logos, guns, cloned units, dense battlefield debris.

### Pocket Kaiju Observatory

> Primary request: Create a pixel-zine background plate for the original game Pocket Kaiju Observatory.
> Scene/backdrop: one gentle towering mossy creature in side profile being observed by one tiny field researcher on a simple platform with binoculars; wide quiet cream sky and lower-right UI-safe area.
> Color palette: exactly four dominant colors—warm cream #f5f0e6, near-black #1a1a1a, pale pink #f4c7c3, baby blue #a8d5e5.
> Avoid: copyrighted monster silhouettes, roaring pose, city destruction, teeth, extra people, fake scientific labels.

### Rotate Dungeon

> Primary request: Create a pixel-zine background plate for the original game Rotate Dungeon.
> Scene/backdrop: a circular stone chamber shown as a bold cutaway diagram, one small adventurer at lower left and one glowing geometric key on a plinth; the chamber's wall segments visibly imply rotation; quiet cream top band and right edge.
> Color palette: exactly four dominant colors—warm cream #f5f0e6, near-black #1a1a1a, butter yellow #ffe566, baby blue #a8d5e5.
> Avoid: written map labels, medieval heraldry, skeletons, copied dungeon symbols, clutter.

### One Last Lap

> Primary request: Create a pixel-zine background plate for the original game One Last Lap.
> Scene/backdrop: three distinct obsolete delivery robots racing side by side on a rough photocopied road toward a flat setting sun—one squat wheeled cart, one tall single-wheel unit, one four-legged parcel carrier; quiet cream top-left and bottom strip.
> Color palette: exactly four dominant colors—warm cream #f5f0e6, near-black #1a1a1a, hot pink #ff6b9d, butter yellow #ffe566.
> Avoid: cars, racing logos, humanoid robots, identical heads, headlights as eyes, speed-line clutter.

### Bug Witch

> Primary request: Create a pixel-zine background plate for the original game Bug Witch.
> Scene/backdrop: one original witch in a broad asymmetric hat arranging exactly three clearly separated beetle-shaped software familiars on a simplified circuit-board altar; quiet cream top band and left UI-safe column.
> Color palette: exactly four dominant colors—warm cream #f5f0e6, near-black #1a1a1a, hot pink #ff6b9d, baby blue #a8d5e5.
> Avoid: text, code, occult franchise symbols, fused fingers, extra beetles, spider legs, cluttered electronics.

## Critique and production result

The generated-art review used composition, subject, prop/world, and
AI-art-tell passes. Nine plates passed directly. The first Scrapframe plate was
rejected because it drifted into a generic cube head with paired glowing eyes;
only that plate was rerolled. The final batch contains no pseudo-text, fake
logos, malformed focal anatomy, repeated mascot construction, or copied
commercial silhouettes.

The native proofs were reviewed again after four-color reduction. Verdict:
**pass**. Their small shapes remain legible, each game has a distinct focal
silhouette, and quiet cream regions survive conversion rather than becoming
generated-detail noise.
