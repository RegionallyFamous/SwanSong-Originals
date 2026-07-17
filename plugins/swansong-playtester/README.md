# SwanSong Playtester

This plugin lets an agent play the built SwanSong Originals cartridges as a
black box. SwanSong Desktop boots the unmodified `.wsc` file in its own engine,
applies an exact frame/input plan, and returns both the native screenshot and a
replayable evidence report through SwanSong's MCP server.

The key quality rule is simple: a successful build or boot is not a gameplay
pass. The agent must inspect the screen, choose an input from the visible state,
observe the result, and exercise the game-specific checks in `scripts/games.json`.

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
