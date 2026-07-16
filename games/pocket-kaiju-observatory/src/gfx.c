#include "rf_swan.h"
#include "gfx.h"
#include "gameplay_art.h"

static const uint16_t __far *behavior_for(uint8_t behavior) {
	if (behavior == 0) return art_behavior_0;
	if (behavior == 1) return art_behavior_1;
	return art_behavior_2;
}

static const uint16_t __far *creature_for(uint8_t behavior) {
	if (behavior == 0) return art_creature_0;
	if (behavior == 1) return art_creature_1;
	return art_creature_2;
}

void gfx_show_intro(void) {
	rf_gfx_show_intro(game_intro_tiles, sizeof(game_intro_tiles),
		game_intro_map, game_palette);
}

void gfx_init(void) {
	rf_gfx_load(game_tiles, sizeof(game_tiles), game_palette, art_hud_bg[0]);
}

void gfx_render(uint8_t camera, uint8_t kaiju, uint8_t behavior,
	uint8_t disturbance, uint16_t sun, uint8_t evidence, bool zoom,
	uint8_t result) {
	uint8_t i;
	uint8_t camera_x = (uint8_t)(1 + camera);
	uint8_t creature_x = (uint8_t)(1 + kaiju);
	uint8_t low = zoom ? 6 : 3;
	uint8_t high = zoom ? 10 : 7;
	uint8_t safe_left = kaiju > high ? (uint8_t)(1 + kaiju - high) : 1;
	uint8_t safe_right = kaiju > low ? (uint8_t)(1 + kaiju - low) : 1;

	rf_gfx_fill(art_sky[0], 0, 0, 28, 14);
	rf_gfx_fill(art_ground[0], 0, 14, 28, 4);
	for (i = 0; i < 3; ++i) {
		if (evidence & (uint8_t)(1 << i)) {
			rf_gfx_put_image((uint8_t)(1 + i * 3), 0, behavior_for(i), 2, 2);
		} else {
			rf_gfx_put_tile((uint8_t)(1 + i * 3), 0, art_pip_empty[0]);
		}
	}
	rf_gfx_put_image(10, 0, behavior_for(behavior), 2, 2);
	rf_gfx_put_image(13, 0, zoom ? art_zoom_2 : art_zoom_1, 2, 2);
	for (i = 0; i < 10; ++i) {
		rf_gfx_put_tile((uint8_t)(17 + i), 0,
			disturbance > (uint8_t)(i * 10) ? art_pip_full[0] : art_pip_empty[0]);
	}
	for (i = 0; i < 10; ++i) {
		rf_gfx_put_tile((uint8_t)(17 + i), 2,
			sun > (uint16_t)(i * 180) ? art_pip_full[0] : art_pip_empty[0]);
	}
	rf_gfx_put_image(creature_x, 5, creature_for(behavior), 6, 8);
	for (i = safe_left; i <= safe_right && i < 28; ++i) {
		rf_gfx_put_tile(i, 13, art_pip_full[0]);
	}
	rf_gfx_put_image(camera_x, 13, art_camera, 3, 3);
	if (result) {
		rf_gfx_put_image(12, 7, result == 1 ? art_result_win : art_result_loss, 4, 4);
		rf_gfx_put_image(13, 11, art_loop, 2, 2);
	}
}
