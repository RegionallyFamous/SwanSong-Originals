#ifndef SWANSONG_MOTE_GFX_H
#define SWANSONG_MOTE_GFX_H

#include <stdbool.h>
#include <stdint.h>

void gfx_show_intro(void);
void gfx_init(void);
void gfx_render(uint8_t track, bool playing, uint8_t tempo,
	uint8_t scope, uint8_t step);

#endif
