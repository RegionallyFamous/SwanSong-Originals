#ifndef SWANSONG_LAP_MODEL_H
#define SWANSONG_LAP_MODEL_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
	LAP_RESULT_PLAYING = 0,
	LAP_RESULT_SOLO = 1,
	LAP_RESULT_BATTERY = 2,
	LAP_RESULT_COOPERATIVE = 3
} lap_result_t;

typedef struct {
	int8_t lane_direction;
	bool accelerate;
	bool brake;
	bool tow;
} lap_input_t;

typedef struct {
	uint8_t lap;
	uint8_t progress;
	uint8_t speed;
	uint8_t battery;
	uint8_t lane;
	lap_result_t result;
	bool helped;
	bool crash_zone;
	bool rival_zone;
} lap_state_t;

typedef struct {
	uint16_t tone_hz;
	uint8_t tone_frames;
	bool dirty;
} lap_event_t;

void lap_reset(lap_state_t *state);
void lap_step(lap_state_t *state, const lap_input_t *input,
	uint32_t session_tick, lap_event_t *event);
uint8_t lap_rival_lane(uint8_t progress);
bool lap_rival_contact(uint8_t progress, uint8_t lane);

#endif
