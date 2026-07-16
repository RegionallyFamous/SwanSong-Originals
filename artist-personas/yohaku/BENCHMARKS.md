# Yohaku benchmark suite

The ten SwanSong Originals full-screen masters are the v1 benchmark batch.
They cover grids, action lanes, a music instrument, a large object, a dark
scene, a gentle creature, a busy tactics board, pseudo-3D racing, and logic UI.

| Benchmark | Image | Score | Notes |
| --- | --- | ---: | --- |
| Signature subject | `docs/art/full-screen/orbital-courier-gameplay-master.png` | 5 | Clear game-first grid, courier, parcel, goal, and compact HUD |
| Pose/state shift | `docs/art/full-screen/harpoon-moon-gameplay-master.png` | 4 | Strong action lane and resource states; native pass simplifies crater detail |
| Related character | `docs/art/full-screen/pocket-kaiju-observatory-gameplay-master.png` | 5 | Original gentle creature and readable camera relationship |
| Object | `docs/art/full-screen/scrapframe-garage-gameplay-master.png` | 4 | Machine and choices read; native pass removes micro-panel noise |
| Scene | `docs/art/full-screen/radio-ghost-gameplay-master.png` | 5 | Dark-value stress test retains receiver, needle, waveform, and single ghost |
| Icon/tiny read | `docs/art/full-screen/mote-sound-terminal-gameplay-master.png` | 5 | Three tracks, beat cells, scope lanes, and transport state remain distinct |
| Busy composition | `docs/art/full-screen/turncoat-tactics-gameplay-master.png` | 5 | Board hierarchy survives multiple units and HUD state |
| Production use | `docs/art/full-screen/one-last-lap-gameplay-master.png` | 4 | Road/lane read is strong; native pass reduces distant scenic texture |
| Edge case | `docs/art/full-screen/rotate-dungeon-gameplay-master.png` | 5 | Regular grid and rotation grammar stay clear; verified room data remains separate |
| Logic system | `docs/art/full-screen/bug-witch-gameplay-master.png` | 5 | Five sockets and three familiar families are explicit |

All benchmarks score at least 4/5 on style consistency and production
usefulness. The native reducers and live renderers are the final acceptance
step; a master does not pass solely because its high-resolution composition is
attractive.
