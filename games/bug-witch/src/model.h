#ifndef SWANSONG_BUG_WITCH_MODEL_H
#define SWANSONG_BUG_WITCH_MODEL_H

#include <stdbool.h>
#include <stdint.h>

#define BUG_PUZZLE_COUNT 5u
#define BUG_CELL_COUNT 5u
#define BUG_UNDO_DEPTH 6u
#define BUG_SIGNAL_STEP_FRAMES 6u
#define BUG_CLEAR_HOLD_FRAMES 42u
#define BUG_NO_HINT 0xFFu

typedef struct {
	uint8_t input;
	uint8_t target;
	uint8_t limit;
	uint8_t mask;
} bug_puzzle_t;

extern const bug_puzzle_t bug_puzzles[BUG_PUZZLE_COUNT];

typedef enum {
	BUG_PHASE_EDIT = 0,
	BUG_PHASE_SIGNAL,
	BUG_PHASE_PUZZLE_CLEAR,
	BUG_PHASE_PAUSED,
	BUG_PHASE_COMPLETE
} bug_phase_t;

typedef enum {
	BUG_SFX_NONE = 0,
	BUG_SFX_MOVE,
	BUG_SFX_SELECT,
	BUG_SFX_PLACE,
	BUG_SFX_CLEAR,
	BUG_SFX_UNDO,
	BUG_SFX_CAST,
	BUG_SFX_FAIL,
	BUG_SFX_HINT,
	BUG_SFX_SOLVE,
	BUG_SFX_FINISH,
	BUG_SFX_PAUSE
} bug_sfx_t;

/*
 * This inventory defines both the portable state and its canonical hash.
 * Adding state here necessarily adds it to bug_state_hash() in model.c.
 */
#define BUG_STATE_FIELDS(SCALAR, ARRAY) \
	ARRAY(uint8_t, cells, BUG_CELL_COUNT, u8) \
	ARRAY(uint8_t, undo_cells, BUG_UNDO_DEPTH * BUG_CELL_COUNT, u8) \
	ARRAY(uint8_t, medals, BUG_PUZZLE_COUNT, u8) \
	SCALAR(uint16_t, phase_frames, u16) \
	SCALAR(uint8_t, cursor, u8) \
	SCALAR(uint8_t, selected, u8) \
	SCALAR(uint8_t, puzzle, u8) \
	SCALAR(uint8_t, signal_step, u8) \
	SCALAR(uint8_t, signal_value, u8) \
	SCALAR(uint8_t, edits, u8) \
	SCALAR(uint8_t, attempts, u8) \
	SCALAR(uint8_t, hints, u8) \
	SCALAR(uint8_t, undo_count, u8) \
	SCALAR(uint8_t, undo_uses, u8) \
	SCALAR(uint8_t, total_medals, u8) \
	SCALAR(uint8_t, hint_cell, u8) \
	SCALAR(bug_phase_t, phase, u8) \
	SCALAR(bug_phase_t, resume_phase, u8) \
	SCALAR(bool, failed, boolean) \
	SCALAR(bool, complete, boolean) \
	SCALAR(bool, hint_visible, boolean)

#define BUG_STATE_DECLARE_SCALAR(type, name, hash_kind) type name;
#define BUG_STATE_DECLARE_ARRAY(type, name, count, hash_kind) type name[count];
typedef struct {
	BUG_STATE_FIELDS(BUG_STATE_DECLARE_SCALAR, BUG_STATE_DECLARE_ARRAY)
} bug_state_t;
#undef BUG_STATE_DECLARE_ARRAY
#undef BUG_STATE_DECLARE_SCALAR

typedef struct {
	int8_t cursor_direction;
	int8_t selection_direction;
	bool place;
	bool clear;
	bool run;
	bool replay;
	bool pause;
	bool retry;
	bool return_title;
} bug_input_t;

typedef struct {
	bug_sfx_t sfx;
	uint16_t tone_hz;
	uint8_t tone_frames;
	bool reset_session;
	bool dirty;
	bool phase_changed;
	bool solved;
	bool completed;
	bool returned_title;
} bug_event_t;

bool bug_run_program(const uint8_t cells[BUG_CELL_COUNT], uint8_t puzzle);
uint8_t bug_solution_cell(uint8_t puzzle, uint8_t cell);
void bug_reset(bug_state_t *state);
void bug_reset_current(bug_state_t *state);
void bug_step(bug_state_t *state, const bug_input_t *input,
	bug_event_t *event);
uint16_t bug_progress(const bug_state_t *state);
uint32_t bug_state_hash(const bug_state_t *state);

#endif
