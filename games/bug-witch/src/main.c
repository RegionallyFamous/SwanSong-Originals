#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "rf_swan.h"
#include "gfx.h"

static const uint8_t puzzle_input[5] = {0, 0, 1, 1, 0};
static const uint8_t puzzle_target[5] = {1, 1, 0, 1, 0};
static const uint8_t puzzle_limit[5] = {1, 2, 1, 2, 3};
static const uint8_t puzzle_mask[5] = {1, 3, 4, 5, 7};

static bool run_program(const uint8_t *cells, uint8_t puzzle) {
	uint8_t signal = puzzle_input[puzzle];
	uint8_t used = 0;
	uint8_t mask = 0;
	uint8_t i;
	for (i = 0; i < 5; ++i) {
		if (cells[i] == 1) { signal ^= 1; mask |= 1; ++used; }
		else if (cells[i] == 2) { signal = 1; mask |= 2; ++used; }
		else if (cells[i] == 3) { signal = 0; mask |= 4; ++used; }
	}
	return signal == puzzle_target[puzzle] && used <= puzzle_limit[puzzle] &&
		(mask & puzzle_mask[puzzle]) == puzzle_mask[puzzle];
}

void main(void) {
	uint8_t cells[5] = {0, 0, 0, 0, 0};
	uint8_t cursor = 0;
	uint8_t selected = 1;
	uint8_t puzzle = 0;
	bool failed = false;
	bool complete = false;
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

		if (complete && (input->pressed & WS_KEY_A)) {
			memset(cells, 0, sizeof(cells)); cursor = 0; selected = 1;
			puzzle = 0; failed = false; complete = false; dirty = true;
		}
		if (!complete) {
			int8_t dx = rf_dx(input->pressed);
			int8_t dy = rf_dy(input->pressed);
			if (dx) { cursor = rf_clamp_u8((int16_t)cursor + dx, 0, 4); dirty = true; }
			if (dy) {
				selected = (uint8_t)((selected - 1 + (dy > 0 ? 1 : 2)) % 3 + 1);
				dirty = true;
			}
			if (input->pressed & WS_KEY_A) { cells[cursor] = selected; failed = false; dirty = true; }
			if (input->pressed & WS_KEY_B) { cells[cursor] = 0; failed = false; dirty = true; }
			if (input->pressed & WS_KEY_START) {
				if (run_program(cells, puzzle)) {
					rf_beep(720, 10);
					memset(cells, 0, sizeof(cells)); cursor = 0; failed = false;
					if (++puzzle >= 5) complete = true;
				} else {
					failed = true; rf_beep(120, 8);
				}
				dirty = true;
			}
		}
		if (dirty) {
			gfx_render(cells, cursor, selected, puzzle < 5 ? puzzle : 4, failed, complete);
			dirty = false;
		}
	}
}
