#include <assert.h>
#include <string.h>

#include "model.h"

int main(void) {
	rotate_state_t state;
	rotate_state_t initial;
	rotate_input_t input = {0, 0, false, false, false};
	rotate_event_t event;
	uint8_t room;

	rotate_reset(&state);
	initial = state;
	assert(state.x == 1 && state.y == 1 && !state.vertical);
	assert(rotate_blocked(0, false, 0, 1));
	input.rotate = true;
	rotate_step(&state, &input, &event);
	assert(state.vertical && event.orientation_changed && event.tone_hz == 520);
	input.rotate = false;
	input.reset_room = true;
	rotate_step(&state, &input, &event);
	assert(!state.vertical && state.x == 1 && state.y == 1);

	input.reset_room = false;
	for (room = 0; room < 5; ++room) {
		assert(state.room == room);
		state.x = rotate_key_x(room);
		state.y = rotate_key_y(room);
		rotate_step(&state, &input, &event);
		assert(state.key);
		state.x = 10;
		state.y = 1;
		rotate_step(&state, &input, &event);
	}
	assert(state.result == ROTATE_RESULT_COMPLETE);
	input.replay = true;
	rotate_step(&state, &input, &event);
	assert(event.reset_session && memcmp(&state, &initial, sizeof(state)) == 0);
	return 0;
}
