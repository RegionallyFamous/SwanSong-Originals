#ifndef SWANSONG_LAP_GFX_H
#define SWANSONG_LAP_GFX_H

#include <stdbool.h>

#include "model.h"

void gfx_reset_title(void);
void gfx_show_intro(bool show_prompt);
void gfx_init(void);
void gfx_render(const lap_state_t *state);

#endif
