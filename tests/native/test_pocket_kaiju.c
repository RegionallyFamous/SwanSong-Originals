#include <assert.h>
#include <string.h>

#include "model.h"

int main(void) {
	kaiju_state_t state;
	kaiju_state_t initial;
	kaiju_input_t input = {0, false, false, false, false};
	kaiju_event_t event;
	uint8_t behavior;
	uint16_t random_after;

	kaiju_reset(&state);
	initial = state;
	input.direction = 1;
	kaiju_step(&state, &input, 1, &event);
	assert(state.camera == 3 && event.dirty);
	assert(event.tone_hz == 110 && event.tone_frames == 3);

	input.direction = 0;
	input.photograph = true;
	for (behavior = 0; behavior < 3; ++behavior) {
		state.behavior = behavior;
		state.camera = 10;
		state.kaiju = 15;
		kaiju_step(&state, &input, (uint32_t)behavior + 1, &event);
		assert(event.tone_hz == 700);
	}
	assert(state.result == KAIJU_RESULT_COMPLETE && state.evidence == 7);

	input.replay = true;
	kaiju_step(&state, &input, 8, &event);
	assert(event.reset_session && memcmp(&state, &initial, sizeof(state)) == 0);

	state.behavior = 2;
	memset(&input, 0, sizeof(input));
	kaiju_step(&state, &input, 20, &event);
	random_after = state.random_state;
	kaiju_reset(&state);
	state.behavior = 2;
	kaiju_step(&state, &input, 20, &event);
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
