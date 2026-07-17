#include <swan/legacy.h>

#include "swan_game_runtime.h"
#include "gfx.h"
#include "gameplay_art.h"
#include "model.h"

void gfx_show_intro(void) {
	swan_game_gfx_show_intro(game_intro_tiles, sizeof(game_intro_tiles),
		game_intro_map, game_palette);
}

void gfx_init(void) {
	rf_gfx_load(game_tiles, sizeof(game_tiles), game_palette, art_hud_bg[0]);
}

void gfx_render(uint8_t room, bool vertical, uint8_t px, uint8_t py,
	bool key, uint8_t result) {
	uint8_t y;
	uint8_t x;
	for (x = 0; x < 5; ++x) {
		rf_gfx_put_tile((uint8_t)(1 + x * 2), 0,
			x < room ? art_pip_full[0] : art_pip_empty[0]);
	}
	rf_gfx_put_image(13, 0, key ? art_key : art_pip_empty, key ? 2 : 1, key ? 2 : 1);
	rf_gfx_put_image(18, 0, vertical ? art_orient_v : art_orient_h, 2, 2);
	for (y = 0; y < 8; ++y) {
		for (x = 0; x < 12; ++x) {
			const uint16_t __far *meta = rotate_blocked(room, vertical, x, y) ?
				art_wall : art_floor;
			rf_gfx_put_image((uint8_t)(2 + x * 2), (uint8_t)(2 + y * 2), meta, 2, 2);
		}
	}
	if (!key) rf_gfx_put_image((uint8_t)(2 + rotate_key_x(room) * 2),
		(uint8_t)(2 + rotate_key_y(room) * 2), art_key, 2, 2);
	rf_gfx_put_image(22, 4, art_exit, 2, 2);
	rf_gfx_put_image((uint8_t)(2 + px * 2), (uint8_t)(2 + py * 2), art_player, 2, 2);
	if (result) {
		rf_gfx_put_image(12, 7, art_result_win, 4, 4);
		rf_gfx_put_image(13, 11, art_loop, 2, 2);
	}
}
