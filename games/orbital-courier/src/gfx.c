#include <swan/gfx.h>

#include "swan_assets.h"
#include "gfx.h"
#include "model.h"

#define VIEW_COLS 14u
#define VIEW_ROWS 8u
#define GAMEPLAY_MAP_WIDTH SWAN_ASSET_GAMEPLAY_WIDTH_TILES
#define FONT_BASE 640u
#define FONT_BLANK FONT_BASE
#define ACTOR_SPRITE_CAPACITY 16u

#define ASSET_FLOOR_A_X 0u
#define ASSET_FLOOR_B_X 2u
#define ASSET_WALL_X 4u
#define ASSET_OUTER_WALL_X 6u
#define ASSET_PARCEL_X 8u
#define ASSET_DEPOT_X 10u
#define ASSET_COURIER_A_X 12u
#define ASSET_COURIER_B_X 14u
#define ASSET_COURIER_CARRY_A_X 0u
#define ASSET_COURIER_CARRY_B_X 2u
#define ASSET_CARGO_EMPTY_X 4u
#define ASSET_CARGO_FULL_X 6u
#define ASSET_TARGET_HUD_X 8u
#define ASSET_LOOP_X 10u
#define ASSET_FUEL_FULL_X 13u
#define ASSET_FUEL_EMPTY_X 14u
#define ASSET_ROUTE_ON_X 15u
#define ASSET_ROUTE_OFF_X 12u

#define ASSET_ROW_WORLD 0u
#define ASSET_ROW_HUD 2u
#define ASSET_ROW_HUD_LOWER 3u

#define FONT_ROW(value) (uint8_t)(0xFFu ^ (value)), (uint8_t)(value)
#define FONT_GLYPH(a, b, c, d, e, f, g) \
	FONT_ROW(a), FONT_ROW(b), FONT_ROW(c), FONT_ROW(d), \
	FONT_ROW(e), FONT_ROW(f), FONT_ROW(g), FONT_ROW(0)

/* Compact code-native 5x7 lettering; the source art remains unlettered. */
static const uint8_t SWAN_FAR font_tiles[] = {
	FONT_GLYPH(0, 0, 0, 0, 0, 0, 0),
	FONT_GLYPH(0x38, 0x44, 0x44, 0x7C, 0x44, 0x44, 0x44),
	FONT_GLYPH(0x78, 0x44, 0x44, 0x78, 0x44, 0x44, 0x78),
	FONT_GLYPH(0x3C, 0x40, 0x40, 0x40, 0x40, 0x40, 0x3C),
	FONT_GLYPH(0x78, 0x44, 0x44, 0x44, 0x44, 0x44, 0x78),
	FONT_GLYPH(0x7C, 0x40, 0x40, 0x78, 0x40, 0x40, 0x7C),
	FONT_GLYPH(0x7C, 0x40, 0x40, 0x78, 0x40, 0x40, 0x40),
	FONT_GLYPH(0x3C, 0x40, 0x40, 0x4C, 0x44, 0x44, 0x3C),
	FONT_GLYPH(0x44, 0x44, 0x44, 0x7C, 0x44, 0x44, 0x44),
	FONT_GLYPH(0x7C, 0x10, 0x10, 0x10, 0x10, 0x10, 0x7C),
	FONT_GLYPH(0x0C, 0x04, 0x04, 0x04, 0x44, 0x44, 0x38),
	FONT_GLYPH(0x44, 0x48, 0x50, 0x60, 0x50, 0x48, 0x44),
	FONT_GLYPH(0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x7C),
	FONT_GLYPH(0x44, 0x6C, 0x54, 0x54, 0x44, 0x44, 0x44),
	FONT_GLYPH(0x44, 0x64, 0x54, 0x4C, 0x44, 0x44, 0x44),
	FONT_GLYPH(0x38, 0x44, 0x44, 0x44, 0x44, 0x44, 0x38),
	FONT_GLYPH(0x78, 0x44, 0x44, 0x78, 0x40, 0x40, 0x40),
	FONT_GLYPH(0x38, 0x44, 0x44, 0x44, 0x54, 0x48, 0x34),
	FONT_GLYPH(0x78, 0x44, 0x44, 0x78, 0x50, 0x48, 0x44),
	FONT_GLYPH(0x3C, 0x40, 0x40, 0x38, 0x04, 0x04, 0x78),
	FONT_GLYPH(0x7C, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10),
	FONT_GLYPH(0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x38),
	FONT_GLYPH(0x44, 0x44, 0x44, 0x44, 0x44, 0x28, 0x10),
	FONT_GLYPH(0x44, 0x44, 0x44, 0x54, 0x54, 0x6C, 0x44),
	FONT_GLYPH(0x44, 0x44, 0x28, 0x10, 0x28, 0x44, 0x44),
	FONT_GLYPH(0x44, 0x44, 0x28, 0x10, 0x10, 0x10, 0x10),
	FONT_GLYPH(0x7C, 0x04, 0x08, 0x10, 0x20, 0x40, 0x7C),
	FONT_GLYPH(0x38, 0x44, 0x4C, 0x54, 0x64, 0x44, 0x38),
	FONT_GLYPH(0x10, 0x30, 0x10, 0x10, 0x10, 0x10, 0x38),
	FONT_GLYPH(0x38, 0x44, 0x04, 0x08, 0x10, 0x20, 0x7C),
	FONT_GLYPH(0x78, 0x04, 0x04, 0x38, 0x04, 0x04, 0x78),
	FONT_GLYPH(0x08, 0x18, 0x28, 0x48, 0x7C, 0x08, 0x08),
	FONT_GLYPH(0x7C, 0x40, 0x40, 0x78, 0x04, 0x04, 0x78),
	FONT_GLYPH(0x38, 0x40, 0x40, 0x78, 0x44, 0x44, 0x38),
	FONT_GLYPH(0x7C, 0x04, 0x08, 0x10, 0x20, 0x20, 0x20),
	FONT_GLYPH(0x38, 0x44, 0x44, 0x38, 0x44, 0x44, 0x38),
	FONT_GLYPH(0x38, 0x44, 0x44, 0x3C, 0x04, 0x04, 0x38),
};

static bool title_loaded;
static uint8_t actor_sprite_count;

static uint16_t font_tile(char character) {
	if (character >= 'A' && character <= 'Z')
		return (uint16_t)(FONT_BASE + 1u + (uint8_t)(character - 'A'));
	if (character >= '0' && character <= '9')
		return (uint16_t)(FONT_BASE + 27u + (uint8_t)(character - '0'));
	return FONT_BLANK;
}

static void put_text(uint8_t layer, uint8_t x, uint8_t y,
	const char SWAN_FAR *text) {
	while (*text && x < 28) {
		char character = *text++;
		(void)swan_gfx_put_tile(layer, x++, y,
			SWAN_TILE_ATTR(font_tile(character), 1));
	}
}

static void put_number(uint8_t layer, uint8_t x, uint8_t y,
	uint16_t value, uint8_t digits) {
	uint16_t divisor = 1;
	uint8_t index;
	for (index = 1; index < digits; ++index) divisor = (uint16_t)(divisor * 10u);
	for (index = 0; index < digits; ++index) {
		(void)swan_gfx_put_tile(layer, (uint8_t)(x + index), y,
			SWAN_TILE_ATTR(font_tile((char)('0' + (value / divisor) % 10u)), 1));
		divisor = divisor > 1 ? (uint16_t)(divisor / 10u) : 1;
	}
}

static void fill_plate(uint8_t layer, uint8_t x, uint8_t y,
	uint8_t width, uint8_t height) {
	(void)swan_gfx_fill(layer, x, y, width, height,
		SWAN_TILE_ATTR(FONT_BLANK, 1));
}

static void set_asset_palette(uint8_t slot,
	const uint16_t SWAN_FAR *source) {
	uint16_t palette[4];
	uint8_t index;
	for (index = 0; index < 4; ++index) palette[index] = source[index];
	(void)swan_gfx_set_palette(slot, palette);
}

static void load_font(void) {
	static const uint16_t ui_palette[4] = {0x001, 0x112, 0xF37, 0xFFD};
	(void)swan_gfx_load_tiles(FONT_BASE, font_tiles,
		(uint16_t)(sizeof(font_tiles) / 16u));
	(void)swan_gfx_set_palette(1, ui_palette);
}

static swan_tile_attr_t asset_tile(uint8_t asset_x, uint8_t asset_y) {
	return swan_asset_gameplay_map[
		(uint16_t)asset_y * GAMEPLAY_MAP_WIDTH + asset_x];
}

static void put_gameplay_region(uint8_t layer, uint8_t screen_x,
	uint8_t screen_y, uint8_t asset_x, uint8_t asset_y,
	uint8_t width, uint8_t height) {
	uint8_t x;
	uint8_t y;
	for (y = 0; y < height; ++y) {
		for (x = 0; x < width; ++x) {
			(void)swan_gfx_put_tile(layer, (uint8_t)(screen_x + x),
				(uint8_t)(screen_y + y),
				asset_tile((uint8_t)(asset_x + x),
					(uint8_t)(asset_y + y)));
		}
	}
}

static void put_meta(uint8_t screen_x, uint8_t screen_y,
	uint8_t asset_x, uint8_t asset_y) {
	put_gameplay_region(0, screen_x, screen_y, asset_x, asset_y, 2, 2);
}

static uint8_t camera_axis(uint8_t position, uint8_t lead, uint8_t maximum) {
	uint8_t camera = position > lead ? (uint8_t)(position - lead) : 0;
	return camera > maximum ? maximum : camera;
}

static void put_world_cell(uint8_t screen_x, uint8_t screen_y,
	uint8_t map_x, uint8_t map_y, const courier_state_t *state) {
	uint8_t meta_x;
	uint8_t meta_y = ASSET_ROW_WORLD;
	if (orbital_blocked(map_x, map_y)) {
		meta_x = (map_x == 0 || map_x == COURIER_MAP_COLS - 1u ||
			map_y == 0 || map_y == COURIER_MAP_ROWS - 1u) ?
			ASSET_OUTER_WALL_X : ASSET_WALL_X;
	} else if (!state->parcel && map_x == COURIER_PARCEL_X &&
		map_y == COURIER_PARCEL_Y) {
		meta_x = ASSET_PARCEL_X;
	} else if (!state->relay_complete && map_x == COURIER_RELAY_X &&
		map_y == COURIER_RELAY_Y) {
		meta_x = ASSET_TARGET_HUD_X;
		meta_y = ASSET_ROW_HUD;
	} else if (map_x == COURIER_DEPOT_X && map_y == COURIER_DEPOT_Y) {
		meta_x = ASSET_DEPOT_X;
	} else if (!state->charger_used && map_x == COURIER_CHARGER_X &&
		map_y == COURIER_CHARGER_Y) {
		meta_x = ASSET_LOOP_X;
		meta_y = ASSET_ROW_HUD;
	} else if (orbital_express_lane(map_x, map_y)) {
		meta_x = ASSET_FLOOR_B_X;
	} else {
		meta_x = ((map_x + map_y) & 1u) ? ASSET_FLOOR_A_X : ASSET_FLOOR_B_X;
	}
	put_meta(screen_x, screen_y, meta_x, meta_y);
}

static uint8_t sprite_flags(swan_tile_attr_t tile) {
	uint8_t flags = SWAN_SPRITE_FLAG_PRIORITY;
	if ((tile & SWAN_TILE_HFLIP) != 0) flags |= SWAN_SPRITE_FLAG_HFLIP;
	if ((tile & SWAN_TILE_VFLIP) != 0) flags |= SWAN_SPRITE_FLAG_VFLIP;
	return flags;
}

static void put_asset_metasprite(int16_t x, int16_t y,
	uint8_t asset_x, uint8_t asset_y) {
	uint8_t tile_x;
	uint8_t tile_y;
	for (tile_y = 0; tile_y < 2; ++tile_y) {
		for (tile_x = 0; tile_x < 2; ++tile_x) {
			swan_tile_attr_t tile = asset_tile((uint8_t)(asset_x + tile_x),
				(uint8_t)(asset_y + tile_y));
			swan_sprite_t sprite = {
				.x = (int16_t)(x + tile_x * 8),
				.y = (int16_t)(y + tile_y * 8),
				.tile = swan_gfx_tile_index(tile),
				.palette = 0,
				.flags = sprite_flags(tile),
				.visible = true,
			};
			if (actor_sprite_count < ACTOR_SPRITE_CAPACITY) {
				(void)swan_gfx_set_sprite(actor_sprite_count, &sprite);
				++actor_sprite_count;
			}
		}
	}
}

static void finish_sprites(void) {
	while (actor_sprite_count < ACTOR_SPRITE_CAPACITY) {
		swan_sprite_t sprite = {
			.x = 224, .y = 144, .tile = 0, .palette = 0,
			.flags = 0, .visible = false,
		};
		(void)swan_gfx_set_sprite(actor_sprite_count, &sprite);
		++actor_sprite_count;
	}
}

static bool in_view(uint8_t x, uint8_t y, uint8_t camera_x,
	uint8_t camera_y) {
	return x >= camera_x && x < camera_x + VIEW_COLS &&
		y >= camera_y && y < camera_y + VIEW_ROWS;
}

static void draw_actors(const courier_state_t *state, uint8_t camera_x,
	uint8_t camera_y) {
	uint8_t traffic;
	for (traffic = 0; traffic < COURIER_TRAFFIC_COUNT; ++traffic) {
		uint8_t x;
		uint8_t y;
		uint8_t asset_x;
		uint8_t asset_y;
		courier_traffic_position(state->traffic_turn, traffic, &x, &y);
		if (!in_view(x, y, camera_x, camera_y)) continue;
		asset_x = traffic == 0 ? ASSET_COURIER_A_X :
			traffic == 1 ? ASSET_COURIER_B_X : ASSET_COURIER_CARRY_A_X;
		asset_y = traffic == 2 ? ASSET_ROW_HUD : ASSET_ROW_WORLD;
		put_asset_metasprite((int16_t)(x - camera_x) * 16,
			(int16_t)(2u + (y - camera_y) * 2u) * 8,
			asset_x, asset_y);
	}
	put_asset_metasprite((int16_t)(state->x - camera_x) * 16,
		(int16_t)(2u + (state->y - camera_y) * 2u) * 8,
		state->parcel ? ((state->steps & 1u) ? ASSET_COURIER_CARRY_B_X :
			ASSET_COURIER_CARRY_A_X) :
			((state->steps & 1u) ? ASSET_COURIER_B_X : ASSET_COURIER_A_X),
		state->parcel ? ASSET_ROW_HUD : ASSET_ROW_WORLD);
}

static void draw_hud(const courier_state_t *state) {
	uint8_t index;
	fill_plate(1, 0, 0, 28, 2);
	put_text(1, 0, 0, "FUEL");
	for (index = 0; index < 8; ++index) {
		put_gameplay_region(1, (uint8_t)(5u + index), 0,
			state->fuel > (uint8_t)(index * 8u) ? ASSET_FUEL_FULL_X :
				ASSET_FUEL_EMPTY_X,
			ASSET_ROW_HUD, 1, 1);
	}
	put_text(1, 15, 0, "LEG");
	for (index = 0; index < 3; ++index) {
		put_gameplay_region(1, (uint8_t)(19u + index * 2u), 0,
			index < state->leg ? ASSET_ROUTE_ON_X : ASSET_ROUTE_OFF_X,
			index < state->leg ? ASSET_ROW_HUD : ASSET_ROW_HUD_LOWER, 1, 1);
	}
	put_text(1, 25, 0, "H");
	put_number(1, 27, 0, state->collisions, 1);
	if (state->leg == 0) put_text(1, 1, 1, "PICK UP CARGO  CHARGE OPTIONAL");
	else if (state->leg == 1) put_text(1, 3, 1, "CARGO TO EAST RELAY");
	else if (state->leg == 2) put_text(1, 2, 1, "FINAL DEPOT NORTH EAST");
	else put_text(1, 5, 1, "ROUTE COMPLETE");
}

static void draw_feedback(const courier_state_t *state) {
	const char SWAN_FAR *message = 0;
	if (!state->feedback_frames) return;
	switch (state->feedback) {
		case COURIER_SFX_BLOCKED: message = "ROUTE BLOCKED"; break;
		case COURIER_SFX_PICKUP: message = "CARGO SECURED"; break;
		case COURIER_SFX_CHARGE: message = "FUEL TOPPED"; break;
		case COURIER_SFX_RELAY: message = "RELAY VERIFIED"; break;
		case COURIER_SFX_COLLISION: message = "TRAFFIC HIT"; break;
		default: break;
	}
	if (message) {
		fill_plate(0, 7, 13, 15, 2);
		put_text(0, 8, 13, message);
	}
}

static void draw_overlay(const courier_state_t *state) {
	if (state->phase == COURIER_PHASE_PAUSED) {
		fill_plate(0, 6, 6, 16, 6);
		put_text(0, 10, 7, "PAUSED");
		put_text(0, 8, 9, "START RESUME");
		put_text(0, 9, 10, "B RETRY");
	} else if (state->tutorial == 0) {
		fill_plate(0, 6, 6, 17, 5);
		put_text(0, 8, 7, "D PAD TO MOVE");
		put_text(0, 8, 9, "START PAUSE");
	} else {
		draw_feedback(state);
	}
}

static void draw_result(const courier_state_t *state) {
	uint16_t total_seconds = (uint16_t)(state->elapsed_frames / 75u);
	char rank[2] = {'A', 0};
	rank[0] = state->rank == 0 ? 'S' : state->rank == 1 ? 'A' :
		state->rank == 2 ? 'B' : 'C';
	fill_plate(0, 4, 3, 20, 14);
	if (state->result == COURIER_RESULT_DELIVERED)
		put_text(0, 6, 4, "DELIVERY COMPLETE");
	else
		put_text(0, 9, 4, "FUEL EMPTY");
	put_text(0, 7, 7, "RANK");
	put_text(0, 14, 7, rank);
	put_text(0, 7, 9, "TIME");
	put_number(0, 14, 9, (uint16_t)(total_seconds / 60u), 2);
	put_number(0, 17, 9, (uint16_t)(total_seconds % 60u), 2);
	put_text(0, 7, 11, "SCORE");
	put_number(0, 14, 11, state->score, 4);
	put_text(0, 7, 13, "TRAFFIC");
	put_number(0, 16, 13, state->collisions, 2);
	put_text(0, 5, 15, "A AGAIN START TITLE");
}

void orbital_gfx_reset_title(void) {
	title_loaded = false;
}

void orbital_gfx_show_intro(bool show_prompt) {
	uint8_t x;
	uint8_t y;
	if (!title_loaded) {
		swan_gfx_hide_sprites();
		swan_gfx_set_sprites_enabled(false);
		(void)swan_gfx_load_tiles(0, swan_asset_intro_tiles,
			SWAN_ASSET_INTRO_TILE_COUNT);
		set_asset_palette(0, swan_asset_intro_palette);
		load_font();
		(void)swan_gfx_fill(0, 0, 0, 32, 32, SWAN_TILE_ATTR(0, 0));
		for (y = 0; y < 18; ++y) {
			for (x = 0; x < 28; ++x) {
				(void)swan_gfx_put_tile(0, x, y,
					swan_asset_intro_map[(uint16_t)y * 28u + x]);
			}
		}
		fill_plate(0, 1, 1, 15, 5);
		put_text(0, 3, 2, "ORBITAL");
		put_text(0, 3, 3, "COURIER");
		put_text(0, 3, 5, "SPECIAL DELIVERY");
		fill_plate(0, 3, 14, 14, 2);
		swan_gfx_set_scroll(0, 0, 0);
		swan_gfx_set_layer_enabled(0, true);
		swan_gfx_set_layer_enabled(1, false);
		title_loaded = true;
	}
	fill_plate(0, 4, 14, 12, 1);
	if (show_prompt) put_text(0, 4, 14, "PRESS START");
}

void orbital_gfx_init(void) {
	static const swan_gfx_clip_t hud_clip = {0, 0, 224, 16};
	static const swan_gfx_clip_t play_clip = {0, 16, 224, 128};
	(void)swan_gfx_load_tiles(0, swan_asset_gameplay_tiles,
		SWAN_ASSET_GAMEPLAY_TILE_COUNT);
	set_asset_palette(0, swan_asset_gameplay_palette);
	set_asset_palette(8, swan_asset_gameplay_palette);
	load_font();
	(void)swan_gfx_fill(0, 0, 0, 32, 32, asset_tile(ASSET_FLOOR_A_X,
		ASSET_ROW_WORLD));
	(void)swan_gfx_fill(1, 0, 0, 32, 32, SWAN_TILE_ATTR(FONT_BLANK, 1));
	swan_gfx_set_scroll(0, 0, 0);
	swan_gfx_set_scroll(1, 0, 0);
	(void)swan_gfx_set_layer_clip(1, SWAN_GFX_CLIP_INSIDE, &hud_clip);
	(void)swan_gfx_set_sprite_clip(&play_clip);
	swan_gfx_set_layer_enabled(0, true);
	swan_gfx_set_layer_enabled(1, true);
	swan_gfx_hide_sprites();
	swan_gfx_set_sprites_enabled(true);
}

void orbital_gfx_render(const courier_state_t *state) {
	uint8_t camera_x = camera_axis(state->x, 6,
		(uint8_t)(COURIER_MAP_COLS - VIEW_COLS));
	uint8_t camera_y = camera_axis(state->y, 3,
		(uint8_t)(COURIER_MAP_ROWS - VIEW_ROWS));
	uint8_t x;
	uint8_t y;
	actor_sprite_count = 0;
	for (y = 0; y < VIEW_ROWS; ++y) {
		for (x = 0; x < VIEW_COLS; ++x) {
			put_world_cell((uint8_t)(x * 2u), (uint8_t)(2u + y * 2u),
				(uint8_t)(camera_x + x), (uint8_t)(camera_y + y), state);
		}
	}
	draw_hud(state);
	if (state->phase == COURIER_PHASE_RESULT) {
		draw_result(state);
	} else {
		draw_actors(state, camera_x, camera_y);
		draw_overlay(state);
	}
	finish_sprites();
}

#undef FONT_GLYPH
#undef FONT_ROW
