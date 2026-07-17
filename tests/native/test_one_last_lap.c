#include <assert.h>
#include <string.h>

#include "model.h"

static const lap_input_t idle = {0, false, false, false};

int main(void) {
	lap_state_t state;
	lap_state_t reset_copy;
	lap_input_t input = idle;
	lap_event_t event;

	lap_reset(&state);
	assert(state.lap == 1 && state.battery == 70 && state.lane == 1);
	reset_copy = state;
	state.progress = 88;
	lap_reset(&state);
	assert(memcmp(&state, &reset_copy, sizeof(state)) == 0);

	input.lane_direction = -1;
	lap_step(&state, &input, 1, &event);
	assert(state.lane == 0 && event.dirty);
	input.lane_direction = 1;
	lap_step(&state, &input, 2, &event);
	assert(state.lane == 1 && event.dirty);

	assert(lap_rival_contact(10, 1));
	assert(!lap_rival_contact(10, 2));
	state.progress = 10;
	state.lane = 1;
	state.speed = 6;
	state.battery = 70;
	lap_step(&state, &idle, 3, &event);
	assert(state.speed == 4 && state.battery == 66);
	assert(event.tone_hz == 180 && event.tone_frames == 6);

	lap_reset(&state);
	state.lap = 2;
	state.progress = 48;
	state.speed = 4;
	input = idle;
	input.tow = true;
	lap_step(&state, &input, 5, &event);
	assert(state.helped && state.speed == 0 && state.battery == 60);

	lap_reset(&state);
	state.lap = 3;
	state.progress = 99;
	state.speed = 1;
	lap_step(&state, &idle, 8, &event);
	assert(state.result == LAP_RESULT_SOLO);
	assert(event.tone_hz == 620);

	lap_reset(&state);
	state.lap = 3;
	state.progress = 99;
	state.speed = 1;
	state.helped = true;
	lap_step(&state, &idle, 8, &event);
	assert(state.result == LAP_RESULT_COOPERATIVE);
	assert(event.tone_hz == 760);

	lap_reset(&state);
	state.battery = 1;
	state.speed = 1;
	lap_step(&state, &idle, 8, &event);
	assert(state.result == LAP_RESULT_BATTERY && state.speed == 0);

	return 0;
}
