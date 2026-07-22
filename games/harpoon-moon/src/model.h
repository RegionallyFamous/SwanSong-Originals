#ifndef SWANSONG_HARPOON_MODEL_H
#define SWANSONG_HARPOON_MODEL_H

#include <stdbool.h>
#include <stdint.h>

#define HARPOON_WORLD_MIN 0
#define HARPOON_WORLD_MAX 20
#define HARPOON_START_OXYGEN 2400u
#define HARPOON_MAX_CHARGE 30u
#define HARPOON_SCOUT_COUNT 3u
#define HARPOON_BOSS_HEALTH 3u

typedef enum {
	HARPOON_RESULT_PLAYING = 0,
	HARPOON_RESULT_WIN = 1,
	HARPOON_RESULT_OXYGEN = 2
} harpoon_result_t;

typedef enum {
	HARPOON_PHASE_TUTORIAL = 0,
	HARPOON_PHASE_HUNT = 1,
	HARPOON_PHASE_BOSS_ONE = 2,
	HARPOON_PHASE_BOSS_TWO = 3,
	HARPOON_PHASE_BOSS_THREE = 4,
	HARPOON_PHASE_PAUSED = 5,
	HARPOON_PHASE_RESULT = 6
} harpoon_phase_t;

typedef enum {
	HARPOON_TUTORIAL_MOVE = 0,
	HARPOON_TUTORIAL_LURE = 1,
	HARPOON_TUTORIAL_CHARGE = 2,
	HARPOON_TUTORIAL_COMPLETE = 3
} harpoon_tutorial_t;

typedef enum {
	HARPOON_BEHAVIOR_CALM = 0,
	HARPOON_BEHAVIOR_SHY = 1,
	HARPOON_BEHAVIOR_DIVER = 2,
	HARPOON_BEHAVIOR_TRACK = 3,
	HARPOON_BEHAVIOR_SWEEP = 4,
	HARPOON_BEHAVIOR_ABYSS = 5
} harpoon_behavior_t;

typedef enum {
	HARPOON_SFX_NONE = 0,
	HARPOON_SFX_MOVE = 1,
	HARPOON_SFX_LURE = 2,
	HARPOON_SFX_CHARGE = 3,
	HARPOON_SFX_READY = 4,
	HARPOON_SFX_SHOT = 5,
	HARPOON_SFX_HIT = 6,
	HARPOON_SFX_TAG = 7,
	HARPOON_SFX_BOSS = 8,
	HARPOON_SFX_MISS = 9,
	HARPOON_SFX_RECOVER = 10,
	HARPOON_SFX_PAUSE = 11,
	HARPOON_SFX_RETRY = 12,
	HARPOON_SFX_WIN = 13,
	HARPOON_SFX_FAILURE = 14
} harpoon_sfx_t;

typedef enum {
	HARPOON_MUSIC_KEEP = 0,
	HARPOON_MUSIC_TITLE = 1,
	HARPOON_MUSIC_HUNT = 2,
	HARPOON_MUSIC_BOSS = 3,
	HARPOON_MUSIC_RESULT = 4
} harpoon_music_t;

/*
 * This inventory is also consumed by the native hash-coverage test. Keep every
 * deterministic gameplay field here so reset/replay evidence cannot omit new
 * state by accident.
 */
#define HARPOON_STATE_FIELDS(X) \
	X(uint32_t, elapsed_frames) \
	X(uint16_t, random_state) \
	X(uint16_t, oxygen) \
	X(uint16_t, score) \
	X(uint16_t, phase_frames) \
	X(uint16_t, behavior_frames) \
	X(uint8_t, skiff) \
	X(uint8_t, creature) \
	X(uint8_t, creature_depth) \
	X(uint8_t, tags) \
	X(uint8_t, boss_hp) \
	X(uint8_t, charge) \
	X(uint8_t, lure_meter) \
	X(uint8_t, lure_window) \
	X(uint8_t, recovery_frames) \
	X(uint8_t, shots) \
	X(uint8_t, misses) \
	X(uint8_t, streak) \
	X(uint8_t, rank) \
	X(uint8_t, feedback_frames) \
	X(uint8_t, hit_flash) \
	X(int8_t, creature_direction) \
	X(harpoon_result_t, result) \
	X(harpoon_phase_t, phase) \
	X(harpoon_phase_t, resume_phase) \
	X(harpoon_tutorial_t, tutorial) \
	X(harpoon_behavior_t, behavior) \
	X(harpoon_sfx_t, feedback)

#define HARPOON_DECLARE_FIELD(type, name) type name;
typedef struct {
	HARPOON_STATE_FIELDS(HARPOON_DECLARE_FIELD)
} harpoon_state_t;
#undef HARPOON_DECLARE_FIELD

typedef struct {
	int8_t direction;
	bool charge_held;
	bool lure_held;
	bool fire_released;
	bool pause;
	bool retry;
} harpoon_input_t;

typedef struct {
	harpoon_sfx_t sfx;
	harpoon_music_t music;
	bool reset_session;
	bool phase_changed;
	bool dirty;
} harpoon_event_t;

void harpoon_reset(harpoon_state_t *state);
void harpoon_step(harpoon_state_t *state, const harpoon_input_t *input,
	uint32_t session_tick, harpoon_event_t *event);

uint8_t harpoon_min_charge(const harpoon_state_t *state);
uint8_t harpoon_max_charge(const harpoon_state_t *state);
uint8_t harpoon_lure_target(const harpoon_state_t *state);
uint16_t harpoon_progress(const harpoon_state_t *state);
uint32_t harpoon_state_hash(const harpoon_state_t *state);

#endif
