#ifndef SWANSONG_ROTATE_MODEL_H
#define SWANSONG_ROTATE_MODEL_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
	ROTATE_RESULT_PLAYING = 0,
	ROTATE_RESULT_COMPLETE = 1
} rotate_result_t;

typedef struct {
	uint8_t room;
	uint8_t x;
	uint8_t y;
	rotate_result_t result;
	bool vertical;
	bool key;
} rotate_state_t;

typedef struct {
	int8_t dx;
	int8_t dy;
	bool rotate;
	bool reset_room;
	bool replay;
} rotate_input_t;

typedef struct {
	uint16_t tone_hz;
	uint8_t tone_frames;
	bool orientation_changed;
	bool vertical;
	bool reset_session;
	bool dirty;
} rotate_event_t;

bool rotate_blocked(uint8_t room, bool vertical, uint8_t x, uint8_t y);
uint8_t rotate_key_x(uint8_t room);
uint8_t rotate_key_y(uint8_t room);
void rotate_reset(rotate_state_t *state);
void rotate_step(rotate_state_t *state, const rotate_input_t *input,
	rotate_event_t *event);

#endif
