#include "rf_swan.h"
#include "gfx.h"
#include "gameplay_art.h"

static uint16_t visual_target(uint8_t clue) {
	if (clue == 0) return 934;
	if (clue == 1) return 995;
	return 1042;
}

static const uint16_t __far *wave_for(uint8_t level) {
	switch (level & 7) {
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

static const uint16_t __far *clue_for(uint8_t clue) {
	if (clue == 0) return art_clue_0;
	if (clue == 1) return art_clue_1;
	return art_clue_2;
}

void gfx_show_intro(void) {
	rf_gfx_show_intro(game_intro_tiles, sizeof(game_intro_tiles),
		game_intro_map, game_palette);
}

void gfx_init(void) {
	rf_gfx_load(game_tiles, sizeof(game_tiles), game_palette, art_hud_bg[0]);
}

void gfx_render(uint16_t frequency, uint8_t gain, uint16_t time,
	uint8_t clue, uint8_t result, bool gate) {
	uint8_t x;
	uint16_t target = visual_target(clue < 3 ? clue : 2);
	uint16_t distance = frequency > target ? frequency - target : target - frequency;

	rf_gfx_fill(art_panel[0], 0, 0, 28, 18);
	for (x = 0; x < 3; ++x) {
		if (x < clue) rf_gfx_put_image((uint8_t)(1 + x * 3), 0, clue_for(x), 2, 2);
		else rf_gfx_put_tile((uint8_t)(1 + x * 3), 0, art_pip_empty[0]);
	}
	rf_gfx_put_image(11, 0, gate ? art_gate_wide : art_gate_narrow, 2, 2);
	for (x = 0; x < 9; ++x) {
		rf_gfx_put_tile((uint8_t)(15 + x), 0,
			x < gain ? art_pip_full[0] : art_pip_empty[0]);
	}
	for (x = 0; x < 8; ++x) {
		rf_gfx_put_tile((uint8_t)(16 + x), 2,
			time > (uint16_t)(x * 562) ? art_pip_full[0] : art_pip_empty[0]);
	}
	for (x = 0; x < 24; ++x) {
		uint8_t level = (uint8_t)(x * 3 + frequency / 2 + rf_frame_count() / 4);
		rf_gfx_put_image((uint8_t)(2 + x), 5, wave_for(level), 1, 8);
	}
	x = (uint8_t)(3 + ((frequency - 880) * 22) / 200);
	rf_gfx_put_image(x, 5, art_needle, 1, 8);
	if (distance <= (gate ? 8 : 3)) rf_gfx_put_image(19, 5, art_ghost, 4, 6);
	rf_gfx_put_image(1, 14, art_receiver, 4, 3);
	if (result) {
		rf_gfx_put_image(12, 7, result == 1 ? art_result_win : art_result_loss, 4, 4);
		rf_gfx_put_image(13, 11, art_loop, 2, 2);
	}
}
