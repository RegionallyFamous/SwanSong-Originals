#ifndef SWANSONG_TACTICS_GFX_H
#define SWANSONG_TACTICS_GFX_H

#include <stdint.h>

#define ALLY_CAPACITY 7
#define ENEMY_CAPACITY 4

typedef struct {
	int8_t x;
	int8_t y;
	uint8_t hp;
} unit_t;

void gfx_show_intro(void);
void gfx_init(void);
void gfx_render(uint8_t cursor_x, uint8_t cursor_y, uint8_t selected,
	uint8_t turns, uint8_t recruits, uint8_t result,
	const unit_t *allies, const unit_t *enemies);

#endif
