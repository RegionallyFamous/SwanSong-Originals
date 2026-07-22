#ifndef SWANSONG_BUG_GFX_H
#define SWANSONG_BUG_GFX_H

#include <stdbool.h>
#include <stdint.h>

#include "model.h"

void gfx_reset_title(void);
void gfx_show_intro(uint8_t option, bool show_prompt, uint8_t best_medals,
	bool tutorial_learned);
void gfx_init(void);
void gfx_render(const bug_state_t *state, bool tutorial,
	bool tutorial_done, uint32_t tick, uint8_t best_medals);

#endif
