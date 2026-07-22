# Production title-screen source concepts

These high-resolution ImageGen sources extend the project-owned Yohaku art
contract for the production rebuild. They are concept/source material, not ROM
evidence. Every selected source must be deliberately cropped, reduced to the
native four-color palette, rebuilt as owned tiles, combined with code-rendered
lettering, and inspected at exactly 224×144 before it can ship.

Built-in ImageGen was used from project-owned game briefs and the approved
Yohaku style contract. Selected correction passes referenced only the immediately
preceding generated candidate. No commercial game, character, logo, screen, or
franchise reference was supplied. The shared prompt required:

- a 14:9 title composition matching the game's native orientation (landscape
  224×144, or portrait 144×224 for Rotate Dungeon);
- the source game's Yohaku pixel-zine and risograph grammar;
- a large game-mechanic silhouette rather than a poster or instruction screen;
- reserved negative space for native code-rendered title and `PRESS START`;
- no generated words, letters, numbers, pseudo-writing, logos, or watermark;
- independently original subjects and no commercial silhouettes; and
- readability after reduction to the native raster.

## Selected sources

| Game | Source | SHA-256 | Game-specific composition check |
| --- | --- | --- | --- |
| Orbital Courier | `orbital-courier-title-master.png` | `2b7005ad14b8298aabda93e58439d078a2a5fcba628647d45c735997e4effeb9` | courier, parcel, route arc, and delivery ring remain separate silhouettes |
| Bug Witch | `bug-witch-title-master.png` | `93068b0213ca322b262ebaebd7bfa63e3b77c0e8dec2f38d65e4c5ea67f2adf3` | exactly three familiar types and exactly five sockets; the first four-socket generation was rejected |
| Radio Ghost | `radio-ghost-title-master.png` | `f762754d70d9ac9e6e6e72a947a265f0845b6457335fcc1ed75bbe6fb054c57d` | one receiver, one tuning needle, one spectral ribbon, and a moon-to-dawn arc |
| Harpoon Moon | `harpoon-moon-title-master.png` | `06b594a0133c91a15a43b6f667ab9c3e8d2d3d5790e454695bedd5ea08ab19e6` | one skiff, one light lure, one constellation creature, and one partially hidden leviathan silhouette |
| Turncoat Tactics | `turncoat-tactics-title-master.png` | `136273574898d970cebc009e886ece304aaa4948e8cde32383c0db7bd271b5a2` | exactly three distinct machines, one allegiance crossing, and one beacon; a pseudo-label candidate was rejected |
| Pocket Kaiju Observatory | `pocket-kaiju-observatory-title-master.png` | `fc5dcfa8b4eacdb2686aefae3a7c0aaa7d3fb8dbc4965af687fc9edd9680404b` | exactly one creature, one observer, one camera, and three non-linguistic behavior clues; a letter-marked candidate was rejected |
| Rotate Dungeon | `rotate-dungeon-title-master.png` | `6675a7f1fed0d1805eb32f2b55413a155ad27b76d37cdc596ed94a6e554ce0f4` | portrait composition, one adventurer token, one key, a continuous route, and paired rotational wall positions |
| Scrapframe Garage | `scrapframe-garage-title-master.png` | `a0f829f5b107137546425995c21848bd78eb25b35aef88e91d148efc9ef8f515` | one faceless six-wheel utility chassis, two gloved hands, one wrench, three parts, and three diagnostic icons |
| Mote Sound Terminal | `mote-sound-terminal-title-master.png` | `346896e02e8e8f8ccb6f70297586c514a71e30539dc1f51b8fe037e635404944` | three distinct crystals, four waveform displays, and an exact 4×4 beat matrix; 15-pad and 12-pad candidates were rejected |

## Game-specific prompt additions

### Orbital Courier

Create a tiny practical orbital courier vaulting across a curved service
station exterior with one unmistakable parcel, a readable route, and a luminous
delivery ring. Keep a courier/parcel/destination triangle and an urgent but
inviting night-shift mood.

### Bug Witch

Create the original shadowy witch at a workbench as three visibly different
beetle familiars route pink and blue circuit sparks into exactly five separated
spell sockets. Keep the mood clever, mischievous, and cozy. The correction pass
changed only the apparatus count and replaced book-like marks with abstract
square circuit diagrams.

### Radio Ghost

Create one chunky analog receiver feeding a bright tuning needle into a broad
waveform while one spectral ribbon resolves from static. A moon-to-dawn arc
should communicate the timed night investigation. Keep the mystery eerie but
inviting.

### Harpoon Moon

Create one tiny non-humanoid lunar skiff casting one curved light lure toward
one constellation-like moon creature over a cratered horizon. Suggest the
late-game leviathan only as a partial deep silhouette, keeping the scene about
discovery and timing rather than weapons.

### Turncoat Tactics

Create exactly three different abstract machines on a crooked isometric paper
grid: a tripod, tracked wedge, and round shield drone. Show the drone crossing
a dashed allegiance boundary toward a distant beacon. A correction pass
removed glyph-like marks beside the beacon and replaced them with four solid
diamond sparks.

### Pocket Kaiju Observatory

Create one gentle original mossy creature observed from one blind and one
tripod camera. Use three separate leaf, ripple, and curl pictograms to suggest
behavior. A correction pass removed letter-shaped sleep marks and retained only
a spiral plus solid diamonds.

### Rotate Dungeon

Author in portrait for the game's vertical hardware orientation. Show one
adventurer token, one key, a continuous path, and four large wall segments in
paired solid/ghosted rotational positions. Do not derive the native composition
by rotating a landscape screen.

### Scrapframe Garage

Show one low, headless six-wheel utility chassis with an open hatch, exactly
two gloved hands, one wrench, and exactly three separated part trays. Use spark,
vibration, and broken-circuit pictograms as non-linguistic diagnostic cues.

### Mote Sound Terminal

Show one pocket console with four waveform channels, one speaker, one transport
dial, exactly three differently shaped crystal cartridges, and exactly sixteen
beat lights in a four-by-four matrix. Two correction candidates were rejected
for rendering a three-by-five and then a three-by-four grid; the selected third
pass is the first count-correct version.

Generated text is never authoritative. Titles, prompts, HUD glyphs, grids, and
mechanical counts are authored and validated on the native pixel grid.
