#include <stdbool.h>
#include <stdint.h>

#include "rf_swan.h"
#include "gfx.h"

#define FRAME_RATE 75
#define NIGHT_FRAMES 4500

static uint16_t target_for(uint8_t clue) {
	if (clue == 0) return 934;
	if (clue == 1) return 995;
	return 1042;
}

void main(void) {
	uint16_t frequency = 880;
	uint16_t time = NIGHT_FRAMES;
	uint8_t gain = 5;
	uint8_t clue = 0;
	uint8_t result = 0;
	bool gate = false;
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
		int8_t dy;
		rf_frame();
		input = rf_input();
		dx = rf_dx((rf_frame_count() % 3) == 0 ? input->held : input->pressed);
		dy = rf_dy((rf_frame_count() % 3) == 0 ? input->held : input->pressed);

		if (result && (input->pressed & WS_KEY_A)) {
			frequency = 880; time = NIGHT_FRAMES; gain = 5; clue = 0; result = 0; gate = false;
			dirty = true;
		}
		if (!result) {
			if (dx) {
				frequency = (uint16_t)rf_clamp_u8((int16_t)((frequency - 880) / 2) + dx, 0, 100) * 2 + 880;
				dirty = true;
			}
			if (dy) {
				gain = rf_clamp_u8((int16_t)gain - dy, 0, 9);
				dirty = true;
			}
			if (input->pressed & WS_KEY_B) { gate = !gate; dirty = true; }
			if (input->pressed & WS_KEY_A) {
				uint16_t target = target_for(clue);
				uint16_t distance = frequency > target ? frequency - target : target - frequency;
				if (distance <= 3 && gain >= 3) {
					++clue;
					rf_beep((uint16_t)(440 + clue * 80), 12);
					if (clue == 3) result = 1;
				} else {
					time = time > 300 ? time - 300 : 0;
					rf_beep(110, 8);
				}
				dirty = true;
			}
			if (time) --time;
			else result = 2;
		}
		if (dirty || (rf_frame_count() & 7) == 0) {
			gfx_render(frequency, gain, time, clue, result, gate);
			dirty = false;
		}
	}
}
