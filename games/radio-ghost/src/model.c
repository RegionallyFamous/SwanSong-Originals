#include <string.h>

#include "model.h"

static const uint16_t target_frequency[RADIO_SIGNAL_COUNT] = {934u, 1001u, 962u};
static const uint8_t ideal_gain[RADIO_SIGNAL_COUNT] = {4u, 6u, 7u};
static const uint8_t phase_noise[RADIO_SIGNAL_COUNT] = {1u, 2u, 3u};

static void radio_hash_u8(uint32_t *hash, uint8_t value) {
	*hash = (*hash ^ value) * 16777619u;
}

static void radio_hash_i8(uint32_t *hash, int8_t value) {
	radio_hash_u8(hash, (uint8_t)value);
}

static void radio_hash_u16(uint32_t *hash, uint16_t value) {
	radio_hash_u8(hash, (uint8_t)value);
	radio_hash_u8(hash, (uint8_t)(value >> 8));
}

static void radio_hash_boolean(uint32_t *hash, bool value) {
	radio_hash_u8(hash, value ? 1u : 0u);
}

uint32_t radio_state_hash(const radio_state_t *state) {
	uint32_t hash = 2166136261u;
#define RADIO_STATE_HASH_SCALAR(type, name, hash_kind) \
	radio_hash_##hash_kind(&hash, state->name);
#define RADIO_STATE_HASH_ARRAY(type, name, count, hash_kind) do { \
	uint8_t index; \
	for (index = 0; index < (count); ++index) \
		radio_hash_##hash_kind(&hash, state->name[index]); \
} while (0);
	RADIO_STATE_FIELDS(RADIO_STATE_HASH_SCALAR, RADIO_STATE_HASH_ARRAY)
#undef RADIO_STATE_HASH_ARRAY
#undef RADIO_STATE_HASH_SCALAR
	return hash;
}

static uint8_t clamp_u8(int16_t value, uint8_t low, uint8_t high) {
	if (value < low) return low;
	if (value > high) return high;
	return (uint8_t)value;
}

static uint8_t absolute_u8_difference(uint8_t a, uint8_t b) {
	return a > b ? (uint8_t)(a - b) : (uint8_t)(b - a);
}

static uint16_t frequency_distance(const radio_state_t *state) {
	uint16_t target = radio_target_for(state->clue);
	return state->frequency > target ?
		(uint16_t)(state->frequency - target) :
		(uint16_t)(target - state->frequency);
}

uint16_t radio_target_for(uint8_t clue) {
	return target_frequency[clue < RADIO_SIGNAL_COUNT ? clue : RADIO_SIGNAL_COUNT - 1u];
}

uint8_t radio_ideal_gain_for(uint8_t clue) {
	return ideal_gain[clue < RADIO_SIGNAL_COUNT ? clue : RADIO_SIGNAL_COUNT - 1u];
}

uint8_t radio_noise_level(const radio_state_t *state) {
	uint8_t clue = state->clue < RADIO_SIGNAL_COUNT ? state->clue : RADIO_SIGNAL_COUNT - 1u;
	uint8_t error = absolute_u8_difference(state->gain, ideal_gain[clue]);
	uint8_t noise = (uint8_t)(phase_noise[clue] + error * 2u + (state->gate ? 1u : 0u));
	return noise > 9u ? 9u : noise;
}

uint8_t radio_signal_strength(const radio_state_t *state) {
	uint16_t distance;
	uint8_t proximity;
	uint8_t gain;
	uint8_t noise;
	int16_t strength;
	if (state->mode == RADIO_MODE_TITLE || state->clue >= RADIO_SIGNAL_COUNT)
		return 0;
	distance = frequency_distance(state);
	proximity = distance >= 48u ? 0u : (uint8_t)(12u - distance / 4u);
	gain = state->gain > radio_ideal_gain_for(state->clue) ?
		radio_ideal_gain_for(state->clue) : state->gain;
	noise = radio_noise_level(state);
	strength = (int16_t)proximity + gain - (int16_t)(noise / 2u);
	if (strength < 0) return 0;
	if (strength > 15) return 15;
	return (uint8_t)strength;
}

int8_t radio_target_direction(const radio_state_t *state) {
	uint16_t target;
	uint16_t tolerance;
	if (state->mode == RADIO_MODE_TITLE || state->clue >= RADIO_SIGNAL_COUNT)
		return 0;
	target = radio_target_for(state->clue);
	tolerance = state->gate ? 8u : 3u;
	if (state->frequency + tolerance < target) return 1;
	if (state->frequency > target + tolerance) return -1;
	return 0;
}

uint16_t radio_state_progress(const radio_state_t *state) {
	uint16_t progress;
	if (state->result == RADIO_RESULT_SIGNAL) return 1000u;
	if (state->mode == RADIO_MODE_TUTORIAL)
		return (uint16_t)state->tutorial_step * 50u;
	if (state->mode != RADIO_MODE_NIGHT) return 0;
	progress = (uint16_t)state->clue * 333u;
	if (state->result == RADIO_RESULT_PLAYING && state->clue < RADIO_SIGNAL_COUNT)
		progress = (uint16_t)(progress + radio_signal_strength(state) * 10u);
	return progress > 999u ? 999u : progress;
}

static void reset_common(radio_state_t *state, radio_mode_t mode) {
	memset(state, 0, sizeof(*state));
	state->frequency = 950u;
	state->time = RADIO_NIGHT_FRAMES;
	state->gain = 4u;
	state->mode = mode;
}

void radio_reset_title(radio_state_t *state) {
	reset_common(state, RADIO_MODE_TITLE);
}

void radio_reset_tutorial(radio_state_t *state) {
	reset_common(state, RADIO_MODE_TUTORIAL);
	state->frequency = 928u;
	state->gain = 3u;
	state->tutorial_step = RADIO_TUTORIAL_TUNE;
}

void radio_reset_night(radio_state_t *state) {
	reset_common(state, RADIO_MODE_NIGHT);
}

void radio_set_paused(radio_state_t *state, bool paused) {
	state->paused = paused;
}

static void set_sfx(radio_event_t *event, radio_sfx_t effect, int8_t pan) {
	event->sfx = effect;
	event->pan = pan;
}

static void step_title(radio_state_t *state, const radio_input_t *input,
	radio_event_t *event) {
	if (state->title_ticks != UINT16_MAX) ++state->title_ticks;
	if (!state->attract && state->title_ticks >= RADIO_TITLE_ATTRACT_FRAMES) {
		state->attract = true;
		event->dirty = true;
	}
	if ((state->title_ticks & 15u) == 0u) event->dirty = true;
	if (input->menu_direction != 0) {
		state->title_choice = input->menu_direction > 0 ? 1u : 0u;
		set_sfx(event, input->menu_direction > 0 ?
			RADIO_SFX_TUNE_RIGHT : RADIO_SFX_TUNE_LEFT,
			input->menu_direction);
		event->dirty = true;
	}
	if (input->confirm) {
		set_sfx(event, RADIO_SFX_CONFIRM, 0);
		if (state->title_choice == 0u) event->start_tutorial = true;
		else event->start_night = true;
		event->dirty = true;
	}
}

static void change_frequency(radio_state_t *state, int8_t direction,
	radio_event_t *event) {
	int16_t frequency = (int16_t)state->frequency + (int16_t)direction * 6;
	if (frequency < (int16_t)RADIO_FREQUENCY_MIN) frequency = RADIO_FREQUENCY_MIN;
	if (frequency > (int16_t)RADIO_FREQUENCY_MAX) frequency = RADIO_FREQUENCY_MAX;
	state->frequency = (uint16_t)frequency;
	state->last_direction = radio_target_direction(state);
	state->lock_quality = 0u;
	set_sfx(event, direction > 0 ? RADIO_SFX_TUNE_RIGHT : RADIO_SFX_TUNE_LEFT,
		state->last_direction);
	state->feedback_flash = 4u;
	event->dirty = true;
}

static void change_gain(radio_state_t *state, int8_t direction,
	radio_event_t *event) {
	state->gain = clamp_u8((int16_t)state->gain + direction, 0u, 9u);
	state->lock_quality = 0u;
	set_sfx(event, RADIO_SFX_GAIN, direction);
	state->feedback_flash = 4u;
	event->dirty = true;
}

static bool lock_is_valid(const radio_state_t *state, uint16_t distance,
	uint8_t gain_error) {
	uint8_t tolerance = state->gate ? 8u : 3u;
	uint8_t gain_tolerance = state->gate ? 2u : 1u;
	return distance <= tolerance && gain_error <= gain_tolerance &&
		radio_noise_level(state) <= 6u && radio_signal_strength(state) >= 8u;
}

static void finish_signal(radio_state_t *state, radio_event_t *event) {
	uint32_t score = state->score + state->time / 6u +
		(uint16_t)state->narrow_locks * 100u;
	state->score = score > 9999u ? 9999u : (uint16_t)score;
	state->result = RADIO_RESULT_SIGNAL;
	set_sfx(event, RADIO_SFX_COMPLETE, 0);
	event->show_result = true;
}

static void finish_dawn(radio_state_t *state, radio_event_t *event) {
	state->result = RADIO_RESULT_DAWN;
	set_sfx(event, RADIO_SFX_FAILURE, 0);
	event->show_result = true;
	event->dirty = true;
}

static void attempt_lock(radio_state_t *state, radio_event_t *event) {
	uint16_t distance = frequency_distance(state);
	uint8_t gain_error = absolute_u8_difference(state->gain,
		radio_ideal_gain_for(state->clue));
	if (lock_is_valid(state, distance, gain_error)) {
		int16_t quality = 100 - (int16_t)distance * 7 -
			(int16_t)gain_error * 12 - (state->gate ? 8 : 0);
		if (quality < 25) quality = 25;
		state->lock_quality = (uint8_t)quality;
		state->score = (uint16_t)(state->score + (uint16_t)quality * 3u);
		if (!state->gate) ++state->narrow_locks;
		else if (state->gain) --state->gain;
		++state->clue;
		state->last_direction = 0;
		state->feedback_flash = 24u;
		set_sfx(event, RADIO_SFX_LOCK, 0);
		if (state->clue >= RADIO_SIGNAL_COUNT) finish_signal(state, event);
	} else {
		++state->wrong_locks;
		state->time = state->time > RADIO_BAD_LOCK_PENALTY ?
			(uint16_t)(state->time - RADIO_BAD_LOCK_PENALTY) : 0u;
		state->lock_quality = 0;
		state->feedback_flash = 18u;
		set_sfx(event, RADIO_SFX_ERROR, radio_target_direction(state));
		if (state->time == 0u) finish_dawn(state, event);
	}
	event->dirty = true;
}

static void step_tutorial(radio_state_t *state, const radio_input_t *input,
	radio_event_t *event) {
	if (state->tutorial_step == RADIO_TUTORIAL_READY) {
		if (input->lock || input->confirm) {
			set_sfx(event, RADIO_SFX_CONFIRM, 0);
			event->start_night = true;
			event->dirty = true;
		}
		return;
	}
	if (input->frequency_direction != 0) {
		change_frequency(state, input->frequency_direction, event);
		if (state->tutorial_step == RADIO_TUTORIAL_TUNE &&
			state->frequency == radio_target_for(0))
			state->tutorial_step = RADIO_TUTORIAL_GAIN;
	}
	if (input->gain_direction != 0) {
		change_gain(state, input->gain_direction, event);
		if (state->tutorial_step == RADIO_TUTORIAL_GAIN && state->gain == 4u)
			state->tutorial_step = RADIO_TUTORIAL_GATE;
	}
	if (input->toggle_gate) {
		state->gate = !state->gate;
		state->lock_quality = 0u;
		set_sfx(event, RADIO_SFX_GATE, 0);
		if (state->tutorial_step == RADIO_TUTORIAL_GATE && state->gate)
			state->tutorial_step = RADIO_TUTORIAL_LOCK;
		event->dirty = true;
	}
	if (input->lock && state->tutorial_step == RADIO_TUTORIAL_LOCK) {
		uint16_t distance = frequency_distance(state);
		uint8_t gain_error = absolute_u8_difference(state->gain,
			radio_ideal_gain_for(0));
		if (lock_is_valid(state, distance, gain_error)) {
			state->tutorial_step = RADIO_TUTORIAL_READY;
			state->feedback_flash = 24u;
			state->lock_quality = 100u;
			set_sfx(event, RADIO_SFX_LOCK, 0);
		} else {
			state->feedback_flash = 18u;
			set_sfx(event, RADIO_SFX_ERROR, radio_target_direction(state));
		}
		event->dirty = true;
	}
}

static void step_night(radio_state_t *state, const radio_input_t *input,
	radio_event_t *event) {
	if (input->pause) {
		state->paused = !state->paused;
		set_sfx(event, RADIO_SFX_PAUSE, 0);
		event->pause_changed = true;
		event->dirty = true;
		return;
	}
	if (state->paused || state->result != RADIO_RESULT_PLAYING) return;
	if (input->frequency_direction != 0)
		change_frequency(state, input->frequency_direction, event);
	if (input->gain_direction != 0)
		change_gain(state, input->gain_direction, event);
	if (input->toggle_gate) {
		state->gate = !state->gate;
		state->lock_quality = 0u;
		set_sfx(event, RADIO_SFX_GATE, 0);
		state->feedback_flash = 6u;
		event->dirty = true;
	}
	if (input->lock) attempt_lock(state, event);
	if (state->feedback_flash) {
		--state->feedback_flash;
		event->dirty = true;
	}
	if (state->result == RADIO_RESULT_PLAYING) {
		if (state->time) --state->time;
		if (state->time == 0u) finish_dawn(state, event);
	}
}

void radio_step(radio_state_t *state, const radio_input_t *input,
	radio_event_t *event) {
	memset(event, 0, sizeof(*event));
	if (state->mode == RADIO_MODE_TITLE) {
		step_title(state, input, event);
		return;
	}
	if (state->mode == RADIO_MODE_TUTORIAL) {
		step_tutorial(state, input, event);
		return;
	}
	step_night(state, input, event);
}
