#include <assert.h>
#include <string.h>

#include "model.h"

static void press(turncoat_state_t *state, char action, turncoat_event_t *event) {
	turncoat_input_t input = {0, 0, false, false, false, false};
	if (action == 'U') input.dy = -1;
	else if (action == 'D') input.dy = 1;
	else if (action == 'L') input.dx = -1;
	else if (action == 'R') input.dx = 1;
	else if (action == 'A') input.select_or_act = true;
	else if (action == 'B') input.recruit = true;
	else if (action == 'T') input.end_turn = true;
	else assert(0 && "unknown tactics action");
	turncoat_step(state, &input, event);
}

int main(void) {
	static const char canonical_win_route[] = "UARARAUAALLDDDADAUUARABADA";
	turncoat_state_t state;
	turncoat_state_t initial;
	turncoat_input_t input = {0, 0, false, false, false, false};
	turncoat_event_t event;
	uint8_t i;
	bool recruited_before_win = false;

	turncoat_reset(&state);
	initial = state;
	assert(state.turns == 18 && turncoat_enemy_count(&state) == 4);
	input.recruit = true;
	turncoat_step(&state, &input, &event);
	assert(state.turns == 18 && state.recruits == 0);
	assert(event.tone_hz == 120 && event.tone_frames == 3);
	input.recruit = false;
	input.dx = 1;
	turncoat_step(&state, &input, &event);
	assert(state.cursor_x == 1 && event.dirty);

	turncoat_reset(&state);
	for (i = 0; canonical_win_route[i]; ++i) {
		press(&state, canonical_win_route[i], &event);
		if (state.recruits) {
			recruited_before_win = true;
			assert(state.result == TURNCOAT_RESULT_PLAYING ||
				state.result == TURNCOAT_RESULT_WIN);
		}
	}
	assert(recruited_before_win && state.recruits == 1);
	assert(state.result == TURNCOAT_RESULT_WIN);
	assert(turncoat_enemy_count(&state) == 0);
	assert(event.tone_hz == 760 && event.tone_frames == 12);
	input = (turncoat_input_t){0, 0, false, false, false, true};
	turncoat_step(&state, &input, &event);
	assert(event.reset_session && memcmp(&state, &initial, sizeof(state)) == 0);

	turncoat_reset(&state);
	state.turns = 1;
	memset(&input, 0, sizeof(input));
	input.end_turn = true;
	turncoat_step(&state, &input, &event);
	assert(state.result == TURNCOAT_RESULT_LOSS);
	assert(event.tone_hz == 80 && event.tone_frames == 12);

	turncoat_reset(&state);
	state.allies[1].x = 7;
	state.allies[1].y = 2;
	assert(turncoat_beacon_secured(&state));
	return 0;
}
