#!/usr/bin/env bash
set -u

if ! command -v mednafen >/dev/null 2>&1; then
	printf 'SKIP Mednafen is not installed.\n'
	exit 0
fi
if ! command -v ffmpeg >/dev/null 2>&1; then
	printf 'SKIP FFmpeg is required for rendered-frame verification.\n'
	exit 0
fi

status=0
for rom in "$@"; do
	tmpdir="$(mktemp -d -t swan-smoke.XXXXXX)"
	log="$tmpdir/mednafen.log"
	movie="$tmpdir/capture.mov"
	stats="$tmpdir/frame.stats"
	env MEDNAFEN_ALLOWMULTI=1 SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
		mednafen -sound 0 -qtrecord "$movie" "$rom" >"$log" 2>&1 &
	pid=$!
	sleep 0.8
	kill -INT "$pid" >/dev/null 2>&1 || true
	wait "$pid" >/dev/null 2>&1 || true
	frame_ok=0
	if [ -s "$movie" ] && ffmpeg -loglevel error -sseof -0.1 -i "$movie" \
		-frames:v 1 -vf "signalstats,metadata=print:file=$stats" -f null -; then
		if awk -F= '/YMAX/ && $2 + 0 > 20 { found=1 } END { exit !found }' "$stats"; then
			frame_ok=1
		fi
	fi

	if rg -q 'Using module: wswan' "$log" &&
		rg -q 'Recorded Checksum:' "$log" &&
		rg -q 'Real Checksum:' "$log" &&
		[ "$frame_ok" -eq 1 ]; then
		printf 'OK   %s reached a rendered Mednafen frame\n' "$rom"
	else
		printf 'FAIL %s did not reach a rendered WonderSwan frame\n' "$rom"
		sed -n '1,160p' "$log"
		status=1
	fi
	rm -rf "$tmpdir"
done

exit "$status"
