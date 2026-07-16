#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "rf_swan.h"
#include "native_art.h"

static const char __far title[] = "MOTE SOUND TERMINAL";
static const char __far subtitle[] = "Original hangar synth";
static const char __far help[] = "LR track UD tempo A play";
static const char __far fmt_track[] = "TRACK %u  ";
static const char __far fmt_tempo[] = "TEMPO %u\nSCOPE %u (B)\n";
static const char __far fmt_status[] = "STATUS: %s\n";
static const char __far fmt_bar[] = "%u ";
static const char __far playing_text[] = "PLAYING";
static const char __far paused_text[] = "PAUSED";
static const char __far track_1[] = "COLONY SUNRISE\n";
static const char __far track_2[] = "HANGAR AFTERGLOW\n";
static const char __far track_3[] = "ORBITAL SORTIE\n";
static const char __far note[] = "Original pocket synths.\n";

static uint16_t note_hz(uint8_t track, uint8_t step) {
	static const uint16_t scale[8] = {196, 220, 247, 262, 294, 330, 392, 440};
	uint8_t index = (uint8_t)((step * (track + 1) + track * 2) & 7);
	return (uint16_t)(scale[index] + track * 24);
}

static void print_track(uint8_t track) {
	if (track == 0) printf(track_1);
	else if (track == 1) printf(track_2);
	else printf(track_3);
}

static void render(uint8_t track, bool playing, uint8_t tempo,
	uint8_t scope, uint8_t step) {
	uint8_t row;
	rf_clear();
	rf_header(title, subtitle);
	printf(fmt_track, track + 1);
	print_track(track);
	printf(fmt_status, playing ? playing_text : paused_text);
	for (row = 0; row < 3; ++row) {
		uint8_t level = (uint8_t)(((step + row * 3 + scope * 2) % 15) + 1);
		printf(fmt_bar, row + 1);
		rf_print_bar(level, 15, 14);
		putchar('\n');
	}
	printf(fmt_tempo, tempo, scope + 1);
	printf(note);
	rf_footer(help);
}

void main(void) {
	uint8_t track = 0;
	uint8_t tempo = 12;
	uint8_t scope = 0;
	uint8_t step = 0;
	uint8_t tick = 0;
	bool playing = true;
	bool dirty = true;

	rf_init(false);
	RF_LOAD_NATIVE_ART();
	while (1) {
		const rf_input_t *input;
		int8_t dx;
		int8_t dy;
		rf_frame();
		input = rf_input();
		dx = rf_dx(input->pressed);
		dy = rf_dy(input->pressed);

		if (dx) {
			track = (uint8_t)((track + (dx > 0 ? 1 : 2)) % 3);
			step = 0;
			dirty = true;
		}
		if (dy) {
			tempo = rf_clamp_u8((int16_t)tempo + (dy > 0 ? 1 : -1), 5, 20);
			dirty = true;
		}
		if (input->pressed & WS_KEY_A) {
			playing = !playing;
			if (!playing) rf_sound_off();
			dirty = true;
		}
		if (input->pressed & WS_KEY_B) {
			scope ^= 1;
			dirty = true;
		}
		if (input->pressed & WS_KEY_START) {
			track = 0;
			tempo = 12;
			scope = 0;
			step = 0;
			tick = 0;
			playing = true;
			dirty = true;
		}

		if (playing && ++tick >= tempo) {
			tick = 0;
			step = (uint8_t)((step + 1) & 15);
			rf_tone(note_hz(track, step), 7);
			dirty = true;
		}
		if (dirty) {
			render(track, playing, tempo, scope, step);
			dirty = false;
		}
	}
}
