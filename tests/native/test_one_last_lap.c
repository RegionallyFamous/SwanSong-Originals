#include <assert.h>
#include <string.h>

#include "model.h"

static const lap_input_t idle = {0, false, false, false};

static lap_state_t finish_race(bool tow) {
	lap_state_t state;
	lap_input_t input = {1, false, false, false};
	lap_event_t event;
	uint32_t tick;

	lap_reset(&state);
	lap_step(&state, &input, 1, &event);
	assert(state.lane == 2);
	input.lane_direction = 0;
	input.accelerate = true;
	input.tow = tow;
	for (tick = 2; tick < 2000 && state.result == LAP_RESULT_PLAYING; ++tick)
		lap_step(&state, &input, tick, &event);
	return state;
}

int main(void) {
	lap_state_t state;
	lap_state_t reset_copy;
	lap_input_t input = idle;
	lap_event_t event;
	lap_state_t completed;
	lap_state_t repeated;

	lap_reset(&state);
	assert(state.lap == 1 && state.battery == 120 && state.lane == 1);
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
	state.battery = 120;
	lap_step(&state, &idle, 3, &event);
	assert(state.speed == 4 && state.battery == 117);
	assert(event.tone_hz == 180 && event.tone_frames == 6);

	lap_reset(&state);
	state.lap = 2;
	state.progress = 48;
	state.speed = 4;
	input = idle;
	input.tow = true;
	lap_step(&state, &input, 5, &event);
	assert(state.helped && state.speed == 0 && state.battery == 120);

	completed = finish_race(true);
	repeated = finish_race(true);
	assert(completed.result == LAP_RESULT_COOPERATIVE && completed.helped);
	assert(memcmp(&completed, &repeated, sizeof(completed)) == 0);
	completed = finish_race(false);
	assert(completed.result == LAP_RESULT_SOLO && !completed.helped);

	lap_reset(&state);
	state.battery = 2;
	input = idle;
	input.accelerate = true;
	input.brake = true;
	lap_step(&state, &input, 6, &event);
	assert(state.battery == 1 && state.result == LAP_RESULT_PLAYING);
	lap_step(&state, &input, 12, &event);
	assert(state.battery == 0 && state.result == LAP_RESULT_BATTERY);

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
