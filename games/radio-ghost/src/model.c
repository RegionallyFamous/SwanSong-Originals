#include <string.h>

#include "model.h"

static uint8_t clamp_u8(int16_t value, uint8_t low, uint8_t high) {
	if (value < low) return low;
	if (value > high) return high;
	return (uint8_t)value;
}

uint16_t radio_target_for(uint8_t clue) {
	if (clue == 0) return 934;
	if (clue == 1) return 995;
	return 1042;
}

void radio_reset(radio_state_t *state) {
	memset(state, 0, sizeof(*state));
	state->frequency = 880;
	state->time = RADIO_NIGHT_FRAMES;
	state->gain = 5;
}

void radio_step(radio_state_t *state, const radio_input_t *input,
	radio_event_t *event) {
	memset(event, 0, sizeof(*event));
	if (state->result != RADIO_RESULT_PLAYING && input->replay) {
		radio_reset(state);
		event->reset_session = true;
		event->dirty = true;
		return;
	}
	if (state->result != RADIO_RESULT_PLAYING) return;
	if (input->frequency_direction) {
		int16_t frequency = (int16_t)state->frequency +
			(int16_t)input->frequency_direction * 6;
		if (frequency < 880) frequency = 880;
		if (frequency > 1080) frequency = 1080;
		state->frequency = (uint16_t)frequency;
		event->tone_hz = 110;
		event->tone_frames = 3;
		event->dirty = true;
	}
	if (input->gain_direction) {
		state->gain = clamp_u8((int16_t)state->gain - input->gain_direction, 0, 9);
		event->tone_hz = 110;
		event->tone_frames = 3;
		event->dirty = true;
	}
	if (input->toggle_gate) {
		state->gate = !state->gate;
		event->tone_hz = 110;
		event->tone_frames = 3;
		event->dirty = true;
	}
	if (input->lock) {
		uint16_t target = radio_target_for(state->clue);
		uint16_t distance = state->frequency > target ?
			state->frequency - target : target - state->frequency;
		if (distance <= 3 && state->gain >= 3) {
			++state->clue;
			event->tone_hz = (uint16_t)(440 + state->clue * 80);
			event->tone_frames = 12;
			if (state->clue == 3) state->result = RADIO_RESULT_SIGNAL;
		} else {
			state->time = state->time > 300 ? (uint16_t)(state->time - 300) : 0;
			event->tone_hz = 110;
			event->tone_frames = 8;
		}
		event->dirty = true;
	}
	if (state->time) --state->time;
	else state->result = RADIO_RESULT_DAWN;
}
