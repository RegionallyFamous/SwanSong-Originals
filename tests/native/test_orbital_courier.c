#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "model.h"

static void move(courier_state_t *state, int8_t dx, int8_t dy) {
	courier_input_t input = {dx, dy};
	courier_event_t event;
	courier_step(state, &input, &event);
	assert(event.dirty);
}

int main(void) {
	static const courier_input_t pickup[] = {
		{1, 0}, {0, 1}, {0, 1}, {0, 1}, {0, 1}, {0, 1}, {0, 1}
	};
	static const courier_input_t delivery[] = {
		{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0},
		{0, -1}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0},
		{0, -1}, {0, -1}, {0, -1}, {0, -1}, {0, -1}
	};
	courier_state_t state;
	courier_state_t reset_copy;
	courier_event_t event;
	size_t i;

	courier_reset(&state);
	reset_copy = state;
	assert(state.x == 2 && state.y == 1 && state.fuel == 40);
	move(&state, 0, -1);
	assert(state.x == 2 && state.y == 1 && state.fuel == 40 && state.steps == 0);
	assert(orbital_blocked(9, 0) && !orbital_blocked(9, 3));

	for (i = 0; i < sizeof(pickup) / sizeof(pickup[0]); ++i) {
		courier_step(&state, &pickup[i], &event);
	}
	assert(state.parcel && state.x == 3 && state.y == 7);
	for (i = 0; i < sizeof(delivery) / sizeof(delivery[0]); ++i) {
		courier_step(&state, &delivery[i], &event);
	}
	assert(state.result == COURIER_RESULT_DELIVERED);
	assert(state.steps == 27 && state.fuel == 13);

	courier_reset(&state);
	for (i = 0; i < 40; ++i) {
		move(&state, state.x == 2 ? -1 : 1, 0);
	}
	assert(state.result == COURIER_RESULT_FUEL && state.fuel == 0);
	courier_reset(&state);
	assert(memcmp(&state, &reset_copy, sizeof(state)) == 0);
	return 0;
}
