#include "rf_swan.h"
#include "gfx.h"
#include "gameplay_art.h"

static const uint16_t __far *machine_for(uint8_t job) {
	if (job == 0) return art_machine_0;
	if (job == 1) return art_machine_1;
	return art_machine_2;
}

static const uint16_t __far *part_for(uint8_t part, bool selected) {
	if (part == 0) return selected ? art_part_0_on : art_part_0_off;
	if (part == 1) return selected ? art_part_1_on : art_part_1_off;
	return selected ? art_part_2_on : art_part_2_off;
}

void gfx_show_intro(void) {
	rf_gfx_show_intro(game_intro_tiles, sizeof(game_intro_tiles),
		game_intro_map, game_palette);
}

void gfx_init(void) {
	rf_gfx_load(game_tiles, sizeof(game_tiles), game_palette, art_hud_bg[0]);
}

void gfx_render(uint8_t job, uint8_t selected, uint8_t score,
	uint8_t phase, bool last_ok) {
	uint8_t i;
	rf_gfx_fill(art_hud_bg[0], 0, 0, 28, 18);
	for (i = 0; i < 3; ++i) {
		rf_gfx_put_tile((uint8_t)(1 + i * 2), 0,
			i < job ? art_pip_full[0] : art_pip_empty[0]);
		rf_gfx_put_tile((uint8_t)(21 + i * 2), 0,
			i < score ? art_pip_full[0] : art_pip_empty[0]);
	}
	rf_gfx_put_image(7, 3, machine_for(job), 14, 7);
	for (i = 0; i < 3; ++i) {
		rf_gfx_put_image((uint8_t)(2 + i * 9), 13,
			part_for(i, phase == 0 && i == selected), 4, 4);
	}
	if (phase == 1) {
		rf_gfx_put_image(12, 7, last_ok ? art_result_win : art_result_loss, 4, 4);
		rf_gfx_put_image(13, 11, art_loop, 2, 2);
	} else if (phase == 2) {
		rf_gfx_fill(art_hud_bg[0], 8, 5, 12, 9);
		rf_gfx_put_image(12, 6, art_result_win, 4, 4);
		for (i = 0; i < 3; ++i) {
			rf_gfx_put_tile((uint8_t)(11 + i * 3), 11,
				i < score ? art_pip_full[0] : art_pip_empty[0]);
		}
		rf_gfx_put_image(13, 13, art_loop, 2, 2);
	}
}
