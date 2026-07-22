#include <limits.h>
#include <string.h>

#include "model.h"

const bug_puzzle_t bug_puzzles[BUG_PUZZLE_COUNT] = {
	{0, 1, 1, 1}, {0, 1, 2, 3}, {1, 0, 1, 4},
	{1, 1, 2, 5}, {0, 0, 3, 7}
};

/* One authored solution per puzzle, used only by the optional hint display. */
static const uint8_t bug_solutions[BUG_PUZZLE_COUNT][BUG_CELL_COUNT] = {
	{1, 0, 0, 0, 0},
	{1, 2, 0, 0, 0},
	{3, 0, 0, 0, 0},
	{3, 1, 0, 0, 0},
	{1, 2, 3, 0, 0}
};

static void bug_hash_u8(uint32_t *hash, uint8_t value) {
	*hash = (*hash ^ value) * 16777619u;
}

static void bug_hash_u16(uint32_t *hash, uint16_t value) {
	bug_hash_u8(hash, (uint8_t)value);
	bug_hash_u8(hash, (uint8_t)(value >> 8));
}

static void bug_hash_boolean(uint32_t *hash, bool value) {
	bug_hash_u8(hash, value ? 1u : 0u);
}

uint32_t bug_state_hash(const bug_state_t *state) {
	uint32_t hash = 2166136261u;
#define BUG_STATE_HASH_SCALAR(type, name, hash_kind) \
	bug_hash_##hash_kind(&hash, state->name);
#define BUG_STATE_HASH_ARRAY(type, name, count, hash_kind) do { \
	uint8_t index; \
	for (index = 0; index < (count); ++index) \
		bug_hash_##hash_kind(&hash, state->name[index]); \
} while (0);
	BUG_STATE_FIELDS(BUG_STATE_HASH_SCALAR, BUG_STATE_HASH_ARRAY)
#undef BUG_STATE_HASH_ARRAY
#undef BUG_STATE_HASH_SCALAR
	return hash;
}

static uint8_t clamp_u8(int16_t value, uint8_t low, uint8_t high) {
	if (value < low) return low;
	if (value > high) return high;
	return (uint8_t)value;
}

static void set_sfx(bug_event_t *event, bug_sfx_t sfx,
	uint16_t legacy_hz, uint8_t legacy_frames) {
	event->sfx = sfx;
	event->tone_hz = legacy_hz;
	event->tone_frames = legacy_frames;
}

static uint8_t apply_cell(uint8_t signal, uint8_t cell) {
	if (cell == 1) return (uint8_t)(signal ^ 1u);
	if (cell == 2) return 1;
	if (cell == 3) return 0;
	return signal;
}

bool bug_run_program(const uint8_t cells[BUG_CELL_COUNT], uint8_t puzzle) {
	uint8_t signal;
	uint8_t used = 0;
	uint8_t mask = 0;
	uint8_t i;
	if (puzzle >= BUG_PUZZLE_COUNT) return false;
	signal = bug_puzzles[puzzle].input;
	for (i = 0; i < BUG_CELL_COUNT; ++i) {
		signal = apply_cell(signal, cells[i]);
		if (cells[i] >= 1 && cells[i] <= 3) {
			mask |= (uint8_t)(1u << (cells[i] - 1u));
			++used;
		}
	}
	return signal == bug_puzzles[puzzle].target &&
		used <= bug_puzzles[puzzle].limit &&
		(mask & bug_puzzles[puzzle].mask) == bug_puzzles[puzzle].mask;
}

uint8_t bug_solution_cell(uint8_t puzzle, uint8_t cell) {
	if (puzzle >= BUG_PUZZLE_COUNT || cell >= BUG_CELL_COUNT) return 0;
	return bug_solutions[puzzle][cell];
}

static void update_hint(bug_state_t *state) {
	uint8_t cell;
	state->hint_cell = BUG_NO_HINT;
	if (!state->hint_visible || state->puzzle >= BUG_PUZZLE_COUNT) return;
	for (cell = 0; cell < BUG_CELL_COUNT; ++cell) {
		if (state->cells[cell] != bug_solutions[state->puzzle][cell]) {
			state->hint_cell = cell;
			return;
		}
	}
}

static void clear_round_state(bug_state_t *state, bool reset_selection) {
	memset(state->cells, 0, sizeof(state->cells));
	memset(state->undo_cells, 0, sizeof(state->undo_cells));
	state->cursor = 0;
	if (reset_selection) state->selected = 1;
	state->signal_step = 0;
	state->signal_value = 0;
	state->phase_frames = 0;
	state->edits = 0;
	state->attempts = 0;
	state->hints = 0;
	state->undo_count = 0;
	state->undo_uses = 0;
	state->hint_cell = BUG_NO_HINT;
	state->failed = false;
	state->hint_visible = false;
	state->phase = BUG_PHASE_EDIT;
	state->resume_phase = BUG_PHASE_EDIT;
}

void bug_reset(bug_state_t *state) {
	memset(state, 0, sizeof(*state));
	state->selected = 1;
	state->hint_cell = BUG_NO_HINT;
	state->phase = BUG_PHASE_EDIT;
	state->resume_phase = BUG_PHASE_EDIT;
}

void bug_reset_current(bug_state_t *state) {
	clear_round_state(state, true);
}

static void push_undo(bug_state_t *state) {
	uint8_t destination;
	if (state->undo_count == BUG_UNDO_DEPTH) {
		memmove(state->undo_cells,
			&state->undo_cells[BUG_CELL_COUNT],
			(BUG_UNDO_DEPTH - 1u) * BUG_CELL_COUNT);
		state->undo_count = BUG_UNDO_DEPTH - 1u;
	}
	destination = (uint8_t)(state->undo_count * BUG_CELL_COUNT);
	memcpy(&state->undo_cells[destination], state->cells, BUG_CELL_COUNT);
	++state->undo_count;
}

static bool pop_undo(bug_state_t *state) {
	uint8_t source;
	if (state->undo_count == 0) return false;
	--state->undo_count;
	source = (uint8_t)(state->undo_count * BUG_CELL_COUNT);
	memcpy(state->cells, &state->undo_cells[source], BUG_CELL_COUNT);
	memset(&state->undo_cells[source], 0, BUG_CELL_COUNT);
	if (state->undo_uses < UINT8_MAX) ++state->undo_uses;
	update_hint(state);
	return true;
}

static void start_cast(bug_state_t *state, bug_event_t *event) {
	state->phase = BUG_PHASE_SIGNAL;
	state->resume_phase = BUG_PHASE_SIGNAL;
	state->phase_frames = 0;
	state->signal_step = 0;
	state->signal_value = bug_puzzles[state->puzzle].input;
	state->failed = false;
	if (state->attempts < UINT8_MAX) ++state->attempts;
	set_sfx(event, BUG_SFX_CAST, 420, 5);
	event->phase_changed = true;
	event->dirty = true;
}

static uint8_t medal_for(const bug_state_t *state) {
	if (state->attempts == 1 && state->hints == 0 && state->undo_uses == 0)
		return 3;
	if (state->hints == 0) return 2;
	return 1;
}

static void finish_cast(bug_state_t *state, bug_event_t *event) {
	if (bug_run_program(state->cells, state->puzzle)) {
		uint8_t medal = medal_for(state);
		state->medals[state->puzzle] = medal;
		state->total_medals = (uint8_t)(state->total_medals + medal);
		state->failed = false;
		state->hint_visible = false;
		state->hint_cell = BUG_NO_HINT;
		state->undo_count = 0;
		event->solved = true;
		if (state->puzzle + 1u >= BUG_PUZZLE_COUNT) {
			state->complete = true;
			state->phase = BUG_PHASE_COMPLETE;
			state->resume_phase = BUG_PHASE_COMPLETE;
			set_sfx(event, BUG_SFX_FINISH, 840, 18);
			event->completed = true;
		} else {
			state->phase = BUG_PHASE_PUZZLE_CLEAR;
			state->resume_phase = BUG_PHASE_PUZZLE_CLEAR;
			set_sfx(event, BUG_SFX_SOLVE, 720, 12);
		}
	} else {
		bool new_hint = state->attempts >= 2 && !state->hint_visible;
		state->failed = true;
		state->phase = BUG_PHASE_EDIT;
		state->resume_phase = BUG_PHASE_EDIT;
		if (new_hint) {
			state->hint_visible = true;
			if (state->hints < UINT8_MAX) ++state->hints;
			update_hint(state);
			set_sfx(event, BUG_SFX_HINT, 560, 10);
		} else {
			set_sfx(event, BUG_SFX_FAIL, 120, 10);
		}
	}
	state->phase_frames = 0;
	event->phase_changed = true;
	event->dirty = true;
}

static void advance_signal(bug_state_t *state, bug_event_t *event) {
	++state->phase_frames;
	event->dirty = true;
	if (state->phase_frames < BUG_SIGNAL_STEP_FRAMES) return;
	state->phase_frames = 0;
	if (state->signal_step < BUG_CELL_COUNT) {
		state->signal_value = apply_cell(state->signal_value,
			state->cells[state->signal_step]);
		++state->signal_step;
		return;
	}
	finish_cast(state, event);
}

static void advance_clear(bug_state_t *state, bug_event_t *event) {
	++state->phase_frames;
	if (state->phase_frames >= BUG_CLEAR_HOLD_FRAMES) {
		++state->puzzle;
		clear_round_state(state, false);
		event->phase_changed = true;
		event->dirty = true;
	}
}

uint16_t bug_progress(const bug_state_t *state) {
	uint16_t base;
	if (state->complete) return 1000;
	base = (uint16_t)state->puzzle * 200u;
	if (state->phase == BUG_PHASE_SIGNAL)
		base = (uint16_t)(base + state->signal_step * 32u);
	else if (state->phase == BUG_PHASE_PUZZLE_CLEAR)
		base = (uint16_t)(base + 180u);
	return base > 999u ? 999u : base;
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

	if (input->pause) {
		if (state->phase == BUG_PHASE_PAUSED) {
			state->phase = state->resume_phase;
		} else if (state->phase == BUG_PHASE_EDIT ||
			state->phase == BUG_PHASE_SIGNAL) {
			state->resume_phase = state->phase;
			state->phase = BUG_PHASE_PAUSED;
		} else {
			return;
		}
		set_sfx(event, BUG_SFX_PAUSE, 300, 4);
		event->phase_changed = true;
		event->dirty = true;
		return;
	}

	if (state->phase == BUG_PHASE_PAUSED) {
		if (input->return_title) {
			event->returned_title = true;
			return;
		}
		if (input->retry) {
			bug_reset_current(state);
			set_sfx(event, BUG_SFX_UNDO, 260, 6);
			event->phase_changed = true;
			event->dirty = true;
			return;
		}
		if (input->place) {
			state->phase = state->resume_phase;
			set_sfx(event, BUG_SFX_PAUSE, 360, 4);
			event->phase_changed = true;
			event->dirty = true;
		}
		return;
	}

	if (state->phase == BUG_PHASE_SIGNAL) {
		advance_signal(state, event);
		return;
	}
	if (state->phase == BUG_PHASE_PUZZLE_CLEAR) {
		advance_clear(state, event);
		return;
	}
	if (state->phase != BUG_PHASE_EDIT) return;

	if (input->cursor_direction) {
		uint8_t cursor = clamp_u8((int16_t)state->cursor +
			input->cursor_direction, 0, BUG_CELL_COUNT - 1u);
		if (cursor != state->cursor) {
			state->cursor = cursor;
			set_sfx(event, BUG_SFX_MOVE, 220, 2);
			event->dirty = true;
		}
	}
	if (input->selection_direction) {
		state->selected = (uint8_t)((state->selected - 1u +
			(input->selection_direction > 0 ? 1u : 2u)) % 3u + 1u);
		set_sfx(event, BUG_SFX_SELECT, 320, 3);
		event->dirty = true;
	}
	if (input->place) {
		if (state->cells[state->cursor] != state->selected) {
			push_undo(state);
			state->cells[state->cursor] = state->selected;
			if (state->edits < UINT8_MAX) ++state->edits;
			update_hint(state);
		}
		state->failed = false;
		set_sfx(event, BUG_SFX_PLACE, 110, 3);
		event->dirty = true;
	}
	if (input->clear) {
		if (state->cells[state->cursor] != 0) {
			push_undo(state);
			state->cells[state->cursor] = 0;
			if (state->edits < UINT8_MAX) ++state->edits;
			update_hint(state);
			set_sfx(event, BUG_SFX_CLEAR, 92, 3);
		} else if (pop_undo(state)) {
			set_sfx(event, BUG_SFX_UNDO, 260, 5);
		} else {
			set_sfx(event, BUG_SFX_CLEAR, 92, 2);
		}
		state->failed = false;
		event->dirty = true;
	}
	if (input->run) start_cast(state, event);
}
