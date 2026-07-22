#include <assert.h>
#include <string.h>

#include "model.h"

static radio_event_t step(radio_state_t *state, radio_input_t input) {
	radio_event_t event;
	radio_step(state, &input, &event);
	return event;
}

static void lock_exact(radio_state_t *state) {
	radio_input_t input = {0};
	state->frequency = radio_target_for(state->clue);
	state->gain = radio_ideal_gain_for(state->clue);
	input.lock = true;
	(void)step(state, input);
}

static void test_title_and_tutorial(void) {
	radio_state_t state;
	radio_input_t input = {0};
	radio_event_t event;
	uint16_t tick;

	radio_reset_title(&state);
	assert(state.mode == RADIO_MODE_TITLE && state.title_choice == 0u);
	for (tick = 0u; tick < RADIO_TITLE_ATTRACT_FRAMES; ++tick)
		event = step(&state, input);
	assert(state.attract && event.dirty);
	input.menu_direction = 1;
	event = step(&state, input);
	assert(state.title_choice == 1u && event.sfx == RADIO_SFX_TUNE_RIGHT);
	memset(&input, 0, sizeof(input));
	input.confirm = true;
	event = step(&state, input);
	assert(event.start_night && !event.start_tutorial);

	radio_reset_tutorial(&state);
	assert(state.frequency == 928u && state.gain == 3u);
	input = (radio_input_t){.frequency_direction = 1};
	event = step(&state, input);
	assert(state.frequency == 934u &&
		state.tutorial_step == RADIO_TUTORIAL_GAIN);
	input = (radio_input_t){.gain_direction = 1};
	(void)step(&state, input);
	assert(state.gain == 4u && state.tutorial_step == RADIO_TUTORIAL_GATE);
	input = (radio_input_t){.toggle_gate = true};
	(void)step(&state, input);
	assert(state.gate && state.tutorial_step == RADIO_TUTORIAL_LOCK);
	input = (radio_input_t){.lock = true};
	event = step(&state, input);
	assert(state.tutorial_step == RADIO_TUTORIAL_READY &&
		event.sfx == RADIO_SFX_LOCK);
	event = step(&state, input);
	assert(event.start_night && event.sfx == RADIO_SFX_CONFIRM);
}

static void test_gain_noise_gate_and_directions(void) {
	radio_state_t state;
	radio_input_t input = {0};
	radio_event_t event;
	uint8_t narrow_noise;

	radio_reset_night(&state);
	assert(radio_target_direction(&state) == -1);
	state.frequency = (uint16_t)(radio_target_for(0) - 6u);
	state.gain = radio_ideal_gain_for(0);
	narrow_noise = radio_noise_level(&state);
	input.lock = true;
	event = step(&state, input);
	assert(state.clue == 0u && state.wrong_locks == 1u &&
		event.sfx == RADIO_SFX_ERROR);

	radio_reset_night(&state);
	state.gate = true;
	state.frequency = (uint16_t)(radio_target_for(0) - 6u);
	state.gain = (uint8_t)(radio_ideal_gain_for(0) + 2u);
	assert(radio_noise_level(&state) > narrow_noise);
	event = step(&state, input);
	assert(state.clue == 1u && state.gain == 5u &&
		event.sfx == RADIO_SFX_LOCK);
	assert(radio_target_direction(&state) == 1);

	/* Excess gain amplifies noise enough to reject an otherwise exact target. */
	state.frequency = radio_target_for(1);
	state.gain = 9u;
	event = step(&state, input);
	assert(state.clue == 1u && event.sfx == RADIO_SFX_ERROR);

	state.gate = false;
	lock_exact(&state);
	assert(state.clue == 2u && radio_target_direction(&state) == -1);
	lock_exact(&state);
	assert(state.result == RADIO_RESULT_SIGNAL && state.clue == 3u);
	assert(state.score > 0u && state.narrow_locks == 2u);
}

static void test_pause_failure_and_reset(void) {
	radio_state_t state;
	radio_input_t input = {0};
	radio_event_t event;
	uint16_t frozen;
	uint8_t attempts = 0u;

	radio_reset_night(&state);
	input.pause = true;
	event = step(&state, input);
	assert(state.paused && event.pause_changed && event.sfx == RADIO_SFX_PAUSE);
	frozen = state.time;
	input = (radio_input_t){0};
	(void)step(&state, input);
	assert(state.time == frozen);
	input.pause = true;
	(void)step(&state, input);
	assert(!state.paused && state.time == frozen);
	input = (radio_input_t){0};
	(void)step(&state, input);
	assert(state.time == (uint16_t)(frozen - 1u));

	radio_reset_night(&state);
	input = (radio_input_t){.lock = true};
	while (state.result == RADIO_RESULT_PLAYING) {
		event = step(&state, input);
		++attempts;
		assert(attempts < 20u);
	}
	assert(state.result == RADIO_RESULT_DAWN && state.time == 0u);
	assert(event.sfx == RADIO_SFX_FAILURE && event.show_result);
	assert(state.wrong_locks == 12u && state.score == 0u);

	radio_reset_night(&state);
	assert(state.frequency == 950u && state.time == RADIO_NIGHT_FRAMES &&
		state.gain == 4u && !state.gate && state.clue == 0u &&
		state.result == RADIO_RESULT_PLAYING);
}

static void test_canonical_hash_inventory(void) {
	radio_state_t state;
	radio_state_t changed;
	uint32_t original;

	radio_reset_night(&state);
	state.score = 321u;
	state.title_ticks = 77u;
	state.clue = 1u;
	state.title_choice = 1u;
	state.tutorial_step = RADIO_TUTORIAL_GATE;
	state.narrow_locks = 1u;
	state.wrong_locks = 2u;
	state.feedback_flash = 3u;
	state.lock_quality = 88u;
	state.last_direction = -1;
	state.gate = true;
	state.paused = true;
	state.attract = true;
	original = radio_state_hash(&state);

#define MUTATE_u8(value) ((uint8_t)((uint8_t)(value) ^ 1u))
#define MUTATE_i8(value) ((int8_t)((uint8_t)(value) ^ 1u))
#define MUTATE_u16(value) ((uint16_t)((uint16_t)(value) ^ 1u))
#define MUTATE_boolean(value) (!(value))
#define TEST_SCALAR(type, name, hash_kind) do { \
	changed = state; \
	changed.name = (type)MUTATE_##hash_kind(changed.name); \
	assert(radio_state_hash(&changed) != original); \
} while (0);
#define TEST_ARRAY(type, name, count, hash_kind) do { \
	uint8_t index; \
	for (index = 0u; index < (count); ++index) { \
		changed = state; \
		changed.name[index] = (type)MUTATE_##hash_kind(changed.name[index]); \
		assert(radio_state_hash(&changed) != original); \
	} \
} while (0);
	RADIO_STATE_FIELDS(TEST_SCALAR, TEST_ARRAY)
#undef TEST_ARRAY
#undef TEST_SCALAR
#undef MUTATE_boolean
#undef MUTATE_u16
#undef MUTATE_i8
#undef MUTATE_u8
}

static void test_deterministic_completion(void) {
	radio_state_t a;
	radio_state_t b;
	radio_input_t input = {0};

	radio_reset_night(&a);
	radio_reset_night(&b);
	lock_exact(&a);
	lock_exact(&b);
	lock_exact(&a);
	lock_exact(&b);
	lock_exact(&a);
	lock_exact(&b);
	(void)input;
	assert(memcmp(&a, &b, sizeof(a)) == 0);
	assert(radio_state_hash(&a) == radio_state_hash(&b));
	assert(a.result == RADIO_RESULT_SIGNAL);
}

int main(void) {
	test_title_and_tutorial();
	test_gain_noise_gate_and_directions();
	test_pause_failure_and_reset();
	test_canonical_hash_inventory();
	test_deterministic_completion();
	return 0;
}
