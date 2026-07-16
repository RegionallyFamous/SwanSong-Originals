#include <stdbool.h>
#include <stdint.h>

#include "rf_swan.h"
#include "gfx.h"

void main(void) {
	uint8_t skiff = 3;
	uint8_t creature = 16;
	uint8_t tags = 0;
	uint8_t boss_hp = 3;
	uint8_t charge = 0;
	uint8_t result = 0;
	uint16_t oxygen = 1200;
	bool dirty = true;
	uint8_t intro;

	rf_init(false);
	gfx_show_intro();
	for (intro = 0; intro < 36; ++intro) {
		rf_frame();
		if (rf_input()->pressed) break;
	}
	gfx_init();
	while (1) {
		const rf_input_t *input;
		int8_t dx;
		rf_frame();
		input = rf_input();

		if (result && (input->pressed & WS_KEY_A)) {
			skiff = 3; creature = 16; tags = 0; boss_hp = 3;
			charge = 0; result = 0; oxygen = 1200; dirty = true;
		}
		if (!result) {
			dx = rf_dx(input->pressed);
			if (dx) {
				skiff = rf_clamp_u8((int16_t)skiff + dx, 0, 20);
				dirty = true;
			}
			if ((input->held & WS_KEY_A) && charge < 20) {
				++charge;
				dirty = true;
			}
			if ((input->held & WS_KEY_B) && (rf_frame_count() & 3) == 0) {
				if (creature < skiff) ++creature;
				else if (creature > skiff) --creature;
				dirty = true;
			}
			if ((input->released & WS_KEY_A) && charge) {
				uint8_t distance = skiff > creature ? skiff - creature : creature - skiff;
				uint8_t reach = (uint8_t)(charge / 2 + 2);
				if (distance <= reach) {
					if (tags < 3) ++tags;
					else if (boss_hp) --boss_hp;
					creature = (uint8_t)(4 + rf_random() % 16);
					rf_beep(620, 10);
				} else {
					oxygen = oxygen > 75 ? oxygen - 75 : 0;
					rf_beep(120, 8);
				}
				charge = 0;
				dirty = true;
			}
			if ((rf_frame_count() % 45) == 0) {
				int8_t drift = (rf_random() & 1) ? 1 : -1;
				creature = rf_clamp_u8((int16_t)creature + drift, 1, 20);
				dirty = true;
			}
			if (oxygen) --oxygen;
			if (tags == 3 && boss_hp == 0) result = 1;
			else if (oxygen == 0) result = 2;
		}
		if (dirty || (rf_frame_count() & 7) == 0) {
			gfx_render(skiff, creature, oxygen, tags, boss_hp, charge, result);
			dirty = false;
		}
	}
}
