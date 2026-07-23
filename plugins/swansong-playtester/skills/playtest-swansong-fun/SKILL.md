---
name: playtest-swansong-fun
description: Play SwanSong WonderSwan ROMs through the deterministic playtest MCP and evaluate engagement, clarity, pacing, tension, mastery, frustration, and replay pull from visible and audible evidence. Use when asked whether a SwanSong Original is fun, which games are most enjoyable, where interest rises or drops, or what gameplay changes would improve the experience.
---

# Playtest SwanSong Fun

Play the shipping ROM before judging it. Treat “fun” as a calibrated design
assessment, not universal human taste or a substitute for human playtesting.

## Prepare the playtest

1. Read `../play-swansong-games/SKILL.md` and follow its execution, input,
   evidence, determinism, and clean-room rules. This skill adds experiential
   evaluation; it never relaxes functional validation.
2. Read the plugin-level `../../scripts/games.json` and
   `references/fun-rubric.md` before scoring.
3. Use the built ROM in the repository `dist/` directory. Rebuild it first when
   its source is newer. Do not inspect source code, outcome contracts, or
   pre-authored success routes until the discovery pass is complete.
4. Use SwanSong as the sole execution authority. Set `confirmShareCapture` to
   true only because the user requested a visual and audio playtest.

## Run three passes

Keep each pass on a fresh boot and retain its exact input plan and evidence.
Use `swansong_playtest_plan` for ordinary ROM sessions. Use the observed-play
tools only when the project supports them and a useful session exceeds the
one-shot frame ceiling.

### 1. Discovery pass

- Hold at least 60 neutral frames and inspect the title screen, start prompt,
  attract behavior, and audio before pressing anything.
- Start using only the high-level goal, declared controls, and information the
  game itself presents. Do not consult a solved route.
- Continue until the first result, ten meaningful decisions, or the bounded
  session limit. Inspect every returned frame and choose the next input from
  what is visible or audible.
- Record time to first action, first understandable goal, first meaningful
  choice, first reward, first setback, and first result when the evidence makes
  those frames observable.

### 2. Competence pass

- Read the game’s scenario contracts and use them as coverage scaffolding.
- Reach a meaningful success or utility outcome and one failure or boundary.
- Test whether greater understanding creates better decisions, smoother
  execution, or a stronger tension arc. Distinguish deliberate difficulty from
  unclear feedback, waiting, or repeated low-value input.
- Inspect audio where it drives timing, location, state, reward, or tension.

### 3. Replay pass

- Restart promptly and choose a different route, strategy, risk level, or
  optimization goal when the game supports one.
- Observe whether the opening remains inviting after understanding the rules,
  whether mastery shortens dead time, and whether a second attempt creates a
  new decision instead of only repeating execution.
- For utility-style titles, replace route variety with experimentation depth:
  try a different configuration and judge how clearly the tool invites and
  rewards exploration.

When comparing several games, give each the same pass structure and a similar
decision budget. Do not rank a deeply tested game against a title seen only at
boot without marking the coverage imbalance.

## Judge from evidence

Separate every conclusion into:

- **Observation:** what the frame, audio, input trace, or outcome showed.
- **Interpretation:** why that moment likely helps or hurts engagement.
- **Recommendation:** the smallest change likely to improve the experience.

Score the dimensions in `references/fun-rubric.md` from 1 through 5. Do not
average them into false precision. Use `strong`, `mixed`, or `weak` for the
overall fun outlook, include confidence, and call out untested paths.

Never:

- call a game fun merely because it boots, responds, or can be completed;
- treat visual polish, difficulty, novelty, or content volume as a proxy for
  engagement;
- claim to feel enjoyment or represent all human players;
- use a known solution to score first-play discoverability; or
- hide harness failures, functional defects, or incomplete coverage inside a
  subjective verdict.

## Report

Start with a compact comparison table containing game, functional verdict,
fun outlook, replay pull, confidence, and passes completed. For each game add:

1. the playable hook in one sentence;
2. the exact routes and session limits exercised;
3. two or three spark moments with frame/audio evidence;
4. the main friction points with evidence;
5. the seven rubric scores and short reasons;
6. one high-leverage change, one low-cost polish change, and one experiment;
7. coverage debt and the evidence needed to raise confidence.

For an anthology comparison, rank titles by **current replay pull**, not
universal quality, and explain close calls. End with cross-game patterns only
when the same friction appears in at least two titles.

Do not edit a game or the SDK unless the user also requests implementation.
When implementation is requested, prefer title-specific changes first. Promote
an issue into the SDK only when repeated evidence shows a reusable cross-game
need.
