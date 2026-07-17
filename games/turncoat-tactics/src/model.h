#ifndef SWANSONG_TURNCOAT_MODEL_H
#define SWANSONG_TURNCOAT_MODEL_H

#include <stdbool.h>
#include <stdint.h>

#define ALLY_CAPACITY 7u
#define ENEMY_CAPACITY 4u

typedef struct {
	int8_t x;
	int8_t y;
	uint8_t hp;
} unit_t;

typedef enum {
	TURNCOAT_RESULT_PLAYING = 0,
	TURNCOAT_RESULT_WIN = 1,
	TURNCOAT_RESULT_LOSS = 2
} turncoat_result_t;

typedef struct {
	unit_t allies[ALLY_CAPACITY];
	unit_t enemies[ENEMY_CAPACITY];
	uint8_t cursor_x;
	uint8_t cursor_y;
	uint8_t selected;
	uint8_t turns;
	uint8_t recruits;
	turncoat_result_t result;
} turncoat_state_t;

typedef struct {
	int8_t dx;
	int8_t dy;
	bool select_or_act;
	bool recruit;
	bool end_turn;
	bool replay;
} turncoat_input_t;

typedef struct {
	uint16_t tone_hz;
	uint8_t tone_frames;
	bool reset_session;
	bool dirty;
} turncoat_event_t;

void turncoat_reset(turncoat_state_t *state);
void turncoat_step(turncoat_state_t *state, const turncoat_input_t *input,
	turncoat_event_t *event);
uint8_t turncoat_enemy_count(const turncoat_state_t *state);
bool turncoat_beacon_secured(const turncoat_state_t *state);

#endif
