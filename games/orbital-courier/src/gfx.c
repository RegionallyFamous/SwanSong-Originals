#include <swan/gfx.h>

#include "swan_assets.h"
#include "gfx.h"
#include "model.h"

#define VIEW_COLS 14
#define VIEW_ROWS 8
#define MAP_COLS 20
#define MAP_ROWS 9
#define GAMEPLAY_MAP_WIDTH SWAN_ASSET_GAMEPLAY_WIDTH_TILES

#define ASSET_FLOOR_A_X 0
#define ASSET_FLOOR_B_X 2
#define ASSET_WALL_X 4
#define ASSET_OUTER_WALL_X 6
#define ASSET_PARCEL_X 8
#define ASSET_DEPOT_X 10
#define ASSET_COURIER_A_X 12
#define ASSET_COURIER_B_X 14
#define ASSET_COURIER_CARRY_A_X 0
#define ASSET_COURIER_CARRY_B_X 2
#define ASSET_CARGO_EMPTY_X 4
#define ASSET_CARGO_FULL_X 6
#define ASSET_TARGET_HUD_X 8
#define ASSET_LOOP_X 10
#define ASSET_HUD_BG_X 12
#define ASSET_FUEL_FULL_X 13
#define ASSET_FUEL_EMPTY_X 14
#define ASSET_ROUTE_ON_X 15
#define ASSET_ROUTE_OFF_X 12
#define ASSET_RESULT_WIN_X 0
#define ASSET_RESULT_LOSS_X 4

#define ASSET_ROW_WORLD 0
#define ASSET_ROW_HUD 2
#define ASSET_ROW_HUD_LOWER 3
#define ASSET_ROW_RESULT 4

static bool gameplay_loaded;
static bool render_initialized;
static uint8_t rendered_camera_x;
static uint8_t rendered_camera_y;
static uint8_t rendered_x;
static uint8_t rendered_y;
static uint8_t rendered_fuel;
static uint8_t rendered_progress;
static uint8_t rendered_result;
static bool rendered_parcel;

static void set_palette(const uint16_t SWAN_FAR *source) {
	uint16_t palette[4];
	uint8_t index;
	for (index = 0; index < 4; ++index) palette[index] = source[index];
	(void)swan_gfx_set_palette(0, palette);
}

static void put_gameplay_region(uint8_t screen_x, uint8_t screen_y,
	uint8_t asset_x, uint8_t asset_y, uint8_t width, uint8_t height) {
	uint8_t x;
	uint8_t y;
	for (y = 0; y < height; ++y) {
		for (x = 0; x < width; ++x) {
			(void)swan_gfx_put_tile(0, (uint8_t)(screen_x + x),
				(uint8_t)(screen_y + y),
				swan_asset_gameplay_map[
					(uint16_t)(asset_y + y) * GAMEPLAY_MAP_WIDTH + asset_x + x]);
		}
	}
}

static void put_meta(uint8_t screen_x, uint8_t screen_y,
	uint8_t asset_x, uint8_t asset_y) {
	put_gameplay_region(screen_x, screen_y, asset_x, asset_y, 2, 2);
}

static uint8_t camera_axis(uint8_t position, uint8_t lead, uint8_t maximum) {
	uint8_t camera = position > lead ? (uint8_t)(position - lead) : 0;
	return camera > maximum ? maximum : camera;
}

static void put_world_cell(uint8_t screen_x, uint8_t screen_y,
	uint8_t map_x, uint8_t map_y, bool parcel) {
	uint8_t meta_x;
	if (orbital_blocked(map_x, map_y)) {
		meta_x = (map_x == 0 || map_x == 19 || map_y == 0 || map_y == 8) ?
			ASSET_OUTER_WALL_X : ASSET_WALL_X;
	} else if (!parcel && map_x == 3 && map_y == 7) {
		meta_x = ASSET_PARCEL_X;
	} else if (map_x == 17 && map_y == 1) {
		meta_x = ASSET_DEPOT_X;
	} else {
		meta_x = ((map_x + map_y) & 1) ? ASSET_FLOOR_A_X : ASSET_FLOOR_B_X;
	}
	put_meta(screen_x, screen_y, meta_x, ASSET_ROW_WORLD);
}

void orbital_gfx_show_intro(void) {
	uint8_t x;
	uint8_t y;
	(void)swan_gfx_load_tiles(0, swan_asset_intro_tiles,
		SWAN_ASSET_INTRO_TILE_COUNT);
	set_palette(swan_asset_intro_palette);
	(void)swan_gfx_fill(0, 0, 0, 32, 32, SWAN_TILE_ATTR(0, 0));
	for (y = 0; y < 18; ++y) {
		for (x = 0; x < 28; ++x) {
			(void)swan_gfx_put_tile(0, x, y,
				swan_asset_intro_map[(uint16_t)y * 28u + x]);
		}
	}
	swan_gfx_set_scroll(0, 0, 0);
	swan_gfx_set_layer_enabled(0, true);
	swan_gfx_set_layer_enabled(1, false);
	render_initialized = false;
}

void orbital_gfx_init(void) {
	if (!gameplay_loaded) {
		(void)swan_gfx_load_tiles(0, swan_asset_gameplay_tiles,
			SWAN_ASSET_GAMEPLAY_TILE_COUNT);
		set_palette(swan_asset_gameplay_palette);
		gameplay_loaded = true;
	}
	(void)swan_gfx_fill(0, 0, 0, 32, 32,
		swan_asset_gameplay_map[ASSET_ROW_HUD * GAMEPLAY_MAP_WIDTH + ASSET_HUD_BG_X]);
	swan_gfx_set_scroll(0, 0, 0);
	swan_gfx_set_layer_enabled(0, true);
	swan_gfx_set_layer_enabled(1, false);
}

void orbital_gfx_render(uint8_t px, uint8_t py, bool parcel, uint8_t fuel,
	uint8_t steps, uint8_t result) {
	uint8_t camera_x = camera_axis(px, 6, MAP_COLS - VIEW_COLS);
	uint8_t camera_y = camera_axis(py, 3, MAP_ROWS - VIEW_ROWS);
	uint8_t y;
	uint8_t x;
	uint8_t progress = (uint8_t)(((uint16_t)steps * 5) / 40);
	uint8_t courier_x;

	if (progress > 5) progress = 5;
	if (!render_initialized) {
		(void)swan_gfx_fill(0, 0, 0, 28, 2,
			swan_asset_gameplay_map[ASSET_ROW_HUD * GAMEPLAY_MAP_WIDTH + ASSET_HUD_BG_X]);
		put_meta(26, 0, ASSET_TARGET_HUD_X, ASSET_ROW_HUD);
	}
	if (!render_initialized || fuel != rendered_fuel) {
		for (x = 0; x < 5; ++x) {
			uint8_t pip_x = fuel > (uint8_t)(x * 8) ?
				ASSET_FUEL_FULL_X : ASSET_FUEL_EMPTY_X;
			put_gameplay_region((uint8_t)(1 + x * 2), 0, pip_x,
				ASSET_ROW_HUD, 1, 1);
		}
	}
	if (!render_initialized || parcel != rendered_parcel)
		put_meta(12, 0, parcel ? ASSET_CARGO_FULL_X : ASSET_CARGO_EMPTY_X,
			ASSET_ROW_HUD);
	if (!render_initialized || progress != rendered_progress) {
		for (x = 0; x < 5; ++x) {
			uint8_t dot_x = x < progress ? ASSET_ROUTE_ON_X : ASSET_ROUTE_OFF_X;
			uint8_t dot_y = x < progress ? ASSET_ROW_HUD : ASSET_ROW_HUD_LOWER;
			put_gameplay_region((uint8_t)(16 + x * 2), 0, dot_x, dot_y, 1, 1);
		}
	}

	if (!render_initialized || camera_x != rendered_camera_x ||
		camera_y != rendered_camera_y) {
		for (y = 0; y < VIEW_ROWS; ++y) {
			for (x = 0; x < VIEW_COLS; ++x) {
				put_world_cell((uint8_t)(x * 2), (uint8_t)(2 + y * 2),
					(uint8_t)(camera_x + x), (uint8_t)(camera_y + y), parcel);
			}
		}
	} else if (px != rendered_x || py != rendered_y ||
		parcel != rendered_parcel) {
		put_world_cell((uint8_t)((rendered_x - camera_x) * 2),
			(uint8_t)(2 + (rendered_y - camera_y) * 2),
			rendered_x, rendered_y, parcel);
	}

	if (parcel) {
		courier_x = (steps & 1) ? ASSET_COURIER_CARRY_B_X : ASSET_COURIER_CARRY_A_X;
	} else {
		courier_x = (steps & 1) ? ASSET_COURIER_B_X : ASSET_COURIER_A_X;
	}
	put_meta((uint8_t)((px - camera_x) * 2),
		(uint8_t)(2 + (py - camera_y) * 2), courier_x,
		parcel ? ASSET_ROW_HUD : ASSET_ROW_WORLD);

	if (result && (!render_initialized || result != rendered_result)) {
		put_gameplay_region(12, 6,
			result == 1 ? ASSET_RESULT_WIN_X : ASSET_RESULT_LOSS_X,
			ASSET_ROW_RESULT, 4, 4);
		put_meta(13, 10, ASSET_LOOP_X, ASSET_ROW_HUD);
	}
	render_initialized = true;
	rendered_camera_x = camera_x;
	rendered_camera_y = camera_y;
	rendered_x = px;
	rendered_y = py;
	rendered_fuel = fuel;
	rendered_progress = progress;
	rendered_result = result;
	rendered_parcel = parcel;
}
