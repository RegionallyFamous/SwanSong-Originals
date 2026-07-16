#ifndef SWANSONG_HARPOON_GFX_H
#define SWANSONG_HARPOON_GFX_H

#include <stdint.h>

void gfx_show_intro(void);
void gfx_init(void);
void gfx_render(uint8_t skiff, uint8_t creature, uint16_t oxygen,
	uint8_t tags, uint8_t boss_hp, uint8_t charge, uint8_t result);

#endif
