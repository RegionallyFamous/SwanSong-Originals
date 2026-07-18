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

	/* Narrow gate rejects a signal six units away. */
	state.frequency = (uint16_t)(radio_target_for(0) - 6u);
	input.lock = true;
	radio_step(&state, &input, &event);
	assert(state.clue == 0 && state.time == RADIO_NIGHT_FRAMES - 301u);
	assert(event.tone_hz == 110 && event.tone_frames == 8);

	/* Wide gate accepts it, but each successful wide lock costs one gain pip. */
	radio_reset(&state);
	state.gate = true;
	state.frequency = (uint16_t)(radio_target_for(0) - 6u);
	radio_step(&state, &input, &event);
	assert(state.clue == 1 && state.gain == 4 && event.tone_hz == 520);
	state.frequency = (uint16_t)(radio_target_for(1) - 6u);
	radio_step(&state, &input, &event);
	assert(state.clue == 1 && state.gain == 4);
	state.gain = 5;
	radio_step(&state, &input, &event);
	assert(state.clue == 2 && state.gain == 4 && event.tone_hz == 600);
	state.gain = 5;
	state.frequency = (uint16_t)(radio_target_for(2) - 6u);
	radio_step(&state, &input, &event);
	assert(state.result == RADIO_RESULT_SIGNAL && state.clue == 3);

	/* Exact narrow-gate locks remain the lower-cost path. */
	radio_reset(&state);
	state.frequency = radio_target_for(0);
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
	memset(&input, 0, sizeof(input));
	input.frequency_direction = 1;
	input.gain_direction = -1;
	input.toggle_gate = true;
	radio_step(&state, &input, &event);
	assert(state.frequency == 886 && state.gain == 6 && state.gate);
	assert(event.tone_frames == 3);
	return 0;
}
