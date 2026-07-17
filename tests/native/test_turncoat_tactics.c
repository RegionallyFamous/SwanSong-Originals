#include <assert.h>
#include <string.h>

#include "model.h"

int main(void) {
	turncoat_state_t state;
	turncoat_state_t initial;
	turncoat_input_t input = {0, 0, false, false, false, false};
	turncoat_event_t event;
	uint8_t i;

	turncoat_reset(&state);
	initial = state;
	assert(state.turns == 18 && turncoat_enemy_count(&state) == 4);
	input.dx = 1;
	turncoat_step(&state, &input, &event);
	assert(state.cursor_x == 1 && event.dirty);

	turncoat_reset(&state);
	state.allies[0].x = 3;
	state.allies[0].y = 4;
	state.cursor_x = 4;
	state.cursor_y = 4;
	input.dx = 0;
	input.recruit = true;
	turncoat_step(&state, &input, &event);
	assert(state.recruits == 1 && state.allies[3].hp == 1);
	assert(event.tone_hz == 580 && event.tone_frames == 10);

	turncoat_reset(&state);
	for (i = 0; i < ENEMY_CAPACITY; ++i) state.enemies[i].hp = 0;
	memset(&input, 0, sizeof(input));
	turncoat_step(&state, &input, &event);
	assert(state.result == TURNCOAT_RESULT_WIN);
	input.replay = true;
	turncoat_step(&state, &input, &event);
	assert(event.reset_session && memcmp(&state, &initial, sizeof(state)) == 0);

	turncoat_reset(&state);
	state.turns = 1;
	memset(&input, 0, sizeof(input));
	input.end_turn = true;
	turncoat_step(&state, &input, &event);
	assert(state.result == TURNCOAT_RESULT_LOSS);

	turncoat_reset(&state);
	state.allies[1].x = 7;
	state.allies[1].y = 2;
	assert(turncoat_beacon_secured(&state));
	return 0;
}
