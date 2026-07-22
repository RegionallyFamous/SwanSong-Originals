#include <assert.h>
#include <string.h>

#include "model.h"

static const lap_input_t idle = {0, false, false, false, false};

static void finish_countdown(lap_state_t *state) {
	lap_event_t event;
	uint16_t frame;
	for (frame = 0; frame < LAP_COUNTDOWN_STEP_FRAMES * 3u; ++frame)
		lap_step(state, &idle, frame, &event);
	assert(state->phase == LAP_PHASE_RACING && state->countdown == 0);
	assert(event.sfx == LAP_SFX_GO);
}

static uint8_t safe_lane(const lap_state_t *state) {
	uint8_t lane = state->lane;
	uint8_t rival;
	for (rival = 0; rival < LAP_RIVAL_COUNT; ++rival) {
		int16_t delta = lap_rival_delta(state, rival);
		if (delta >= -40 && delta <= 180 && state->rival_lane[rival] == lane)
			lane = (uint8_t)((lane + 1u) % 3u);
	}
	return lane;
}

static void assert_state_hash_coverage(void) {
	lap_state_t baseline;
	lap_state_t changed;
	uint32_t baseline_hash;
	lap_reset(&baseline);
	baseline_hash = lap_state_hash(&baseline);
	changed = baseline;
	assert(lap_state_hash(&changed) == baseline_hash);
#define ASSERT_HASH_MUTATION(statement) do { \
	changed = baseline; \
	statement; \
	assert(lap_state_hash(&changed) != baseline_hash); \
} while (0)
	ASSERT_HASH_MUTATION(changed.distance_total ^= 1u);
	ASSERT_HASH_MUTATION(changed.rival_distance[0] ^= 1u);
	ASSERT_HASH_MUTATION(changed.rival_distance[1] ^= 1u);
	ASSERT_HASH_MUTATION(changed.rival_distance[2] ^= 1u);
	ASSERT_HASH_MUTATION(changed.elapsed_frames ^= 1u);
	ASSERT_HASH_MUTATION(changed.battery ^= 1u);
	ASSERT_HASH_MUTATION(changed.score ^= 1u);
	ASSERT_HASH_MUTATION(changed.phase_frames ^= 1u);
	ASSERT_HASH_MUTATION(changed.collision_cooldown ^= 1u);
	ASSERT_HASH_MUTATION(changed.hazard_mask ^= 1u);
	ASSERT_HASH_MUTATION(changed.lap ^= 1u);
	ASSERT_HASH_MUTATION(changed.progress ^= 1u);
	ASSERT_HASH_MUTATION(changed.speed ^= 1u);
	ASSERT_HASH_MUTATION(changed.lane ^= 1u);
	ASSERT_HASH_MUTATION(changed.rival_lane[0] ^= 1u);
	ASSERT_HASH_MUTATION(changed.rival_lane[1] ^= 1u);
	ASSERT_HASH_MUTATION(changed.rival_lane[2] ^= 1u);
	ASSERT_HASH_MUTATION(changed.overtakes ^= 1u);
	ASSERT_HASH_MUTATION(changed.collisions ^= 1u);
	ASSERT_HASH_MUTATION(changed.rank ^= 1u);
	ASSERT_HASH_MUTATION(changed.countdown ^= 1u);
	ASSERT_HASH_MUTATION(changed.result = LAP_RESULT_SOLO);
	ASSERT_HASH_MUTATION(changed.phase = LAP_PHASE_RACING);
	ASSERT_HASH_MUTATION(changed.resume_phase = LAP_PHASE_RACING);
	ASSERT_HASH_MUTATION(changed.helped = !changed.helped);
	ASSERT_HASH_MUTATION(changed.tow_available = !changed.tow_available);
	ASSERT_HASH_MUTATION(changed.checkpoint_flash = !changed.checkpoint_flash);
#undef ASSERT_HASH_MUTATION
}

static lap_state_t finish_race(bool tow) {
	lap_state_t state;
	lap_input_t input;
	lap_event_t event;
	uint32_t frame = 0;
	lap_reset(&state);
	finish_countdown(&state);
	while (state.result == LAP_RESULT_PLAYING && frame < 20000u) {
		uint8_t target_lane = safe_lane(&state);
		memset(&input, 0, sizeof(input));
		input.accelerate = true;
		if (tow && !state.helped && state.lap == 2 &&
			state.progress >= 34 && state.progress <= 58) {
			target_lane = 0;
			if (state.speed > 8) {
				input.accelerate = false;
				input.brake = true;
			}
			if (state.tow_available && state.progress >= 44 &&
				state.speed <= 8 && state.lane == 0) input.tow = true;
		}
		if (target_lane < state.lane) input.lane_direction = -1;
		else if (target_lane > state.lane) input.lane_direction = 1;
		lap_step(&state, &input, frame++, &event);
	}
	assert(frame < 20000u);
	return state;
}

int main(void) {
	lap_state_t state;
	lap_state_t initial;
	lap_state_t completed;
	lap_state_t repeated;
	lap_input_t input = idle;
	lap_event_t event;
	uint32_t frozen_time;
	uint16_t frozen_battery;
	uint16_t cooperative_score;
	uint16_t solo_score;
	uint8_t cooperative_rank;
	uint8_t solo_rank;
	uint32_t frame;
	assert_state_hash_coverage();

	lap_reset(&state);
	assert(state.lap == 1 && state.battery == LAP_START_BATTERY &&
		state.lane == 1 && state.phase == LAP_PHASE_COUNTDOWN &&
		state.countdown == 3);
	initial = state;
	state.distance_total = 999;
	lap_reset(&state);
	assert(memcmp(&state, &initial, sizeof(state)) == 0);

	finish_countdown(&state);
	input.lane_direction = -1;
	lap_step(&state, &input, 1, &event);
	assert(state.lane == 0 && event.sfx == LAP_SFX_LANE);
	input = idle;
	input.pause = true;
	lap_step(&state, &input, 2, &event);
	assert(state.phase == LAP_PHASE_PAUSED && event.sfx == LAP_SFX_PAUSE);
	frozen_time = state.elapsed_frames;
	frozen_battery = state.battery;
	lap_step(&state, &idle, 3, &event);
	assert(state.elapsed_frames == frozen_time && state.battery == frozen_battery);
	lap_step(&state, &input, 4, &event);
	assert(state.phase == LAP_PHASE_RACING);

	/* A nearby rival produces a real positional collision. */
	lap_reset(&state);
	finish_countdown(&state);
	state.distance_total = 1030u;
	state.speed = 6;
	state.elapsed_frames = 3;
	state.rival_distance[0] = state.distance_total + 12u;
	state.rival_lane[0] = state.lane;
	lap_step(&state, &idle, 5, &event);
	assert(state.collisions == 1 && state.speed == 3 &&
		state.battery == LAP_START_BATTERY -
			(LAP_COLLISION_BATTERY_COST + 1u) &&
		event.sfx == LAP_SFX_COLLISION);

	/* Crossing a slower rival is counted as mastery, not decorative motion. */
	lap_reset(&state);
	finish_countdown(&state);
	state.speed = LAP_MAX_SPEED;
	state.elapsed_frames = 3;
	state.rival_distance[0] = state.distance_total + 1u;
	state.rival_lane[0] = (uint8_t)((state.lane + 1u) % 3u);
	lap_step(&state, &idle, 6, &event);
	assert(state.overtakes == 1 && event.sfx == LAP_SFX_OVERTAKE);

	completed = finish_race(true);
	repeated = finish_race(true);
	assert(completed.result == LAP_RESULT_COOPERATIVE && completed.helped);
	assert(completed.elapsed_frames >= 75u * 115u &&
		completed.elapsed_frames <= 75u * 240u);
	assert(completed.battery >= 1000u);
	assert(completed.score > 0 && completed.rank <= 3);
	cooperative_score = completed.score;
	cooperative_rank = completed.rank;
	assert(memcmp(&completed, &repeated, sizeof(completed)) == 0);
	completed = finish_race(false);
	assert(completed.result == LAP_RESULT_SOLO && !completed.helped);
	solo_score = completed.score;
	solo_rank = completed.rank;
	assert(cooperative_score > solo_score);

	/* Conflicting throttle and brake is an intentional battery failure path. */
	lap_reset(&state);
	finish_countdown(&state);
	input = idle;
	input.accelerate = true;
	input.brake = true;
	for (frame = 0; frame < 3000u && state.result == LAP_RESULT_PLAYING; ++frame)
		lap_step(&state, &input, frame, &event);
	assert(state.result == LAP_RESULT_BATTERY && state.speed == 0);
	assert(event.sfx == LAP_SFX_FAILURE);
	assert(state.score < cooperative_score && state.score < solo_score);
	assert(state.rank >= cooperative_rank && state.rank >= solo_rank);

	return 0;
}
