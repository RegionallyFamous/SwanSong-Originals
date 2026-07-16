#include <stdbool.h>
#include <stdint.h>

#include "rf_swan.h"
#include "gfx.h"

void main(void) {
	static const uint8_t correct[3] = {0, 1, 2};
	uint8_t job = 0;
	uint8_t selected = 0;
	uint8_t score = 0;
	uint8_t phase = 0;
	bool last_ok = false;
	bool dirty = true;
	uint8_t intro;

	rf_init(false);
	gfx_show_intro();
	for (intro = 0; intro < 36; ++intro) {
		rf_frame();
		if (rf_input()->pressed) break;
	}
	gfx_init();
	while (1) {
		const rf_input_t *input;
		int8_t dx;
		rf_frame();
		input = rf_input();
		dx = rf_dx(input->pressed);

		if (phase == 0) {
			if (dx) {
				selected = (uint8_t)((selected + (dx > 0 ? 1 : 2)) % 3);
				dirty = true;
			}
			if (input->pressed & WS_KEY_A) {
				last_ok = selected == correct[job];
				if (last_ok) { ++score; rf_beep(660, 10); }
				else rf_beep(130, 10);
				phase = 1;
				dirty = true;
			}
		} else if (phase == 1 && (input->pressed & WS_KEY_A)) {
			if (++job >= 3) phase = 2;
			else { phase = 0; selected = 0; }
			dirty = true;
		} else if (phase == 2 && (input->pressed & WS_KEY_A)) {
			job = 0; selected = 0; score = 0; phase = 0;
			dirty = true;
		}

		if (dirty) {
			gfx_render(job, selected, score, phase, last_ok);
			dirty = false;
		}
	}
}
