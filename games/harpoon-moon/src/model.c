#include <string.h>

#include "model.h"

#define HARPOON_RANDOM_SEED 0x71D3u
#define HARPOON_RECOVERY_FRAMES 60u
#define HARPOON_MISS_OXYGEN 50u
#define HARPOON_RECOVER_OXYGEN 35u
#define HARPOON_LOW_OXYGEN 600u

static uint8_t clamp_u8(int16_t value, uint8_t low, uint8_t high) {
	if (value < low) return low;
	if (value > high) return high;
	return (uint8_t)value;
}

static uint8_t distance_between(uint8_t left, uint8_t right) {
	return left > right ? (uint8_t)(left - right) : (uint8_t)(right - left);
}

static uint16_t random_next(harpoon_state_t *state) {
	uint16_t value = state->random_state ? state->random_state : HARPOON_RANDOM_SEED;
	value ^= (uint16_t)(value << 7);
	value ^= (uint16_t)(value >> 9);
	value ^= (uint16_t)(value << 8);
	state->random_state = value;
	return value;
}

static uint8_t sfx_priority(harpoon_sfx_t sfx) {
	switch (sfx) {
		case HARPOON_SFX_WIN:
		case HARPOON_SFX_FAILURE: return 9;
		case HARPOON_SFX_BOSS:
		case HARPOON_SFX_TAG: return 8;
		case HARPOON_SFX_HIT:
		case HARPOON_SFX_MISS: return 7;
		case HARPOON_SFX_RECOVER:
		case HARPOON_SFX_READY: return 6;
		case HARPOON_SFX_SHOT: return 5;
		case HARPOON_SFX_LURE:
		case HARPOON_SFX_CHARGE: return 4;
		case HARPOON_SFX_PAUSE:
		case HARPOON_SFX_RETRY: return 3;
		case HARPOON_SFX_MOVE: return 1;
		default: return 0;
	}
}

static void offer_sfx(harpoon_state_t *state, harpoon_event_t *event,
	harpoon_sfx_t sfx) {
	if (sfx_priority(sfx) >= sfx_priority(event->sfx)) event->sfx = sfx;
	if (sfx_priority(sfx) >= sfx_priority(state->feedback)) {
		state->feedback = sfx;
		state->feedback_frames = 45;
	}
}

static void set_phase(harpoon_state_t *state, harpoon_event_t *event,
	harpoon_phase_t phase) {
	state->phase = phase;
	state->phase_frames = 0;
	event->phase_changed = true;
	event->dirty = true;
}

static void setup_scout(harpoon_state_t *state) {
	state->lure_meter = 0;
	state->lure_window = 0;
	state->charge = 0;
	state->behavior_frames = 0;
	state->creature_depth = 0;
	if (state->tags == 1) {
		state->behavior = HARPOON_BEHAVIOR_SHY;
		state->creature = (uint8_t)(16u + random_next(state) % 3u);
		state->creature_direction = -1;
	} else {
		state->behavior = HARPOON_BEHAVIOR_DIVER;
		state->creature = (uint8_t)(3u + random_next(state) % 3u);
		state->creature_depth = 2;
		state->creature_direction = 1;
	}
}

static void setup_boss(harpoon_state_t *state) {
	state->lure_meter = 0;
	state->lure_window = 0;
	state->charge = 0;
	state->behavior_frames = 0;
	if (state->boss_hp == 3) {
		state->behavior = HARPOON_BEHAVIOR_TRACK;
		state->creature = 14;
		state->creature_depth = 0;
		state->creature_direction = -1;
	} else if (state->boss_hp == 2) {
		state->behavior = HARPOON_BEHAVIOR_SWEEP;
		state->creature = 5;
		state->creature_depth = 1;
		state->creature_direction = 1;
	} else {
		state->behavior = HARPOON_BEHAVIOR_ABYSS;
		state->creature = 18;
		state->creature_depth = 2;
		state->creature_direction = -1;
	}
}

uint8_t harpoon_min_charge(const harpoon_state_t *state) {
	if (state->phase == HARPOON_PHASE_BOSS_THREE) return 24;
	if (state->phase == HARPOON_PHASE_BOSS_TWO) return 20;
	if (state->phase == HARPOON_PHASE_BOSS_ONE) return 16;
	if (state->tags == 2) return 16;
	if (state->tags == 1) return 12;
	return 8;
}

uint8_t harpoon_max_charge(const harpoon_state_t *state) {
	if (state->phase == HARPOON_PHASE_BOSS_THREE) return 30;
	if (state->phase == HARPOON_PHASE_BOSS_TWO) return 28;
	if (state->phase == HARPOON_PHASE_BOSS_ONE) return 26;
	if (state->tags == 2) return 26;
	if (state->tags == 1) return 24;
	return 20;
}

uint8_t harpoon_lure_target(const harpoon_state_t *state) {
	if (state->phase == HARPOON_PHASE_BOSS_THREE) return 52;
	if (state->phase == HARPOON_PHASE_BOSS_TWO) return 44;
	if (state->phase == HARPOON_PHASE_BOSS_ONE) return 36;
	if (state->tags == 2) return 42;
	if (state->tags == 1) return 36;
	return 30;
}

static uint8_t lure_duration(const harpoon_state_t *state) {
	if (state->phase == HARPOON_PHASE_BOSS_THREE) return 100;
	if (state->phase == HARPOON_PHASE_BOSS_TWO) return 120;
	if (state->phase == HARPOON_PHASE_BOSS_ONE) return 150;
	return 160;
}

static void update_creature(harpoon_state_t *state, bool lure_held,
	harpoon_event_t *event) {
	uint8_t interval;
	if (lure_held) {
		interval = state->behavior == HARPOON_BEHAVIOR_ABYSS ? 7 : 5;
		if (state->behavior_frames % interval == 0) {
			if (state->creature < state->skiff) ++state->creature;
			else if (state->creature > state->skiff) --state->creature;
			if (state->creature_depth && state->behavior_frames % 10u == 0) {
				--state->creature_depth;
			}
			event->dirty = true;
		}
		return;
	}

	switch (state->behavior) {
		case HARPOON_BEHAVIOR_CALM:
			if (state->behavior_frames % 60u == 0) {
				state->creature_direction = (random_next(state) & 1u) ? 1 : -1;
				state->creature = clamp_u8((int16_t)state->creature +
					state->creature_direction, 2, 19);
				event->dirty = true;
			}
			break;
		case HARPOON_BEHAVIOR_SHY:
			if (state->behavior_frames % 28u == 0) {
				state->creature_direction = state->creature >= state->skiff ? 1 : -1;
				state->creature = clamp_u8((int16_t)state->creature +
					state->creature_direction, 1, 20);
				event->dirty = true;
			}
			break;
		case HARPOON_BEHAVIOR_DIVER:
			if (state->behavior_frames % 32u == 0 && state->lure_window == 0) {
				state->creature_depth = (uint8_t)((state->creature_depth + 1u) % 4u);
				event->dirty = true;
			}
			if (state->behavior_frames % 18u == 0) {
				if (state->creature == 2 || state->creature == 19) {
					state->creature_direction = (int8_t)-state->creature_direction;
				}
				state->creature = clamp_u8((int16_t)state->creature +
					state->creature_direction, 2, 19);
				event->dirty = true;
			}
			break;
		case HARPOON_BEHAVIOR_TRACK:
			if (state->behavior_frames % 24u == 0) {
				state->creature_direction = state->creature > state->skiff ? -1 : 1;
				state->creature = clamp_u8((int16_t)state->creature +
					state->creature_direction, 1, 20);
				event->dirty = true;
			}
			break;
		case HARPOON_BEHAVIOR_SWEEP:
			if (state->behavior_frames % 14u == 0) {
				if (state->creature == 1 || state->creature == 20) {
					state->creature_direction = (int8_t)-state->creature_direction;
				}
				state->creature = clamp_u8((int16_t)state->creature +
					state->creature_direction, 1, 20);
				state->creature_depth = (uint8_t)((state->creature_depth + 1u) % 3u);
				event->dirty = true;
			}
			break;
		case HARPOON_BEHAVIOR_ABYSS:
			if (state->behavior_frames % 9u == 0) {
				state->creature_direction = state->creature > state->skiff ? -1 : 1;
				state->creature = clamp_u8((int16_t)state->creature +
					state->creature_direction, 1, 20);
				if (state->lure_window == 0) {
					state->creature_depth = (uint8_t)((state->creature_depth + 1u) % 4u);
				}
				event->dirty = true;
			}
			break;
	}
}

static void open_lure_window(harpoon_state_t *state, harpoon_event_t *event) {
	state->lure_meter = 0;
	state->lure_window = lure_duration(state);
	state->creature_depth = 0;
	if (state->tutorial == HARPOON_TUTORIAL_LURE) {
		state->tutorial = HARPOON_TUTORIAL_CHARGE;
	}
	offer_sfx(state, event, HARPOON_SFX_LURE);
	event->dirty = true;
}

static void finish_game(harpoon_state_t *state, harpoon_event_t *event,
	harpoon_result_t result) {
	state->result = result;
	state->rank = result == HARPOON_RESULT_WIN ?
		(state->misses == 0 && state->oxygen >= 1200 ? 3 :
		(state->misses <= 2 && state->oxygen >= 600 ? 2 : 1)) : 0;
	set_phase(state, event, HARPOON_PHASE_RESULT);
	event->music = HARPOON_MUSIC_RESULT;
	offer_sfx(state, event,
		result == HARPOON_RESULT_WIN ? HARPOON_SFX_WIN : HARPOON_SFX_FAILURE);
}

static void resolve_shot(harpoon_state_t *state, harpoon_event_t *event) {
	uint8_t min_charge = harpoon_min_charge(state);
	uint8_t max_charge = harpoon_max_charge(state);
	uint8_t reach = (uint8_t)(3u + state->charge / 2u);
	bool good_timing = state->charge >= min_charge && state->charge <= max_charge;
	bool good_target = state->lure_window && state->creature_depth == 0 &&
		distance_between(state->skiff, state->creature) <= reach;
	++state->shots;
	offer_sfx(state, event, HARPOON_SFX_SHOT);
	if (good_timing && good_target) {
		++state->streak;
		state->score = (uint16_t)(state->score + 100u +
			(uint16_t)(max_charge - state->charge) * 3u + state->streak * 5u);
		state->hit_flash = 20;
		if (state->tags < HARPOON_SCOUT_COUNT) {
			++state->tags;
			offer_sfx(state, event, HARPOON_SFX_TAG);
			if (state->tutorial != HARPOON_TUTORIAL_COMPLETE) {
				state->tutorial = HARPOON_TUTORIAL_COMPLETE;
				set_phase(state, event, HARPOON_PHASE_HUNT);
			}
			if (state->tags == HARPOON_SCOUT_COUNT) {
				state->boss_hp = HARPOON_BOSS_HEALTH;
				set_phase(state, event, HARPOON_PHASE_BOSS_ONE);
				setup_boss(state);
				event->music = HARPOON_MUSIC_BOSS;
				offer_sfx(state, event, HARPOON_SFX_BOSS);
			} else {
				setup_scout(state);
			}
		} else {
			--state->boss_hp;
			offer_sfx(state, event, HARPOON_SFX_HIT);
			if (state->boss_hp == 0) {
				state->score = (uint16_t)(state->score + state->oxygen / 4u);
				finish_game(state, event, HARPOON_RESULT_WIN);
			} else {
				set_phase(state, event, state->boss_hp == 2 ?
					HARPOON_PHASE_BOSS_TWO : HARPOON_PHASE_BOSS_THREE);
				setup_boss(state);
				offer_sfx(state, event, HARPOON_SFX_BOSS);
			}
		}
	} else {
		state->streak = 0;
		++state->misses;
		state->oxygen = state->oxygen > HARPOON_MISS_OXYGEN ?
			(uint16_t)(state->oxygen - HARPOON_MISS_OXYGEN) : 0;
		state->recovery_frames = HARPOON_RECOVERY_FRAMES;
		offer_sfx(state, event, HARPOON_SFX_MISS);
	}
	state->charge = 0;
	state->lure_window = 0;
	event->dirty = true;
}

void harpoon_reset(harpoon_state_t *state) {
	memset(state, 0, sizeof(*state));
	state->random_state = HARPOON_RANDOM_SEED;
	state->oxygen = HARPOON_START_OXYGEN;
	state->skiff = 3;
	state->creature = 11;
	state->boss_hp = HARPOON_BOSS_HEALTH;
	state->creature_direction = 1;
	state->phase = HARPOON_PHASE_TUTORIAL;
	state->resume_phase = HARPOON_PHASE_TUTORIAL;
	state->tutorial = HARPOON_TUTORIAL_MOVE;
	state->behavior = HARPOON_BEHAVIOR_CALM;
	state->result = HARPOON_RESULT_PLAYING;
}

void harpoon_step(harpoon_state_t *state, const harpoon_input_t *input,
	uint32_t session_tick, harpoon_event_t *event) {
	uint8_t old_charge;
	uint8_t recovery_amount;
	uint8_t drain = 0;
	(void)session_tick;
	memset(event, 0, sizeof(*event));

	if (input->retry) {
		harpoon_reset(state);
		event->reset_session = true;
		event->phase_changed = true;
		event->sfx = HARPOON_SFX_RETRY;
		event->music = HARPOON_MUSIC_HUNT;
		event->dirty = true;
		return;
	}
	if (state->result != HARPOON_RESULT_PLAYING) return;
	if (input->pause) {
		if (state->phase == HARPOON_PHASE_PAUSED) {
			set_phase(state, event, state->resume_phase);
		} else {
			state->resume_phase = state->phase;
			set_phase(state, event, HARPOON_PHASE_PAUSED);
		}
		offer_sfx(state, event, HARPOON_SFX_PAUSE);
		return;
	}
	if (state->phase == HARPOON_PHASE_PAUSED) return;

	++state->elapsed_frames;
	++state->phase_frames;
	++state->behavior_frames;
	if (state->feedback_frames) {
		--state->feedback_frames;
		if (state->feedback_frames == 0) state->feedback = HARPOON_SFX_NONE;
	}
	if (state->hit_flash) --state->hit_flash;
	if (state->lure_window) --state->lure_window;

	if (input->direction) {
		uint8_t previous = state->skiff;
		state->skiff = clamp_u8((int16_t)state->skiff + input->direction,
			HARPOON_WORLD_MIN, HARPOON_WORLD_MAX);
		if (state->skiff != previous) {
			if (state->tutorial == HARPOON_TUTORIAL_MOVE) {
				state->tutorial = HARPOON_TUTORIAL_LURE;
			}
			offer_sfx(state, event, HARPOON_SFX_MOVE);
			event->dirty = true;
		}
	}

	if (state->recovery_frames) {
		recovery_amount = input->lure_held ? 3u : 1u;
		if (state->recovery_frames <= recovery_amount) {
			state->recovery_frames = 0;
			state->oxygen = (uint16_t)(state->oxygen + HARPOON_RECOVER_OXYGEN);
			if (state->oxygen > HARPOON_START_OXYGEN) state->oxygen = HARPOON_START_OXYGEN;
			offer_sfx(state, event, HARPOON_SFX_RECOVER);
		} else {
			state->recovery_frames = (uint8_t)(state->recovery_frames - recovery_amount);
		}
		event->dirty = true;
	} else {
		if (input->lure_held) {
			uint8_t gain = distance_between(state->skiff, state->creature) <= 10u ? 2u : 1u;
			uint16_t next_meter = (uint16_t)state->lure_meter + gain;
			state->lure_meter = next_meter > 255u ? 255u : (uint8_t)next_meter;
			if (state->lure_meter >= harpoon_lure_target(state)) open_lure_window(state, event);
			event->dirty = true;
		}
		old_charge = state->charge;
		if (input->charge_held && state->charge < HARPOON_MAX_CHARGE) {
			++state->charge;
			if (old_charge == 0) offer_sfx(state, event, HARPOON_SFX_CHARGE);
			if (state->charge == harpoon_min_charge(state)) {
				offer_sfx(state, event, HARPOON_SFX_READY);
			}
			event->dirty = true;
		}
		update_creature(state, input->lure_held, event);
		if (input->fire_released && state->charge) resolve_shot(state, event);
	}

	if (state->tutorial == HARPOON_TUTORIAL_COMPLETE) {
		drain = 1;
		if (state->phase == HARPOON_PHASE_BOSS_TWO && (state->elapsed_frames & 3u) == 0) ++drain;
		if (state->phase == HARPOON_PHASE_BOSS_THREE && (state->elapsed_frames & 1u) == 0) ++drain;
	} else if ((state->elapsed_frames & 3u) == 0) {
		drain = 1;
	}
	if (drain) {
		state->oxygen = state->oxygen > drain ? (uint16_t)(state->oxygen - drain) : 0;
		event->dirty = true;
	}
	if (state->oxygen == 0 && state->result == HARPOON_RESULT_PLAYING) {
		finish_game(state, event, HARPOON_RESULT_OXYGEN);
	} else if (state->oxygen <= HARPOON_LOW_OXYGEN &&
		(state->elapsed_frames % 16u) == 0) {
		event->dirty = true;
	}
}

uint16_t harpoon_progress(const harpoon_state_t *state) {
	if (state->result == HARPOON_RESULT_WIN) return 1000;
	if (state->tags < HARPOON_SCOUT_COUNT) return (uint16_t)(state->tags * 166u);
	return (uint16_t)(500u + (HARPOON_BOSS_HEALTH - state->boss_hp) * 166u);
}

uint32_t harpoon_state_hash(const harpoon_state_t *state) {
	uint32_t hash = 2166136261u;
#define HARPOON_HASH_FIELD(type, name) do { \
		uint32_t value = (uint32_t)state->name; \
		uint8_t byte_index; \
		for (byte_index = 0; byte_index < 4; ++byte_index) { \
			hash ^= (uint8_t)(value >> (byte_index * 8)); \
			hash *= 16777619u; \
		} \
	} while (0);
	HARPOON_STATE_FIELDS(HARPOON_HASH_FIELD)
#undef HARPOON_HASH_FIELD
	return hash;
}
