#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -lt 2 ]; then
	printf 'usage: %s OUTPUT_DIR ROM...\n' "$0" >&2
	exit 2
fi

output_dir=$1
shift
mkdir -p "$output_dir"

for rom in "$@"; do
	name="$(basename "${rom%.*}")"
	tmpdir="$(mktemp -d -t "${name}.XXXXXX")"
	movie="$tmpdir/capture.mov"
	log="$tmpdir/mednafen.log"
	env MEDNAFEN_ALLOWMULTI=1 SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
		mednafen -sound 0 -qtrecord "$movie" "$rom" >"$log" 2>&1 &
	pid=$!
	sleep 0.8
	kill -INT "$pid" >/dev/null 2>&1 || true
	wait "$pid" >/dev/null 2>&1 || true
	test -s "$movie"
	ffmpeg -loglevel error -y -sseof -0.1 -i "$movie" -frames:v 1 \
		"$output_dir/$name.png"
	test -s "$output_dir/$name.png"
	rm -rf "$tmpdir"
	printf 'CAP  %s\n' "$output_dir/$name.png"
done

if command -v magick >/dev/null 2>&1; then
	rm -f "$output_dir/contact-sheet.png"
	row_dir="$(mktemp -d -t swan-capture-rows.XXXXXX)"
	files=("$output_dir"/*.png)
	row=0
	for ((i = 0; i < ${#files[@]}; i += 2)); do
		if ((i + 1 < ${#files[@]})); then
			magick "${files[i]}" "${files[i + 1]}" +append "$row_dir/$row.png"
		else
			cp "${files[i]}" "$row_dir/$row.png"
		fi
		row=$((row + 1))
	done
	magick "$row_dir"/*.png -append "$output_dir/contact-sheet.png"
	rm -rf "$row_dir"
	printf 'SHEET %s\n' "$output_dir/contact-sheet.png"
fi
