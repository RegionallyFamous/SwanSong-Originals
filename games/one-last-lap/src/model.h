#ifndef SWANSONG_LAP_MODEL_H
#define SWANSONG_LAP_MODEL_H

#include <stdbool.h>
#include <stdint.h>

#define LAP_RIVAL_COUNT 3u
#define LAP_TOTAL_LAPS 3u
#define LAP_TRACK_UNITS 9000u
#define LAP_START_BATTERY 6800u
#define LAP_COLLISION_BATTERY_COST 180u
#define LAP_MAX_SPEED 12u
#define LAP_COUNTDOWN_STEP_FRAMES 75u

typedef enum {
	LAP_RESULT_PLAYING = 0,
	LAP_RESULT_SOLO = 1,
	LAP_RESULT_BATTERY = 2,
	LAP_RESULT_COOPERATIVE = 3
} lap_result_t;

typedef enum {
	LAP_PHASE_COUNTDOWN = 0,
	LAP_PHASE_RACING = 1,
	LAP_PHASE_PAUSED = 2,
	LAP_PHASE_RESULT = 3
} lap_phase_t;

typedef enum {
	LAP_SFX_NONE = 0,
	LAP_SFX_LANE,
	LAP_SFX_COUNTDOWN,
	LAP_SFX_GO,
	LAP_SFX_COLLISION,
	LAP_SFX_OVERTAKE,
	LAP_SFX_CHECKPOINT,
	LAP_SFX_TOW,
	LAP_SFX_FINISH,
	LAP_SFX_FAILURE,
	LAP_SFX_PAUSE
} lap_sfx_t;

typedef struct {
	int8_t lane_direction;
	bool accelerate;
	bool brake;
	bool tow;
	bool pause;
} lap_input_t;

/*
 * This single field inventory defines both the state layout and its canonical
 * deterministic hash. New gameplay state therefore cannot be added to the
 * model without also entering the hash expansion in model.c.
 */
#define LAP_STATE_FIELDS(SCALAR, ARRAY) \
	SCALAR(uint32_t, distance_total, u32) \
	ARRAY(uint32_t, rival_distance, LAP_RIVAL_COUNT, u32) \
	SCALAR(uint32_t, elapsed_frames, u32) \
	SCALAR(uint16_t, battery, u16) \
	SCALAR(uint16_t, score, u16) \
	SCALAR(uint16_t, phase_frames, u16) \
	SCALAR(uint16_t, collision_cooldown, u16) \
	SCALAR(uint16_t, hazard_mask, u16) \
	SCALAR(uint8_t, lap, u8) \
	SCALAR(uint8_t, progress, u8) \
	SCALAR(uint8_t, speed, u8) \
	SCALAR(uint8_t, lane, u8) \
	ARRAY(uint8_t, rival_lane, LAP_RIVAL_COUNT, u8) \
	SCALAR(uint8_t, overtakes, u8) \
	SCALAR(uint8_t, collisions, u8) \
	SCALAR(uint8_t, rank, u8) \
	SCALAR(uint8_t, countdown, u8) \
	SCALAR(lap_result_t, result, u8) \
	SCALAR(lap_phase_t, phase, u8) \
	SCALAR(lap_phase_t, resume_phase, u8) \
	SCALAR(bool, helped, boolean) \
	SCALAR(bool, tow_available, boolean) \
	SCALAR(bool, checkpoint_flash, boolean)

#define LAP_STATE_DECLARE_SCALAR(type, name, hash_kind) type name;
#define LAP_STATE_DECLARE_ARRAY(type, name, count, hash_kind) type name[count];
typedef struct {
	LAP_STATE_FIELDS(LAP_STATE_DECLARE_SCALAR, LAP_STATE_DECLARE_ARRAY)
} lap_state_t;
#undef LAP_STATE_DECLARE_ARRAY
#undef LAP_STATE_DECLARE_SCALAR

typedef struct {
	lap_sfx_t sfx;
	uint16_t tone_hz;
	uint8_t tone_frames;
	bool dirty;
	bool phase_changed;
} lap_event_t;

void lap_reset(lap_state_t *state);
void lap_step(lap_state_t *state, const lap_input_t *input,
	uint32_t session_tick, lap_event_t *event);
uint8_t lap_rival_lane(uint32_t rival_distance, uint8_t rival);
bool lap_rival_visible(const lap_state_t *state, uint8_t rival);
int16_t lap_rival_delta(const lap_state_t *state, uint8_t rival);
uint8_t lap_hazard_lane(uint8_t lap_index, uint8_t hazard);
uint16_t lap_hazard_position(uint8_t hazard);
uint32_t lap_state_hash(const lap_state_t *state);

#endif
