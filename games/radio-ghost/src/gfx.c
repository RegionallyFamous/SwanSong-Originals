#include <swan/gfx.h>
#include <swan/legacy.h>

#include "swan_assets.h"
#include "swan_game_runtime.h"
#include "gfx.h"
#include "gameplay_art.h"

#define TRANSPARENT_TILE 429u
#define FONT_BASE 430u
#define FONT_BLANK FONT_BASE
#define UI_PALETTE 1u
#define ACTOR_SPRITE_CAPACITY 24u
#define FONT_ROW(value) 0u, (uint8_t)(value)
#define FONT_GLYPH(a, b, c, d, e, f, g) \
	FONT_ROW(a), FONT_ROW(b), FONT_ROW(c), FONT_ROW(d), \
	FONT_ROW(e), FONT_ROW(f), FONT_ROW(g), FONT_ROW(0)

static const uint8_t SWAN_FAR transparent_tile[16] = {0};
static const uint16_t SWAN_FAR ui_palette[4] = {
	0x0111, 0x0EBB, 0x0EED, 0x0F69,
};

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
static uint16_t record_best_score;
static uint16_t record_best_time;
static uint8_t record_signals;
static bool record_tutorial;

static uint16_t font_tile(char character) {
	if (character >= 'A' && character <= 'Z')
		return (uint16_t)(FONT_BASE + 1u + (uint8_t)(character - 'A'));
	if (character >= '0' && character <= '9')
		return (uint16_t)(FONT_BASE + 27u + (uint8_t)(character - '0'));
	return FONT_BLANK;
}

static void load_ui_tiles(void) {
	uint16_t colors[4];
	uint8_t index;
	(void)swan_gfx_load_tiles(TRANSPARENT_TILE, transparent_tile, 1);
	(void)swan_gfx_load_tiles(FONT_BASE, font_tiles,
		(uint16_t)(sizeof(font_tiles) / 16u));
	for (index = 0u; index < 4u; ++index) colors[index] = ui_palette[index];
	(void)swan_gfx_set_palette(UI_PALETTE, colors);
}

static void clear_overlay(void) {
	(void)swan_gfx_fill(1, 0, 0, 32, 32,
		SWAN_TILE_ATTR(TRANSPARENT_TILE, UI_PALETTE));
}

static void base_fill(uint8_t x, uint8_t y, uint8_t width, uint8_t height,
	uint16_t tile) {
	(void)swan_gfx_fill(0, x, y, width, height,
		SWAN_TILE_ATTR(tile, UI_PALETTE));
}

static void base_tile(uint8_t x, uint8_t y, uint16_t attr) {
	(void)swan_gfx_put_tile(0, x, y, attr);
}

static void base_image(uint8_t x, uint8_t y,
	const uint16_t SWAN_FAR *tiles, uint8_t width, uint8_t height) {
	uint8_t tile_x;
	uint8_t tile_y;
	for (tile_y = 0; tile_y < height; ++tile_y)
		for (tile_x = 0; tile_x < width; ++tile_x)
			base_tile((uint8_t)(x + tile_x), (uint8_t)(y + tile_y),
				tiles[(uint16_t)tile_y * width + tile_x]);
}

static void put_text(uint8_t x, uint8_t y, const char SWAN_FAR *text) {
	while (*text && x < 28u) {
		uint16_t tile = font_tile(*text++);
		base_tile(x++, y, SWAN_TILE_ATTR(tile, UI_PALETTE));
	}
}

static void put_number(uint8_t x, uint8_t y, uint16_t value, uint8_t digits) {
	uint16_t divisor = 1u;
	uint8_t index;
	for (index = 1u; index < digits; ++index)
		divisor = (uint16_t)(divisor * 10u);
	for (index = 0u; index < digits; ++index) {
		base_tile((uint8_t)(x + index), y,
			SWAN_TILE_ATTR(font_tile((char)('0' +
				(value / divisor) % 10u)), UI_PALETTE));
		divisor = divisor > 1u ? (uint16_t)(divisor / 10u) : 1u;
	}
}

static void load_sprite_palette(void) {
	uint16_t colors[4];
	uint8_t index;
	for (index = 0u; index < 4u; ++index) colors[index] = game_palette[index];
	(void)swan_gfx_set_palette(8, colors);
}

static void load_palette(const uint16_t SWAN_FAR *source, uint8_t slot) {
	uint16_t colors[4];
	uint8_t index;
	for (index = 0u; index < 4u; ++index) colors[index] = source[index];
	(void)swan_gfx_set_palette(slot, colors);
}

static uint8_t sprite_flags(uint16_t tile) {
	uint8_t flags = SWAN_SPRITE_FLAG_PRIORITY;
	if ((tile & SWAN_TILE_HFLIP) != 0u) flags |= SWAN_SPRITE_FLAG_HFLIP;
	if ((tile & SWAN_TILE_VFLIP) != 0u) flags |= SWAN_SPRITE_FLAG_VFLIP;
	return flags;
}

static void put_metasprite(int16_t x, int16_t y,
	const uint16_t SWAN_FAR *tiles, uint8_t width, uint8_t height) {
	uint8_t tile_x;
	uint8_t tile_y;
	for (tile_y = 0u; tile_y < height; ++tile_y) {
		for (tile_x = 0u; tile_x < width; ++tile_x) {
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

static const uint16_t SWAN_FAR *wave_for(uint8_t level) {
	switch (level & 7u) {
		case 0: return art_wave_0;
		case 1: return art_wave_1;
		case 2: return art_wave_2;
		case 3: return art_wave_3;
		case 4: return art_wave_4;
		case 5: return art_wave_5;
		case 6: return art_wave_6;
		default: return art_wave_7;
	}
}

void gfx_reset_title(void) {
	title_loaded = false;
}

void gfx_set_records(uint16_t best_score, uint16_t best_time,
	uint8_t signals_discovered, bool tutorial_complete) {
	record_best_score = best_score;
	record_best_time = best_time;
	record_signals = signals_discovered;
	record_tutorial = tutorial_complete;
}

void gfx_show_title(const radio_state_t *state, bool show_prompt) {
	uint8_t pulse;
	if (!title_loaded) {
		swan_gfx_hide_sprites();
		swan_gfx_set_sprites_enabled(false);
		(void)swan_gfx_load_tiles(0, swan_asset_title_art_tiles,
			SWAN_ASSET_TITLE_ART_TILE_COUNT);
		load_palette(swan_asset_title_art_palette, 0);
		(void)swan_gfx_fill(0, 0, 0, 32, 32, SWAN_TILE_ATTR(0, 0));
		{
			uint8_t x;
			uint8_t y;
			for (y = 0u; y < SWAN_ASSET_TITLE_ART_HEIGHT_TILES; ++y)
				for (x = 0u; x < SWAN_ASSET_TITLE_ART_WIDTH_TILES; ++x)
					(void)swan_gfx_put_tile(0, x, y,
						swan_asset_title_art_map[(uint16_t)y *
							SWAN_ASSET_TITLE_ART_WIDTH_TILES + x]);
		}
		swan_gfx_set_scroll(0, 0, 0);
		swan_gfx_set_layer_enabled(0, true);
		load_ui_tiles();
		clear_overlay();
		swan_gfx_set_layer_enabled(1, false);
		base_fill(5, 1, 18, 3, FONT_BLANK);
		put_text(8, 2, "RADIO GHOST");
		title_loaded = true;
	}
	pulse = (uint8_t)((state->title_ticks / 8u) & 7u);
	base_fill(3, 12, 22, 6, FONT_BLANK);
	if (state->attract) {
		put_text(5, 12, "THE DIAL IS LISTENING");
		put_text((uint8_t)(5u + pulse), 13, "SIGNAL");
	} else if (show_prompt) {
		put_text(7, 12, "SELECT THEN A START");
	}
	if (state->title_choice == 0u)
		put_text(4, 15, "X TUTORIAL    NIGHT");
	else
		put_text(4, 15, "  TUTORIAL  X NIGHT");
	if (record_best_score != 0u) {
		put_text(4, 17, "BEST");
		put_number(9, 17, record_best_score, 4);
		put_text(14, 17, "S");
		put_number(15, 17, record_signals, 1);
		put_text(17, 17, "T");
		put_number(18, 17, (uint16_t)(record_best_time / 75u), 2);
	} else {
		put_text(4, 17, record_tutorial ? "SCHOOL CLEARED" : "SCHOOL FIRST");
	}
}

void gfx_init(void) {
	static const swan_gfx_clip_t scope_clip = {
		.x = 16u, .y = 48u, .width = 192u, .height = 56u,
	};
	rf_gfx_load(game_tiles, sizeof(game_tiles), game_palette, art_hud_bg[0]);
	load_ui_tiles();
	load_sprite_palette();
	clear_overlay();
	swan_gfx_set_scroll(0, 0, 0);
	swan_gfx_set_scroll(1, 0, 0);
	swan_gfx_set_layer_enabled(0, true);
	swan_gfx_set_layer_enabled(1, true);
	(void)swan_gfx_set_layer_clip(1, SWAN_GFX_CLIP_INSIDE, &scope_clip);
	swan_gfx_set_sprites_enabled(true);
	swan_gfx_hide_sprites();
	rf_gfx_fill(art_panel[0], 0, 0, 28, 18);
	rf_gfx_put_image(1, 14, art_receiver, 4, 3);
}

static void draw_meter(uint8_t x, uint8_t y, uint8_t value, uint8_t width) {
	uint8_t index;
	for (index = 0u; index < width; ++index)
		base_tile((uint8_t)(x + index), y,
			index < value ? art_pip_full[0] : art_pip_empty[0]);
}

static void draw_phase_name(uint8_t clue) {
	if (clue == 0u) put_text(8, 0, "WHISPER");
	else if (clue == 1u) put_text(8, 0, "CHORUS");
	else put_text(8, 0, "BEACON");
}

static void draw_hud(const radio_state_t *state) {
	uint16_t seconds = (uint16_t)(state->time / 75u);
	uint8_t signal = radio_signal_strength(state);
	uint8_t noise = radio_noise_level(state);
	uint8_t clue;
	base_fill(0, 0, 28, 5, FONT_BLANK);
	put_text(0, 0, "PHASE");
	put_number(6, 0, (uint16_t)(state->clue + 1u), 1);
	draw_phase_name(state->clue);
	put_text(0, 1, "FREQ");
	put_number(5, 1, state->frequency, 4);
	put_text(11, 1, "GAIN");
	put_number(16, 1, state->gain, 1);
	put_text(19, 1, state->gate ? "WIDE" : "NARROW");
	put_text(0, 2, "SIG");
	draw_meter(4, 2, signal > 8u ? 8u : signal, 8u);
	put_text(13, 2, "NOISE");
	draw_meter(19, 2, noise > 8u ? 8u : noise, 8u);
	put_text(0, 3, "SEEK");
	if (radio_target_direction(state) < 0) put_text(5, 3, "LEFT");
	else if (radio_target_direction(state) > 0) put_text(5, 3, "RIGHT");
	else put_text(5, 3, "CENTER");
	put_text(15, 3, "DAWN");
	put_number(20, 3, seconds, 2);
	put_text(0, 4, "LOCKS");
	for (clue = 0u; clue < RADIO_SIGNAL_COUNT; ++clue)
		base_image((uint8_t)(7u + clue * 3u), 4,
			clue < state->clue ? (clue == 0u ? art_clue_0 :
				clue == 1u ? art_clue_1 : art_clue_2) : art_loop, 2, 2);
	put_text(18, 4, "A LOCK");
}

static void draw_scope(const radio_state_t *state) {
	uint8_t x;
	uint8_t signal = radio_signal_strength(state);
	uint8_t noise = radio_noise_level(state);
	uint8_t motion = (uint8_t)(state->time / 5u + state->frequency / 3u);
	for (x = 0u; x < 24u; ++x) {
		const uint16_t SWAN_FAR *wave;
		uint8_t y;
		uint8_t level = (uint8_t)(motion + x * (uint8_t)(noise + 1u));
		if (signal > 9u && x > 8u && x < 16u)
			level = (uint8_t)(level + signal + x * 2u);
		wave = wave_for(level);
		for (y = 0u; y < 7u; ++y)
			(void)swan_gfx_put_tile(1, (uint8_t)(2u + x),
				(uint8_t)(6u + y), wave[y]);
	}
	x = (uint8_t)(3u + ((state->frequency - RADIO_FREQUENCY_MIN) * 22u) /
		(RADIO_FREQUENCY_MAX - RADIO_FREQUENCY_MIN));
	{
		uint8_t y;
		for (y = 0u; y < 7u; ++y)
			(void)swan_gfx_put_tile(1, x, (uint8_t)(6u + y),
				art_needle[y]);
	}
}

static void draw_ghost(const radio_state_t *state) {
	uint8_t signal = radio_signal_strength(state);
	int8_t direction = radio_target_direction(state);
	int16_t x;
	int16_t y;
	if (signal < 6u || state->clue >= RADIO_SIGNAL_COUNT) return;
	x = direction < 0 ? 72 : direction > 0 ? 152 : 112;
	y = (int16_t)(48 + (radio_noise_level(state) > 5u ?
		(int16_t)((state->time >> 1) & 3u) : 0));
	put_metasprite(x, y, art_ghost, 4, 6);
}

static void draw_feedback(const radio_state_t *state) {
	if (!state->feedback_flash) return;
	base_fill(9, 13, 10, 2, FONT_BLANK);
	if (state->lock_quality) put_text(11, 13, "LOCKED");
	else put_text(11, 13, "ADJUST");
}

static void draw_tutorial(const radio_state_t *state) {
	base_fill(2, 0, 24, 5, FONT_BLANK);
	put_text(9, 0, "TUNING SCHOOL");
	if (state->tutorial_step == RADIO_TUTORIAL_TUNE) {
		put_text(4, 2, "RIGHT TUNE TO 0934");
	} else if (state->tutorial_step == RADIO_TUTORIAL_GAIN) {
		put_text(4, 2, "UP SET GAIN TO 4");
	} else if (state->tutorial_step == RADIO_TUTORIAL_GATE) {
		put_text(4, 2, "B OPENS WIDE GATE");
	} else if (state->tutorial_step == RADIO_TUTORIAL_LOCK) {
		put_text(4, 2, "A LOCKS THE SIGNAL");
	} else {
		put_text(5, 2, "PRACTICE LOCKED");
		put_text(7, 3, "A BEGIN NIGHT");
	}
	put_text(4, 4, "SOUND AND ARROW AGREE");
}

static void draw_pause(void) {
	base_fill(4, 5, 20, 9, FONT_BLANK);
	put_text(10, 6, "PAUSED");
	put_text(7, 9, "START RESUME");
	put_text(7, 11, "A RETRY  B TITLE");
}

static void draw_result(const radio_state_t *state) {
	base_fill(0, 0, 28, 18, FONT_BLANK);
	if (state->result == RADIO_RESULT_SIGNAL)
		put_text(6, 1, "SIGNAL ASSEMBLED");
	else
		put_text(9, 1, "DAWN BROKE");
	put_metasprite(96, 24, art_ghost, 4, 6);
	put_text(6, 10, "LOCKS");
	put_number(14, 10, state->clue, 1);
	put_text(6, 11, "ERRORS");
	put_number(14, 11, state->wrong_locks, 2);
	put_text(6, 12, "SCORE");
	put_number(14, 12, state->score, 4);
	put_text(6, 13, "BEST");
	put_number(14, 13, record_best_score, 4);
	put_text(9, 15, "A AGAIN");
	put_text(9, 16, "B SCHOOL");
	put_text(8, 17, "START TITLE");
}

void gfx_render(const radio_state_t *state) {
	actor_sprite_count = 0u;
	clear_overlay();
	draw_scope(state);
	if (state->result != RADIO_RESULT_PLAYING) {
		swan_gfx_set_layer_enabled(1, false);
		draw_result(state);
	} else {
		swan_gfx_set_layer_enabled(1, !state->paused);
		draw_hud(state);
		if (!state->paused) {
			draw_ghost(state);
			draw_feedback(state);
		}
		if (state->mode == RADIO_MODE_TUTORIAL) draw_tutorial(state);
		if (state->paused) draw_pause();
	}
	finish_sprites();
}

#undef FONT_GLYPH
#undef FONT_ROW
