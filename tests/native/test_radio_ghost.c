#include <assert.h>
#include <string.h>

#include "model.h"

int main(void) {
	radio_state_t state;
	radio_state_t initial;
	radio_input_t input = {0, 0, false, false, false};
	radio_event_t event;

	radio_reset(&state);
	initial = state;
	assert(state.frequency == 880 && state.time == RADIO_NIGHT_FRAMES && state.gain == 5);
	state.frequency = radio_target_for(0);
	input.lock = true;
	radio_step(&state, &input, &event);
	assert(state.clue == 1 && event.tone_hz == 520);
	state.frequency = 994;
	radio_step(&state, &input, &event);
	assert(state.clue == 2);
	state.frequency = radio_target_for(2);
	radio_step(&state, &input, &event);
	assert(state.result == RADIO_RESULT_SIGNAL && state.clue == 3);

	input.replay = true;
	radio_step(&state, &input, &event);
	assert(event.reset_session && memcmp(&state, &initial, sizeof(state)) == 0);

	radio_reset(&state);
	state.time = 0;
	memset(&input, 0, sizeof(input));
	radio_step(&state, &input, &event);
	assert(state.result == RADIO_RESULT_DAWN);

	radio_reset(&state);
	input.frequency_direction = 1;
	input.gain_direction = -1;
	input.toggle_gate = true;
	radio_step(&state, &input, &event);
	assert(state.frequency == 886 && state.gain == 6 && state.gate);
	assert(event.tone_frames == 3);
	return 0;
}
