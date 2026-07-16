#include <stdbool.h>
#include <stdint.h>

#include "rf_swan.h"
#include "gfx.h"

static uint16_t note_hz(uint8_t track, uint8_t step) {
	static const uint16_t scale[8] = {196, 220, 247, 262, 294, 330, 392, 440};
	uint8_t index = (uint8_t)((step * (track + 1) + track * 2) & 7);
	return (uint16_t)(scale[index] + track * 24);
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
	gfx_show_intro();
	for (tick = 0; tick < 36; ++tick) {
		rf_frame();
		if (rf_input()->pressed) break;
	}
	tick = 0;
	gfx_init();
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
			gfx_render(track, playing, tempo, scope, step);
			dirty = false;
		}
	}
}
