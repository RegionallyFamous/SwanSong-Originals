#include <swan/gfx.h>
#include <swan/legacy.h>

#include "swan_assets.h"
#include "swan_game_runtime.h"
#include "gfx.h"
#include "gameplay_art.h"

#define OVERLAY_BASE 390u
#define OVERLAY_BLANK OVERLAY_BASE
#define OVERLAY_GLOW (OVERLAY_BASE + 1u)
#define OVERLAY_SPARK (OVERLAY_BASE + 2u)
#define FONT_BASE 400u
#define FONT_BLANK FONT_BASE
#define ACTOR_SPRITE_CAPACITY 28u
#define FONT_ROW(value) (uint8_t)(0xFFu ^ (value)), (uint8_t)(value)
#define FONT_GLYPH(a, b, c, d, e, f, g) \
	FONT_ROW(a), FONT_ROW(b), FONT_ROW(c), FONT_ROW(d), \
	FONT_ROW(e), FONT_ROW(f), FONT_ROW(g), FONT_ROW(0)

static const uint8_t SWAN_FAR overlay_tiles[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x18, 0x00, 0x18, 0x00, 0x7E, 0x18, 0xFF,
	0x18, 0xFF, 0x00, 0x7E, 0x00, 0x18, 0x00, 0x18,
	0x00, 0x00, 0x00, 0x18, 0x18, 0x3C, 0x3C, 0x7E,
	0x3C, 0x7E, 0x18, 0x3C, 0x00, 0x18, 0x00, 0x00,
};

/* Hand-authored 5x7 code font: ink plate (color 1), pink glyph (color 2). */
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

static const char SWAN_FAR puzzle_0[] = "TEACH FLIP";
static const char SWAN_FAR puzzle_1[] = "ORDER LIGHT";
static const char SWAN_FAR puzzle_2[] = "SHADOW ZERO";
static const char SWAN_FAR puzzle_3[] = "REVERSE CHAIN";
static const char SWAN_FAR puzzle_4[] = "FINAL ALL THREE";

static bool title_loaded;
static uint8_t actor_sprite_count;

static const uint16_t SWAN_FAR *socket_for(uint8_t kind) {
	if (kind == 1) return art_socket_1;
	if (kind == 2) return art_socket_2;
	if (kind == 3) return art_socket_3;
	return art_socket_0;
}

static const uint16_t SWAN_FAR *beetle_for(uint8_t kind, bool selected) {
	if (kind == 0) return selected ? art_beetle_0_on : art_beetle_0_off;
	if (kind == 1) return selected ? art_beetle_1_on : art_beetle_1_off;
	return selected ? art_beetle_2_on : art_beetle_2_off;
}

static const uint16_t SWAN_FAR *small_for(uint8_t kind) {
	if (kind == 0) return art_beetle_small_0;
	if (kind == 1) return art_beetle_small_1;
	return art_beetle_small_2;
}

static const char SWAN_FAR *puzzle_title(uint8_t puzzle) {
	if (puzzle == 0) return puzzle_0;
	if (puzzle == 1) return puzzle_1;
	if (puzzle == 2) return puzzle_2;
	if (puzzle == 3) return puzzle_3;
	return puzzle_4;
}

static uint16_t font_tile(char character) {
	if (character >= 'A' && character <= 'Z')
		return (uint16_t)(FONT_BASE + 1u + (uint8_t)(character - 'A'));
	if (character >= '0' && character <= '9')
		return (uint16_t)(FONT_BASE + 27u + (uint8_t)(character - '0'));
	return FONT_BLANK;
}

static void put_text(uint8_t x, uint8_t y, const char SWAN_FAR *text) {
	while (*text && x < 28) rf_gfx_put_tile(x++, y, font_tile(*text++));
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

static uint8_t sprite_flags(uint16_t tile) {
	uint8_t flags = SWAN_SPRITE_FLAG_PRIORITY;
	if ((tile & SWAN_TILE_HFLIP) != 0) flags |= SWAN_SPRITE_FLAG_HFLIP;
	if ((tile & SWAN_TILE_VFLIP) != 0) flags |= SWAN_SPRITE_FLAG_VFLIP;
	return flags;
}

static void put_sprite(int16_t x, int16_t y, uint16_t tile) {
	swan_sprite_t sprite = {
		.x = x, .y = y, .tile = swan_gfx_tile_index(tile),
		.palette = 0, .flags = sprite_flags(tile), .visible = true,
	};
	if (actor_sprite_count < ACTOR_SPRITE_CAPACITY) {
		(void)swan_gfx_set_sprite(actor_sprite_count, &sprite);
		++actor_sprite_count;
	}
}

static void put_metasprite(int16_t x, int16_t y,
	const uint16_t SWAN_FAR *tiles, uint8_t width, uint8_t height) {
	uint8_t tile_x;
	uint8_t tile_y;
	for (tile_y = 0; tile_y < height; ++tile_y) {
		for (tile_x = 0; tile_x < width; ++tile_x) {
			put_sprite((int16_t)(x + tile_x * 8),
				(int16_t)(y + tile_y * 8),
				tiles[(uint16_t)tile_y * width + tile_x]);
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

static void load_font(void) {
	(void)swan_gfx_load_tiles(FONT_BASE, font_tiles,
		(uint16_t)(sizeof(font_tiles) / 16u));
}

static void set_palette(uint8_t slot, const uint16_t SWAN_FAR *source) {
	uint16_t colors[4];
	uint8_t index;
	for (index = 0; index < 4; ++index) colors[index] = source[index];
	(void)swan_gfx_set_palette(slot, colors);
}

static void load_game_extensions(void) {
	uint16_t colors[4];
	uint8_t index;
	load_font();
	(void)swan_gfx_load_tiles(OVERLAY_BASE, overlay_tiles,
		(uint16_t)(sizeof(overlay_tiles) / 16u));
	for (index = 0; index < 4; ++index) colors[index] = game_palette[index];
	(void)swan_gfx_set_palette(4, colors);
	(void)swan_gfx_set_palette(8, colors);
	(void)swan_gfx_fill(1, 0, 0, 32, 32,
		SWAN_TILE_ATTR(OVERLAY_BLANK, 4));
	swan_gfx_set_layer_enabled(1, true);
	swan_gfx_set_sprites_enabled(true);
	swan_gfx_hide_sprites();
}

void gfx_reset_title(void) {
	title_loaded = false;
}

void gfx_show_intro(uint8_t option, bool show_prompt, uint8_t best_medals,
	bool tutorial_learned) {
	if (!title_loaded) {
		uint8_t x;
		uint8_t y;
		swan_gfx_hide_sprites();
		swan_gfx_set_sprites_enabled(false);
		swan_gfx_set_layer_enabled(1, false);
		(void)swan_gfx_load_tiles(0, swan_asset_title_art_tiles,
			SWAN_ASSET_TITLE_ART_TILE_COUNT);
		set_palette(0, swan_asset_title_art_palette);
		(void)swan_gfx_fill(0, 0, 0, 32, 32, SWAN_TILE_ATTR(0, 0));
		for (y = 0; y < 18; ++y) {
			for (x = 0; x < 28; ++x) {
				(void)swan_gfx_put_tile(0, x, y,
					swan_asset_title_art_map[(uint16_t)y * 28u + x]);
			}
		}
		swan_gfx_set_scroll(0, 0, 0);
		swan_gfx_set_layer_enabled(0, true);
		load_font();
		rf_gfx_fill(FONT_BLANK, 6, 0, 16, 2);
		put_text(9, 1, "BUG WITCH");
		rf_gfx_fill(FONT_BLANK, 5, 11, 18, 7);
		put_text(9, 12, "PLAY");
		put_text(9, 14, "SPELLBOOK");
		put_text(6, 16, "X MOVE A CHOOSE");
		title_loaded = true;
	}
	rf_gfx_fill(FONT_BLANK, 20, 0, 7, 2);
	if (best_medals > 0u) {
		put_text(20, 0, "BEST");
		put_number(24, 0, best_medals, 2);
	}
	rf_gfx_fill(FONT_BLANK, 20, 2, 7, 1);
	if (tutorial_learned) put_text(20, 2, "LEARNED");
	put_text(7, 12, option == 0 ? "X" : " ");
	put_text(7, 14, option == 1 ? "X" : " ");
	rf_gfx_fill(FONT_BLANK, 8, 17, 11, 1);
	if (show_prompt) put_text(8, 17, "START PLAY");
}

void gfx_init(void) {
	rf_gfx_load(game_tiles, sizeof(game_tiles), game_palette, art_hud_bg[0]);
	load_game_extensions();
}

static uint8_t used_cells(const bug_state_t *state) {
	uint8_t count = 0;
	uint8_t cell;
	for (cell = 0; cell < BUG_CELL_COUNT; ++cell)
		if (state->cells[cell]) ++count;
	return count;
}

static void draw_hud(const bug_state_t *state, bool tutorial) {
	uint8_t i;
	uint8_t puzzle = state->puzzle < BUG_PUZZLE_COUNT ? state->puzzle : 0;
	uint8_t used = used_cells(state);
	for (i = 0; i < BUG_PUZZLE_COUNT; ++i) {
		rf_gfx_put_tile((uint8_t)(1 + i * 2), 0,
			i < puzzle ? art_pip_full[0] : art_pip_empty[0]);
	}
	for (i = 0; i < 3; ++i) {
		if (bug_puzzles[puzzle].mask & (uint8_t)(1u << i))
			rf_gfx_put_image((uint8_t)(11 + i * 3), 0, small_for(i), 2, 2);
	}
	for (i = 0; i < 3; ++i) {
		rf_gfx_put_tile((uint8_t)(22 + i * 2), 0,
			i < bug_puzzles[puzzle].limit && i >= used ?
			art_pip_full[0] : art_pip_empty[0]);
	}
	rf_gfx_fill(FONT_BLANK, 1, 2, 20, 1);
	put_text(1, 2, tutorial ? "SPELLBOOK LESSON" : puzzle_title(puzzle));
}

static void draw_board(const bug_state_t *state) {
	uint8_t puzzle = state->puzzle < BUG_PUZZLE_COUNT ? state->puzzle : 0;
	uint8_t i;
	rf_gfx_put_image(0, 6,
		bug_puzzles[puzzle].input ? art_signal_on : art_signal_off, 2, 2);
	for (i = 0; i < BUG_CELL_COUNT; ++i) {
		uint8_t sx = (uint8_t)(2 + i * 5);
		rf_gfx_put_image(sx, 4, socket_for(state->cells[i]), 4, 4);
		if (i < BUG_CELL_COUNT - 1u)
			rf_gfx_put_tile((uint8_t)(sx + 4), 6, art_wire[0]);
	}
	rf_gfx_put_image(26, 6,
		bug_puzzles[puzzle].target ? art_signal_on : art_signal_off, 2, 2);
}

static void draw_second_layer(const bug_state_t *state, uint32_t tick) {
	(void)swan_gfx_fill(1, 0, 0, 32, 32,
		SWAN_TILE_ATTR(OVERLAY_BLANK, 4));
	if (((tick >> 3) & 1u) == 0 && state->phase == BUG_PHASE_EDIT)
		(void)swan_gfx_put_tile(1, (uint8_t)(3 + state->cursor * 5u), 3,
			SWAN_TILE_ATTR(OVERLAY_GLOW, 4));
	if (state->hint_visible && state->hint_cell < BUG_CELL_COUNT &&
		((tick >> 2) & 1u) == 0)
		(void)swan_gfx_put_tile(1, (uint8_t)(3 + state->hint_cell * 5u), 5,
			SWAN_TILE_ATTR(OVERLAY_SPARK, 4));
	if (state->phase == BUG_PHASE_SIGNAL && state->signal_step < BUG_CELL_COUNT)
		(void)swan_gfx_put_tile(1, (uint8_t)(3 + state->signal_step * 5u), 6,
			SWAN_TILE_ATTR(OVERLAY_SPARK, 4));
}

static void draw_familiar_sprites(const bug_state_t *state, uint32_t tick) {
	uint8_t familiar;
	for (familiar = 0; familiar < 3; ++familiar) {
		bool selected = state->selected == familiar + 1u;
		int16_t y = (int16_t)(13 * 8 +
			(selected && ((tick >> 3) & 1u) ? -1 : 0));
		put_metasprite((int16_t)(4 + familiar * 7) * 8, y,
			beetle_for(familiar, selected), 3, 3);
	}
}

static void draw_signal_sprite(const bug_state_t *state) {
	static const uint8_t points[7] = {8, 32, 72, 112, 152, 192, 216};
	uint8_t step = state->signal_step > BUG_CELL_COUNT ?
		BUG_CELL_COUNT : state->signal_step;
	uint8_t start = points[step];
	uint8_t end = points[step + 1u];
	uint8_t x = (uint8_t)(start +
		((uint16_t)(end - start) * state->phase_frames) /
		BUG_SIGNAL_STEP_FRAMES);
	put_sprite((int16_t)x - 4, state->signal_value ? 48 : 56,
		OVERLAY_SPARK);
}

static void draw_status(const bug_state_t *state, bool tutorial_done) {
	if (tutorial_done) {
		rf_gfx_fill(FONT_BLANK, 5, 8, 18, 5);
		put_text(7, 9, "LESSON LEARNED");
		put_text(10, 11, "A TITLE");
		return;
	}
	if (state->phase == BUG_PHASE_PAUSED) {
		rf_gfx_fill(FONT_BLANK, 3, 7, 22, 6);
		put_text(10, 8, "PAUSED");
		put_text(4, 10, "A RESUME B RETRY");
		put_text(8, 12, "START TITLE");
		return;
	}
	if (state->phase == BUG_PHASE_PUZZLE_CLEAR) {
		rf_gfx_put_image(12, 8, art_result_win, 4, 4);
		put_text(8, 12, "MEDAL");
		put_number(14, 12, state->medals[state->puzzle], 1);
		return;
	}
	if (state->phase == BUG_PHASE_SIGNAL) {
		rf_gfx_fill(FONT_BLANK, 9, 9, 10, 2);
		put_text(10, 10, "CASTING");
		return;
	}
	if (state->failed) {
		rf_gfx_put_image(12, 8, art_result_loss, 4, 4);
		put_text(state->hint_visible ? 8 : 7, 12,
			state->hint_visible ? "FOLLOW GLOW" : "TRY NEW ORDER");
	}
}

static void draw_footer(const bug_state_t *state, bool tutorial,
	bool tutorial_done) {
	rf_gfx_fill(FONT_BLANK, 0, 17, 28, 1);
	if (tutorial_done) put_text(9, 17, "A TITLE");
	else if (tutorial && state->cells[0] == 0)
		put_text(2, 17, state->selected == 1 ?
			"A PLACE FLIP FIRST" : "UP DOWN PICK FLIP");
	else if (tutorial) put_text(4, 17, "START SEND SPARK");
	else if (state->phase == BUG_PHASE_EDIT)
		put_text(1, 17, "A PLACE B CLEAR START CAST");
}

static void draw_result(const bug_state_t *state, uint8_t best_medals) {
	uint8_t puzzle;
	rf_gfx_fill(art_field[0], 0, 0, 28, 18);
	rf_gfx_put_image(12, 2, art_result_win, 4, 4);
	rf_gfx_fill(FONT_BLANK, 4, 6, 20, 10);
	put_text(6, 7, "SPELLBOOK SEALED");
	put_text(8, 9, "TOTAL MEDALS");
	put_number(21, 9, state->total_medals, 2);
	put_text(8, 10, "BEST");
	put_number(13, 10, best_medals, 2);
	for (puzzle = 0; puzzle < BUG_PUZZLE_COUNT; ++puzzle) {
		uint8_t medal;
		for (medal = 0; medal < 3; ++medal) {
			rf_gfx_put_tile((uint8_t)(4 + puzzle * 4u + medal), 11,
				medal < state->medals[puzzle] ?
				art_pip_full[0] : art_pip_empty[0]);
		}
	}
	put_text(7, 14, "A REPLAY");
	put_text(7, 15, "START TITLE");
}

void gfx_render(const bug_state_t *state, bool tutorial,
	bool tutorial_done, uint32_t tick, uint8_t best_medals) {
	actor_sprite_count = 0;
	draw_second_layer(state, tick);
	if (state->complete && !tutorial) {
		draw_result(state, best_medals);
		finish_sprites();
		return;
	}
	rf_gfx_fill(art_field[0], 0, 0, 28, 18);
	draw_hud(state, tutorial);
	draw_board(state);
	draw_familiar_sprites(state, tick);
	if (state->phase == BUG_PHASE_SIGNAL) draw_signal_sprite(state);
	draw_status(state, tutorial_done);
	draw_footer(state, tutorial, tutorial_done);
	finish_sprites();
}

#undef FONT_GLYPH
#undef FONT_ROW
