#ifndef SWANSONG_ORBITAL_GFX_H
#define SWANSONG_ORBITAL_GFX_H

#include <stdbool.h>

#include "model.h"

void orbital_gfx_reset_title(void);
void orbital_gfx_show_intro(bool show_prompt);
void orbital_gfx_init(void);
void orbital_gfx_render(const courier_state_t *state);

#endif
