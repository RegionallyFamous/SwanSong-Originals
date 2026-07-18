#include <assert.h>
#include <string.h>

#include "model.h"

int main(void) {
	mote_state_t state;
	mote_state_t reset_copy;
	mote_input_t input = {0, 0, false, false, false};
	mote_event_t event;
	uint8_t i;

	mote_reset(&state);
	reset_copy = state;
	assert(state.track == 0 && state.tempo == 12 && state.playing);
	input.track_direction = -1;
	mote_step(&state, &input, &event);
	assert(state.track == 2);
	mote_reset(&state);
	memset(&input, 0, sizeof(input));
	input.track_direction = 1;
	input.tempo_direction = 1;
	input.toggle_scope = true;
	mote_step(&state, &input, &event);
	assert(state.track == 1 && state.tempo == 13 && state.scope == 1);
	assert(event.dirty && state.tick == 1 && state.step == 0);

	mote_reset(&state);
	memset(&input, 0, sizeof(input));
	input.tempo_direction = -1;
	for (i = 0; i < 12; ++i) mote_step(&state, &input, &event);
	assert(state.tempo == 5);
	input.tempo_direction = 1;
	for (i = 0; i < 20; ++i) mote_step(&state, &input, &event);
	assert(state.tempo == 20);

	mote_reset(&state);
	memset(&input, 0, sizeof(input));
	input.toggle_play = true;
	mote_step(&state, &input, &event);
	assert(!state.playing && event.sound_off);
	mote_step(&state, &input, &event);
	assert(state.playing && state.tick == 1);

	/* Pause preserves the partial beat; resume continues from that phase. */
	mote_reset(&state);
	state.step = 3;
	state.tick = 5;
	memset(&input, 0, sizeof(input));
	input.toggle_play = true;
	mote_step(&state, &input, &event);
	assert(!state.playing && state.step == 3 && state.tick == 5);
	memset(&input, 0, sizeof(input));
	for (i = 0; i < 10; ++i) mote_step(&state, &input, &event);
	assert(state.step == 3 && state.tick == 5 && !event.play_note);
	input.toggle_play = true;
	mote_step(&state, &input, &event);
	assert(state.playing && state.step == 3 && state.tick == 6);
	memset(&input, 0, sizeof(input));
	for (i = 0; i < 6; ++i) mote_step(&state, &input, &event);
	assert(state.step == 4 && state.tick == 0 && event.play_note);

	mote_reset(&state);
	state.track = 1;
	state.tempo = 13;
	memset(&input, 0, sizeof(input));
	for (i = 0; i < state.tempo; ++i) mote_step(&state, &input, &event);
	assert(state.step == 1 && state.tick == 0 && event.dirty && event.play_note);

	input.reset = true;
	mote_step(&state, &input, &event);
	assert(event.reset_session && !event.play_note);
	assert(state.tick == 0);
	assert(memcmp(&state, &reset_copy, sizeof(state)) == 0);
	return 0;
}
