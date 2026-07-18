#include <assert.h>
#include <string.h>

#include "model.h"

static void confirm(scrapframe_state_t *state, scrapframe_event_t *event) {
	const scrapframe_input_t input = {0, true};
	scrapframe_step(state, &input, event);
}

int main(void) {
	scrapframe_state_t state;
	scrapframe_state_t initial;
	scrapframe_event_t event;
	scrapframe_input_t input = {0, false};

	scrapframe_reset(&state);
	initial = state;
	confirm(&state, &event);
	assert(state.last_ok && state.score == 1 && event.tone_hz == 660);
	confirm(&state, &event);
	input.selection_direction = 1;
	scrapframe_step(&state, &input, &event);
	assert(event.tone_hz == 110 && event.tone_frames == 3);
	confirm(&state, &event);
	assert(state.last_ok && state.score == 2);
	confirm(&state, &event);
	input.selection_direction = 1;
	scrapframe_step(&state, &input, &event);
	scrapframe_step(&state, &input, &event);
	confirm(&state, &event);
	assert(state.last_ok && state.score == 3);
	confirm(&state, &event);
	assert(state.phase == 2 && event.tone_hz == 880 && event.tone_frames == 18);
	confirm(&state, &event);
	assert(event.reset_session && memcmp(&state, &initial, sizeof(state)) == 0);

	scrapframe_reset(&state);
	input.selection_direction = 1;
	input.confirm = true;
	scrapframe_step(&state, &input, &event);
	assert(!state.last_ok && state.phase == 1 && state.job == 0 &&
		event.tone_hz == 130);
	input.selection_direction = 0;
	confirm(&state, &event);
	assert(state.phase == 0 && state.job == 0 && state.selected == 0 &&
		state.score == 0);
	confirm(&state, &event);
	assert(state.last_ok && state.phase == 1 && state.job == 0 &&
		state.score == 1);
	return 0;
}
