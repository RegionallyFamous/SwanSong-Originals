#include <assert.h>
#include <string.h>

#include "model.h"

static harpoon_state_t complete_hunt(void) {
	harpoon_state_t state;
	harpoon_input_t input = {0};
	harpoon_event_t event;
	uint8_t hit;

	harpoon_reset(&state);
	input.fire_released = true;
	for (hit = 0; hit < 6; ++hit) {
		state.creature = state.skiff;
		state.creature_depth = 0;
		state.lure_window = 2;
		state.charge = harpoon_min_charge(&state);
		harpoon_step(&state, &input, (uint32_t)hit + 2u, &event);
		assert(event.dirty);
		if (hit < 3) {
			assert(state.tags == (uint8_t)(hit + 1u));
			assert(state.boss_hp == HARPOON_BOSS_HEALTH);
		} else {
			assert(state.tags == HARPOON_SCOUT_COUNT);
			assert(state.boss_hp == (uint8_t)(5u - hit));
		}
	}
	assert(state.result == HARPOON_RESULT_WIN);
	assert(state.phase == HARPOON_PHASE_RESULT);
	assert(state.rank != 0u);
	return state;
}

int main(void) {
	harpoon_state_t state;
	harpoon_state_t initial;
	harpoon_input_t input = {0};
	harpoon_event_t event;
	harpoon_state_t completed;
	harpoon_state_t repeated;
	uint32_t paused_hash;

	harpoon_reset(&state);
	initial = state;
	assert(state.skiff == 3u && state.creature == 11u);
	assert(state.oxygen == HARPOON_START_OXYGEN);
	assert(state.tutorial == HARPOON_TUTORIAL_MOVE);
	input.direction = 1;
	harpoon_step(&state, &input, 1u, &event);
	assert(state.skiff == 4u && event.dirty);
	assert(state.tutorial == HARPOON_TUTORIAL_LURE);

	completed = complete_hunt();
	repeated = complete_hunt();
	assert(memcmp(&completed, &repeated, sizeof(completed)) == 0);
	assert(harpoon_state_hash(&completed) == harpoon_state_hash(&repeated));

	state = completed;
	memset(&input, 0, sizeof(input));
	input.retry = true;
	harpoon_step(&state, &input, 10u, &event);
	assert(event.reset_session);
	assert(memcmp(&state, &initial, sizeof(state)) == 0);

	harpoon_reset(&state);
	state.phase = HARPOON_PHASE_HUNT;
	state.resume_phase = HARPOON_PHASE_HUNT;
	state.tutorial = HARPOON_TUTORIAL_COMPLETE;
	memset(&input, 0, sizeof(input));
	input.pause = true;
	harpoon_step(&state, &input, 20u, &event);
	assert(state.phase == HARPOON_PHASE_PAUSED);
	paused_hash = harpoon_state_hash(&state);
	input.pause = false;
	harpoon_step(&state, &input, 21u, &event);
	assert(harpoon_state_hash(&state) == paused_hash);
	input.pause = true;
	harpoon_step(&state, &input, 22u, &event);
	assert(state.phase == HARPOON_PHASE_HUNT);

	harpoon_reset(&state);
	state.phase = HARPOON_PHASE_HUNT;
	state.tutorial = HARPOON_TUTORIAL_COMPLETE;
	state.oxygen = 1u;
	memset(&input, 0, sizeof(input));
	harpoon_step(&state, &input, 1u, &event);
	assert(state.result == HARPOON_RESULT_OXYGEN);
	assert(state.phase == HARPOON_PHASE_RESULT);
	assert(event.sfx == HARPOON_SFX_FAILURE);
	return 0;
}
