#include <stdbool.h>
#include <stdint.h>

#include "rf_swan.h"
#include "gfx.h"

bool orbital_blocked(uint8_t x, uint8_t y) {
	if (x == 0 || x == 19 || y == 0 || y == 8) return true;
	if (y == 3 && x > 3 && x < 15 && x != 9) return true;
	if (x == 12 && y > 3 && y < 8 && y != 6) return true;
	return false;
}

void main(void) {
	uint8_t px = 2;
	uint8_t py = 1;
	uint8_t fuel = 40;
	uint8_t steps = 0;
	uint8_t result = 0;
	bool parcel = false;
	bool dirty = true;

	rf_init(false);
	orbital_gfx_show_intro();
	for (steps = 0; steps < 36; ++steps) {
		rf_frame();
		if (rf_input()->pressed) break;
	}
	steps = 0;
	orbital_gfx_init();
	while (1) {
		const rf_input_t *input;
		int8_t dx;
		int8_t dy;
		rf_frame();
		input = rf_input();

		if (result && (input->pressed & WS_KEY_A)) {
			px = 2; py = 1; fuel = 40; steps = 0; result = 0; parcel = false;
			dirty = true;
		}
		if (!result) {
			dx = rf_dx(input->pressed);
			dy = rf_dy(input->pressed);
			if (dx || dy) {
				uint8_t nx = (uint8_t)(px + dx);
				uint8_t ny = (uint8_t)(py + dy);
				if (!orbital_blocked(nx, ny)) {
					px = nx; py = ny; ++steps;
					if (fuel) --fuel;
					rf_beep(330, 2);
				} else {
					rf_beep(120, 4);
				}
				if (px == 3 && py == 7) parcel = true;
				if (parcel && px == 17 && py == 1) {
					result = 1;
					rf_beep(660, 12);
				} else if (fuel == 0) {
					result = 2;
				}
				dirty = true;
			}
		}
		if (dirty) {
			orbital_gfx_render(px, py, parcel, fuel, steps, result);
			dirty = false;
		}
	}
}
