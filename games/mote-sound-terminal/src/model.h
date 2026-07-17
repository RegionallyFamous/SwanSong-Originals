#ifndef SWANSONG_MOTE_MODEL_H
#define SWANSONG_MOTE_MODEL_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
	uint8_t track;
	uint8_t tempo;
	uint8_t scope;
	uint8_t step;
	uint8_t tick;
	bool playing;
} mote_state_t;

typedef struct {
	int8_t track_direction;
	int8_t tempo_direction;
	bool toggle_play;
	bool toggle_scope;
	bool reset;
} mote_input_t;

typedef struct {
	uint16_t tone_hz;
	uint8_t tone_volume;
	bool sound_off;
	bool reset_session;
	bool dirty;
} mote_event_t;

uint16_t mote_note_hz(uint8_t track, uint8_t step);
void mote_reset(mote_state_t *state);
void mote_step(mote_state_t *state, const mote_input_t *input,
	mote_event_t *event);

#endif
