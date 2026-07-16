# Emulator frame evidence

`boot-frames.png` is a contact sheet captured from the ten locally built ROMs
with `tools/capture_mednafen.sh` on 2026-07-16. Each panel is the last frame of
a 0.8-second headless Mednafen run. It demonstrates that every cartridge enters
its game loop and renders its accent-ink UI, game-specific native art stamp, and
initial state; it is not evidence of a complete playthrough or physical
hardware compatibility. Individual native-resolution captures are retained in
`native-frames/`.

- Mednafen: 1.32.1
- Image SHA-256: `914d72ff0b6bd245d708e708cc2a3c75497aa9313725ed9721352cc54ec4a4d3`
