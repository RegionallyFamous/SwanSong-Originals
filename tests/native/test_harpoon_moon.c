#include <assert.h>
#include <string.h>

#include "model.h"

static harpoon_state_t complete_hunt(void) {
	harpoon_state_t state;
	harpoon_input_t input = {0, false, false, true, false};
	harpoon_event_t event;
	uint8_t hit;

	harpoon_reset(&state);
	for (hit = 0; hit < 6; ++hit) {
		state.creature = state.skiff;
		state.charge = 20;
		harpoon_step(&state, &input, (uint32_t)hit + 2, &event);
		assert(event.tone_hz == 620 && event.tone_frames == 10);
		if (hit < 3) {
			assert(state.tags == (uint8_t)(hit + 1));
			assert(state.boss_hp == 3);
		} else {
			assert(state.tags == 3);
			assert(state.boss_hp == (uint8_t)(5 - hit));
		}
	}
	assert(state.result == HARPOON_RESULT_WIN);
	return state;
}

int main(void) {
	harpoon_state_t state;
	harpoon_state_t initial;
	harpoon_input_t input = {0, false, false, false, false};
	harpoon_event_t event;
	harpoon_state_t completed;
	harpoon_state_t repeated;
	uint16_t first_random;

	harpoon_reset(&state);
	initial = state;
	assert(state.skiff == 3 && state.creature == 16 && state.oxygen == 1200);
	input.direction = 1;
	harpoon_step(&state, &input, 1, &event);
	assert(state.skiff == 4 && event.dirty);

	completed = complete_hunt();
	repeated = complete_hunt();
	assert(memcmp(&completed, &repeated, sizeof(completed)) == 0);
	state = completed;
	first_random = completed.random_state;

	input.replay = true;
	harpoon_step(&state, &input, 10, &event);
	assert(event.reset_session && memcmp(&state, &initial, sizeof(state)) == 0);
	state.creature = state.skiff;
	state.charge = 1;
	input.replay = false;
	input.fire_released = true;
	harpoon_step(&state, &input, 2, &event);
	assert(state.random_state != 0 && state.random_state != first_random);

	harpoon_reset(&state);
	state.oxygen = 1;
	memset(&input, 0, sizeof(input));
	harpoon_step(&state, &input, 1, &event);
	assert(state.result == HARPOON_RESULT_OXYGEN);
	return 0;
}
