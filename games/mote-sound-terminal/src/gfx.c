#include <swan/gfx.h>

#include "swan_assets.h"
#include "gfx.h"

#define GAMEPLAY_MAP_WIDTH SWAN_ASSET_GAMEPLAY_WIDTH_TILES

#define ASSET_TRACK_0_X 0
#define ASSET_TRACK_1_X 2
#define ASSET_TRACK_2_X 4
#define ASSET_PLAY_X 6
#define ASSET_PAUSE_X 8
#define ASSET_SCOPE_A_X 10
#define ASSET_SCOPE_B_X 12
#define ASSET_BAR_X 0
#define ASSET_BEAT_OFF_X 8
#define ASSET_BEAT_ON_X 9
#define ASSET_BEAT_NOW_X 10
#define ASSET_PIP_FULL_X 11
#define ASSET_PIP_EMPTY_X 12
#define ASSET_HUD_BG_X 13

#define ASSET_ROW_ICONS 0
#define ASSET_ROW_METERS 2

static bool gameplay_loaded;
static bool render_initialized;
static uint8_t rendered_track;
static uint8_t rendered_tempo;
static uint8_t rendered_scope;
static uint8_t rendered_step;
static bool rendered_playing;

static void set_palette(const uint16_t SWAN_FAR *source) {
	uint16_t palette[4];
	uint8_t index;
	for (index = 0; index < 4; ++index) palette[index] = source[index];
	(void)swan_gfx_set_palette(0, palette);
}

static void put_region(uint8_t screen_x, uint8_t screen_y,
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

void gfx_show_intro(void) {
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
}

void gfx_init(void) {
	if (!gameplay_loaded) {
		(void)swan_gfx_load_tiles(0, swan_asset_gameplay_tiles,
			SWAN_ASSET_GAMEPLAY_TILE_COUNT);
		set_palette(swan_asset_gameplay_palette);
		gameplay_loaded = true;
	}
	(void)swan_gfx_fill(0, 0, 0, 32, 32,
		swan_asset_gameplay_map[ASSET_ROW_METERS * GAMEPLAY_MAP_WIDTH +
			ASSET_HUD_BG_X]);
	swan_gfx_set_scroll(0, 0, 0);
	swan_gfx_set_layer_enabled(0, true);
	swan_gfx_set_layer_enabled(1, false);
	render_initialized = false;
}

static void put_track_state(uint8_t track) {
	uint8_t x;
	for (x = 0; x < 3; ++x) {
		put_region((uint8_t)(1 + x * 3), 2,
			x == track ? ASSET_PIP_FULL_X : ASSET_PIP_EMPTY_X,
			ASSET_ROW_METERS, 1, 1);
	}
}

static void put_bar_column(uint8_t x, uint8_t scope, uint8_t step,
	const swan_audio_voice_t *voices) {
	uint8_t lane;
	for (lane = 0; lane < 3; ++lane) {
		const swan_audio_voice_t *voice = &voices[(uint8_t)(lane + scope)];
		uint8_t level = voice->owner == SWAN_VOICE_SILENT ? 0 :
			(uint8_t)(voice->volume + voice->note + step + x * (lane + 1));
		put_region((uint8_t)(2 + x), (uint8_t)(3 + lane * 3),
			(uint8_t)(ASSET_BAR_X + (level & 7)), ASSET_ROW_METERS, 1, 3);
	}
}

static void put_beat(uint8_t x, uint8_t track, uint8_t step) {
	uint8_t asset_x = x == step ? ASSET_BEAT_NOW_X :
		(((x + track * 3) & 3) == 0 ? ASSET_BEAT_ON_X : ASSET_BEAT_OFF_X);
	put_region((uint8_t)(6 + x), 13, asset_x, ASSET_ROW_METERS, 1, 1);
}

static void put_all_beats(uint8_t track, uint8_t step) {
	uint8_t x;
	for (x = 0; x < 16; ++x) {
		put_beat(x, track, step);
	}
}

static void put_tempo(uint8_t tempo) {
	uint8_t x;
	for (x = 0; x < 16; ++x) {
		put_region((uint8_t)(6 + x), 16,
			x < (uint8_t)(tempo - 4) ? ASSET_PIP_FULL_X : ASSET_PIP_EMPTY_X,
			ASSET_ROW_METERS, 1, 1);
	}
}

void gfx_render(uint8_t track, bool playing, uint8_t tempo,
	uint8_t scope, uint8_t step, const swan_audio_voice_t *voices) {
	uint8_t x;

	if (!render_initialized) {
		for (x = 0; x < 3; ++x)
			put_region((uint8_t)(1 + x * 3), 0, (uint8_t)(x * 2),
				ASSET_ROW_ICONS, 2, 2);
		put_track_state(track);
		put_region(20, 0, playing ? ASSET_PAUSE_X : ASSET_PLAY_X,
			ASSET_ROW_ICONS, 2, 2);
		put_region(24, 0, scope ? ASSET_SCOPE_B_X : ASSET_SCOPE_A_X,
			ASSET_ROW_ICONS, 2, 2);
		for (x = 0; x < 24; ++x) put_bar_column(x, scope, step, voices);
		put_all_beats(track, step);
		put_tempo(tempo);
		render_initialized = true;
	} else {
		bool track_changed = track != rendered_track;
		if (track_changed) {
			put_track_state(track);
			put_all_beats(track, step);
		}
		if (playing != rendered_playing) {
			put_region(20, 0, playing ? ASSET_PAUSE_X : ASSET_PLAY_X,
				ASSET_ROW_ICONS, 2, 2);
			for (x = 0; x < 24; ++x)
				put_bar_column(x, scope, step, voices);
		}
		if (scope != rendered_scope) {
			put_region(24, 0, scope ? ASSET_SCOPE_B_X : ASSET_SCOPE_A_X,
				ASSET_ROW_ICONS, 2, 2);
			for (x = 0; x < 24; ++x)
				put_bar_column(x, scope, step, voices);
		}
		if (tempo != rendered_tempo) put_tempo(tempo);
		if (step != rendered_step) {
			/* A two-column scope sweep keeps animation below the input-frame budget. */
			for (x = step; x < 24; x = (uint8_t)(x + 16))
				put_bar_column(x, scope, step, voices);
			if (!track_changed) {
				put_beat(rendered_step, track, step);
				put_beat(step, track, step);
			}
		}
	}
	rendered_track = track;
	rendered_playing = playing;
	rendered_tempo = tempo;
	rendered_scope = scope;
	rendered_step = step;
}
