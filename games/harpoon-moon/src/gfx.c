#include <swan/gfx.h>

#include "swan_assets.h"
#include "gameplay_art.h"
#include "gfx.h"
#include "model.h"

#define FONT_BASE 384u
#define FONT_BLANK FONT_BASE
#define ACTOR_SPRITE_CAPACITY 36u

#define FONT_ROW(value) (uint8_t)(0xFFu ^ (value)), (uint8_t)(value)
#define FONT_GLYPH(a, b, c, d, e, f, g) \
	FONT_ROW(a), FONT_ROW(b), FONT_ROW(c), FONT_ROW(d), \
	FONT_ROW(e), FONT_ROW(f), FONT_ROW(g), FONT_ROW(0)

/* Compact code-native 5x7 lettering; the source title art is unlettered. */
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

static void set_palette(uint8_t slot, const uint16_t SWAN_FAR *source) {
	uint16_t palette[4];
	uint8_t index;
	for (index = 0; index < 4; ++index) palette[index] = source[index];
	(void)swan_gfx_set_palette(slot, palette);
}

static void load_font(void) {
	static const uint16_t ui_palette[4] = {0x112, 0x112, 0xFD7, 0xFFE};
	(void)swan_gfx_load_tiles(FONT_BASE, font_tiles,
		(uint16_t)(sizeof(font_tiles) / 16u));
	(void)swan_gfx_set_palette(1, ui_palette);
}

static void put_image(uint8_t layer, uint8_t x, uint8_t y,
	const uint16_t SWAN_FAR *tiles, uint8_t width, uint8_t height) {
	uint8_t tile_x;
	uint8_t tile_y;
	for (tile_y = 0; tile_y < height; ++tile_y) {
		for (tile_x = 0; tile_x < width; ++tile_x) {
			(void)swan_gfx_put_tile(layer, (uint8_t)(x + tile_x),
				(uint8_t)(y + tile_y), tiles[(uint16_t)tile_y * width + tile_x]);
		}
	}
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

static void put_pips(uint8_t layer, uint8_t x, uint8_t y,
	uint8_t filled, uint8_t total) {
	uint8_t index;
	for (index = 0; index < total; ++index) {
		(void)swan_gfx_put_tile(layer, (uint8_t)(x + index), y,
			index < filled ? art_pip_full[0] : art_pip_empty[0]);
	}
}

static void draw_title_base(void) {
	uint8_t x;
	uint8_t y;
	for (y = 0; y < 18; ++y) {
		for (x = 0; x < 28; ++x) {
			(void)swan_gfx_put_tile(0, x, y,
				swan_asset_title_art_map[(uint16_t)y * 28u + x]);
		}
	}
}

static void draw_title_record(uint16_t best_score, uint8_t best_rank) {
	char rank_text[2] = {'B', 0};
	if (best_rank == 3) rank_text[0] = 'S';
	else if (best_rank == 2) rank_text[0] = 'A';
	fill_plate(0, 16, 1, 11, 4);
	put_text(0, 18, 2, "BEST");
	if (best_score) {
		put_number(0, 18, 3, best_score, 4);
		put_text(0, 24, 3, rank_text);
	} else put_text(0, 18, 3, "NO HUNT");
}

void harpoon_gfx_reset_title(void) {
	title_loaded = false;
}

void harpoon_gfx_show_title(bool show_prompt, bool attract, uint8_t page,
	bool training, uint16_t best_score, uint8_t best_rank) {
	if (!title_loaded) {
		swan_gfx_hide_sprites();
		swan_gfx_set_sprites_enabled(false);
		(void)swan_gfx_load_tiles(0, swan_asset_title_art_tiles,
			SWAN_ASSET_TITLE_ART_TILE_COUNT);
		set_palette(0, swan_asset_title_art_palette);
		load_font();
		swan_gfx_set_scroll(0, 0, 0);
		swan_gfx_set_layer_enabled(0, true);
		swan_gfx_set_layer_enabled(1, false);
		title_loaded = true;
	}
	draw_title_base();
	fill_plate(0, 1, 1, 13, 5);
	put_text(0, 2, 2, "HARPOON");
	put_text(0, 2, 3, "MOON");
	put_text(0, 2, 5, "LURE THE DEEP");
	if (attract) {
		fill_plate(0, 3, 10, 22, 7);
		put_text(0, 5, 11, "SIGNAL  HOLD B LURE");
		put_text(0, 5, 13, "GOLD  RELEASE A");
		put_text(0, 5, 15, "MISS  HOLD B REEL");
		put_text(0, 7, 16, "ANY INPUT RETURN");
	} else if (page == 1) {
		fill_plate(0, 3, 11, 22, 6);
		put_text(0, 5, 12, "MOVE  BOTH D PADS");
		put_text(0, 5, 13, "LURE  HOLD B");
		put_text(0, 5, 14, "FIRE  HOLD RELEASE A");
		put_text(0, 5, 15, "START  PAUSE");
	} else if (page == 2) {
		draw_title_record(best_score, best_rank);
		fill_plate(0, 3, 13, 22, 4);
		put_text(0, 6, 14, "RECORD JOURNAL");
		put_text(0, 6, 15, "LEFT RIGHT PAGES");
	} else {
		fill_plate(0, 3, 13, 22, 4);
		if (show_prompt) put_text(0, 8, 13, "PRESS START");
		put_text(0, 6, 15, training ? "B TRAINING ON" : "B TRAINING OFF");
		put_text(0, 5, 16, "LEFT RIGHT BRIEF");
	}
}

void harpoon_gfx_init(void) {
	static const swan_gfx_clip_t hud_clip = {0, 0, 224, 16};
	static const swan_gfx_clip_t play_clip = {0, 16, 224, 128};
	(void)swan_gfx_load_tiles(0, game_tiles, GAME_TILE_COUNT);
	set_palette(0, game_palette);
	set_palette(8, game_palette);
	load_font();
	(void)swan_gfx_fill(0, 0, 0, 32, 32, art_sky[0]);
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

static void draw_world(const harpoon_state_t *state) {
	uint8_t index;
	uint8_t scroll = (uint8_t)((state->elapsed_frames / 12u) % 28u);
	uint8_t skiff_x = (uint8_t)(1u + state->skiff);
	uint8_t creature_x = (uint8_t)(1u + state->creature);
	uint8_t left = skiff_x < creature_x ? skiff_x : creature_x;
	uint8_t right = skiff_x > creature_x ? skiff_x : creature_x;
	(void)swan_gfx_fill(0, 0, 0, 28, 13, art_sky[0]);
	(void)swan_gfx_fill(0, 0, 13, 28, 5, art_ground[0]);
	for (index = 0; index < 7; ++index) {
		uint8_t x = (uint8_t)((index * 5u + 28u - scroll) % 28u);
		(void)swan_gfx_put_tile(0, x, (uint8_t)(2u + index % 5u),
			(index & 1u) ? art_pip_full[0] : art_pip_empty[0]);
	}
	for (index = 0; index < 6; ++index) {
		uint8_t x = (uint8_t)((index * 6u + scroll / 2u) % 28u);
		(void)swan_gfx_put_tile(0, x, (uint8_t)(14u + index % 3u),
			art_pip_empty[0]);
	}
	if (state->lure_window || state->hit_flash) {
		for (index = (uint8_t)(left + 2u); index < right; ++index)
			(void)swan_gfx_put_tile(0, index, 11, art_beam[0]);
	}
}

static void draw_hud(const harpoon_state_t *state) {
	uint8_t oxygen_pips = (uint8_t)((state->oxygen + 299u) / 300u);
	uint8_t phase = state->phase >= HARPOON_PHASE_BOSS_ONE &&
		state->phase <= HARPOON_PHASE_BOSS_THREE ?
		(uint8_t)(state->phase - HARPOON_PHASE_BOSS_ONE + 1u) : 0;
	if (oxygen_pips > 8) oxygen_pips = 8;
	fill_plate(1, 0, 0, 28, 2);
	put_text(1, 0, 0, "O2");
	put_pips(1, 3, 0, oxygen_pips, 8);
	put_text(1, 12, 0, "TAG");
	put_pips(1, 15, 0, state->tags, 3);
	if (phase) {
		put_text(1, 19, 0, "HP");
		put_pips(1, 21, 0, state->boss_hp, 3);
		put_text(1, 25, 0, "P");
		put_number(1, 26, 0, phase, 1);
	}
	if (state->oxygen <= 600 && ((state->elapsed_frames / 8u) & 1u))
		put_text(1, 0, 1, "OXYGEN LOW");
	else if (state->tutorial == HARPOON_TUTORIAL_MOVE)
		put_text(1, 4, 1, "MOVE UNDER SIGNAL");
	else if (state->tutorial == HARPOON_TUTORIAL_LURE)
		put_text(1, 5, 1, "HOLD B TO LURE");
	else if (state->tutorial == HARPOON_TUTORIAL_CHARGE)
		put_text(1, 3, 1, "HOLD A GOLD RELEASE");
	else if (state->phase == HARPOON_PHASE_BOSS_ONE)
		put_text(1, 5, 1, "TRACK THE WAKE");
	else if (state->phase == HARPOON_PHASE_BOSS_TWO)
		put_text(1, 4, 1, "SWEEP THEN STRIKE");
	else if (state->phase == HARPOON_PHASE_BOSS_THREE)
		put_text(1, 4, 1, "ABYSS SHORT WINDOW");
	else put_text(1, 5, 1, "LURE TIME RELEASE");
}

static void draw_meter(const harpoon_state_t *state) {
	uint8_t filled;
	fill_plate(0, 2, 16, 24, 2);
	if (state->recovery_frames) {
		put_text(0, 3, 16, "REEL");
		filled = (uint8_t)((60u - state->recovery_frames) / 6u);
		put_pips(0, 9, 16, filled, 10);
		put_text(0, 20, 16, "HOLD B");
	} else if (state->charge) {
		put_text(0, 3, 16, "CHARGE");
		filled = (uint8_t)((state->charge + 2u) / 3u);
		put_pips(0, 10, 16, filled, 10);
		if (state->charge >= harpoon_min_charge(state) &&
			state->charge <= harpoon_max_charge(state))
			put_text(0, 21, 16, "GOLD");
		else if (state->charge > harpoon_max_charge(state))
			put_text(0, 21, 16, "LATE");
	} else {
		put_text(0, 3, 16, "LURE");
		filled = state->lure_window ? 10u :
			(uint8_t)(((uint16_t)state->lure_meter * 10u) /
			harpoon_lure_target(state));
		put_pips(0, 9, 16, filled, 10);
		put_text(0, 20, 16, state->lure_window ? "OPEN" : "HOLD B");
	}
}

static void draw_feedback(const harpoon_state_t *state) {
	const char SWAN_FAR *message = 0;
	if (!state->feedback_frames) return;
	switch (state->feedback) {
		case HARPOON_SFX_LURE: message = "SIGNAL EXPOSED"; break;
		case HARPOON_SFX_READY: message = "GOLD WINDOW"; break;
		case HARPOON_SFX_TAG: message = "CREATURE TAGGED"; break;
		case HARPOON_SFX_BOSS: message = "LEVIATHAN PHASE"; break;
		case HARPOON_SFX_HIT: message = "DEEP HIT"; break;
		case HARPOON_SFX_MISS: message = "MISS HOLD B REEL"; break;
		case HARPOON_SFX_RECOVER: message = "LINE RECOVERED"; break;
		default: break;
	}
	if (message) {
		fill_plate(0, 5, 3, 19, 2);
		put_text(0, 6, 3, message);
	}
}

static void draw_actors(const harpoon_state_t *state) {
	int16_t skiff_x = (int16_t)(8u + state->skiff * 8u);
	int16_t creature_x = (int16_t)(8u + state->creature * 8u);
	int16_t creature_y = (int16_t)((state->tags < HARPOON_SCOUT_COUNT ?
		5u : 4u) * 8u + state->creature_depth * 8u);
	actor_sprite_count = 0;
	put_metasprite(skiff_x, 88, art_skiff, 4, 3);
	if (state->tags < HARPOON_SCOUT_COUNT)
		put_metasprite(creature_x, creature_y, art_creature, 4, 3);
	else put_metasprite(creature_x, creature_y, art_boss, 6, 4);
	finish_sprites();
}

static void draw_pause(void) {
	fill_plate(0, 6, 5, 17, 8);
	put_text(0, 10, 6, "PAUSED");
	put_text(0, 8, 8, "START RESUME");
	put_text(0, 9, 10, "B RESTART");
	put_text(0, 8, 12, "OXYGEN FROZEN");
}

static void draw_result(const harpoon_state_t *state,
	uint16_t best_score, uint8_t best_rank) {
	uint16_t seconds = (uint16_t)(state->elapsed_frames / 75u);
	char rank_text[2] = {'B', 0};
	(void)best_score;
	(void)best_rank;
	if (state->rank == 3) rank_text[0] = 'S';
	else if (state->rank == 2) rank_text[0] = 'A';
	else if (state->rank == 0) rank_text[0] = '-';
	swan_gfx_hide_sprites();
	fill_plate(0, 3, 2, 22, 15);
	put_image(0, 4, 3,
		state->result == HARPOON_RESULT_WIN ? art_result_win : art_result_loss,
		4, 4);
	put_text(0, 9, 3, state->result == HARPOON_RESULT_WIN ?
		"HUNT COMPLETE" : "OXYGEN EMPTY");
	put_text(0, 10, 5, "RANK");
	put_text(0, 16, 5, rank_text);
	put_text(0, 6, 8, "TIME");
	put_number(0, 13, 8, (uint16_t)(seconds / 60u), 2);
	put_number(0, 16, 8, (uint16_t)(seconds % 60u), 2);
	put_text(0, 6, 9, "SCORE");
	put_number(0, 13, 9, state->score, 4);
	put_text(0, 6, 10, "OXYGEN");
	put_number(0, 14, 10, state->oxygen, 4);
	put_text(0, 6, 11, "MISSES");
	put_number(0, 15, 11, state->misses, 2);
	put_text(0, 5, 13, state->misses ? "TIP REEL EVERY MISS" : "PERFECT LINE CONTROL");
	put_text(0, 5, 15, "A AGAIN START TITLE");
}

void harpoon_gfx_render(const harpoon_state_t *state,
	uint16_t best_score, uint8_t best_rank) {
	draw_world(state);
	draw_hud(state);
	if (state->phase == HARPOON_PHASE_RESULT) {
		draw_result(state, best_score, best_rank);
		return;
	}
	draw_actors(state);
	draw_meter(state);
	if (state->phase == HARPOON_PHASE_PAUSED) draw_pause();
	else draw_feedback(state);
}

#undef FONT_GLYPH
#undef FONT_ROW
