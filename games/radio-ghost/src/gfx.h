#ifndef SWANSONG_RADIO_GFX_H
#define SWANSONG_RADIO_GFX_H

#include <stdbool.h>
#include <stdint.h>

void gfx_show_intro(void);
void gfx_init(void);
void gfx_render(uint16_t frequency, uint8_t gain, uint16_t time,
	uint8_t clue, uint8_t result, bool gate);

#endif
