#ifndef SWANSONG_ROTATE_GFX_H
#define SWANSONG_ROTATE_GFX_H

#include <stdbool.h>
#include <stdint.h>

bool rotate_blocked(uint8_t room, bool vertical, uint8_t x, uint8_t y);
uint8_t rotate_key_x(uint8_t room);
uint8_t rotate_key_y(uint8_t room);
void gfx_show_intro(void);
void gfx_init(void);
void gfx_render(uint8_t room, bool vertical, uint8_t px, uint8_t py,
	bool key, uint8_t result);

#endif
