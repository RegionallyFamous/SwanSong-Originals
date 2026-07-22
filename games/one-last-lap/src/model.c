#include <limits.h>
#include <string.h>

#include "model.h"

#define LAP_TOTAL_UNITS ((uint32_t)LAP_TRACK_UNITS * LAP_TOTAL_LAPS)
#define LAP_TOW_START ((uint32_t)LAP_TRACK_UNITS + 3900u)
#define LAP_TOW_END ((uint32_t)LAP_TRACK_UNITS + 5200u)
#define LAP_TOW_LANE 0u
#define LAP_COLLISION_DISTANCE 72

static const uint8_t rival_speed[LAP_RIVAL_COUNT] = {9, 10, 11};
static const uint16_t hazard_positions[3] = {2500, 5200, 7600};
static const uint8_t hazard_lanes[3][3] = {
	{1, 2, 0}, {2, 0, 1}, {0, 1, 2}
};

static void lap_hash_u8(uint32_t *hash, uint8_t value) {
	*hash = (*hash ^ value) * 16777619u;
}

static void lap_hash_u16(uint32_t *hash, uint16_t value) {
	lap_hash_u8(hash, (uint8_t)value);
	lap_hash_u8(hash, (uint8_t)(value >> 8));
}

static void lap_hash_u32(uint32_t *hash, uint32_t value) {
	lap_hash_u16(hash, (uint16_t)value);
	lap_hash_u16(hash, (uint16_t)(value >> 16));
}

static void lap_hash_boolean(uint32_t *hash, bool value) {
	lap_hash_u8(hash, value ? 1u : 0u);
}

uint32_t lap_state_hash(const lap_state_t *state) {
	uint32_t hash = 2166136261u;
#define LAP_STATE_HASH_SCALAR(type, name, hash_kind) \
	lap_hash_##hash_kind(&hash, state->name);
#define LAP_STATE_HASH_ARRAY(type, name, count, hash_kind) do { \
	uint8_t index; \
	for (index = 0; index < (count); ++index) \
		lap_hash_##hash_kind(&hash, state->name[index]); \
} while (0);
	LAP_STATE_FIELDS(LAP_STATE_HASH_SCALAR, LAP_STATE_HASH_ARRAY)
#undef LAP_STATE_HASH_ARRAY
#undef LAP_STATE_HASH_SCALAR
	return hash;
}

static uint8_t clamp_u8(int16_t value, uint8_t low, uint8_t high) {
	if (value < low) return low;
	if (value > high) return high;
	return (uint8_t)value;
}

static void set_sfx(lap_event_t *event, lap_sfx_t sfx,
	uint16_t legacy_hz, uint8_t legacy_frames) {
	event->sfx = sfx;
	event->tone_hz = legacy_hz;
	event->tone_frames = legacy_frames;
}

uint8_t lap_rival_lane(uint32_t distance, uint8_t rival) {
	uint32_t segment = distance / 1050u;
	return (uint8_t)((segment + rival * 2u) % 3u);
}

int16_t lap_rival_delta(const lap_state_t *state, uint8_t rival) {
	int32_t delta;
	if (rival >= LAP_RIVAL_COUNT) return INT16_MAX;
	delta = (int32_t)state->rival_distance[rival] -
		(int32_t)state->distance_total;
	if (delta < INT16_MIN) return INT16_MIN;
	if (delta > INT16_MAX) return INT16_MAX;
	return (int16_t)delta;
}

bool lap_rival_visible(const lap_state_t *state, uint8_t rival) {
	int16_t delta = lap_rival_delta(state, rival);
	return delta > -180 && delta < 1800;
}

uint8_t lap_hazard_lane(uint8_t lap_index, uint8_t hazard) {
	if (lap_index >= LAP_TOTAL_LAPS || hazard >= 3) return 0;
	return hazard_lanes[lap_index][hazard];
}

uint16_t lap_hazard_position(uint8_t hazard) {
	return hazard < 3 ? hazard_positions[hazard] : 0;
}

static void update_summary(lap_state_t *state) {
	uint32_t lap_index = state->distance_total / LAP_TRACK_UNITS;
	uint32_t position = state->distance_total % LAP_TRACK_UNITS;
	if (lap_index >= LAP_TOTAL_LAPS) {
		state->lap = LAP_TOTAL_LAPS;
		state->progress = 100;
	} else {
		state->lap = (uint8_t)(lap_index + 1u);
		state->progress = (uint8_t)((position * 100u) / LAP_TRACK_UNITS);
	}
	state->tow_available = !state->helped &&
		state->distance_total >= LAP_TOW_START - 750u &&
		state->distance_total <= LAP_TOW_END;
}

static void finish(lap_state_t *state, lap_result_t result,
	lap_event_t *event) {
	int32_t completion_score = result == LAP_RESULT_BATTERY ?
		(int32_t)((state->distance_total * 600u) / LAP_TOTAL_UNITS) : 1200;
	int32_t score = completion_score + (int32_t)state->overtakes * 140 +
		(int32_t)(state->battery / 20u) - (int32_t)state->collisions * 130 -
		(int32_t)(state->elapsed_frames / 180u);
	if (state->helped) score += 500;
	if (score < 0) score = 0;
	if (score > UINT16_MAX) score = UINT16_MAX;
	state->score = (uint16_t)score;
	state->rank = score >= 1900 ? 0 : score >= 1550 ? 1 : score >= 1200 ? 2 : 3;
	state->result = result;
	state->phase = LAP_PHASE_RESULT;
	state->speed = 0;
	event->phase_changed = true;
	event->dirty = true;
	if (result == LAP_RESULT_BATTERY)
		set_sfx(event, LAP_SFX_FAILURE, 120, 12);
	else
		set_sfx(event, LAP_SFX_FINISH,
			result == LAP_RESULT_COOPERATIVE ? 760 : 620, 16);
}

static void apply_collision(lap_state_t *state, lap_event_t *event) {
	state->speed = state->speed > 3 ? (uint8_t)(state->speed - 3u) : 1u;
	state->battery = state->battery > LAP_COLLISION_BATTERY_COST ?
		(uint16_t)(state->battery - LAP_COLLISION_BATTERY_COST) : 0;
	if (state->collisions < UINT8_MAX) ++state->collisions;
	state->collision_cooldown = 90;
	set_sfx(event, LAP_SFX_COLLISION, 150, 8);
	event->dirty = true;
}

static void update_rivals(lap_state_t *state, uint32_t previous_distance,
	lap_event_t *event) {
	uint8_t rival;
	for (rival = 0; rival < LAP_RIVAL_COUNT; ++rival) {
		uint32_t previous_rival = state->rival_distance[rival];
		int32_t delta;
		state->rival_distance[rival] += rival_speed[rival];
		state->rival_lane[rival] = lap_rival_lane(
			state->rival_distance[rival], rival);
		if (previous_distance <= previous_rival &&
			state->distance_total > state->rival_distance[rival]) {
			if (state->overtakes < UINT8_MAX) ++state->overtakes;
			set_sfx(event, LAP_SFX_OVERTAKE, 520, 5);
			event->dirty = true;
		}
		delta = (int32_t)state->rival_distance[rival] -
			(int32_t)state->distance_total;
		if (state->collision_cooldown == 0 &&
			delta >= -LAP_COLLISION_DISTANCE &&
			delta <= LAP_COLLISION_DISTANCE &&
			state->lane == state->rival_lane[rival]) {
			apply_collision(state, event);
		}
	}
}

static void update_hazards(lap_state_t *state, uint32_t previous_distance,
	lap_event_t *event) {
	uint8_t lap_index = (uint8_t)(previous_distance / LAP_TRACK_UNITS);
	uint16_t previous_position = (uint16_t)(previous_distance % LAP_TRACK_UNITS);
	uint16_t position = (uint16_t)(state->distance_total % LAP_TRACK_UNITS);
	uint8_t hazard;
	if (lap_index >= LAP_TOTAL_LAPS ||
		state->distance_total / LAP_TRACK_UNITS != lap_index) return;
	for (hazard = 0; hazard < 3; ++hazard) {
		uint16_t bit = (uint16_t)(1u << (lap_index * 3u + hazard));
		uint16_t hazard_position = lap_hazard_position(hazard);
		if (!(state->hazard_mask & bit) && previous_position < hazard_position &&
			position >= hazard_position) {
			state->hazard_mask |= bit;
			if (state->lane == lap_hazard_lane(lap_index, hazard) &&
				state->collision_cooldown == 0) apply_collision(state, event);
		}
	}
}

void lap_reset(lap_state_t *state) {
	uint8_t rival;
	memset(state, 0, sizeof(*state));
	state->battery = LAP_START_BATTERY;
	state->lane = 1;
	state->lap = 1;
	state->phase = LAP_PHASE_COUNTDOWN;
	state->resume_phase = LAP_PHASE_COUNTDOWN;
	state->countdown = 3;
	for (rival = 0; rival < LAP_RIVAL_COUNT; ++rival) {
		state->rival_distance[rival] = (uint32_t)(850u + rival * 550u);
		state->rival_lane[rival] = lap_rival_lane(
			state->rival_distance[rival], rival);
	}
}

void lap_step(lap_state_t *state, const lap_input_t *input,
	uint32_t session_tick, lap_event_t *event) {
	uint32_t previous_distance;
	(void)session_tick;
	memset(event, 0, sizeof(*event));

	if (state->phase == LAP_PHASE_RESULT) return;
	if (input->pause) {
		if (state->phase == LAP_PHASE_PAUSED) {
			state->phase = state->resume_phase;
		} else {
			state->resume_phase = state->phase;
			state->phase = LAP_PHASE_PAUSED;
		}
		set_sfx(event, LAP_SFX_PAUSE, 300, 4);
		event->phase_changed = true;
		event->dirty = true;
		return;
	}
	if (state->phase == LAP_PHASE_PAUSED) return;

	if (input->lane_direction) {
		uint8_t lane = clamp_u8((int16_t)state->lane +
			input->lane_direction, 0, 2);
		if (lane != state->lane) {
			state->lane = lane;
			set_sfx(event, LAP_SFX_LANE, input->lane_direction > 0 ? 360 : 300, 2);
			event->dirty = true;
		}
	}

	if (state->phase == LAP_PHASE_COUNTDOWN) {
		++state->phase_frames;
		if (state->phase_frames >= LAP_COUNTDOWN_STEP_FRAMES) {
			state->phase_frames = 0;
			if (state->countdown > 1) {
				--state->countdown;
				set_sfx(event, LAP_SFX_COUNTDOWN, 440, 5);
			} else {
				state->countdown = 0;
				state->phase = LAP_PHASE_RACING;
				state->resume_phase = LAP_PHASE_RACING;
				set_sfx(event, LAP_SFX_GO, 660, 8);
				event->phase_changed = true;
			}
			event->dirty = true;
		}
		return;
	}

	++state->elapsed_frames;
	state->checkpoint_flash = false;
	if (state->collision_cooldown) --state->collision_cooldown;

	if (input->accelerate && !input->brake &&
		state->elapsed_frames % 24u == 0 && state->speed < LAP_MAX_SPEED &&
		state->battery) {
		++state->speed;
		event->dirty = true;
	}
	if (input->brake && state->elapsed_frames % 10u == 0 && state->speed) {
		state->speed = state->speed > 1 ? (uint8_t)(state->speed - 2u) : 0;
		event->dirty = true;
	}
	if (!input->accelerate && !input->brake &&
		state->elapsed_frames % 45u == 0 && state->speed) {
		--state->speed;
		event->dirty = true;
	}
	if (input->accelerate && input->brake &&
		state->elapsed_frames % 4u == 0 && state->battery) {
		uint16_t strain = state->battery > 16u ? 16u : state->battery;
		state->battery = (uint16_t)(state->battery - strain);
		event->dirty = true;
	}

	state->tow_available = !state->helped &&
		state->distance_total >= LAP_TOW_START - 750u &&
		state->distance_total <= LAP_TOW_END;
	if (input->tow && !state->helped &&
		state->distance_total >= LAP_TOW_START &&
		state->distance_total <= LAP_TOW_END &&
		state->lane == LAP_TOW_LANE && state->speed <= 8u) {
		state->helped = true;
		state->tow_available = false;
		state->speed = 4;
		state->battery = state->battery > 200u ?
			(uint16_t)(state->battery - 200u) : 0;
		set_sfx(event, LAP_SFX_TOW, 580, 14);
		event->dirty = true;
	}

	if (state->elapsed_frames % 4u == 0) {
		uint8_t old_lap = state->lap;
		previous_distance = state->distance_total;
		if (state->speed && state->battery) {
			uint16_t drain = state->speed >= 10u ? 2u : 1u;
			state->distance_total += state->speed;
			state->battery = state->battery > drain ?
				(uint16_t)(state->battery - drain) : 0;
		}
		update_rivals(state, previous_distance, event);
		if (state->distance_total != previous_distance)
			update_hazards(state, previous_distance, event);
		update_summary(state);
		if (state->lap != old_lap && state->distance_total < LAP_TOTAL_UNITS) {
			state->checkpoint_flash = true;
			set_sfx(event, LAP_SFX_CHECKPOINT, 700, 10);
		}
		if (state->distance_total != previous_distance ||
			state->elapsed_frames % 8u == 0) event->dirty = true;
	}

	if (state->distance_total >= LAP_TOTAL_UNITS) {
		finish(state, state->helped ? LAP_RESULT_COOPERATIVE : LAP_RESULT_SOLO,
			event);
	} else if (state->battery == 0) {
		finish(state, LAP_RESULT_BATTERY, event);
	}
}
