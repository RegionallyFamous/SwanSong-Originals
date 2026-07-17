#ifndef SWANSONG_ORBITAL_GFX_H
#define SWANSONG_ORBITAL_GFX_H

#include <stdbool.h>
#include <stdint.h>

void orbital_gfx_show_intro(void);
void orbital_gfx_init(void);
void orbital_gfx_render(uint8_t px, uint8_t py, bool parcel, uint8_t fuel,
	uint8_t steps, uint8_t result);

#endif
