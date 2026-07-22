#ifndef SWANSONG_HARPOON_GFX_H
#define SWANSONG_HARPOON_GFX_H

#include <stdbool.h>
#include <stdint.h>

#include "model.h"

void harpoon_gfx_reset_title(void);
void harpoon_gfx_show_title(bool show_prompt, bool attract, uint8_t page,
	bool training, uint16_t best_score, uint8_t best_rank);
void harpoon_gfx_init(void);
void harpoon_gfx_render(const harpoon_state_t *state,
	uint16_t best_score, uint8_t best_rank);

#endif
