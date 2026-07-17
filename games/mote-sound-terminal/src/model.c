#include <string.h>

#include "model.h"

static uint8_t clamp_u8(int16_t value, uint8_t low, uint8_t high) {
	if (value < low) return low;
	if (value > high) return high;
	return (uint8_t)value;
}

uint16_t mote_note_hz(uint8_t track, uint8_t step) {
	static const uint16_t scale[8] = {196, 220, 247, 262, 294, 330, 392, 440};
	uint8_t index = (uint8_t)((step * (track + 1) + track * 2) & 7);
	return (uint16_t)(scale[index] + track * 24);
}

void mote_reset(mote_state_t *state) {
	memset(state, 0, sizeof(*state));
	state->tempo = 12;
	state->playing = true;
}

void mote_step(mote_state_t *state, const mote_input_t *input,
	mote_event_t *event) {
	memset(event, 0, sizeof(*event));
	if (input->track_direction) {
		state->track = (uint8_t)((state->track +
			(input->track_direction > 0 ? 1 : 2)) % 3);
		state->step = 0;
		event->dirty = true;
	}
	if (input->tempo_direction) {
		state->tempo = clamp_u8((int16_t)state->tempo +
			(input->tempo_direction > 0 ? 1 : -1), 5, 20);
		event->dirty = true;
	}
	if (input->toggle_play) {
		state->playing = !state->playing;
		if (!state->playing) event->sound_off = true;
		event->dirty = true;
	}
	if (input->toggle_scope) {
		state->scope ^= 1;
		event->dirty = true;
	}
	if (input->reset) {
		mote_reset(state);
		event->reset_session = true;
		event->dirty = true;
	}
	if (state->playing && ++state->tick >= state->tempo) {
		state->tick = 0;
		state->step = (uint8_t)((state->step + 1) & 15);
		event->tone_hz = mote_note_hz(state->track, state->step);
		event->tone_volume = 7;
		event->dirty = true;
	}
}
