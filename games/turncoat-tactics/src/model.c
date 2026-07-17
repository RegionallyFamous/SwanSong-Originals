#include <string.h>

#include "model.h"

static uint8_t clamp_u8(int16_t value, uint8_t low, uint8_t high) {
	if (value < low) return low;
	if (value > high) return high;
	return (uint8_t)value;
}

static uint8_t distance(const unit_t *unit, int8_t x, int8_t y) {
	uint8_t dx = unit->x > x ? (uint8_t)(unit->x - x) : (uint8_t)(x - unit->x);
	uint8_t dy = unit->y > y ? (uint8_t)(unit->y - y) : (uint8_t)(y - unit->y);
	return (uint8_t)(dx + dy);
}

static int8_t ally_at(const turncoat_state_t *state, int8_t x, int8_t y) {
	uint8_t i;
	for (i = 0; i < ALLY_CAPACITY; ++i) {
		if (state->allies[i].hp && state->allies[i].x == x && state->allies[i].y == y)
			return (int8_t)i;
	}
	return -1;
}

static int8_t enemy_at(const turncoat_state_t *state, int8_t x, int8_t y) {
	uint8_t i;
	for (i = 0; i < ENEMY_CAPACITY; ++i) {
		if (state->enemies[i].hp && state->enemies[i].x == x && state->enemies[i].y == y)
			return (int8_t)i;
	}
	return -1;
}

uint8_t turncoat_enemy_count(const turncoat_state_t *state) {
	uint8_t count = 0;
	uint8_t i;
	for (i = 0; i < ENEMY_CAPACITY; ++i) if (state->enemies[i].hp) ++count;
	return count;
}

static int8_t empty_recruit_slot(const turncoat_state_t *state) {
	uint8_t i;
	for (i = 3; i < ALLY_CAPACITY; ++i) if (!state->allies[i].hp) return (int8_t)i;
	return -1;
}

static uint8_t first_living_ally(const turncoat_state_t *state) {
	uint8_t i;
	for (i = 0; i < ALLY_CAPACITY; ++i) if (state->allies[i].hp) return i;
	return 0;
}

bool turncoat_beacon_secured(const turncoat_state_t *state) {
	uint8_t i;
	for (i = 0; i < ALLY_CAPACITY; ++i) {
		if (state->allies[i].hp && state->allies[i].x == 7 && state->allies[i].y == 2)
			return true;
	}
	return false;
}

static void enemy_turn(turncoat_state_t *state) {
	uint8_t i;
	for (i = 0; i < ENEMY_CAPACITY; ++i) {
		uint8_t ally;
		bool attacked = false;
		if (!state->enemies[i].hp) continue;
		for (ally = 0; ally < ALLY_CAPACITY; ++ally) {
			if (state->allies[ally].hp &&
				distance(&state->enemies[i], state->allies[ally].x,
					state->allies[ally].y) == 1) {
				--state->allies[ally].hp;
				attacked = true;
				break;
			}
		}
		if (!attacked && state->enemies[i].x > 0 &&
			ally_at(state, (int8_t)(state->enemies[i].x - 1), state->enemies[i].y) < 0 &&
			enemy_at(state, (int8_t)(state->enemies[i].x - 1), state->enemies[i].y) < 0) {
			--state->enemies[i].x;
		}
	}
}

void turncoat_reset(turncoat_state_t *state) {
	memset(state, 0, sizeof(*state));
	state->allies[0] = (unit_t){0, 2, 3};
	state->allies[1] = (unit_t){0, 1, 2};
	state->allies[2] = (unit_t){0, 3, 2};
	state->enemies[0] = (unit_t){4, 0, 2};
	state->enemies[1] = (unit_t){5, 2, 2};
	state->enemies[2] = (unit_t){4, 4, 1};
	state->enemies[3] = (unit_t){6, 3, 1};
	state->cursor_y = 2;
	state->turns = 18;
}

void turncoat_step(turncoat_state_t *state, const turncoat_input_t *input,
	turncoat_event_t *event) {
	bool acted = false;
	memset(event, 0, sizeof(*event));
	if (state->result != TURNCOAT_RESULT_PLAYING && input->replay) {
		turncoat_reset(state);
		event->reset_session = true;
		event->dirty = true;
		return;
	}
	if (state->result != TURNCOAT_RESULT_PLAYING) return;
	if (input->dx || input->dy) {
		state->cursor_x = clamp_u8((int16_t)state->cursor_x + input->dx, 0, 7);
		state->cursor_y = clamp_u8((int16_t)state->cursor_y + input->dy, 0, 5);
		event->dirty = true;
	}
	if (input->select_or_act) {
		int8_t ai = ally_at(state, (int8_t)state->cursor_x, (int8_t)state->cursor_y);
		int8_t ei = enemy_at(state, (int8_t)state->cursor_x, (int8_t)state->cursor_y);
		if (ai >= 0) state->selected = (uint8_t)ai;
		else if (distance(&state->allies[state->selected],
			(int8_t)state->cursor_x, (int8_t)state->cursor_y) == 1) {
			if (ei >= 0) --state->enemies[(uint8_t)ei].hp;
			else {
				state->allies[state->selected].x = (int8_t)state->cursor_x;
				state->allies[state->selected].y = (int8_t)state->cursor_y;
			}
			acted = true;
		}
		event->dirty = true;
	}
	if (input->recruit) {
		int8_t ei = enemy_at(state, (int8_t)state->cursor_x, (int8_t)state->cursor_y);
		int8_t slot = empty_recruit_slot(state);
		if (ei >= 0 && state->enemies[(uint8_t)ei].hp == 1 && slot >= 0 &&
			distance(&state->allies[state->selected],
				(int8_t)state->cursor_x, (int8_t)state->cursor_y) == 1) {
			state->allies[(uint8_t)slot] = (unit_t){state->enemies[(uint8_t)ei].x,
				state->enemies[(uint8_t)ei].y, 1};
			state->enemies[(uint8_t)ei].hp = 0;
			++state->recruits;
			acted = true;
			event->tone_hz = 580;
			event->tone_frames = 10;
			event->dirty = true;
		}
	}
	if (input->end_turn) acted = true;
	if (acted) {
		if (state->turns) --state->turns;
		enemy_turn(state);
		if (!state->allies[state->selected].hp) state->selected = first_living_ally(state);
		event->dirty = true;
	}
	if (turncoat_beacon_secured(state) || turncoat_enemy_count(state) == 0)
		state->result = TURNCOAT_RESULT_WIN;
	else if (state->allies[0].hp == 0 || state->turns == 0)
		state->result = TURNCOAT_RESULT_LOSS;
}
