#include <stdbool.h>
#include <stdint.h>

#include "rf_swan.h"
#include "gfx.h"

bool rotate_blocked(uint8_t room, bool vertical, uint8_t x, uint8_t y) {
	uint8_t gap;
	if (x == 0 || x == 11 || y == 0 || y == 7) return true;
	if (!vertical) {
		gap = (uint8_t)(1 + room % 6);
		return x == (uint8_t)(3 + room % 5) && y != gap;
	}
	gap = (uint8_t)(1 + (room * 2) % 10);
	return y == (uint8_t)(2 + room % 4) && x != gap;
}

uint8_t rotate_key_x(uint8_t room) { return (uint8_t)(2 + room); }
uint8_t rotate_key_y(uint8_t room) { return (uint8_t)(6 - (room & 1)); }

void main(void) {
	uint8_t room = 0;
	uint8_t px = 1;
	uint8_t py = 1;
	uint8_t result = 0;
	bool vertical = false;
	bool key = false;
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
			room = 0; px = 1; py = 1; result = 0; vertical = false; key = false;
			rf_set_orientation(false); dirty = true;
		}
		if (!result) {
			int8_t dx = rf_dx(input->pressed);
			int8_t dy = rf_dy(input->pressed);
			if (input->pressed & WS_KEY_START) {
				vertical = !vertical;
				rf_set_orientation(vertical);
				if (rotate_blocked(room, vertical, px, py)) { px = 1; py = 1; }
				rf_beep(vertical ? 520 : 360, 6);
				dirty = true;
			}
			if (input->pressed & WS_KEY_B) {
				px = 1; py = 1; key = false; vertical = false;
				rf_set_orientation(false); dirty = true;
			}
			if (dx || dy) {
				uint8_t nx = (uint8_t)(px + dx);
				uint8_t ny = (uint8_t)(py + dy);
				if (!rotate_blocked(room, vertical, nx, ny)) { px = nx; py = ny; }
				else rf_beep(100, 3);
				dirty = true;
			}
			if (px == rotate_key_x(room) && py == rotate_key_y(room)) key = true;
			if (key && px == 10 && py == 1) {
				if (++room >= 5) result = 1;
				else { px = 1; py = 1; key = false; }
				dirty = true;
			}
		}
		if (dirty) {
			gfx_render(room < 5 ? room : 4, vertical, px, py, key, result);
			dirty = false;
		}
	}
}
