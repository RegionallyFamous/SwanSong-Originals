#include <string.h>

#include "model.h"

bool orbital_blocked(uint8_t x, uint8_t y) {
	if (x == 0 || x == 19 || y == 0 || y == 8) return true;
	if (y == 3 && x > 3 && x < 15 && x != 9) return true;
	if (x == 12 && y > 3 && y < 8 && y != 6) return true;
	return false;
}

void courier_reset(courier_state_t *state) {
	memset(state, 0, sizeof(*state));
	state->x = 2;
	state->y = 1;
	state->fuel = 40;
}

void courier_step(courier_state_t *state, const courier_input_t *input,
	courier_event_t *event) {
	memset(event, 0, sizeof(*event));
	if (state->result != COURIER_RESULT_PLAYING || (!input->dx && !input->dy)) return;
	{
		uint8_t nx = (uint8_t)(state->x + input->dx);
		uint8_t ny = (uint8_t)(state->y + input->dy);
		if (!orbital_blocked(nx, ny)) {
			state->x = nx;
			state->y = ny;
			++state->steps;
			if (state->fuel) --state->fuel;
			event->tone_hz = 330;
			event->tone_frames = 2;
		} else {
			event->tone_hz = 120;
			event->tone_frames = 4;
		}
	}
	if (state->x == 3 && state->y == 7) state->parcel = true;
	if (state->parcel && state->x == 17 && state->y == 1) {
		state->result = COURIER_RESULT_DELIVERED;
		event->tone_hz = 660;
		event->tone_frames = 12;
	} else if (state->fuel == 0) {
		state->result = COURIER_RESULT_FUEL;
	}
	event->dirty = true;
}
