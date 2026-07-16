#ifndef SWANSONG_LAP_GFX_H
#define SWANSONG_LAP_GFX_H

#include <stdbool.h>
#include <stdint.h>

void gfx_show_intro(void);
void gfx_init(void);
void gfx_render(uint8_t lap, uint8_t progress, uint8_t speed,
	uint8_t battery, uint8_t lane, bool helped, uint8_t result);

#endif
