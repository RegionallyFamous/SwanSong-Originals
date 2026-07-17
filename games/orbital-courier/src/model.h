#ifndef SWANSONG_COURIER_MODEL_H
#define SWANSONG_COURIER_MODEL_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
	COURIER_RESULT_PLAYING = 0,
	COURIER_RESULT_DELIVERED = 1,
	COURIER_RESULT_FUEL = 2
} courier_result_t;

typedef struct {
	uint8_t x;
	uint8_t y;
	uint8_t fuel;
	uint8_t steps;
	courier_result_t result;
	bool parcel;
} courier_state_t;

typedef struct {
	int8_t dx;
	int8_t dy;
} courier_input_t;

typedef struct {
	uint16_t tone_hz;
	uint8_t tone_frames;
	bool dirty;
} courier_event_t;

bool orbital_blocked(uint8_t x, uint8_t y);
void courier_reset(courier_state_t *state);
void courier_step(courier_state_t *state, const courier_input_t *input,
	courier_event_t *event);

#endif
