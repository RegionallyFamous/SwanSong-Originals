#include "rf_swan.h"
#include "gfx.h"
#include "gameplay_art.h"

static const uint16_t __far *unit_for(uint8_t team, uint8_t kind) {
	if (team == 0) {
		if (kind == 0) return art_unit_0_0;
		if (kind == 1) return art_unit_0_1;
		return art_unit_0_2;
	}
	if (kind == 0) return art_unit_1_0;
	if (kind == 1) return art_unit_1_1;
	return art_unit_1_2;
}

void gfx_show_intro(void) {
	rf_gfx_show_intro(game_intro_tiles, sizeof(game_intro_tiles),
		game_intro_map, game_palette);
}

void gfx_init(void) {
	rf_gfx_load(game_tiles, sizeof(game_tiles), game_palette, art_hud_bg[0]);
}

void gfx_render(uint8_t cursor_x, uint8_t cursor_y, uint8_t selected,
	uint8_t turns, uint8_t recruits, uint8_t result,
	const unit_t *allies, const unit_t *enemies) {
	uint8_t y;
	uint8_t x;
	uint8_t i;

	rf_gfx_fill(art_hud_bg[0], 0, 0, 28, 18);
	for (i = 0; i < 9; ++i) {
		rf_gfx_put_tile((uint8_t)(1 + i), 0,
			turns > (uint8_t)(i * 2) ? art_pip_full[0] : art_pip_empty[0]);
	}
	for (i = 0; i < 3; ++i) {
		rf_gfx_put_tile((uint8_t)(13 + i), 0,
			i < allies[0].hp ? art_pip_full[0] : art_pip_empty[0]);
	}
	for (i = 0; i < 4; ++i) {
		rf_gfx_put_tile((uint8_t)(21 + i), 0,
			i < recruits ? art_pip_full[0] : art_pip_empty[0]);
	}
	for (y = 0; y < 6; ++y) {
		for (x = 0; x < 8; ++x) {
			rf_gfx_put_image((uint8_t)(2 + x * 3), (uint8_t)(3 + y * 2), art_cell, 3, 2);
		}
	}
	rf_gfx_put_tile(24, 8, art_beacon[0]);

	for (i = 0; i < ALLY_CAPACITY; ++i) {
		uint8_t hp;
		uint8_t sx;
		uint8_t sy;
		if (!allies[i].hp) continue;
		sx = (uint8_t)(2 + allies[i].x * 3);
		sy = (uint8_t)(3 + allies[i].y * 2);
		for (hp = 0; hp < allies[i].hp; ++hp) rf_gfx_put_tile((uint8_t)(sx + hp), sy, art_pip_full[0]);
		rf_gfx_put_image((uint8_t)(sx + 1), (uint8_t)(sy + 1), unit_for(0, i % 3), 1, 1);
		if (i == selected) rf_gfx_put_tile((uint8_t)(sx + 2), (uint8_t)(sy + 1), art_cursor[0]);
	}
	for (i = 0; i < ENEMY_CAPACITY; ++i) {
		uint8_t hp;
		uint8_t sx;
		uint8_t sy;
		if (!enemies[i].hp) continue;
		sx = (uint8_t)(2 + enemies[i].x * 3);
		sy = (uint8_t)(3 + enemies[i].y * 2);
		for (hp = 0; hp < enemies[i].hp; ++hp) rf_gfx_put_tile((uint8_t)(sx + hp), sy, art_pip_full[0]);
		rf_gfx_put_image((uint8_t)(sx + 1), (uint8_t)(sy + 1), unit_for(1, i % 3), 1, 1);
	}
	rf_gfx_put_tile((uint8_t)(2 + cursor_x * 3),
		(uint8_t)(4 + cursor_y * 2), art_cursor[0]);
	if (result) {
		rf_gfx_put_image(12, 7, result == 1 ? art_result_win : art_result_loss, 4, 4);
		rf_gfx_put_image(13, 11, art_loop, 2, 2);
	}
}
