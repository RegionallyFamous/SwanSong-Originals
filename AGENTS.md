# Contributor guide

This repository contains ten complete short-session WonderSwan Color games
sharing the pinned SwanSong SDK runtime. Preserve each game's documented v1 loop, exact
controls, ending or utility contract, and deterministic reset behavior.

## Engineering rules

- Target Wonderful `wswan/medium` and emit `.wsc` cartridge images.
- Avoid heap allocation, floating point, recursion, and far function-pointer
  tables. Use fixed-size state and explicit dispatch.
- Let the SwanSong SDK own `main`, VBlank, and the single input sample; games
  consume only the immutable `swan_frame_t` snapshot.
- Keep UI lines within the 28-by-18 text console.
- Put anthology adapters in `shared/`, reusable framework policy in the SDK,
  and title rules in portable `games/*/src/model.c` modules.
- Add or update a gameplay-path test whenever rules change.
- Run `make clean test` and `make smoke` before release commits.

## Art and originality

Image-generated work is concept/source art until deliberately cropped,
quantized, tiled, and tested at native resolution. Record exact prompts,
hashes, and provenance in `docs/art/`.

Do not add third-party characters, franchise terminology, copied music,
dialogue, logos, screenshots, distinctive vehicle silhouettes, or ripped
assets. Keep every shipped game and default asset independently original.
