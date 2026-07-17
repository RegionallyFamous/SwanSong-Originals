---
name: play-swansong-games
description: Play SwanSong Originals ROMs through SwanSong's deterministic MCP tool, inspect screenshots, exercise controls and outcomes, and produce evidence-backed playtest reports.
---

# Play SwanSong Games

Use SwanSong's playtest tool whenever the user asks to play, test, smoke test,
reproduce, or validate one of the anthology games.

## Required loop

1. Read `scripts/games.json` and use each game's goal, controls, and required
   checks as the test contract. Resolve its ROM beneath the repository `dist/`
   directory and run `make dist` first whenever source is newer than that ROM.
2. Call `swansong_playtest_plan` with a fresh boot plan containing at least 60
   neutral frames. Set `confirmShareCapture` to true only because this plugin's
   purpose is visual playtesting and the user asked to share game frames with
   the agent.
3. Inspect every returned SwanSong screenshot. Choose the next input from the
   visible game state; do not claim success from source code, a boot, a changing
   hash, or a host-side model alone. For audio mechanics, also inspect or listen
   to the returned final-window WAV and compare its audio metrics and hash.
4. Maintain one accumulated `swan-song-frame-input-plan-v1` per game. Each
   ordinary press is a native SwanSong input held for one frame followed by at
   least two neutral frames. Use longer holds only for charge, lure, hide,
   acceleration, or braking mechanics.
5. For these horizontal SwanSong games, translate semantic directions as
   `up=x3`, `right=x2`, `down=x1`, and `left=x4`; keep `a`, `b`, and `start` unchanged.
   Never pass generic direction names to SwanSong.
6. Re-run the complete accumulated plan with `swansong_playtest_plan` after
   every decision and inspect its returned final frame. This replay-from-boot
   loop is the session trace.
7. Exercise at least one meaningful state change, one boundary or failure
   condition, the documented success or utility path when practical, and reset.
8. Repeat an unchanged plan once when determinism matters; its capture hash and
   report must match. Report the complete plan and SwanSong evidence metadata
   for every failure.

## Verdict rules

- **Pass:** observed controls and required state transitions agree with the
  game contract, and the tested completion/reset path works.
- **Pass with risk:** the exercised path works, but a required check was not
  reached. Name the untested check; never silently convert coverage into proof.
- **Fail:** a visible state contradicts the contract, input produces no expected
  response after a neutral edge, SwanSong crashes or hangs, or a completion or
  reset path is unreachable in a reproducible trace.
- **Harness failure:** SwanSong execution, the ROM build, capture, or a required
  dependency fails before gameplay evidence exists. Keep this distinct from a
  game bug.

## Default report

Return a compact table with game, verdict, exercised path, final visible state,
and evidence. Follow it with reproducible issues ordered by severity. Each issue
must include the ROM digest, exact input plan, expected result, observed result,
final frame number, and capture hash. State explicitly which checks remain
untested.

Treat SwanSong as a black box and the sole execution authority. Memory
inspection may be added later as diagnostic evidence, but it must not replace
playing and visually observing the shipping ROM through SwanSong.
