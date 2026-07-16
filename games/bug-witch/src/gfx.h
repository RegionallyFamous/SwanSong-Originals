#ifndef SWANSONG_BUG_GFX_H
#define SWANSONG_BUG_GFX_H

#include <stdbool.h>
#include <stdint.h>

void gfx_show_intro(void);
void gfx_init(void);
void gfx_render(const uint8_t *cells, uint8_t cursor, uint8_t selected,
	uint8_t puzzle, bool failed, bool complete);

#endif
