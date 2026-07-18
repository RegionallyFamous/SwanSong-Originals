#include <assert.h>
#include <string.h>

#include "model.h"

static kaiju_state_t complete_observation(void) {
	kaiju_state_t state;
	kaiju_input_t input = {0, false, false, false, false};
	kaiju_event_t event;
	uint32_t tick;

	kaiju_reset(&state);
	state.camera = 10;
	state.kaiju = 15;
	input.photograph = true;
	kaiju_step(&state, &input, 1, &event);
	assert(state.evidence == 1 && state.disturbance == 24);

	input.photograph = false;
	input.hide = true;
	for (tick = 2; tick < 150; ++tick)
		kaiju_step(&state, &input, tick, &event);
	assert(state.disturbance == 0 && state.behavior == 0);

	input.hide = false;
	kaiju_step(&state, &input, 150, &event);
	assert(state.behavior == 1);
	input.photograph = true;
	kaiju_step(&state, &input, 151, &event);
	assert(state.evidence == 3);

	input.photograph = false;
	for (tick = 152; tick <= 300; ++tick)
		kaiju_step(&state, &input, tick, &event);
	assert(state.behavior == 2);
	state.camera = state.kaiju >= 3 ? (uint8_t)(state.kaiju - 3) :
		(uint8_t)(state.kaiju + 3);
	input.photograph = true;
	kaiju_step(&state, &input, 301, &event);
	assert(state.evidence == 7 && state.result == KAIJU_RESULT_COMPLETE);
	return state;
}

int main(void) {
	kaiju_state_t state;
	kaiju_state_t initial;
	kaiju_input_t input = {0, false, false, false, false};
	kaiju_event_t event;
	kaiju_state_t completed;
	kaiju_state_t repeated;
	uint16_t random_after;

	kaiju_reset(&state);
	initial = state;
	input.direction = 1;
	kaiju_step(&state, &input, 1, &event);
	assert(state.camera == 3 && event.dirty);
	assert(event.tone_hz == 110 && event.tone_frames == 3);

	assert(kaiju_camera_in_range(10, 15, false));
	assert(kaiju_camera_in_range(20, 15, false));
	assert(!kaiju_camera_in_range(14, 15, false));
	assert(kaiju_camera_in_range(5, 15, true));
	assert(!kaiju_camera_in_range(20, 15, true));

	completed = complete_observation();
	repeated = complete_observation();
	assert(memcmp(&completed, &repeated, sizeof(completed)) == 0);
	state = completed;

	input.replay = true;
	kaiju_step(&state, &input, 8, &event);
	assert(event.reset_session && memcmp(&state, &initial, sizeof(state)) == 0);

	memset(&input, 0, sizeof(input));
	kaiju_reset(&state);
	kaiju_step(&state, &input, 300, &event);
	random_after = state.random_state;
	kaiju_reset(&state);
	kaiju_step(&state, &input, 300, &event);
	assert(state.random_state == random_after);

	kaiju_reset(&state);
	state.disturbance = 96;
	state.camera = 0;
	state.kaiju = 15;
	input.photograph = true;
	kaiju_step(&state, &input, 1, &event);
	assert(state.result == KAIJU_RESULT_FAILED && state.disturbance == 100);
	return 0;
}
