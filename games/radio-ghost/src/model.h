#ifndef SWANSONG_RADIO_MODEL_H
#define SWANSONG_RADIO_MODEL_H

#include <stdbool.h>
#include <stdint.h>

#define RADIO_NIGHT_FRAMES 4500u

typedef enum {
	RADIO_RESULT_PLAYING = 0,
	RADIO_RESULT_SIGNAL = 1,
	RADIO_RESULT_DAWN = 2
} radio_result_t;

typedef struct {
	uint16_t frequency;
	uint16_t time;
	uint8_t gain;
	uint8_t clue;
	radio_result_t result;
	bool gate;
} radio_state_t;

typedef struct {
	int8_t frequency_direction;
	int8_t gain_direction;
	bool lock;
	bool toggle_gate;
	bool replay;
} radio_input_t;

typedef struct {
	uint16_t tone_hz;
	uint8_t tone_frames;
	bool reset_session;
	bool dirty;
} radio_event_t;

uint16_t radio_target_for(uint8_t clue);
void radio_reset(radio_state_t *state);
void radio_step(radio_state_t *state, const radio_input_t *input,
	radio_event_t *event);

#endif
