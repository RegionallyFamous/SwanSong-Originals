#include <swan/legacy.h>

#include "swan_game_runtime.h"
#include "gfx.h"
#include "gameplay_art.h"

void gfx_show_intro(void) {
	swan_game_gfx_show_intro(game_intro_tiles, sizeof(game_intro_tiles),
		game_intro_map, game_palette);
}

void gfx_init(void) {
	rf_gfx_load(game_tiles, sizeof(game_tiles), game_palette, art_hud_bg[0]);
}

void gfx_render(uint8_t skiff, uint8_t creature, uint16_t oxygen,
	uint8_t tags, uint8_t boss_hp, uint8_t charge, uint8_t result) {
	uint8_t i;
	uint8_t skiff_x = (uint8_t)(1 + skiff);
	uint8_t creature_x = (uint8_t)(1 + creature);
	int8_t direction = creature_x >= skiff_x ? 1 : -1;
	uint8_t reach = (uint8_t)(charge / 2 + 2);
	int16_t beam_x = (int16_t)skiff_x + 2;

	rf_gfx_fill(art_sky[0], 0, 0, 28, 14);
	rf_gfx_fill(art_ground[0], 0, 14, 28, 4);
	for (i = 0; i < 8; ++i) {
		rf_gfx_put_tile((uint8_t)(1 + i), 0,
			oxygen > (uint16_t)(i * 150) ? art_pip_full[0] : art_pip_empty[0]);
	}
	for (i = 0; i < 3; ++i) {
		rf_gfx_put_tile((uint8_t)(12 + i * 2), 0,
			i < tags ? art_pip_full[0] : art_pip_empty[0]);
		rf_gfx_put_tile((uint8_t)(21 + i * 2), 0,
			tags == 3 && i < boss_hp ? art_pip_full[0] : art_pip_empty[0]);
	}
	for (i = 0; i < 10; ++i) {
		rf_gfx_put_tile((uint8_t)(9 + i), 16,
			i < charge / 2 ? art_pip_full[0] : art_pip_empty[0]);
	}
	rf_gfx_put_image(skiff_x, 11, art_skiff, 4, 3);
	rf_gfx_put_image(creature_x, tags < 3 ? 6 : 5,
		tags < 3 ? art_creature : art_boss, tags < 3 ? 4 : 6, tags < 3 ? 3 : 4);
	for (i = 0; i < reach; ++i) {
		beam_x += direction;
		if (beam_x > 0 && beam_x < 28) rf_gfx_put_tile((uint8_t)beam_x, 11, art_beam[0]);
	}
	if (result) {
		rf_gfx_put_image(12, 7, result == 1 ? art_result_win : art_result_loss, 4, 4);
		rf_gfx_put_image(13, 11, art_loop, 2, 2);
	}
}
