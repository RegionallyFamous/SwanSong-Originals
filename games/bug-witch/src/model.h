#ifndef SWANSONG_BUG_WITCH_MODEL_H
#define SWANSONG_BUG_WITCH_MODEL_H

#include <stdbool.h>
#include <stdint.h>

#define BUG_PUZZLE_COUNT 5u
#define BUG_CELL_COUNT 5u

typedef struct {
	uint8_t input;
	uint8_t target;
	uint8_t limit;
	uint8_t mask;
} bug_puzzle_t;

extern const bug_puzzle_t bug_puzzles[BUG_PUZZLE_COUNT];

typedef struct {
	uint8_t cells[BUG_CELL_COUNT];
	uint8_t cursor;
	uint8_t selected;
	uint8_t puzzle;
	bool failed;
	bool complete;
} bug_state_t;

typedef struct {
	int8_t cursor_direction;
	int8_t selection_direction;
	bool place;
	bool clear;
	bool run;
	bool replay;
} bug_input_t;

typedef struct {
	uint16_t tone_hz;
	uint8_t tone_frames;
	bool reset_session;
	bool dirty;
} bug_event_t;

bool bug_run_program(const uint8_t cells[BUG_CELL_COUNT], uint8_t puzzle);
void bug_reset(bug_state_t *state);
void bug_step(bug_state_t *state, const bug_input_t *input,
	bug_event_t *event);

#endif
