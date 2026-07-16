#include <stdbool.h>
#include <stdint.h>

#include "rf_swan.h"
#include "gfx.h"

void main(void) {
	uint8_t lap = 1;
	uint8_t progress = 0;
	uint8_t speed = 0;
	uint8_t battery = 70;
	uint8_t lane = 1;
	uint8_t result = 0;
	bool helped = false;
	bool crash_zone = false;
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
		rf_frame();
		input = rf_input();

		if (result && (input->pressed & WS_KEY_A)) {
			lap = 1; progress = 0; speed = 0; battery = 70; lane = 1;
			result = 0; helped = false; crash_zone = false; dirty = true;
		}
		if (!result) {
			int8_t dx = rf_dx(input->pressed);
			if (dx) { lane = rf_clamp_u8((int16_t)lane + dx, 0, 2); dirty = true; }
			if ((input->held & WS_KEY_A) && (rf_frame_count() % 10) == 0 && speed < 6 && battery) {
				++speed; dirty = true;
			}
			if ((input->held & WS_KEY_B) && (rf_frame_count() % 6) == 0 && speed) {
				--speed; dirty = true;
			}
			if ((input->pressed & WS_KEY_START) && !helped && lap == 2 && progress >= 40 && progress <= 56) {
				helped = true; speed = 0; battery = battery > 10 ? battery - 10 : 0;
				rf_beep(560, 12); dirty = true;
			}
			if ((rf_frame_count() & 7) == 0 && speed) {
				uint16_t next = (uint16_t)progress + speed;
				if (next >= 100) {
					progress = (uint8_t)(next - 100);
					if (++lap > 3) result = 1;
				} else progress = (uint8_t)next;
				if (battery) --battery;
				dirty = true;
			}
			if (!crash_zone && ((progress >= 24 && progress <= 30 && lane == 1) ||
				(progress >= 68 && progress <= 74 && lane == 2))) {
				speed = 1; battery = battery > 6 ? battery - 6 : 0; crash_zone = true;
				rf_beep(100, 8); dirty = true;
			}
			if (!((progress >= 20 && progress <= 34) || (progress >= 64 && progress <= 78))) crash_zone = false;
			if (battery == 0) { speed = 0; result = 2; dirty = true; }
		}
		if (dirty || (rf_frame_count() & 15) == 0) {
			gfx_render(lap > 3 ? 3 : lap, progress, speed, battery, lane, helped, result);
			dirty = false;
		}
	}
}
