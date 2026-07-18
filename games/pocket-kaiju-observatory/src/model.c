#include <string.h>

#include "model.h"

#define KAIJU_RANDOM_SEED 0x71D3u

static uint8_t clamp_u8(int16_t value, uint8_t low, uint8_t high) {
	if (value < low) return low;
	if (value > high) return high;
	return (uint8_t)value;
}

static uint16_t random_next(kaiju_state_t *state) {
	uint16_t value = state->random_state ? state->random_state : KAIJU_RANDOM_SEED;
	value ^= (uint16_t)(value << 7);
	value ^= (uint16_t)(value >> 9);
	value ^= (uint16_t)(value << 8);
	state->random_state = value;
	return value;
}

void kaiju_reset(kaiju_state_t *state) {
	memset(state, 0, sizeof(*state));
	state->camera = 2;
	state->kaiju = 15;
	state->sun = 1800;
	state->random_state = KAIJU_RANDOM_SEED;
}

bool kaiju_camera_in_range(uint8_t camera, uint8_t kaiju, bool zoom) {
	uint8_t distance = camera > kaiju ? camera - kaiju : kaiju - camera;
	uint8_t low = zoom ? 6 : 3;
	uint8_t high = zoom ? 10 : 7;
	return distance >= low && distance <= high;
}

void kaiju_step(kaiju_state_t *state, const kaiju_input_t *input,
	uint32_t session_tick, kaiju_event_t *event) {
	memset(event, 0, sizeof(*event));
	if (state->result != KAIJU_RESULT_PLAYING && input->replay) {
		kaiju_reset(state);
		event->reset_session = true;
		event->dirty = true;
		return;
	}
	if (state->result != KAIJU_RESULT_PLAYING) return;
	if (input->direction) {
		state->camera = clamp_u8((int16_t)state->camera + input->direction, 0, 20);
		event->tone_hz = input->direction > 0 ? 110 : 92;
		event->tone_frames = 3;
		event->dirty = true;
	}
	if (input->toggle_zoom) {
		state->zoom = !state->zoom;
		event->dirty = true;
	}
	if (input->hide && state->disturbance && (session_tick & 3u) == 0) {
		--state->disturbance;
		event->dirty = true;
	}
	if (input->photograph) {
		state->disturbance = clamp_u8((int16_t)state->disturbance + 24, 0, 100);
		if (kaiju_camera_in_range(state->camera, state->kaiju, state->zoom)) {
			state->evidence |= (uint8_t)(1u << state->behavior);
			event->tone_hz = 700;
			event->tone_frames = 8;
		} else {
			event->tone_hz = 140;
			event->tone_frames = 6;
		}
		event->dirty = true;
	}
	if (session_tick % 150 == 0) {
		state->behavior = (uint8_t)((state->behavior + 1) % 3);
		event->dirty = true;
	}
	if (state->behavior == 2 && session_tick % 20 == 0) {
		state->kaiju = (uint8_t)(8 + random_next(state) % 12);
		event->dirty = true;
	}
	if (state->sun) --state->sun;
	if ((state->evidence & 7u) == 7u) state->result = KAIJU_RESULT_COMPLETE;
	else if (state->disturbance >= 100 || state->sun == 0) state->result = KAIJU_RESULT_FAILED;
}
