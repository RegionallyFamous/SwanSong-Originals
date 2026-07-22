#ifndef SWANSONG_RADIO_MODEL_H
#define SWANSONG_RADIO_MODEL_H

#include <stdbool.h>
#include <stdint.h>

#define RADIO_SIGNAL_COUNT 3u
#define RADIO_NIGHT_FRAMES 5400u
#define RADIO_BAD_LOCK_PENALTY 450u
#define RADIO_TITLE_ATTRACT_FRAMES 755u
#define RADIO_FREQUENCY_MIN 880u
#define RADIO_FREQUENCY_MAX 1080u

typedef enum {
	RADIO_RESULT_PLAYING = 0,
	RADIO_RESULT_SIGNAL = 1,
	RADIO_RESULT_DAWN = 2
} radio_result_t;

typedef enum {
	RADIO_MODE_TITLE = 0,
	RADIO_MODE_TUTORIAL = 1,
	RADIO_MODE_NIGHT = 2
} radio_mode_t;

typedef enum {
	RADIO_TUTORIAL_TUNE = 0,
	RADIO_TUTORIAL_GAIN = 1,
	RADIO_TUTORIAL_GATE = 2,
	RADIO_TUTORIAL_LOCK = 3,
	RADIO_TUTORIAL_READY = 4
} radio_tutorial_step_t;

typedef enum {
	RADIO_SFX_NONE = 0,
	RADIO_SFX_TUNE_LEFT,
	RADIO_SFX_TUNE_RIGHT,
	RADIO_SFX_GAIN,
	RADIO_SFX_GATE,
	RADIO_SFX_CONFIRM,
	RADIO_SFX_LOCK,
	RADIO_SFX_ERROR,
	RADIO_SFX_COMPLETE,
	RADIO_SFX_FAILURE,
	RADIO_SFX_PAUSE
} radio_sfx_t;

typedef struct {
	int8_t frequency_direction;
	int8_t gain_direction;
	int8_t menu_direction;
	bool lock;
	bool toggle_gate;
	bool confirm;
	bool pause;
} radio_input_t;

/*
 * One inventory defines both the state layout and the canonical deterministic
 * hash. Adding gameplay state therefore requires adding it to the trace hash.
 */
#define RADIO_STATE_FIELDS(SCALAR, ARRAY) \
	SCALAR(uint16_t, frequency, u16) \
	SCALAR(uint16_t, time, u16) \
	SCALAR(uint16_t, score, u16) \
	SCALAR(uint16_t, title_ticks, u16) \
	SCALAR(uint8_t, gain, u8) \
	SCALAR(uint8_t, clue, u8) \
	SCALAR(uint8_t, title_choice, u8) \
	SCALAR(radio_tutorial_step_t, tutorial_step, u8) \
	SCALAR(uint8_t, narrow_locks, u8) \
	SCALAR(uint8_t, wrong_locks, u8) \
	SCALAR(uint8_t, feedback_flash, u8) \
	SCALAR(uint8_t, lock_quality, u8) \
	SCALAR(int8_t, last_direction, i8) \
	SCALAR(radio_result_t, result, u8) \
	SCALAR(radio_mode_t, mode, u8) \
	SCALAR(bool, gate, boolean) \
	SCALAR(bool, paused, boolean) \
	SCALAR(bool, attract, boolean)

#define RADIO_STATE_DECLARE_SCALAR(type, name, hash_kind) type name;
#define RADIO_STATE_DECLARE_ARRAY(type, name, count, hash_kind) type name[count];
typedef struct {
	RADIO_STATE_FIELDS(RADIO_STATE_DECLARE_SCALAR, RADIO_STATE_DECLARE_ARRAY)
} radio_state_t;
#undef RADIO_STATE_DECLARE_ARRAY
#undef RADIO_STATE_DECLARE_SCALAR

typedef struct {
	radio_sfx_t sfx;
	int8_t pan;
	bool start_tutorial;
	bool start_night;
	bool show_result;
	bool pause_changed;
	bool dirty;
} radio_event_t;

uint16_t radio_target_for(uint8_t clue);
uint8_t radio_ideal_gain_for(uint8_t clue);
uint8_t radio_noise_level(const radio_state_t *state);
uint8_t radio_signal_strength(const radio_state_t *state);
int8_t radio_target_direction(const radio_state_t *state);
uint16_t radio_state_progress(const radio_state_t *state);

void radio_reset_title(radio_state_t *state);
void radio_reset_tutorial(radio_state_t *state);
void radio_reset_night(radio_state_t *state);
void radio_set_paused(radio_state_t *state, bool paused);
void radio_step(radio_state_t *state, const radio_input_t *input,
	radio_event_t *event);
uint32_t radio_state_hash(const radio_state_t *state);

#endif
