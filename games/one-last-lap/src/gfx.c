#include "rf_swan.h"
#include "gfx.h"
#include "gameplay_art.h"

static uint8_t lane_x(uint8_t lane, uint8_t y) {
	uint8_t spread = (uint8_t)(y - 4);
	uint8_t left = (uint8_t)(13 - spread);
	uint8_t right = (uint8_t)(14 + spread);
	return (uint8_t)(left + 1 + lane * ((right - left) / 3));
}

void gfx_show_intro(void) {
	rf_gfx_show_intro(game_intro_tiles, sizeof(game_intro_tiles),
		game_intro_map, game_palette);
}

void gfx_init(void) {
	rf_gfx_load(game_tiles, sizeof(game_tiles), game_palette, art_hud_bg[0]);
}

void gfx_render(uint8_t lap, uint8_t progress, uint8_t speed,
	uint8_t battery, uint8_t lane, bool helped, uint8_t result) {
	uint8_t y;
	uint8_t x;
	uint8_t rival_lane = (uint8_t)((progress / 25 + 1) % 3);

	rf_gfx_fill(art_sky[0], 0, 0, 28, 6);
	rf_gfx_fill(art_road[0], 0, 6, 28, 12);
	for (y = 6; y < 18; ++y) {
		uint8_t spread = (uint8_t)(y - 4);
		uint8_t left = (uint8_t)(13 - spread);
		uint8_t right = (uint8_t)(14 + spread);
		uint8_t third = (uint8_t)((right - left) / 3);
		for (x = 0; x < left; ++x) rf_gfx_put_tile(x, y, art_sky[0]);
		for (x = (uint8_t)(right + 1); x < 28; ++x) rf_gfx_put_tile(x, y, art_sky[0]);
		rf_gfx_put_tile(left, y, art_edge[0]);
		rf_gfx_put_tile(right, y, art_edge[0]);
		rf_gfx_put_tile((uint8_t)(left + third), y, art_edge[0]);
		rf_gfx_put_tile((uint8_t)(left + third * 2), y, art_edge[0]);
	}
	for (x = 0; x < 3; ++x) {
		rf_gfx_put_tile((uint8_t)(1 + x * 2), 0,
			x < lap ? art_pip_full[0] : art_pip_empty[0]);
	}
	for (x = 0; x < 7; ++x) {
		rf_gfx_put_tile((uint8_t)(8 + x), 0,
			battery > (uint8_t)(x * 10) ? art_pip_full[0] : art_pip_empty[0]);
	}
	for (x = 0; x < 6; ++x) {
		rf_gfx_put_tile((uint8_t)(17 + x), 0,
			x < speed ? art_speed[0] : art_pip_empty[0]);
	}
	if (helped) rf_gfx_put_image(24, 0, art_tow, 2, 2);
	for (x = 0; x < 5; ++x) {
		rf_gfx_put_tile((uint8_t)(10 + x * 2), 3,
			progress >= (uint8_t)(x * 20) ? art_pip_full[0] : art_pip_empty[0]);
	}
	rf_gfx_put_image(lane_x(rival_lane, 8), 8, art_rival, 2, 2);
	rf_gfx_put_image(lane_x((uint8_t)((rival_lane + 2) % 3), 10), 10, art_rival, 2, 2);
	if ((progress >= 24 && progress <= 30) || (progress >= 68 && progress <= 74)) {
		rf_gfx_put_image(lane_x(progress < 50 ? 1 : 2, 11), 11, art_hazard, 2, 2);
	}
	if (!helped && lap == 2 && progress >= 34 && progress <= 62) {
		rf_gfx_put_image(22, 8, art_stranded, 3, 2);
	}
	rf_gfx_put_image(lane_x(lane, 14), 14, art_player, 4, 3);
	if (result) {
		rf_gfx_put_image(12, 7, result == 1 ? art_result_win : art_result_loss, 4, 4);
		rf_gfx_put_image(13, 11, art_loop, 2, 2);
	}
}
