#include <assert.h>
#include <string.h>

#include "model.h"

static void clear_input(bug_input_t *input) {
	memset(input, 0, sizeof(*input));
}

static bug_event_t cast_and_finish(bug_state_t *state) {
	bug_input_t input;
	bug_event_t event;
	uint16_t frames = 0;
	clear_input(&input);
	input.run = true;
	bug_step(state, &input, &event);
	assert(state->phase == BUG_PHASE_SIGNAL);
	clear_input(&input);
	while (state->phase == BUG_PHASE_SIGNAL) {
		bug_step(state, &input, &event);
		assert(++frames <= (BUG_CELL_COUNT + 1u) * BUG_SIGNAL_STEP_FRAMES);
	}
	assert(frames == (BUG_CELL_COUNT + 1u) * BUG_SIGNAL_STEP_FRAMES);
	return event;
}

static void advance_clear(bug_state_t *state) {
	bug_input_t input;
	bug_event_t event;
	uint8_t frame;
	clear_input(&input);
	assert(state->phase == BUG_PHASE_PUZZLE_CLEAR);
	for (frame = 0; frame < BUG_CLEAR_HOLD_FRAMES; ++frame)
		bug_step(state, &input, &event);
	assert(state->phase == BUG_PHASE_EDIT);
}

static void load_authored_solution(bug_state_t *state) {
	uint8_t cell;
	for (cell = 0; cell < BUG_CELL_COUNT; ++cell)
		state->cells[cell] = bug_solution_cell(state->puzzle, cell);
}

static void solve_spellbook(bug_state_t *state) {
	uint8_t puzzle;
	for (puzzle = 0; puzzle < BUG_PUZZLE_COUNT; ++puzzle) {
		bug_event_t event;
		assert(state->puzzle == puzzle);
		load_authored_solution(state);
		assert(bug_run_program(state->cells, state->puzzle));
		event = cast_and_finish(state);
		assert(event.solved);
		assert(state->medals[puzzle] == 3);
		if (puzzle + 1u < BUG_PUZZLE_COUNT) advance_clear(state);
	}
	assert(state->complete);
	assert(state->phase == BUG_PHASE_COMPLETE);
	assert(state->total_medals == 15);
	assert(bug_progress(state) == 1000);
}

static void assert_every_state_field_is_hashed(const bug_state_t *state) {
	uint32_t baseline = bug_state_hash(state);
#define MUTATE_u8(type, value) ((value) = (type)((uint8_t)(value) + 1u))
#define MUTATE_u16(type, value) ((value) = (type)((uint16_t)(value) + 1u))
#define MUTATE_boolean(type, value) ((value) = !(value))
#define TEST_SCALAR(type, name, hash_kind) do { \
	bug_state_t copy = *state; \
	MUTATE_##hash_kind(type, copy.name); \
	assert(bug_state_hash(&copy) != baseline); \
} while (0);
#define TEST_ARRAY(type, name, count, hash_kind) do { \
	bug_state_t copy = *state; \
	MUTATE_##hash_kind(type, copy.name[0]); \
	assert(bug_state_hash(&copy) != baseline); \
} while (0);
	BUG_STATE_FIELDS(TEST_SCALAR, TEST_ARRAY)
#undef TEST_ARRAY
#undef TEST_SCALAR
#undef MUTATE_boolean
#undef MUTATE_u16
#undef MUTATE_u8
}

static void test_rules_and_authored_progression(void) {
	uint8_t cells[BUG_CELL_COUNT] = {0};
	uint8_t puzzle;
	for (puzzle = 0; puzzle < BUG_PUZZLE_COUNT; ++puzzle) {
		uint8_t cell;
		memset(cells, 0, sizeof(cells));
		for (cell = 0; cell < BUG_CELL_COUNT; ++cell)
			cells[cell] = bug_solution_cell(puzzle, cell);
		assert(bug_run_program(cells, puzzle));
	}
	memset(cells, 0, sizeof(cells));
	assert(!bug_run_program(cells, 0));
	cells[0] = 2;
	cells[1] = 1;
	assert(!bug_run_program(cells, 1));
	assert(!bug_run_program(cells, BUG_PUZZLE_COUNT));
}

static void test_controls_undo_and_pause(void) {
	bug_state_t state;
	bug_input_t input;
	bug_event_t event;

	bug_reset(&state);
	clear_input(&input);
	input.cursor_direction = 1;
	input.selection_direction = 1;
	bug_step(&state, &input, &event);
	assert(state.cursor == 1 && state.selected == 2);
	assert(event.sfx == BUG_SFX_SELECT);

	clear_input(&input);
	input.place = true;
	bug_step(&state, &input, &event);
	assert(state.cells[1] == 2 && state.undo_count == 1);
	assert(event.tone_hz == 110 && event.tone_frames == 3);

	clear_input(&input);
	input.clear = true;
	bug_step(&state, &input, &event);
	assert(state.cells[1] == 0 && state.undo_count == 2);
	bug_step(&state, &input, &event);
	assert(state.cells[1] == 2 && state.undo_count == 1);
	assert(state.undo_uses == 1 && event.sfx == BUG_SFX_UNDO);

	clear_input(&input);
	input.pause = true;
	bug_step(&state, &input, &event);
	assert(state.phase == BUG_PHASE_PAUSED);
	clear_input(&input);
	input.place = true;
	bug_step(&state, &input, &event);
	assert(state.phase == BUG_PHASE_EDIT);

	clear_input(&input);
	input.pause = true;
	bug_step(&state, &input, &event);
	clear_input(&input);
	input.retry = true;
	bug_step(&state, &input, &event);
	assert(state.phase == BUG_PHASE_EDIT && state.selected == 1);
	assert(state.cells[1] == 0 && state.undo_count == 0);

	clear_input(&input);
	input.pause = true;
	bug_step(&state, &input, &event);
	clear_input(&input);
	input.return_title = true;
	bug_step(&state, &input, &event);
	assert(event.returned_title && state.phase == BUG_PHASE_PAUSED);
}

static void test_signal_failure_and_hint(void) {
	bug_state_t state;
	bug_input_t input;
	bug_event_t event;
	uint8_t frame;

	bug_reset(&state);
	clear_input(&input);
	input.run = true;
	bug_step(&state, &input, &event);
	assert(event.sfx == BUG_SFX_CAST && bug_progress(&state) == 0);
	clear_input(&input);
	for (frame = 0; frame < BUG_SIGNAL_STEP_FRAMES; ++frame)
		bug_step(&state, &input, &event);
	assert(state.signal_step == 1 && bug_progress(&state) == 32);
	while (state.phase == BUG_PHASE_SIGNAL) bug_step(&state, &input, &event);
	assert(state.failed && state.attempts == 1);
	assert(!state.hint_visible && event.sfx == BUG_SFX_FAIL);

	event = cast_and_finish(&state);
	assert(state.failed && state.attempts == 2);
	assert(state.hint_visible && state.hint_cell == 0);
	assert(state.hints == 1 && event.sfx == BUG_SFX_HINT);
}

static void test_deterministic_completion_and_reset(void) {
	bug_state_t first;
	bug_state_t second;
	bug_state_t initial;
	bug_input_t input;
	bug_event_t event;
	uint32_t final_hash;

	bug_reset(&first);
	initial = first;
	assert_every_state_field_is_hashed(&first);
	solve_spellbook(&first);
	final_hash = bug_state_hash(&first);

	bug_reset(&second);
	solve_spellbook(&second);
	assert(bug_state_hash(&second) == final_hash);
	assert(memcmp(&first, &second, sizeof(first)) == 0);

	clear_input(&input);
	input.replay = true;
	bug_step(&first, &input, &event);
	assert(event.reset_session && event.dirty);
	assert(memcmp(&first, &initial, sizeof(first)) == 0);
	assert(bug_state_hash(&first) == bug_state_hash(&initial));
}

int main(void) {
	test_rules_and_authored_progression();
	test_controls_undo_and_pause();
	test_signal_failure_and_hint();
	test_deterministic_completion_and_reset();
	return 0;
}
