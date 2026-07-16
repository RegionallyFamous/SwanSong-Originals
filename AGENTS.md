# Contributor guide

This repository contains ten complete short-session WonderSwan Color games
sharing one small engine. Preserve each game's documented v1 loop, exact
controls, ending or utility contract, and deterministic reset behavior.

## Engineering rules

- Target Wonderful `wswan/medium` and emit `.wsc` cartridge images.
- Avoid heap allocation, floating point, recursion, and far function-pointer
  tables. Use fixed-size state and explicit dispatch.
- Sample input exactly once per VBlank through the shared engine.
- Keep UI lines within the 28-by-18 text console.
- Put reusable code in `engine/`; keep title rules in `games/`.
- Add or update a gameplay-path test whenever rules change.
- Run `make clean test` and `make smoke` before release commits.

## Art and originality

Image-generated work is concept/source art until deliberately cropped,
quantized, tiled, and tested at native resolution. Record exact prompts,
hashes, and provenance in `docs/art/`.

Do not add third-party characters, franchise terminology, copied music,
dialogue, logos, screenshots, distinctive vehicle silhouettes, or ripped
assets. Keep every shipped game and default asset independently original.
