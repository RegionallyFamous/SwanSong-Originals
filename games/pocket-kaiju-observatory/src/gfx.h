#ifndef SWANSONG_KAIJU_GFX_H
#define SWANSONG_KAIJU_GFX_H

#include <stdbool.h>
#include <stdint.h>

void gfx_show_intro(void);
void gfx_init(void);
void gfx_render(uint8_t camera, uint8_t kaiju, uint8_t behavior,
	uint8_t disturbance, uint16_t sun, uint8_t evidence, bool zoom,
	uint8_t result);

#endif
