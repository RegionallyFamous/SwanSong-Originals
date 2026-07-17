#include <swan/legacy.h>

#include "swan_game_runtime.h"
#include "gfx.h"
#include "gameplay_art.h"
#include "model.h"

static const uint16_t __far *socket_for(uint8_t kind) {
	if (kind == 1) return art_socket_1;
	if (kind == 2) return art_socket_2;
	if (kind == 3) return art_socket_3;
	return art_socket_0;
}

static const uint16_t __far *beetle_for(uint8_t kind, bool selected) {
	if (kind == 0) return selected ? art_beetle_0_on : art_beetle_0_off;
	if (kind == 1) return selected ? art_beetle_1_on : art_beetle_1_off;
	return selected ? art_beetle_2_on : art_beetle_2_off;
}

static const uint16_t __far *small_for(uint8_t kind) {
	if (kind == 0) return art_beetle_small_0;
	if (kind == 1) return art_beetle_small_1;
	return art_beetle_small_2;
}

void gfx_show_intro(void) {
	swan_game_gfx_show_intro(game_intro_tiles, sizeof(game_intro_tiles),
		game_intro_map, game_palette);
}

void gfx_init(void) {
	rf_gfx_load(game_tiles, sizeof(game_tiles), game_palette, art_hud_bg[0]);
}

void gfx_render(const uint8_t *cells, uint8_t cursor, uint8_t selected,
	uint8_t puzzle, bool failed, bool complete) {
	uint8_t i;
	uint8_t used = 0;
	rf_gfx_fill(art_field[0], 0, 0, 28, 18);
	for (i = 0; i < 5; ++i) {
		rf_gfx_put_tile((uint8_t)(1 + i * 2), 0,
			i < puzzle ? art_pip_full[0] : art_pip_empty[0]);
		if (cells[i]) ++used;
	}
	for (i = 0; i < 3; ++i) {
		if (bug_puzzles[puzzle].mask & (uint8_t)(1 << i)) {
			rf_gfx_put_image((uint8_t)(11 + i * 3), 0, small_for(i), 2, 2);
		}
	}
	for (i = 0; i < 3; ++i) {
		rf_gfx_put_tile((uint8_t)(22 + i * 2), 0,
			i < bug_puzzles[puzzle].limit && i >= used ? art_pip_full[0] : art_pip_empty[0]);
	}
	rf_gfx_put_image(0, 6,
		bug_puzzles[puzzle].input ? art_signal_on : art_signal_off, 2, 2);
	for (i = 0; i < 5; ++i) {
		uint8_t sx = (uint8_t)(2 + i * 5);
		rf_gfx_put_image(sx, 4, socket_for(cells[i]), 4, 4);
		if (i == cursor) rf_gfx_put_tile(sx, 3, art_pip_full[0]);
		if (i < 4) rf_gfx_put_tile((uint8_t)(sx + 4), 6, art_wire[0]);
	}
	rf_gfx_put_image(26, 6,
		bug_puzzles[puzzle].target ? art_signal_on : art_signal_off, 2, 2);
	for (i = 0; i < 3; ++i) {
		rf_gfx_put_image((uint8_t)(4 + i * 7), 13,
			beetle_for(i, selected == i + 1), 3, 3);
	}
	rf_gfx_put_image(24, 13, art_run, 3, 3);
	if (failed) rf_gfx_put_image(12, 7, art_result_loss, 4, 4);
	if (complete) {
		rf_gfx_put_image(12, 7, art_result_win, 4, 4);
		rf_gfx_put_image(13, 11, art_loop, 2, 2);
	}
}
