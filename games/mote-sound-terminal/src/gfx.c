#include "rf_swan.h"
#include "gfx.h"
#include "gameplay_art.h"

static const uint16_t __far *bar_for(uint8_t value) {
	switch (value & 7) {
		case 0: return art_bar_0;
		case 1: return art_bar_1;
		case 2: return art_bar_2;
		case 3: return art_bar_3;
		case 4: return art_bar_4;
		case 5: return art_bar_5;
		case 6: return art_bar_6;
		default: return art_bar_7;
	}
}

static const uint16_t __far *track_for(uint8_t track) {
	if (track == 0) return art_track_0;
	if (track == 1) return art_track_1;
	return art_track_2;
}

void gfx_show_intro(void) {
	rf_gfx_show_intro(game_intro_tiles, sizeof(game_intro_tiles),
		game_intro_map, game_palette);
}

void gfx_init(void) {
	rf_gfx_load(game_tiles, sizeof(game_tiles), game_palette, art_hud_bg[0]);
}

void gfx_render(uint8_t track, bool playing, uint8_t tempo,
	uint8_t scope, uint8_t step) {
	uint8_t lane;
	uint8_t x;

	rf_gfx_fill(art_hud_bg[0], 0, 0, 28, 18);
	for (x = 0; x < 3; ++x) {
		rf_gfx_put_image((uint8_t)(1 + x * 3), 0, track_for(x), 2, 2);
		rf_gfx_put_tile((uint8_t)(1 + x * 3), 2,
			x == track ? art_pip_full[0] : art_pip_empty[0]);
	}
	rf_gfx_put_image(20, 0, playing ? art_pause : art_play, 2, 2);
	rf_gfx_put_image(24, 0, scope ? art_scope_b : art_scope_a, 2, 2);

	for (lane = 0; lane < 3; ++lane) {
		for (x = 0; x < 24; ++x) {
			uint8_t level = (uint8_t)(step + x * (lane + 1) +
				lane * 3 + scope * 2);
			rf_gfx_put_image((uint8_t)(2 + x), (uint8_t)(3 + lane * 3),
				bar_for(level), 1, 3);
		}
	}
	for (x = 0; x < 16; ++x) {
		uint16_t tile = x == step ? art_beat_now[0] :
			(((x + track * 3) & 3) == 0 ? art_beat_on[0] : art_beat_off[0]);
		rf_gfx_put_tile((uint8_t)(6 + x), 13, tile);
	}
	for (x = 0; x < 16; ++x) {
		rf_gfx_put_tile((uint8_t)(6 + x), 16,
			x < (uint8_t)(tempo - 4) ? art_pip_full[0] : art_pip_empty[0]);
	}
}
