# SwanSong Playtester

This plugin connects an agent to SwanSong Desktop's full deterministic MCP
server. In addition to black-box SwanSong Originals playtesting, it exposes the
guarded Translation Lab capture and rectangle probes plus recoverable
observed-play sessions. SwanSong keeps project evidence private unless a tool's
explicit sharing or project-write confirmation allows that operation.

The currently exposed tool families are:

- deterministic one-shot playtest plans;
- deterministic Original/Patched one-shot comparisons;
- Translation Lab paired capture and rectangle ownership/source probes; and
- observed-play start, resume, step, finish, and cancel.

The key quality rule is simple: a successful build or boot is not a gameplay
pass. The agent must inspect the screen, choose an input from the visible state,
observe the result, and exercise the game-specific checks in `scripts/games.json`.
The skill also carries a concise clean-room pattern guide for dual-cluster
gestures, audio-led play, suspend/resume, utility editors, procedural rooms, and
the SDK learning loop. Historical descriptions inform coverage; their code,
assets, names, and exact interfaces are never bundled or copied.

Each game manifest supplies six fresh-boot plans: neutral, interaction,
success/utility, failure/boundary, reset/replay, and deterministic replay.
Ordinary physical-button presses last one frame with two neutral drain frames;
holds are reserved for continuous game mechanics. The deterministic plan
reuses the success/utility input history unchanged.

## Local Codex setup

From the repository root:

```sh
codex plugin marketplace add .
codex plugin add swansong-playtester@swansong-originals
```

Start a new Codex task after installation. The launcher finds SwanSong Desktop
at the standard adjacent checkout used by this workspace. Set
`SWANSONG_DESKTOP_DIR` when it lives elsewhere. There is no fallback emulator:
SwanSong is the only execution and capture path.

Useful direct checks are:

```sh
make playtester-check
```

## ChatGPT and hosted agents

The local plugin uses SwanSong's MCP server over standard input/output. A
ChatGPT workspace agent can use the same tool contract after SwanSong is hosted
behind a public HTTPS Streamable HTTP endpoint and added as a custom MCP app.
Do not configure an agent with instructions that promise these tools before that
app is connected. SwanSong itself can remain on a macOS build host;
only its transport and authentication layer need to change.

## Evidence model

Every observation returns the full `swan-song-frame-input-plan-v1`, SwanSong
engine identity, ROM digest, exact frame number, screenshot, and final audio
window with native-raster, PNG, PCM, and WAV hashes. Each new
action replays the whole history from a fresh SwanSong boot. That makes a
reported bug reproducible and avoids depending on GUI focus or a long-running
player process. Captures are evidence, not automatically a pass: the agent is
responsible for comparing the visible result with the game contract.
Scenarios marked `audio = true` additionally require an inspected WAV
observation; hash movement alone is not an audio verdict.
