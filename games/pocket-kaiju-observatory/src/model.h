#ifndef SWANSONG_KAIJU_MODEL_H
#define SWANSONG_KAIJU_MODEL_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
	KAIJU_RESULT_PLAYING = 0,
	KAIJU_RESULT_COMPLETE = 1,
	KAIJU_RESULT_FAILED = 2
} kaiju_result_t;

typedef struct {
	uint8_t camera;
	uint8_t kaiju;
	uint8_t behavior;
	uint8_t disturbance;
	uint8_t evidence;
	kaiju_result_t result;
	uint16_t sun;
	uint16_t random_state;
	bool zoom;
} kaiju_state_t;

typedef struct {
	int8_t direction;
	bool photograph;
	bool hide;
	bool toggle_zoom;
	bool replay;
} kaiju_input_t;

typedef struct {
	uint16_t tone_hz;
	uint8_t tone_frames;
	bool reset_session;
	bool dirty;
} kaiju_event_t;

void kaiju_reset(kaiju_state_t *state);
void kaiju_step(kaiju_state_t *state, const kaiju_input_t *input,
	uint32_t session_tick, kaiju_event_t *event);
bool kaiju_camera_in_range(uint8_t camera, uint8_t kaiju, bool zoom);

#endif
