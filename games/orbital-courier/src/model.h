#ifndef SWANSONG_COURIER_MODEL_H
#define SWANSONG_COURIER_MODEL_H

#include <stdbool.h>
#include <stdint.h>

#define COURIER_MAP_COLS 28u
#define COURIER_MAP_ROWS 14u
#define COURIER_START_X 2u
#define COURIER_START_Y 1u
#define COURIER_PARCEL_X 4u
#define COURIER_PARCEL_Y 11u
#define COURIER_RELAY_X 18u
#define COURIER_RELAY_Y 11u
#define COURIER_DEPOT_X 25u
#define COURIER_DEPOT_Y 1u
#define COURIER_CHARGER_X 8u
#define COURIER_CHARGER_Y 4u
#define COURIER_START_FUEL 68u
#define COURIER_CHARGE_FUEL 18u
#define COURIER_COLLISION_FUEL 6u
#define COURIER_TRAFFIC_COUNT 3u

typedef enum {
	COURIER_RESULT_PLAYING = 0,
	COURIER_RESULT_DELIVERED = 1,
	COURIER_RESULT_FUEL = 2
} courier_result_t;

typedef enum {
	COURIER_PHASE_ACTIVE = 0,
	COURIER_PHASE_PAUSED = 1,
	COURIER_PHASE_RESULT = 2
} courier_phase_t;

typedef enum {
	COURIER_SFX_NONE = 0,
	COURIER_SFX_MOVE,
	COURIER_SFX_BLOCKED,
	COURIER_SFX_PICKUP,
	COURIER_SFX_CHARGE,
	COURIER_SFX_RELAY,
	COURIER_SFX_COLLISION,
	COURIER_SFX_PAUSE,
	COURIER_SFX_RETRY,
	COURIER_SFX_DELIVERED,
	COURIER_SFX_FAILURE
} courier_sfx_t;

typedef struct {
	int8_t dx;
	int8_t dy;
	bool pause;
	bool retry;
} courier_input_t;

/*
 * This inventory defines both the production state layout and its canonical
 * hash. Adding gameplay state therefore requires adding it to the trace hash.
 */
#define COURIER_STATE_FIELDS(SCALAR) \
	SCALAR(uint32_t, elapsed_frames, u32) \
	SCALAR(uint16_t, score, u16) \
	SCALAR(uint16_t, traffic_turn, u16) \
	SCALAR(uint8_t, x, u8) \
	SCALAR(uint8_t, y, u8) \
	SCALAR(uint8_t, fuel, u8) \
	SCALAR(uint8_t, steps, u8) \
	SCALAR(uint8_t, collisions, u8) \
	SCALAR(uint8_t, collision_cooldown, u8) \
	SCALAR(uint8_t, leg, u8) \
	SCALAR(uint8_t, tutorial, u8) \
	SCALAR(uint8_t, rank, u8) \
	SCALAR(courier_sfx_t, feedback, u8) \
	SCALAR(uint8_t, feedback_frames, u8) \
	SCALAR(courier_result_t, result, u8) \
	SCALAR(courier_phase_t, phase, u8) \
	SCALAR(bool, parcel, boolean) \
	SCALAR(bool, relay_complete, boolean) \
	SCALAR(bool, charger_used, boolean)

#define COURIER_STATE_DECLARE(type, name, hash_kind) type name;
typedef struct {
	COURIER_STATE_FIELDS(COURIER_STATE_DECLARE)
} courier_state_t;
#undef COURIER_STATE_DECLARE

typedef struct {
	courier_sfx_t sfx;
	bool dirty;
	bool phase_changed;
	bool reset;
} courier_event_t;

bool orbital_blocked(uint8_t x, uint8_t y);
bool orbital_express_lane(uint8_t x, uint8_t y);
void courier_traffic_position(uint16_t turn, uint8_t traffic,
	uint8_t *x, uint8_t *y);
void courier_reset(courier_state_t *state);
void courier_step(courier_state_t *state, const courier_input_t *input,
	uint32_t session_tick, courier_event_t *event);
uint16_t courier_progress(const courier_state_t *state);
uint32_t courier_state_hash(const courier_state_t *state);

#endif
