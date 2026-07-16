#include <string.h>

#include <wonderful.h>
#include <ws.h>
#include <wse/memory.h>

#include "gfx.h"
#include "gameplay_art.h"

#define VIEW_COLS 14
#define VIEW_ROWS 8
#define MAP_COLS 20
#define MAP_ROWS 9

static uint16_t tile_attr(uint16_t tile) {
	return WS_SCREEN_ATTR_TILE(tile) | WS_SCREEN_ATTR_PALETTE(0);
}

static void load_palette(void) {
	if (ws_system_is_color_active()) {
		memcpy(WS_SCREEN_COLOR_MEM(0), orbital_palette, sizeof(orbital_palette));
	} else {
		outportw(WS_SCR_PAL_0_PORT, WS_DISPLAY_MONO_PALETTE(0, 7, 4, 2));
	}
}

static void put_image(uint8_t screen_x, uint8_t screen_y,
	const uint16_t __far *tiles, uint8_t width, uint8_t height) {
	uint8_t y;
	uint8_t x;
	for (y = 0; y < height; ++y) {
		for (x = 0; x < width; ++x) {
			wse_screen1.row[screen_y + y].cell[screen_x + x] =
				tile_attr(tiles[(uint16_t)y * width + x]);
		}
	}
}

static void put_meta(uint8_t screen_x, uint8_t screen_y,
	const uint16_t __far *tiles) {
	put_image(screen_x, screen_y, tiles, 2, 2);
}

static uint8_t camera_axis(uint8_t position, uint8_t lead, uint8_t maximum) {
	uint8_t camera = position > lead ? (uint8_t)(position - lead) : 0;
	return camera > maximum ? maximum : camera;
}

void orbital_gfx_show_intro(void) {
	ws_display_set_control(0);
	memset(WS_TILE_MEM(0), 0, sizeof(ws_display_tile_t));
	memcpy(WS_TILE_MEM(1), orbital_intro_tiles, sizeof(orbital_intro_tiles));
	load_palette();
	ws_screen_put_tiles(&wse_screen1, orbital_intro_map, 0, 0, 28, 18);
	ws_display_scroll_screen1_to(0, 0);
	ws_display_set_control(WS_DISPLAY_CTRL_SCR1_ENABLE);
}

void orbital_gfx_init(void) {
	ws_display_set_control(0);
	memcpy(WS_TILE_MEM(0), orbital_game_tiles, sizeof(orbital_game_tiles));
	load_palette();
	ws_screen_fill_tiles(&wse_screen1, tile_attr(orbital_hud_bg[0]),
		0, 0, 32, 32);
	ws_display_scroll_screen1_to(0, 0);
	ws_display_set_control(WS_DISPLAY_CTRL_SCR1_ENABLE);
}

void orbital_gfx_render(uint8_t px, uint8_t py, bool parcel, uint8_t fuel,
	uint8_t steps, uint8_t result) {
	uint8_t camera_x = camera_axis(px, 6, MAP_COLS - VIEW_COLS);
	uint8_t camera_y = camera_axis(py, 3, MAP_ROWS - VIEW_ROWS);
	uint8_t y;
	uint8_t x;
	uint8_t progress = (uint8_t)(((uint16_t)steps * 5) / 40);
	const uint16_t __far *courier;

	if (progress > 5) progress = 5;
	ws_screen_fill_tiles(&wse_screen1, tile_attr(orbital_hud_bg[0]),
		0, 0, 28, 2);

	for (x = 0; x < 5; ++x) {
		const uint16_t __far *pip = fuel > (uint8_t)(x * 8) ?
			orbital_fuel_full : orbital_fuel_empty;
		wse_screen1.row[0].cell[1 + x * 2] = tile_attr(pip[0]);
	}
	put_meta(12, 0, parcel ? orbital_cargo_full : orbital_cargo_empty);
	for (x = 0; x < 5; ++x) {
		const uint16_t __far *dot = x < progress ?
			orbital_route_on : orbital_route_off;
		wse_screen1.row[0].cell[16 + x * 2] = tile_attr(dot[0]);
	}
	put_meta(26, 0, orbital_target_hud);

	for (y = 0; y < VIEW_ROWS; ++y) {
		for (x = 0; x < VIEW_COLS; ++x) {
			uint8_t map_x = (uint8_t)(camera_x + x);
			uint8_t map_y = (uint8_t)(camera_y + y);
			const uint16_t __far *meta;
			if (orbital_blocked(map_x, map_y)) {
				meta = (map_x == 0 || map_x == 19 || map_y == 0 || map_y == 8) ?
					orbital_outer_wall : orbital_wall;
			} else if (!parcel && map_x == 3 && map_y == 7) {
				meta = orbital_parcel;
			} else if (map_x == 17 && map_y == 1) {
				meta = orbital_depot;
			} else {
				meta = ((map_x + map_y) & 1) ? orbital_floor_a : orbital_floor_b;
			}
			put_meta((uint8_t)(x * 2), (uint8_t)(2 + y * 2), meta);
		}
	}

	if (parcel) {
		courier = (steps & 1) ? orbital_courier_carry_b : orbital_courier_carry_a;
	} else {
		courier = (steps & 1) ? orbital_courier_b : orbital_courier_a;
	}
	put_meta((uint8_t)((px - camera_x) * 2),
		(uint8_t)(2 + (py - camera_y) * 2), courier);

	if (result) {
		put_image(12, 6, result == 1 ? orbital_result_win : orbital_result_loss,
			4, 4);
		put_meta(13, 10, orbital_loop);
	}
}
