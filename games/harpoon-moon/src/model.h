#ifndef SWANSONG_HARPOON_MODEL_H
#define SWANSONG_HARPOON_MODEL_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
	HARPOON_RESULT_PLAYING = 0,
	HARPOON_RESULT_WIN = 1,
	HARPOON_RESULT_OXYGEN = 2
} harpoon_result_t;

typedef struct {
	uint8_t skiff;
	uint8_t creature;
	uint8_t tags;
	uint8_t boss_hp;
	uint8_t charge;
	harpoon_result_t result;
	uint16_t oxygen;
	uint16_t random_state;
} harpoon_state_t;

typedef struct {
	int8_t direction;
	bool charge_held;
	bool lure_held;
	bool fire_released;
	bool replay;
} harpoon_input_t;

typedef struct {
	uint16_t tone_hz;
	uint8_t tone_frames;
	bool reset_session;
	bool dirty;
} harpoon_event_t;

void harpoon_reset(harpoon_state_t *state);
void harpoon_step(harpoon_state_t *state, const harpoon_input_t *input,
	uint32_t session_tick, harpoon_event_t *event);

#endif
