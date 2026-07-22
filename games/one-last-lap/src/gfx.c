#include <swan/gfx.h>
#include <swan/legacy.h>

#include "swan_game_runtime.h"
#include "gfx.h"
#include "gameplay_art.h"

#define FONT_BASE 430u
#define FONT_BLANK FONT_BASE
#define ACTOR_SPRITE_CAPACITY 34u
#define FONT_ROW(value) (uint8_t)(0xFFu ^ (value)), (uint8_t)(value)
#define FONT_GLYPH(a, b, c, d, e, f, g) \
	FONT_ROW(a), FONT_ROW(b), FONT_ROW(c), FONT_ROW(d), \
	FONT_ROW(e), FONT_ROW(f), FONT_ROW(g), FONT_ROW(0)

/* Hand-authored 5x7 tiles: black plate (color 1), pink glyph (color 2). */
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

static void put_text(uint8_t x, uint8_t y, const char SWAN_FAR *text) {
	while (*text && x < 28) {
		rf_gfx_put_tile(x++, y, font_tile(*text++));
	}
}

static void put_number(uint8_t x, uint8_t y, uint16_t value, uint8_t digits) {
	uint16_t divisor = 1;
	uint8_t index;
	for (index = 1; index < digits; ++index) divisor = (uint16_t)(divisor * 10u);
	for (index = 0; index < digits; ++index) {
		rf_gfx_put_tile((uint8_t)(x + index), y,
			font_tile((char)('0' + (value / divisor) % 10u)));
		divisor = divisor > 1 ? (uint16_t)(divisor / 10u) : 1;
	}
}

static uint8_t lane_x(uint8_t lane, uint8_t y) {
	uint8_t spread = y > 4 ? (uint8_t)(y - 4) : 0;
	uint8_t left = (uint8_t)(13 - spread);
	uint8_t right = (uint8_t)(14 + spread);
	return (uint8_t)(left + 1 + lane * ((right - left) / 3));
}

static void load_font(void) {
	(void)swan_gfx_load_tiles(FONT_BASE, font_tiles,
		(uint16_t)(sizeof(font_tiles) / 16u));
}

static void load_sprite_palette(void) {
	uint16_t colors[4];
	uint8_t index;
	for (index = 0; index < 4; ++index) colors[index] = game_palette[index];
	/* Color sprite palette zero occupies display-palette slot eight. */
	(void)swan_gfx_set_palette(8, colors);
}

static uint8_t sprite_flags(uint16_t tile) {
	uint8_t flags = SWAN_SPRITE_FLAG_PRIORITY;
	if ((tile & SWAN_TILE_HFLIP) != 0) flags |= SWAN_SPRITE_FLAG_HFLIP;
	if ((tile & SWAN_TILE_VFLIP) != 0) flags |= SWAN_SPRITE_FLAG_VFLIP;
	return flags;
}

static void put_metasprite(int16_t x, int16_t y,
	const uint16_t SWAN_FAR *tiles, uint8_t width, uint8_t height) {
	uint8_t tile_x;
	uint8_t tile_y;
	for (tile_y = 0; tile_y < height; ++tile_y) {
		for (tile_x = 0; tile_x < width; ++tile_x) {
			uint16_t tile = tiles[(uint16_t)tile_y * width + tile_x];
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

static void finish_metasprites(void) {
	while (actor_sprite_count < ACTOR_SPRITE_CAPACITY) {
		swan_sprite_t sprite = {
			.x = 224, .y = 144, .tile = 0, .palette = 0,
			.flags = 0, .visible = false,
		};
		(void)swan_gfx_set_sprite(actor_sprite_count, &sprite);
		++actor_sprite_count;
	}
}

void gfx_reset_title(void) {
	title_loaded = false;
}

void gfx_show_intro(bool show_prompt) {
	if (!title_loaded) {
		swan_gfx_hide_sprites();
		swan_gfx_set_sprites_enabled(false);
		swan_game_gfx_show_intro(game_intro_tiles, sizeof(game_intro_tiles),
			game_intro_map, game_palette);
		load_font();
		rf_gfx_fill(FONT_BLANK, 6, 1, 16, 3);
		put_text(8, 2, "ONE LAST LAP");
		rf_gfx_fill(FONT_BLANK, 7, 13, 14, 3);
		title_loaded = true;
	}
	rf_gfx_fill(FONT_BLANK, 8, 14, 11, 1);
	if (show_prompt) put_text(8, 14, "PRESS START");
}

void gfx_init(void) {
	rf_gfx_load(game_tiles, sizeof(game_tiles), game_palette, art_hud_bg[0]);
	load_font();
	load_sprite_palette();
	swan_gfx_set_sprites_enabled(true);
	swan_gfx_hide_sprites();
}

static void draw_hud(const lap_state_t *state) {
	uint8_t i;
	rf_gfx_fill(FONT_BLANK, 0, 0, 28, 2);
	put_text(0, 0, "LAP");
	for (i = 0; i < LAP_TOTAL_LAPS; ++i) {
		rf_gfx_put_tile((uint8_t)(4 + i), 0,
			i < state->lap ? art_pip_full[0] : art_pip_empty[0]);
	}
	put_text(9, 0, "BAT");
	for (i = 0; i < 8; ++i) {
		rf_gfx_put_tile((uint8_t)(13 + i), 0,
			state->battery > (uint16_t)(i * (LAP_START_BATTERY / 8u)) ?
			art_pip_full[0] : art_pip_empty[0]);
	}
	put_text(22, 0, "V");
	put_number(24, 0, state->speed, 2);
	put_text(0, 1, "LEG");
	for (i = 0; i < 10; ++i) {
		rf_gfx_put_tile((uint8_t)(4 + i), 1,
			state->progress >= (uint8_t)(i * 10u) ?
			art_pip_full[0] : art_pip_empty[0]);
	}
	if (state->helped) put_text(18, 1, "TOW OK");
	else if (state->tow_available) put_text(18, 1, "TOW");
}

static void draw_road(const lap_state_t *state) {
	uint8_t motion = (uint8_t)((state->distance_total / 18u) & 3u);
	uint8_t y;
	uint8_t x;
	rf_gfx_fill(art_sky[0], 0, 0, 28, 6);
	rf_gfx_fill(art_road[0], 0, 6, 28, 12);
	for (y = 6; y < 18; ++y) {
		uint8_t spread = (uint8_t)(y - 4);
		uint8_t left = (uint8_t)(13 - spread);
		uint8_t right = (uint8_t)(14 + spread);
		uint8_t third = (uint8_t)((right - left) / 3u);
		for (x = 0; x < left; ++x) rf_gfx_put_tile(x, y, art_sky[0]);
		for (x = (uint8_t)(right + 1); x < 28; ++x)
			rf_gfx_put_tile(x, y, art_sky[0]);
		rf_gfx_put_tile(left, y, art_edge[0]);
		rf_gfx_put_tile(right, y, art_edge[0]);
		if (((y + motion) & 3u) < 2u) {
			rf_gfx_put_tile((uint8_t)(left + third), y, art_edge[0]);
			rf_gfx_put_tile((uint8_t)(left + third * 2u), y, art_edge[0]);
		}
	}
	for (x = 0; x < 7; ++x) {
		uint8_t star_x = (uint8_t)((x * 5u + 27u - motion) % 27u);
		rf_gfx_put_tile(star_x, (uint8_t)(2 + (x & 1u)), art_pip_empty[0]);
	}
}

static void draw_hazard_sprites(const lap_state_t *state) {
	uint8_t lap_index = state->lap ? (uint8_t)(state->lap - 1u) : 0;
	uint16_t position = (uint16_t)(state->distance_total % LAP_TRACK_UNITS);
	uint8_t hazard;
	for (hazard = 0; hazard < 3; ++hazard) {
		uint16_t target = lap_hazard_position(hazard);
		if (target > position && target - position < 1500u) {
			uint8_t y = (uint8_t)(13u - (target - position) / 150u);
			uint8_t lane = lap_hazard_lane(lap_index, hazard);
			put_metasprite((int16_t)lane_x(lane, y) * 8, (int16_t)y * 8,
				art_hazard, 2, 2);
		}
	}
}

static void draw_rival_sprites(const lap_state_t *state) {
	uint8_t rival;
	for (rival = 0; rival < LAP_RIVAL_COUNT; ++rival) {
		int16_t delta;
		uint8_t y;
		if (!lap_rival_visible(state, rival)) continue;
		delta = lap_rival_delta(state, rival);
		y = delta <= 0 ? 13 : (uint8_t)(13 - (uint16_t)delta / 150u);
		if (y < 4) y = 4;
		put_metasprite((int16_t)lane_x(state->rival_lane[rival], y) * 8,
			(int16_t)y * 8, art_rival, 2, 2);
	}
}

static void draw_overlay(const lap_state_t *state) {
	if (state->phase == LAP_PHASE_COUNTDOWN) {
		rf_gfx_fill(FONT_BLANK, 11, 6, 6, 4);
		put_text(state->countdown ? 13 : 12, 7,
			state->countdown == 3 ? "3" : state->countdown == 2 ? "2" :
			state->countdown == 1 ? "1" : "GO");
	} else if (state->phase == LAP_PHASE_PAUSED) {
		rf_gfx_fill(FONT_BLANK, 9, 6, 10, 4);
		put_text(11, 7, "PAUSE");
	}
	if (state->checkpoint_flash) {
		rf_gfx_fill(FONT_BLANK, 8, 5, 12, 3);
		put_text(9, 6, "CHECKPOINT");
	}
}

static void draw_result(const lap_state_t *state) {
	uint16_t total_seconds = (uint16_t)(state->elapsed_frames / 75u);
	uint16_t minutes = (uint16_t)(total_seconds / 60u);
	uint16_t seconds = (uint16_t)(total_seconds % 60u);
	char rank[2] = {'S', 0};
	rank[0] = state->rank == 0 ? 'S' : state->rank == 1 ? 'A' :
		state->rank == 2 ? 'B' : 'C';
	rf_gfx_fill(FONT_BLANK, 4, 3, 20, 13);
	if (state->result == LAP_RESULT_COOPERATIVE)
		put_text(8, 4, "COOP FINISH");
	else if (state->result == LAP_RESULT_SOLO)
		put_text(8, 4, "SOLO FINISH");
	else
		put_text(8, 4, "BATTERY OUT");
	put_text(8, 7, "RANK");
	put_text(15, 7, rank);
	put_text(8, 9, "TIME");
	put_number(14, 9, minutes, 2);
	put_number(17, 9, seconds, 2);
	put_text(8, 11, "SCORE");
	put_number(15, 11, state->score, 4);
	put_text(7, 14, "A AGAIN START TITLE");
}

void gfx_render(const lap_state_t *state) {
	actor_sprite_count = 0;
	draw_road(state);
	draw_hud(state);
	if (state->phase == LAP_PHASE_RESULT) {
		draw_result(state);
	} else {
		draw_hazard_sprites(state);
		draw_rival_sprites(state);
		if (!state->helped && state->tow_available)
			put_metasprite(22 * 8, 10 * 8, art_stranded, 3, 2);
		put_metasprite((int16_t)lane_x(state->lane, 14) * 8,
			(int16_t)(14 * 8 + (state->speed &&
				((state->distance_total / 24u) & 1u) ? 1 : 0)),
			art_player, 4, 3);
		draw_overlay(state);
	}
	finish_metasprites();
}

#undef FONT_GLYPH
#undef FONT_ROW
