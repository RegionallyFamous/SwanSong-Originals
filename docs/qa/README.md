# Emulator frame evidence

`boot-frames.png` is a contact sheet captured from the ten locally built ROMs
with `tools/capture_mednafen.sh` on 2026-07-16. Each panel is the last frame of
a 0.8-second headless Mednafen run. It demonstrates that every cartridge enters
its game loop and renders a nonblank initial state. Orbital Courier now shows
its full-screen scrolling tile game and icon HUD; the other nine show their
pixel-zine art stamps. This is not evidence of a complete playthrough or
physical hardware compatibility. Individual captures are retained in
`native-frames/`.

- Mednafen: 1.32.1
- Image SHA-256: `84c483dcede05aa75c6086ca3c60317478505de387a52364fd3060a02d366725`
