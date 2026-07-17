#ifndef SWANSONG_SCRAPFRAME_MODEL_H
#define SWANSONG_SCRAPFRAME_MODEL_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
	uint8_t job;
	uint8_t selected;
	uint8_t score;
	uint8_t phase;
	bool last_ok;
} scrapframe_state_t;

typedef struct {
	int8_t selection_direction;
	bool confirm;
} scrapframe_input_t;

typedef struct {
	uint16_t tone_hz;
	uint8_t tone_frames;
	bool reset_session;
	bool dirty;
} scrapframe_event_t;

void scrapframe_reset(scrapframe_state_t *state);
void scrapframe_step(scrapframe_state_t *state,
	const scrapframe_input_t *input, scrapframe_event_t *event);

#endif
