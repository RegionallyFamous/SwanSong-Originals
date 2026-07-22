#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "model.h"

static uint32_t tick;

static courier_event_t step(courier_state_t *state, int8_t dx, int8_t dy) {
	courier_input_t input = {dx, dy, false, false};
	courier_event_t event;
	courier_step(state, &input, tick++, &event);
	return event;
}

static courier_event_t command(courier_state_t *state, bool pause, bool retry) {
	courier_input_t input = {0, 0, pause, retry};
	courier_event_t event;
	courier_step(state, &input, tick++, &event);
	return event;
}

static void move_n(courier_state_t *state, int8_t dx, int8_t dy,
	uint8_t count) {
	uint8_t index;
	for (index = 0; index < count; ++index) {
		courier_event_t event = step(state, dx, dy);
		assert(event.dirty);
	}
}

static void assert_state_hash_coverage(void) {
	courier_state_t baseline;
	courier_state_t changed;
	uint32_t baseline_hash;
	courier_reset(&baseline);
	baseline_hash = courier_state_hash(&baseline);
	changed = baseline;
	assert(courier_state_hash(&changed) == baseline_hash);
#define ASSERT_HASH_MUTATION(statement) do { \
	changed = baseline; \
	statement; \
	assert(courier_state_hash(&changed) != baseline_hash); \
} while (0)
	ASSERT_HASH_MUTATION(changed.elapsed_frames ^= 1u);
	ASSERT_HASH_MUTATION(changed.score ^= 1u);
	ASSERT_HASH_MUTATION(changed.traffic_turn ^= 1u);
	ASSERT_HASH_MUTATION(changed.x ^= 1u);
	ASSERT_HASH_MUTATION(changed.y ^= 1u);
	ASSERT_HASH_MUTATION(changed.fuel ^= 1u);
	ASSERT_HASH_MUTATION(changed.steps ^= 1u);
	ASSERT_HASH_MUTATION(changed.collisions ^= 1u);
	ASSERT_HASH_MUTATION(changed.collision_cooldown ^= 1u);
	ASSERT_HASH_MUTATION(changed.leg ^= 1u);
	ASSERT_HASH_MUTATION(changed.tutorial ^= 1u);
	ASSERT_HASH_MUTATION(changed.rank ^= 1u);
	ASSERT_HASH_MUTATION(changed.feedback = COURIER_SFX_MOVE);
	ASSERT_HASH_MUTATION(changed.feedback_frames ^= 1u);
	ASSERT_HASH_MUTATION(changed.result = COURIER_RESULT_DELIVERED);
	ASSERT_HASH_MUTATION(changed.phase = COURIER_PHASE_PAUSED);
	ASSERT_HASH_MUTATION(changed.parcel = !changed.parcel);
	ASSERT_HASH_MUTATION(changed.relay_complete = !changed.relay_complete);
	ASSERT_HASH_MUTATION(changed.charger_used = !changed.charger_used);
#undef ASSERT_HASH_MUTATION
}

static courier_state_t finish_delivery(bool use_charger) {
	courier_state_t state;
	courier_reset(&state);
	if (use_charger) {
		move_n(&state, 1, 0, 6);
		move_n(&state, 0, 1, 3);
		assert(state.x == COURIER_CHARGER_X && state.y == COURIER_CHARGER_Y);
		assert(state.charger_used && state.fuel == COURIER_START_FUEL);
		move_n(&state, 0, 1, 1);
		move_n(&state, -1, 0, 2);
		move_n(&state, 0, 1, 6);
		move_n(&state, -1, 0, 2);
	} else {
		move_n(&state, 0, 1, 7);
		move_n(&state, 1, 0, 4);
		move_n(&state, 0, 1, 3);
		move_n(&state, -1, 0, 2);
	}
	assert(state.x == COURIER_PARCEL_X && state.y == COURIER_PARCEL_Y);
	assert(state.parcel && state.leg == 1 && state.tutorial == 2);

	move_n(&state, 1, 0, 2);
	move_n(&state, 0, -1, 4);
	move_n(&state, 1, 0, 10);
	move_n(&state, 0, 1, 4);
	move_n(&state, 1, 0, 2);
	assert(state.x == COURIER_RELAY_X && state.y == COURIER_RELAY_Y);
	assert(state.relay_complete && state.leg == 2 && state.tutorial == 3);

	move_n(&state, 0, -1, 1);
	move_n(&state, 1, 0, 7);
	move_n(&state, 0, -1, 9);
	assert(state.x == COURIER_DEPOT_X && state.y == COURIER_DEPOT_Y);
	assert(state.result == COURIER_RESULT_DELIVERED);
	assert(state.phase == COURIER_PHASE_RESULT && state.leg == 3);
	assert(state.score > 0 && state.rank <= 3);
	assert(courier_progress(&state) == 1000);
	return state;
}

int main(void) {
	courier_state_t state;
	courier_state_t reset_copy;
	courier_state_t charged;
	courier_state_t direct;
	courier_state_t repeated;
	courier_event_t event;
	uint8_t traffic_x;
	uint8_t traffic_y;
	uint8_t index;

	assert_state_hash_coverage();
	courier_reset(&state);
	reset_copy = state;
	assert(state.x == COURIER_START_X && state.y == COURIER_START_Y);
	assert(state.fuel == COURIER_START_FUEL && state.tutorial == 0);
	assert(orbital_blocked(0, 5) && orbital_blocked(27, 5));
	assert(orbital_blocked(5, 4) && !orbital_blocked(8, 4));
	assert(orbital_blocked(12, 6) && !orbital_blocked(12, 7));
	assert(orbital_express_lane(12, 7) && !orbital_express_lane(11, 7));

	event = step(&state, 0, -1);
	assert(event.sfx == COURIER_SFX_BLOCKED);
	assert(state.x == COURIER_START_X && state.y == COURIER_START_Y);
	assert(state.fuel == COURIER_START_FUEL && state.steps == 0);
	assert(state.traffic_turn == 0);

	courier_reset(&state);
	state.x = 11;
	state.y = 7;
	event = step(&state, 1, 0);
	assert(event.sfx == COURIER_SFX_MOVE);
	assert(state.fuel == COURIER_START_FUEL - 2u);

	courier_reset(&state);
	state.x = 5;
	state.y = 2;
	courier_traffic_position(1, 0, &traffic_x, &traffic_y);
	assert(traffic_x == 6 && traffic_y == 2);
	event = step(&state, 1, 0);
	assert(event.sfx == COURIER_SFX_COLLISION);
	assert(state.collisions == 1 && state.collision_cooldown == 2);
	assert(state.fuel == COURIER_START_FUEL - 1u - COURIER_COLLISION_FUEL);

	courier_reset(&state);
	event = command(&state, true, false);
	assert(event.sfx == COURIER_SFX_PAUSE && state.phase == COURIER_PHASE_PAUSED);
	repeated = state;
	event = step(&state, 1, 0);
	assert(!event.dirty && memcmp(&state, &repeated, sizeof(state)) == 0);
	event = command(&state, true, false);
	assert(event.phase_changed && state.phase == COURIER_PHASE_ACTIVE);
	(void)step(&state, 1, 0);
	event = command(&state, false, true);
	assert(event.reset && event.sfx == COURIER_SFX_RETRY);
	assert(memcmp(&state, &reset_copy, sizeof(state)) == 0);

	tick = 0;
	charged = finish_delivery(true);
	assert(charged.steps == 59 && charged.fuel == 13);
	assert(charged.charger_used && charged.collisions == 0);
	tick = 0;
	direct = finish_delivery(false);
	assert(direct.steps == 55 && direct.fuel == 8);
	assert(!direct.charger_used && direct.collisions == 0);
	assert(charged.fuel > direct.fuel);

	tick = 0;
	repeated = finish_delivery(true);
	assert(memcmp(&charged, &repeated, sizeof(charged)) == 0);
	assert(courier_state_hash(&charged) == courier_state_hash(&repeated));

	courier_reset(&state);
	for (index = 0; index < COURIER_START_FUEL; ++index) {
		event = step(&state, state.x == COURIER_START_X ? 1 : -1, 0);
	}
	assert(event.sfx == COURIER_SFX_FAILURE);
	assert(state.result == COURIER_RESULT_FUEL && state.fuel == 0);
	assert(state.phase == COURIER_PHASE_RESULT && state.score == 0);
	assert(state.steps == COURIER_START_FUEL && courier_progress(&state) == 0);
	return 0;
}
