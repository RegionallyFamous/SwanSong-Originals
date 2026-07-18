#include <string.h>

#include "model.h"

bool rotate_blocked(uint8_t room, bool vertical, uint8_t x, uint8_t y) {
	uint8_t gap;
	if (x == 0 || x == 11 || y == 0 || y == 7) return true;
	if (!vertical) {
		gap = (uint8_t)(1 + room % 6);
		return x == (uint8_t)(3 + room % 5) && y != gap;
	}
	gap = (uint8_t)(1 + (room * 2) % 10);
	return y == (uint8_t)(2 + room % 4) && x != gap;
}

uint8_t rotate_key_x(uint8_t room) { return (uint8_t)(2 + room); }
uint8_t rotate_key_y(uint8_t room) { return (uint8_t)(6 - (room & 1)); }

void rotate_reset(rotate_state_t *state) {
	memset(state, 0, sizeof(*state));
	state->x = 1;
	state->y = 1;
}

void rotate_step(rotate_state_t *state, const rotate_input_t *input,
	rotate_event_t *event) {
	memset(event, 0, sizeof(*event));
	if (state->result != ROTATE_RESULT_PLAYING && input->replay) {
		rotate_reset(state);
		event->orientation_changed = true;
		event->vertical = false;
		event->reset_session = true;
		event->dirty = true;
		return;
	}
	if (state->result != ROTATE_RESULT_PLAYING) return;
	if (input->rotate) {
		state->vertical = !state->vertical;
		if (rotate_blocked(state->room, state->vertical, state->x, state->y)) {
			state->x = 1;
			state->y = 1;
		}
		event->tone_hz = state->vertical ? 520 : 360;
		event->tone_frames = 6;
		event->orientation_changed = true;
		event->vertical = state->vertical;
		event->dirty = true;
	}
	if (input->reset_room) {
		state->x = 1;
		state->y = 1;
		state->key = false;
		state->vertical = false;
		event->orientation_changed = true;
		event->vertical = false;
		event->dirty = true;
	}
	if (input->dx || input->dy) {
		uint8_t nx = (uint8_t)(state->x + input->dx);
		uint8_t ny = (uint8_t)(state->y + input->dy);
		if (!rotate_blocked(state->room, state->vertical, nx, ny)) {
			state->x = nx;
			state->y = ny;
		} else {
			event->tone_hz = 100;
			event->tone_frames = 3;
		}
		event->dirty = true;
	}
	if (state->x == rotate_key_x(state->room) &&
		state->y == rotate_key_y(state->room)) state->key = true;
	if (state->key && state->x == 10 && state->y == 1) {
		if (++state->room >= 5) {
			state->result = ROTATE_RESULT_COMPLETE;
			event->tone_hz = 760;
			event->tone_frames = 12;
		} else {
			event->tone_hz = 640;
			event->tone_frames = 8;
			state->x = 1;
			state->y = 1;
			state->key = false;
		}
		event->dirty = true;
	}
}
