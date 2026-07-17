#include <string.h>

#include "model.h"

const bug_puzzle_t bug_puzzles[BUG_PUZZLE_COUNT] = {
	{0, 1, 1, 1}, {0, 1, 2, 3}, {1, 0, 1, 4},
	{1, 1, 2, 5}, {0, 0, 3, 7}
};

static uint8_t clamp_u8(int16_t value, uint8_t low, uint8_t high) {
	if (value < low) return low;
	if (value > high) return high;
	return (uint8_t)value;
}

bool bug_run_program(const uint8_t cells[BUG_CELL_COUNT], uint8_t puzzle) {
	uint8_t signal;
	uint8_t used = 0;
	uint8_t mask = 0;
	uint8_t i;
	if (puzzle >= BUG_PUZZLE_COUNT) return false;
	signal = bug_puzzles[puzzle].input;
	for (i = 0; i < BUG_CELL_COUNT; ++i) {
		if (cells[i] == 1) { signal ^= 1; mask |= 1; ++used; }
		else if (cells[i] == 2) { signal = 1; mask |= 2; ++used; }
		else if (cells[i] == 3) { signal = 0; mask |= 4; ++used; }
	}
	return signal == bug_puzzles[puzzle].target &&
		used <= bug_puzzles[puzzle].limit &&
		(mask & bug_puzzles[puzzle].mask) == bug_puzzles[puzzle].mask;
}

void bug_reset(bug_state_t *state) {
	memset(state, 0, sizeof(*state));
	state->selected = 1;
}

void bug_step(bug_state_t *state, const bug_input_t *input,
	bug_event_t *event) {
	memset(event, 0, sizeof(*event));
	if (state->complete && input->replay) {
		bug_reset(state);
		event->reset_session = true;
		event->dirty = true;
		return;
	}
	if (state->complete) return;
	if (input->cursor_direction) {
		state->cursor = clamp_u8((int16_t)state->cursor +
			input->cursor_direction, 0, BUG_CELL_COUNT - 1);
		event->dirty = true;
	}
	if (input->selection_direction) {
		state->selected = (uint8_t)((state->selected - 1 +
			(input->selection_direction > 0 ? 1 : 2)) % 3 + 1);
		event->dirty = true;
	}
	if (input->place) {
		state->cells[state->cursor] = state->selected;
		state->failed = false;
		event->tone_hz = 110;
		event->tone_frames = 3;
		event->dirty = true;
	}
	if (input->clear) {
		state->cells[state->cursor] = 0;
		state->failed = false;
		event->tone_hz = 92;
		event->tone_frames = 3;
		event->dirty = true;
	}
	if (input->run) {
		if (bug_run_program(state->cells, state->puzzle)) {
			event->tone_hz = 720;
			event->tone_frames = 10;
			memset(state->cells, 0, sizeof(state->cells));
			state->cursor = 0;
			state->failed = false;
			if (++state->puzzle >= BUG_PUZZLE_COUNT) state->complete = true;
		} else {
			state->failed = true;
			event->tone_hz = 120;
			event->tone_frames = 8;
		}
		event->dirty = true;
	}
}
