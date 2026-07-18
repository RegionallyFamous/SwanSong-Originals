#include <string.h>

#include "model.h"

static uint8_t clamp_u8(int16_t value, uint8_t low, uint8_t high) {
	if (value < low) return low;
	if (value > high) return high;
	return (uint8_t)value;
}

static void tone(lap_event_t *event, uint16_t hz, uint8_t frames) {
	event->tone_hz = hz;
	event->tone_frames = frames;
}

uint8_t lap_rival_lane(uint8_t progress) {
	return (uint8_t)((progress / 25 + 1) % 3);
}

bool lap_rival_contact(uint8_t progress, uint8_t lane) {
	uint8_t phase = (uint8_t)(progress % 25);
	uint8_t first = lap_rival_lane(progress);
	uint8_t second = (uint8_t)((first + 2) % 3);
	return phase >= 8 && phase <= 14 && (lane == first || lane == second);
}

void lap_reset(lap_state_t *state) {
	memset(state, 0, sizeof(*state));
	state->lap = 1;
	state->battery = 120;
	state->lane = 1;
}

void lap_step(lap_state_t *state, const lap_input_t *input,
	uint32_t session_tick, lap_event_t *event) {
	memset(event, 0, sizeof(*event));
	if (state->result != LAP_RESULT_PLAYING) return;

	if (input->lane_direction) {
		state->lane = clamp_u8((int16_t)state->lane + input->lane_direction, 0, 2);
		event->dirty = true;
	}
	if (input->accelerate && session_tick % 10 == 0 && state->speed < 6 && state->battery) {
		++state->speed;
		event->dirty = true;
	}
	if (input->brake && session_tick % 6 == 0 && state->speed) {
		--state->speed;
		event->dirty = true;
	}
	if (input->accelerate && input->brake && session_tick % 6 == 0 &&
		state->battery) {
		--state->battery;
		event->dirty = true;
	}
	if (input->tow && !state->helped && state->lap == 2 &&
		state->progress >= 40 && state->progress <= 56) {
		state->helped = true;
		state->speed = 0;
		tone(event, 560, 12);
		event->dirty = true;
	}
	if ((session_tick & 7u) == 0 && state->speed) {
		uint16_t next = (uint16_t)state->progress + state->speed;
		if (next >= 100) {
			state->progress = (uint8_t)(next - 100);
			if (++state->lap > 3) {
				state->result = state->helped ? LAP_RESULT_COOPERATIVE : LAP_RESULT_SOLO;
				tone(event, state->helped ? 760 : 620, 12);
			}
		} else {
			state->progress = (uint8_t)next;
		}
		if (state->battery) --state->battery;
		event->dirty = true;
	}
	if (state->result == LAP_RESULT_PLAYING && !state->rival_zone &&
		lap_rival_contact(state->progress, state->lane)) {
		state->speed = state->speed > 2 ? (uint8_t)(state->speed - 2) : 1;
		state->battery = state->battery > 3 ? (uint8_t)(state->battery - 3) : 0;
		state->rival_zone = true;
		tone(event, 180, 6);
		event->dirty = true;
	}
	if ((state->progress % 25) < 8 || (state->progress % 25) > 14) {
		state->rival_zone = false;
	}
	if (state->result == LAP_RESULT_PLAYING && !state->crash_zone &&
		((state->progress >= 24 && state->progress <= 30 && state->lane == 1) ||
		 (state->progress >= 68 && state->progress <= 74 && state->lane == 2))) {
		state->speed = 1;
		state->battery = state->battery > 4 ? (uint8_t)(state->battery - 4) : 0;
		state->crash_zone = true;
		tone(event, 100, 8);
		event->dirty = true;
	}
	if (!((state->progress >= 20 && state->progress <= 34) ||
		  (state->progress >= 64 && state->progress <= 78))) {
		state->crash_zone = false;
	}
	if (state->result == LAP_RESULT_PLAYING && state->battery == 0) {
		state->speed = 0;
		state->result = LAP_RESULT_BATTERY;
		event->dirty = true;
	}
}
