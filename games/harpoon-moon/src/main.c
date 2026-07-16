#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "rf_swan.h"
#include "native_art.h"

static const char __far title[] = "HARPOON MOON";
static const char __far subtitle[] = "Tag the constellation herd";
static const char __far help[] = "Move hold A/fire B lure";
static const char __far fmt_status[] = "O2 %u   TAGS %u/3  BOSS %u\n";
static const char __far fmt_charge[] = "HARPOON ";

static void render(uint8_t skiff, uint8_t creature, uint16_t oxygen,
	uint8_t tags, uint8_t boss_hp, uint8_t charge, uint8_t result) {
	uint8_t x;
	rf_clear();
	rf_header(title, subtitle);
	printf(fmt_status, oxygen / 75, tags, boss_hp);
	rf_playfield_begin();
	printf(".  *   .  *   .  *\n");
	printf("_____________________\n");
	for (x = 0; x < 21; ++x) {
		char c = '~';
		if (x == creature) c = tags < 3 ? '*' : 'M';
		if (x == skiff) c = 'S';
		putchar(c);
	}
	putchar('\n');
	printf("_____________________\n\n");
	rf_playfield_end();
	printf(fmt_charge);
	rf_print_bar(charge, 20, 10);
	putchar('\n');
	if (result == 1) printf("FIELD TAG COMPLETE! A again\n");
	else if (result == 2) printf("OXYGEN EMPTY. A to retry\n");
	else if (tags < 3) printf("Tag three small creatures.\n");
	else printf("The moon leviathan rises.\n");
	rf_footer(help);
}

void main(void) {
	uint8_t skiff = 3;
	uint8_t creature = 16;
	uint8_t tags = 0;
	uint8_t boss_hp = 3;
	uint8_t charge = 0;
	uint8_t result = 0;
	uint16_t oxygen = 1200;
	bool dirty = true;

	rf_init(false);
	RF_LOAD_NATIVE_ART();
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
			render(skiff, creature, oxygen, tags, boss_hp, charge, result);
			dirty = false;
		}
	}
}
