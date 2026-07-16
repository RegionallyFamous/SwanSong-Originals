# Emulator frame evidence

`boot-frames.png` is a contact sheet captured from the ten locally built ROMs
with `tools/capture_mednafen.sh` on 2026-07-16. Each panel is the last frame of
a 0.8-second headless Mednafen run. It demonstrates that every cartridge enters
its game loop and renders its full-screen playfield, live actors or controls,
and icon-only HUD. This is not evidence of a complete playthrough or physical
hardware compatibility. Individual captures are retained in `native-frames/`.

- Mednafen: 1.32.1
- Image SHA-256: `7f28b0a9c519b20c05f7399676d2f46249aae7842c729a3ce96b706e08e42a8c`
