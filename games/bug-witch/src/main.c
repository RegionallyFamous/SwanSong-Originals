#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "rf_swan.h"
#include "native_art.h"

static const char __far title[] = "BUG WITCH";
static const char __far subtitle[] = "Send the signal through";
static const char __far help[] = "LR move UD type A/B edit";
static const char __far fmt_status[] = "PUZZLE %u/5  IN %u  GOAL %u\n";
static const char __far fmt_type[] = "FAMILIAR %s\nLIMIT %u\n";
static const char __far flip_text[] = "FLIP";
static const char __far set_text[] = "SET-1";
static const char __far clear_text[] = "CLEAR";

static const uint8_t puzzle_input[5] = {0, 0, 1, 1, 0};
static const uint8_t puzzle_target[5] = {1, 1, 0, 1, 0};
static const uint8_t puzzle_limit[5] = {1, 2, 1, 2, 3};
static const uint8_t puzzle_mask[5] = {1, 3, 4, 5, 7};

static const char __far *type_name(uint8_t type) {
	if (type == 1) return flip_text;
	if (type == 2) return set_text;
	return clear_text;
}

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

static char cell_char(uint8_t type) {
	if (type == 1) return 'F';
	if (type == 2) return '1';
	if (type == 3) return '0';
	return '.';
}

static void render(const uint8_t *cells, uint8_t cursor, uint8_t selected,
	uint8_t puzzle, bool failed, bool complete) {
	uint8_t i;
	rf_clear();
	rf_header(title, subtitle);
	if (complete) {
		printf("ALL CIRCUITS MENDED!\n\n");
		printf("The beetles curl up\ninside the warm machine.\n\n");
		printf("A: solve them again\n");
		rf_footer(help);
		return;
	}
	printf(fmt_status, puzzle + 1, puzzle_input[puzzle], puzzle_target[puzzle]);
	printf(fmt_type, type_name(selected), puzzle_limit[puzzle]);
	printf("IN:");
	for (i = 0; i < 5; ++i) printf("[%c]", cell_char(cells[i]));
	printf("\nPICK SLOT %u\n\n", cursor + 1);
	printf("F flip 1 set 0 clr\n");
	printf("START run circuit\n");
	if (failed) printf("Signal rejected\n");
	else printf("Need familiar mix %u\n", puzzle_mask[puzzle]);
	rf_footer(help);
}

void main(void) {
	uint8_t cells[5] = {0, 0, 0, 0, 0};
	uint8_t cursor = 0;
	uint8_t selected = 1;
	uint8_t puzzle = 0;
	bool failed = false;
	bool complete = false;
	bool dirty = true;

	rf_init(false);
	RF_LOAD_NATIVE_ART();
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
			render(cells, cursor, selected, puzzle < 5 ? puzzle : 4, failed, complete);
			dirty = false;
		}
	}
}
