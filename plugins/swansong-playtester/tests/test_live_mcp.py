#!/usr/bin/env python3
"""End-to-end SwanSong MCP playtest with real ROM, image, audio, and replay."""

from __future__ import annotations

import base64
import json
from pathlib import Path
import subprocess
import sys
import tempfile


PLUGIN = Path(__file__).resolve().parents[1]
REPO = PLUGIN.parents[1]
SERVER = PLUGIN / "scripts" / "run_swansong_mcp.sh"
ROM = REPO / "dist" / "mote_sound_terminal.wsc"
FIXTURES = Path(__file__).with_name("fixtures")


def exchange(process: subprocess.Popen, request: dict) -> dict:
    assert process.stdin is not None and process.stdout is not None
    process.stdin.write(json.dumps(request, separators=(",", ":")) + "\n")
    process.stdin.flush()
    while True:
        line = process.stdout.readline()
        if not line:
            error = process.stderr.read() if process.stderr is not None else ""
            raise AssertionError(f"SwanSong MCP closed before response: {error}")
        response = json.loads(line)
        if response.get("id") == request.get("id"):
            return response


def call(process: subprocess.Popen, request_id: int, arguments: dict) -> dict:
    response = exchange(process, {
        "jsonrpc": "2.0",
        "id": request_id,
        "method": "tools/call",
        "params": {"name": "swansong_playtest_plan", "arguments": arguments},
    })
    return response["result"]


def media(result: dict) -> tuple[dict, bytes, bytes]:
    content = result["content"]
    assert [part["type"] for part in content] == ["text", "image", "audio"]
    png = base64.b64decode(content[1]["data"], validate=True)
    wav = base64.b64decode(content[2]["data"], validate=True)
    assert png.startswith(b"\x89PNG\r\n\x1a\n")
    assert wav[:4] == b"RIFF" and wav[8:12] == b"WAVE"
    return result["structuredContent"], png, wav


def main() -> None:
    assert ROM.is_file(), "run make dist before the live MCP test"
    neutral = json.loads((FIXTURES / "neutral-120.json").read_text())
    controls = json.loads((FIXTURES / "mote-controls-120.json").read_text())
    process = subprocess.Popen(
        [str(SERVER)],
        cwd=PLUGIN,
        text=True,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    try:
        initialized = exchange(process, {
            "jsonrpc": "2.0",
            "id": 1,
            "method": "initialize",
            "params": {
                "protocolVersion": "2025-11-25",
                "capabilities": {},
                "clientInfo": {"name": "SwanSongOriginalsTest", "version": "1"},
            },
        })
        assert initialized["result"]["serverInfo"]["name"] == "swansong-playtester"

        denied = call(process, 2, {
            "romPath": str(ROM), "plan": neutral, "confirmShareCapture": False,
        })
        assert denied.get("isError") is True

        with tempfile.NamedTemporaryFile(suffix=".wsc") as invalid:
            invalid.write(b"not a SwanSong ROM")
            invalid.flush()
            rejected = call(process, 3, {
                "romPath": invalid.name,
                "plan": neutral,
                "confirmShareCapture": True,
            })
            assert rejected.get("isError") is True

        base_arguments = {
            "romPath": str(ROM), "confirmShareCapture": True,
        }
        neutral_a, png_a, wav_a = media(call(
            process, 4, {**base_arguments, "plan": neutral}
        ))
        neutral_b, png_b, wav_b = media(call(
            process, 5, {**base_arguments, "plan": neutral}
        ))
        acted, png_c, wav_c = media(call(
            process, 6, {**base_arguments, "plan": controls}
        ))

        assert neutral_a == neutral_b
        assert png_a == png_b and wav_a == wav_b
        assert acted["plan"] == controls
        assert acted["finalGameRasterSHA256"] != neutral_a["finalGameRasterSHA256"]
        assert acted["audio"]["pcmFloatSHA256"] != neutral_a["audio"]["pcmFloatSHA256"]
        assert png_c != png_a and wav_c != wav_a
        assert acted["audio"]["sampleFrames"] > 0
        assert acted["audio"]["finalWindowWAVByteCount"] == len(wav_c)
        print(
            "PASS SwanSong MCP denied unconfirmed/invalid input, replayed exactly, "
            "and returned divergent PNG+WAV media after controls"
        )
    finally:
        if process.stdin is not None:
            process.stdin.close()
        try:
            process.wait(timeout=10)
        except subprocess.TimeoutExpired:
            process.terminate()
            process.wait(timeout=5)
        if process.returncode not in (0, -15):
            error = process.stderr.read() if process.stderr is not None else ""
            raise AssertionError(f"SwanSong MCP exited {process.returncode}: {error}")


if __name__ == "__main__":
    main()
