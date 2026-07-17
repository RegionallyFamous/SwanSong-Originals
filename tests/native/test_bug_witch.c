#include <assert.h>
#include <string.h>

#include "model.h"

static void solve(uint8_t cells[BUG_CELL_COUNT], uint8_t puzzle) {
	uint16_t encoded;
	for (encoded = 0; encoded < 1024; ++encoded) {
		uint16_t value = encoded;
		uint8_t i;
		for (i = 0; i < BUG_CELL_COUNT; ++i) {
			cells[i] = (uint8_t)(value & 3u);
			value >>= 2;
		}
		if (bug_run_program(cells, puzzle)) return;
	}
	assert(false);
}

int main(void) {
	bug_state_t state;
	bug_state_t initial;
	bug_event_t event;
	bug_input_t input = {0, 0, false, false, false, false};
	uint8_t puzzle;

	bug_reset(&state);
	initial = state;
	assert(state.selected == 1);
	input.cursor_direction = 1;
	input.selection_direction = 1;
	bug_step(&state, &input, &event);
	assert(state.cursor == 1 && state.selected == 2);
	memset(&input, 0, sizeof(input));
	input.place = true;
	bug_step(&state, &input, &event);
	assert(state.cells[1] == 2 && event.tone_hz == 110 && event.tone_frames == 3);

	bug_reset(&state);
	memset(&input, 0, sizeof(input));
	input.cursor_direction = 0;
	input.selection_direction = 0;
	input.run = true;
	bug_step(&state, &input, &event);
	assert(state.failed && event.tone_hz == 120);

	for (puzzle = 0; puzzle < BUG_PUZZLE_COUNT; ++puzzle) {
		solve(state.cells, state.puzzle);
		bug_step(&state, &input, &event);
		assert(event.tone_hz == 720);
	}
	assert(state.complete);
	memset(&input, 0, sizeof(input));
	input.replay = true;
	bug_step(&state, &input, &event);
	assert(event.reset_session && memcmp(&state, &initial, sizeof(state)) == 0);
	return 0;
}
