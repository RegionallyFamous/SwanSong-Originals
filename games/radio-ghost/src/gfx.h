#ifndef SWANSONG_RADIO_GFX_H
#define SWANSONG_RADIO_GFX_H

#include <stdbool.h>

#include "model.h"

void gfx_reset_title(void);
void gfx_set_records(uint16_t best_score, uint16_t best_time,
	uint8_t signals_discovered, bool tutorial_complete);
void gfx_show_title(const radio_state_t *state, bool show_prompt);
void gfx_init(void);
void gfx_render(const radio_state_t *state);

#endif
