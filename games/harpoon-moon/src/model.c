#include <string.h>

#include "model.h"

#define HARPOON_RANDOM_SEED 0x71D3u

static uint8_t clamp_u8(int16_t value, uint8_t low, uint8_t high) {
	if (value < low) return low;
	if (value > high) return high;
	return (uint8_t)value;
}

static uint16_t random_next(harpoon_state_t *state) {
	uint16_t value = state->random_state ? state->random_state : HARPOON_RANDOM_SEED;
	value ^= (uint16_t)(value << 7);
	value ^= (uint16_t)(value >> 9);
	value ^= (uint16_t)(value << 8);
	state->random_state = value;
	return value;
}

void harpoon_reset(harpoon_state_t *state) {
	memset(state, 0, sizeof(*state));
	state->skiff = 3;
	state->creature = 16;
	state->boss_hp = 3;
	state->oxygen = 1200;
	state->random_state = HARPOON_RANDOM_SEED;
}

void harpoon_step(harpoon_state_t *state, const harpoon_input_t *input,
	uint32_t session_tick, harpoon_event_t *event) {
	memset(event, 0, sizeof(*event));
	if (state->result != HARPOON_RESULT_PLAYING && input->replay) {
		harpoon_reset(state);
		event->reset_session = true;
		event->dirty = true;
		return;
	}
	if (state->result != HARPOON_RESULT_PLAYING) return;
	if (input->direction) {
		state->skiff = clamp_u8((int16_t)state->skiff + input->direction, 0, 20);
		event->dirty = true;
	}
	if (input->charge_held && state->charge < 20) {
		++state->charge;
		event->dirty = true;
	}
	if (input->lure_held && (session_tick & 3u) == 0) {
		if (state->creature < state->skiff) ++state->creature;
		else if (state->creature > state->skiff) --state->creature;
		event->dirty = true;
	}
	if (input->fire_released && state->charge) {
		uint8_t distance = state->skiff > state->creature ?
			state->skiff - state->creature : state->creature - state->skiff;
		uint8_t reach = (uint8_t)(state->charge / 2 + 2);
		if (distance <= reach) {
			if (state->tags < 3) ++state->tags;
			else if (state->boss_hp) --state->boss_hp;
			state->creature = (uint8_t)(4 + random_next(state) % 16);
			event->tone_hz = 620;
			event->tone_frames = 10;
		} else {
			state->oxygen = state->oxygen > 75 ? (uint16_t)(state->oxygen - 75) : 0;
			event->tone_hz = 120;
			event->tone_frames = 8;
		}
		state->charge = 0;
		event->dirty = true;
	}
	if (session_tick % 45 == 0) {
		int8_t drift = (random_next(state) & 1u) ? 1 : -1;
		state->creature = clamp_u8((int16_t)state->creature + drift, 1, 20);
		event->dirty = true;
	}
	if (state->oxygen) --state->oxygen;
	if (state->tags == 3 && state->boss_hp == 0) state->result = HARPOON_RESULT_WIN;
	else if (state->oxygen == 0) state->result = HARPOON_RESULT_OXYGEN;
}
