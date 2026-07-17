#ifndef SWANSONG_ROTATE_GFX_H
#define SWANSONG_ROTATE_GFX_H

#include <stdbool.h>
#include <stdint.h>

void gfx_show_intro(void);
void gfx_init(void);
void gfx_render(uint8_t room, bool vertical, uint8_t px, uint8_t py,
	bool key, uint8_t result);

#endif
