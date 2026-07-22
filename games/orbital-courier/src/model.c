#include <string.h>

#include "model.h"

static void courier_hash_u8(uint32_t *hash, uint8_t value) {
	*hash = (*hash ^ value) * 16777619u;
}

static void courier_hash_u16(uint32_t *hash, uint16_t value) {
	courier_hash_u8(hash, (uint8_t)value);
	courier_hash_u8(hash, (uint8_t)(value >> 8));
}

static void courier_hash_u32(uint32_t *hash, uint32_t value) {
	courier_hash_u16(hash, (uint16_t)value);
	courier_hash_u16(hash, (uint16_t)(value >> 16));
}

static void courier_hash_boolean(uint32_t *hash, bool value) {
	courier_hash_u8(hash, value ? 1u : 0u);
}

uint32_t courier_state_hash(const courier_state_t *state) {
	uint32_t hash = 2166136261u;
#define COURIER_STATE_HASH(type, name, hash_kind) \
	courier_hash_##hash_kind(&hash, state->name);
	COURIER_STATE_FIELDS(COURIER_STATE_HASH)
#undef COURIER_STATE_HASH
	return hash;
}

bool orbital_blocked(uint8_t x, uint8_t y) {
	if (x == 0 || x >= COURIER_MAP_COLS - 1u ||
		y == 0 || y >= COURIER_MAP_ROWS - 1u) return true;
	if (y == 4 && x >= 4 && x <= 22 && x != 8 && x != 18) return true;
	if (x == 12 && y >= 4 && y <= 12 && y != 7) return true;
	if (y == 9 && x >= 2 && x <= 20 && x != 6 && x != 16) return true;
	if (x == 22 && y >= 5 && y <= 12 && y != 10) return true;
	return false;
}

bool orbital_express_lane(uint8_t x, uint8_t y) {
	return y == 7 && x >= 12 && x <= 20;
}

static uint8_t triangle(uint16_t value, uint8_t span) {
	uint8_t phase = (uint8_t)(value % (uint16_t)(span * 2u));
	return phase <= span ? phase : (uint8_t)(span * 2u - phase);
}

void courier_traffic_position(uint16_t turn, uint8_t traffic,
	uint8_t *x, uint8_t *y) {
	if (!x || !y) return;
	switch (traffic) {
		case 0:
			*x = (uint8_t)(5u + triangle(turn, 16));
			*y = 2;
			break;
		case 1:
			*x = (uint8_t)(8u + triangle((uint16_t)(turn + 7u), 12));
			*y = 7;
			break;
		default:
			*x = 20;
			*y = (uint8_t)(5u + triangle((uint16_t)(turn + 3u), 6));
			break;
	}
}

void courier_reset(courier_state_t *state) {
	memset(state, 0, sizeof(*state));
	state->x = COURIER_START_X;
	state->y = COURIER_START_Y;
	state->fuel = COURIER_START_FUEL;
	state->phase = COURIER_PHASE_ACTIVE;
}

static void set_feedback(courier_state_t *state, courier_event_t *event,
	courier_sfx_t sfx, uint8_t frames) {
	state->feedback = sfx;
	state->feedback_frames = frames;
	event->sfx = sfx;
	event->dirty = true;
}

static uint8_t movement_cost(uint8_t x, uint8_t y) {
	return orbital_express_lane(x, y) ? 2u : 1u;
}

static void finish_delivery(courier_state_t *state, courier_event_t *event) {
	uint16_t time_bonus = state->elapsed_frames < 3000u ?
		(uint16_t)((3000u - state->elapsed_frames) / 10u) : 0;
	int32_t score = 1400 + (int32_t)state->fuel * 20 + time_bonus;
	score += state->charger_used ? 100 : 0;
	score -= (int32_t)state->steps * 4;
	score -= (int32_t)state->collisions * 80;
	if (score < 0) score = 0;
	if (score > 9999) score = 9999;
	state->score = (uint16_t)score;
	state->rank = state->score >= 1900u ? 0u :
		state->score >= 1600u ? 1u : state->score >= 1300u ? 2u : 3u;
	state->leg = 3;
	state->tutorial = 4;
	state->result = COURIER_RESULT_DELIVERED;
	state->phase = COURIER_PHASE_RESULT;
	set_feedback(state, event, COURIER_SFX_DELIVERED, 45);
	event->phase_changed = true;
}

static void fail_delivery(courier_state_t *state, courier_event_t *event) {
	state->score = 0;
	state->rank = 3;
	state->result = COURIER_RESULT_FUEL;
	state->phase = COURIER_PHASE_RESULT;
	set_feedback(state, event, COURIER_SFX_FAILURE, 45);
	event->phase_changed = true;
}

static void check_traffic(courier_state_t *state, courier_event_t *event) {
	uint8_t index;
	if (state->collision_cooldown) return;
	for (index = 0; index < COURIER_TRAFFIC_COUNT; ++index) {
		uint8_t traffic_x;
		uint8_t traffic_y;
		courier_traffic_position(state->traffic_turn, index,
			&traffic_x, &traffic_y);
		if (traffic_x == state->x && traffic_y == state->y) {
			state->fuel = state->fuel > COURIER_COLLISION_FUEL ?
				(uint8_t)(state->fuel - COURIER_COLLISION_FUEL) : 0;
			++state->collisions;
			state->collision_cooldown = 2;
			set_feedback(state, event, COURIER_SFX_COLLISION, 18);
			return;
		}
	}
}

void courier_step(courier_state_t *state, const courier_input_t *input,
	uint32_t session_tick, courier_event_t *event) {
	int8_t dx;
	int8_t dy;
	uint8_t nx;
	uint8_t ny;
	uint8_t cost;
	(void)session_tick;
	memset(event, 0, sizeof(*event));

	if (input->retry) {
		courier_reset(state);
		event->sfx = COURIER_SFX_RETRY;
		event->dirty = true;
		event->reset = true;
		return;
	}
	if (state->result != COURIER_RESULT_PLAYING) return;
	if (input->pause) {
		state->phase = state->phase == COURIER_PHASE_PAUSED ?
			COURIER_PHASE_ACTIVE : COURIER_PHASE_PAUSED;
		set_feedback(state, event, COURIER_SFX_PAUSE, 10);
		event->phase_changed = true;
		return;
	}
	if (state->phase != COURIER_PHASE_ACTIVE) return;
	++state->elapsed_frames;
	if (state->feedback_frames) {
		--state->feedback_frames;
		if (!state->feedback_frames) state->feedback = COURIER_SFX_NONE;
		event->dirty = true;
	}

	dx = input->dx;
	dy = input->dy;
	if (dx && dy) dy = 0;
	if (!dx && !dy) return;
	nx = (uint8_t)(state->x + dx);
	ny = (uint8_t)(state->y + dy);
	if (orbital_blocked(nx, ny)) {
		set_feedback(state, event, COURIER_SFX_BLOCKED, 10);
		return;
	}

	state->x = nx;
	state->y = ny;
	++state->steps;
	++state->traffic_turn;
	if (state->collision_cooldown) --state->collision_cooldown;
	cost = movement_cost(nx, ny);
	state->fuel = state->fuel > cost ? (uint8_t)(state->fuel - cost) : 0;
	set_feedback(state, event, COURIER_SFX_MOVE, 3);
	if (state->tutorial == 0) state->tutorial = 1;

	if (!state->charger_used && nx == COURIER_CHARGER_X &&
		ny == COURIER_CHARGER_Y) {
		uint8_t charged = (uint8_t)(state->fuel + COURIER_CHARGE_FUEL);
		state->fuel = charged > COURIER_START_FUEL ? COURIER_START_FUEL : charged;
		state->charger_used = true;
		set_feedback(state, event, COURIER_SFX_CHARGE, 24);
	}
	if (!state->parcel && nx == COURIER_PARCEL_X && ny == COURIER_PARCEL_Y) {
		state->parcel = true;
		state->leg = 1;
		state->tutorial = 2;
		set_feedback(state, event, COURIER_SFX_PICKUP, 24);
	}
	if (state->parcel && !state->relay_complete && nx == COURIER_RELAY_X &&
		ny == COURIER_RELAY_Y) {
		state->relay_complete = true;
		state->leg = 2;
		state->tutorial = 3;
		set_feedback(state, event, COURIER_SFX_RELAY, 24);
	}
	check_traffic(state, event);
	if (state->relay_complete && nx == COURIER_DEPOT_X &&
		ny == COURIER_DEPOT_Y) {
		finish_delivery(state, event);
	} else if (state->fuel == 0) {
		fail_delivery(state, event);
	}
}

uint16_t courier_progress(const courier_state_t *state) {
	if (state->result == COURIER_RESULT_DELIVERED) return 1000;
	if (state->leg == 0) return 0;
	if (state->leg == 1) return 333;
	return 666;
}
