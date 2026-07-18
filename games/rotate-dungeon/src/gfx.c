#include <swan/legacy.h>

#include "swan_game_runtime.h"
#include "gfx.h"
#include "gameplay_art.h"
#include "model.h"

static bool layout_valid;
static uint8_t rendered_room;
static bool rendered_vertical;
static uint8_t rendered_x;
static uint8_t rendered_y;

static void draw_cell(uint8_t room, bool vertical, uint8_t x, uint8_t y,
	bool key) {
	const uint16_t __far *meta = rotate_blocked(room, vertical, x, y) ?
		art_wall : art_floor;
	rf_gfx_put_image((uint8_t)(2 + x * 2), (uint8_t)(2 + y * 2), meta, 2, 2);
	if (!key && x == rotate_key_x(room) && y == rotate_key_y(room))
		rf_gfx_put_image((uint8_t)(2 + x * 2), (uint8_t)(2 + y * 2),
			art_key, 2, 2);
	if (x == 10 && y == 1)
		rf_gfx_put_image((uint8_t)(2 + x * 2), (uint8_t)(2 + y * 2),
			art_exit, 2, 2);
}

void gfx_show_intro(void) {
	swan_game_gfx_show_intro(game_intro_tiles, sizeof(game_intro_tiles),
		game_intro_map, game_palette);
}

void gfx_init(void) {
	rf_gfx_load(game_tiles, sizeof(game_tiles), game_palette, art_hud_bg[0]);
	layout_valid = false;
}

void gfx_render(uint8_t room, bool vertical, uint8_t px, uint8_t py,
	bool key, uint8_t result) {
	uint8_t y;
	uint8_t x;
	uint8_t layout_room = room < 5 ? room : 4;
	bool redraw_layout = !layout_valid || rendered_room != layout_room ||
		rendered_vertical != vertical;

	for (x = 0; x < 5; ++x) {
		rf_gfx_put_tile((uint8_t)(1 + x * 2), 0,
			x < room ? art_pip_full[0] : art_pip_empty[0]);
	}
	for (y = 0; y < 2; ++y) {
		for (x = 0; x < 2; ++x)
			rf_gfx_put_tile((uint8_t)(13 + x), y, art_hud_bg[0]);
	}
	rf_gfx_put_image(13, 0, key ? art_key : art_pip_empty,
		key ? 2 : 1, key ? 2 : 1);
	rf_gfx_put_image(18, 0, vertical ? art_orient_v : art_orient_h, 2, 2);
	if (redraw_layout) {
		for (y = 0; y < 8; ++y) {
			for (x = 0; x < 12; ++x)
				draw_cell(layout_room, vertical, x, y, key);
		}
	} else {
		draw_cell(layout_room, vertical, rendered_x, rendered_y, key);
		draw_cell(layout_room, vertical, px, py, key);
	}
	rf_gfx_put_image((uint8_t)(2 + px * 2), (uint8_t)(2 + py * 2), art_player, 2, 2);
	if (result) {
		rf_gfx_put_image(12, 7, art_result_win, 4, 4);
		rf_gfx_put_image(13, 11, art_loop, 2, 2);
	}
	layout_valid = true;
	rendered_room = layout_room;
	rendered_vertical = vertical;
	rendered_x = px;
	rendered_y = py;
}
