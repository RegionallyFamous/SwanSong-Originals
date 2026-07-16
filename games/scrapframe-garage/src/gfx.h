#ifndef SWANSONG_SCRAP_GFX_H
#define SWANSONG_SCRAP_GFX_H

#include <stdbool.h>
#include <stdint.h>

void gfx_show_intro(void);
void gfx_init(void);
void gfx_render(uint8_t job, uint8_t selected, uint8_t score,
	uint8_t phase, bool last_ok);

#endif
